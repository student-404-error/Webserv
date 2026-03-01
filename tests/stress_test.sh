#!/bin/sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$ROOT_DIR"

REPORT="tests/stress_report.txt"
LOGDIR="tests/stress_logs"
mkdir -p "$LOGDIR"
: > "$REPORT"

log() {
  echo "$1" | tee -a "$REPORT"
}

parse_metric() {
  # usage: parse_metric <file> <label>
  awk -F: -v key="$2" '$1==key {gsub(/^[ \t]+/,"",$2); print $2; exit}' "$1"
}

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || { echo "missing command: $1"; exit 1; }
}

require_cmd curl
require_cmd python3

./webserv ./tests/stress.conf >"$LOGDIR/server.log" 2>&1 &
SERVER_PID=$!
trap 'kill "$SERVER_PID" >/dev/null 2>&1 || true; wait "$SERVER_PID" >/dev/null 2>&1 || true' EXIT INT TERM
sleep 1

if ! kill -0 "$SERVER_PID" >/dev/null 2>&1; then
  log "[FAIL] server did not stay up"
  exit 1
fi

# 1) Concurrent GET load (HTTP/1.1)
TOTAL_GET=2000
CONC=100
seq "$TOTAL_GET" | xargs -n1 -P "$CONC" sh -c \
  'curl -sS --http1.1 --max-time 5 -o /dev/null -w "%{http_code}\n" http://127.0.0.1:8100/index.html || echo 000' \
  >"$LOGDIR/curl_get_codes.txt"
GET_NON200=$(grep -cv '^200$' "$LOGDIR/curl_get_codes.txt" || true)
if [ "$GET_NON200" -eq 0 ]; then
  log "[PASS] concurrent GET (n=$TOTAL_GET,c=$CONC)"
else
  log "[FAIL] concurrent GET (non200=$GET_NON200/$TOTAL_GET)"
fi

# 2) Keep-alive load (single persistent connection)
python3 - <<'PY' >"$LOGDIR/keepalive_result.txt"
import http.client
import sys

host = "127.0.0.1"
port = 8100
requests = 600
errors = 0
try:
    conn = http.client.HTTPConnection(host, port, timeout=5)
    for _ in range(requests):
        conn.request("GET", "/index.html", headers={"Connection": "keep-alive", "Host": "127.0.0.1:8100"})
        resp = conn.getresponse()
        body = resp.read()
        if resp.status != 200:
            errors += 1
    conn.close()
except Exception:
    print("EXCEPTION")
    sys.exit(2)
print(f"errors={errors}")
PY
if grep -q '^errors=0$' "$LOGDIR/keepalive_result.txt"; then
  log "[PASS] keep-alive GET (single-conn n=600)"
else
  log "[FAIL] keep-alive GET ($(cat "$LOGDIR/keepalive_result.txt"))"
fi

# 3) Multi-port concurrent load
TOTAL_PORT=1200
CONC_PORT=60
(seq "$TOTAL_PORT" | xargs -n1 -P "$CONC_PORT" sh -c \
  'curl -sS --http1.1 --max-time 5 -o /dev/null -w "%{http_code}\n" http://127.0.0.1:8100/index.html || echo 000' \
  >"$LOGDIR/codes_8100.txt") &
P1=$!
(seq "$TOTAL_PORT" | xargs -n1 -P "$CONC_PORT" sh -c \
  'curl -sS --http1.1 --max-time 5 -o /dev/null -w "%{http_code}\n" http://127.0.0.1:8101/index.html || echo 000' \
  >"$LOGDIR/codes_8101.txt") &
P2=$!
wait "$P1"
wait "$P2"
P8100_NON200=$(grep -cv '^200$' "$LOGDIR/codes_8100.txt" || true)
P8101_NON200=$(grep -cv '^200$' "$LOGDIR/codes_8101.txt" || true)
if [ "$P8100_NON200" -eq 0 ] && [ "$P8101_NON200" -eq 0 ]; then
  log "[PASS] dual-port concurrent GET"
else
  log "[FAIL] dual-port concurrent GET (8100_non200=$P8100_NON200/$TOTAL_PORT, 8101_non200=$P8101_NON200/$TOTAL_PORT)"
fi

# 4) Body limit behavior (10MB)
python3 - <<'PY' >"$LOGDIR/payload_small.bin"
print('a'*1024, end='')
PY
python3 - <<'PY' >"$LOGDIR/payload_big.bin"
print('a'*(10485760+1), end='')
PY
SMALL_CODE=$(curl -sS -o /dev/null -w "%{http_code}" --max-time 10 -X POST --data-binary @"$LOGDIR/payload_small.bin" http://127.0.0.1:8100/ || true)
BIG_CODE=$(curl -sS -o /dev/null -w "%{http_code}" --max-time 20 -X POST --data-binary @"$LOGDIR/payload_big.bin" http://127.0.0.1:8100/ || true)
if [ "$SMALL_CODE" = "200" ] && [ "$BIG_CODE" = "413" ]; then
  log "[PASS] body-limit policy (small=200, big=413)"
else
  log "[FAIL] body-limit policy (small=$SMALL_CODE, big=$BIG_CODE)"
fi

# 5) Post-load health check
HEALTH_CODE=$(curl -sS -o /dev/null -w "%{http_code}" --max-time 3 http://127.0.0.1:8100/index.html || true)
if [ "$HEALTH_CODE" = "200" ] && kill -0 "$SERVER_PID" >/dev/null 2>&1; then
  log "[PASS] server alive after stress"
else
  log "[FAIL] server health after stress (code=$HEALTH_CODE)"
fi

FAIL_COUNT=$(grep -c "^\[FAIL\]" "$REPORT" || true)
log ""
if [ "$FAIL_COUNT" -eq 0 ]; then
  log "RESULT: PASS"
  exit 0
else
  log "RESULT: FAIL ($FAIL_COUNT failures)"
  exit 1
fi
