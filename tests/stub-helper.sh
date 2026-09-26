#!/bin/sh
[ -n "$STUB_COUNT" ] && echo run >> "$STUB_COUNT"
[ -n "$STUB_ENV" ] && echo "$XSPEEDTEST_BACKEND" >> "$STUB_ENV"
case "$STUB_MODE" in
ok)
    echo '{"phase":"ping","progress":1.0}'
    echo '{"phase":"server","name":"Frankfurt","sponsor":"Hetzner","country":"Germany","ping":12.5}'
    echo '{"phase":"jitter","jitter":1.5}'
    echo '{"phase":"download","mbps":300.5,"progress":0.5}'
    echo '{"phase":"upload","mbps":90.0,"progress":0.5}'
    echo '{"phase":"done","ping":12.5,"jitter":1.5,"download":310.2,"upload":95.4,"server":"Frankfurt, Germany (Hetzner)","isp":"Example","timestamp":"2026-09-26T10:00:00+00:00"}'
    ;;
error)
    echo '{"phase":"error","message":"No network"}'
    exit 1
    ;;
silent)
    echo '{"phase":"download","mbps":10,"progress":0.1}'
    exit 0
    ;;
lingering)
    echo '{"phase":"done","ping":1,"jitter":1,"download":10,"upload":5,"server":"S","isp":"I","timestamp":"t"}'
    exec sleep 1
    ;;
hang)
    echo '{"phase":"download","mbps":10,"progress":0.1}'
    exec sleep 30
    ;;
esac
