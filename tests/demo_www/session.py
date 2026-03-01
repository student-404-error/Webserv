#!/usr/bin/env python3
import os
import time
import random
import string

SESS_DIR = os.path.join(os.path.dirname(__file__), "sessions")
os.makedirs(SESS_DIR, exist_ok=True)


def parse_cookie(raw):
    result = {}
    if not raw:
        return result
    for part in raw.split(';'):
        if '=' in part:
            k, v = part.split('=', 1)
            result[k.strip()] = v.strip()
    return result


def new_sid():
    alphabet = string.ascii_letters + string.digits
    return ''.join(random.choice(alphabet) for _ in range(24))


def sid_path(sid):
    safe = ''.join(ch for ch in sid if ch.isalnum())
    return os.path.join(SESS_DIR, safe + '.txt')


def load_count(sid):
    path = sid_path(sid)
    try:
        with open(path, 'r') as f:
            return int(f.read().strip())
    except Exception:
        return 0


def save_count(sid, count):
    path = sid_path(sid)
    with open(path, 'w') as f:
        f.write(str(count))


cookie_header = os.environ.get('HTTP_COOKIE', '')
cookies = parse_cookie(cookie_header)
sid = cookies.get('sid', '')
set_cookie = False

if not sid or not sid.isalnum():
    sid = new_sid()
    set_cookie = True

count = load_count(sid) + 1
save_count(sid, count)

print('Content-Type: text/plain')
if set_cookie:
    print('Set-Cookie: sid={}; Path=/; HttpOnly'.format(sid))
print('')
print('session demo')
print('sid={}'.format(sid))
print('counter={}'.format(count))
print('time={}'.format(int(time.time())))
