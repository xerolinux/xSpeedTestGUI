#!/usr/bin/env python3
"""Render the README media: one GIF per style, a random hero GIF, the widget popup and the settings page.

Usage: tools/media/make-media.py [build-dir]

Needs a built tree (default: build), qml6, ffmpeg and the KDE platform theme. Frames are captured
offscreen with the demo helper, so no network and no visible windows are involved.
"""
import concurrent.futures
import json
import os
import random
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BUILD = ROOT / (sys.argv[1] if len(sys.argv) > 1 else "build")
MEDIA = ROOT / "docs" / "media"
THUMB = (200, 170)
FULL_WIDTH = 420
BACKDROP = "0x12141c"
SAMPLE_HISTORY = [
    {"timestamp": "2026-09-26T21:14:00+00:00", "download": 294.0, "upload": 147.0, "ping": 15.7, "jitter": 1.8, "server": "Thessaloniki"},
    {"timestamp": "2026-09-26T18:02:00+00:00", "download": 301.4, "upload": 151.2, "ping": 16.1, "jitter": 2.1, "server": "Thessaloniki"},
    {"timestamp": "2026-09-26T09:47:00+00:00", "download": 288.9, "upload": 143.6, "ping": 18.4, "jitter": 2.6, "server": "Thessaloniki"},
    {"timestamp": "2026-09-25T22:31:00+00:00", "download": 296.2, "upload": 149.8, "ping": 15.2, "jitter": 1.5, "server": "Thessaloniki"},
    {"timestamp": "2026-09-25T13:05:00+00:00", "download": 279.5, "upload": 138.1, "ping": 21.0, "jitter": 3.4, "server": "Thessaloniki"},
]
ENV = dict(os.environ,
           QT_QPA_PLATFORM="offscreen",
           QT_QPA_PLATFORMTHEME="kde",
           XDG_CURRENT_DESKTOP="KDE",
           XSPEEDTEST_HELPER=str(ROOT / "tools/media/demo-helper"))


def styles():
    text = (ROOT / "src/qml/Styles.qml").read_text()
    return re.findall(r'\[\d+, "([a-z-]+)", "([^"]+)", "([a-z]+)", "([a-z]+)"', text)


def capture(style, mode, out):
    out.mkdir(parents=True, exist_ok=True)
    if mode == "history":
        (out / "history.json").write_text(json.dumps(SAMPLE_HISTORY))
    env = dict(ENV)
    if mode == "settings":
        env["XDG_DATA_HOME"] = str(out / "data")
        (out / "data").mkdir(exist_ok=True)
        icons = Path.home() / ".local/share/icons"
        if icons.exists():
            (out / "data" / "icons").symlink_to(icons)
    subprocess.run(["qml6", "--apptype", "widget", "-I", str(BUILD / "bin"), str(ROOT / "tools/media/capture.qml"),
                    style, mode, str(out)], env=env, check=True, capture_output=True, timeout=120)


def gif(frames, target, fps, filters, colors):
    target.parent.mkdir(parents=True, exist_ok=True)
    palette = frames / "palette.png"
    source = ["-framerate", "10", "-i", str(frames / "f%03d.png")]
    chain = f"fps={fps},{filters}"
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", *source, "-vf", f"{chain},palettegen=max_colors={colors}:stats_mode=diff",
                    str(palette)], check=True)
    subprocess.run(["ffmpeg", "-y", "-loglevel", "error", *source, "-i", str(palette), "-lavfi",
                    f"[0:v]{chain}[x];[x][1:v]paletteuse=dither=bayer:bayer_scale=5", "-loop", "0", str(target)], check=True)


def render(style):
    with tempfile.TemporaryDirectory() as tmp:
        frames = Path(tmp)
        capture(style, "window", frames)
        gif(frames, MEDIA / "full" / f"{style}.gif", 8, f"scale={FULL_WIDTH}:-2:flags=lanczos", 96)
        gif(frames, MEDIA / "thumb" / f"{style}.gif", 8,
            f"scale={THUMB[0]}:{THUMB[1]}:force_original_aspect_ratio=decrease:flags=lanczos,"
            f"pad={THUMB[0]}:{THUMB[1]}:(ow-iw)/2:(oh-ih)/2:color={BACKDROP}", 64)
    return style


def extras(hero_style):
    with tempfile.TemporaryDirectory() as tmp:
        frames = Path(tmp)
        capture(hero_style, "window", frames)
        gif(frames, MEDIA / "app.gif", 10, "scale=560:-2:flags=lanczos", 128)
    with tempfile.TemporaryDirectory() as tmp:
        frames = Path(tmp)
        capture("downpour", "compact", frames)
        gif(frames, MEDIA / "widget-popup.gif", 10, "scale=300:-2:flags=lanczos", 96)
    for mode in ("settings", "history"):
        with tempfile.TemporaryDirectory() as tmp:
            frames = Path(tmp)
            capture("downpour", mode, frames)
            shutil.copy(frames / "f000.png", MEDIA / f"{mode}.png")


def main():
    catalog = styles()
    hero = random.choice(catalog)
    print("hero style:", hero[1])
    with concurrent.futures.ThreadPoolExecutor(4) as pool:
        for done in pool.map(render, [row[0] for row in catalog]):
            print("rendered", done)
    extras(hero[0])


if __name__ == "__main__":
    main()
