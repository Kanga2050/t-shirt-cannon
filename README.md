# T-Shirt Cannon

Control code for the Cal High Robotics pneumatic T-shirt cannon (Raspberry Pi + Xbox Wireless Controller).

Original: [XboxBedrock/t-shirt-cannon](https://github.com/XboxBedrock/t-shirt-cannon) by Sushrut.

## What it does

| Input (Xbox controller) | Output |
|---|---|
| Left stick Y + right stick X | Arcade drive: left/right drive motors on hardware PWM (GPIO 18/19, 50 Hz), cubic response curve |
| A / X | Fire relays 1 and 2 (GPIO 15 / 14) for the barrel valves |
| Left shoulder | Horn relay (GPIO 23) |
| B / Y | Turntable rotation (GPIO 26, servo-style PWM) |
| Left / right stick press | Winch / barrel elevation (GPIO 13) |

`util.pico_to_psi` converts a Pi Pico ADC reading of a 0.5–4.5 V, 0–300 psi pressure transducer (100k/220k divider) into psi.

## Run

```bash
source ~/tshirtcannon/bin/activate
python3 ~/tshirtcannon/main.py
```

Dependencies: `evdev`, `RPi.GPIO`, `rpi-hardware-pwm`, `pigpio`, `pyserial`.
