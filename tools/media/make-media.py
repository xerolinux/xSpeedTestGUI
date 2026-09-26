#!/usr/bin/env python3
"""Render the README media: one GIF per style, a random hero GIF, the widget popup and the settings page.

Usage: tools/media/make-media.py [build-dir]

Needs a built tree (default: build), qml6, ffmpeg and the KDE platform theme. Frames are captured
offscreen with the demo helper, so no network and no visible windows are involved.
"""
import concurrent.futures
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
    with tempfile.TemporaryDirectory() as tmp:
        frames = Path(tmp)
        capture("downpour", "settings", frames)
        shutil.copy(frames / "f000.png", MEDIA / "settings.png")


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
