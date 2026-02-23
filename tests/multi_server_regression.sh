#!/bin/sh
set -eu

ROOT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
cd "$ROOT_DIR"

./webserv ./tests/multi_server.conf >/tmp/webserv_multi.log 2>&1 &
PID=$!
trap 'kill "$PID" >/dev/null 2>&1 || true; wait "$PID" >/dev/null 2>&1 || true' EXIT INT TERM
sleep 1

check_case() {
    label="$1"
    port="$2"
    host="$3"
    expected="$4"

    body="$(curl -sS --max-time 3 -H "Host: $host" "http://127.0.0.1:${port}/whoami.txt" || true)"
    if [ "$body" = "$expected" ]; then
        echo "[PASS] $label"
    else
        echo "[FAIL] $label"
        echo "  expected: $expected"
        echo "  got:      $body"
        exit 1
    fi
}

check_case "same port host a" 8090 a.test "SITE_A_8080_A_TEST"
check_case "same port host b" 8090 b.test "SITE_B_8080_B_TEST"
check_case "same port unknown host fallback first" 8090 unknown.test "SITE_A_8080_A_TEST"
check_case "different port host c" 8091 c.test "SITE_C_8081_C_TEST"
check_case "different port wrong host fallback by port" 8091 a.test "SITE_C_8081_C_TEST"

echo "All multi-server regression tests passed."
