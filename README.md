# T-Shirt Cannon

Control code for the Cal High Robotics pneumatic T-shirt cannon.

The cannon is driven from a VEX V5 controller. The V5 brain receives the radio link and drives eight 3-wire ports. An Arduino Uno reads those ports and drives the two Jaguar motor controllers on the CIM drivetrain and a 16-channel 12 V relay board for the valves, horn, Jaguar power, turntable and winch.

```
V5 controller ──radio──> V5 brain ──3-wire A–H──> Arduino Uno ──PWM──> 2× Jaguar ──> 2× CIM
                             ▲                         └──────relays──> valves, horn, Jaguar power,
                pressure transducer (port H)                              turntable motor, winch
```

| Folder | Runs on | Language |
|---|---|---|
| `vex/` | V5 brain | C++ (VEXcode V5) |
| `arduino/cannon/` | Arduino Uno | Arduino C++ |
| `pi/` | Raspberry Pi (Jan 2025 – Feb 2026 control system) | Python |

## Controls

| V5 controller | Action |
|---|---|
| Left stick Y + right stick X | Arcade drive, cubic response with a 15% deadband |
| Hold L2 + A | Fire barrel 1 |
| Hold L2 + X | Fire barrel 2 |
| L1 | Horn |
| B / Y | Turntable clockwise / counter-clockwise |
| R1 / R2 | Barrel elevation up / down (winch) |

The controller screen shows tank pressure and whether the fire buttons are armed. The brain screen shows tank pressure.

## Wiring

### V5 brain 3-wire ports → Arduino

Each port uses the white signal wire and the black ground. The red 5 V wire is left unconnected so the brain and Arduino supplies stay separate.

| V5 port | Mode | Signal | Arduino pin |
|---|---|---|---|
| A | PWM out | Left drive, 1–2 ms | D2 |
| B | PWM out | Right drive, 1–2 ms | D3 |
| C | PWM out | Turntable, ±100% | D4 |
| D | PWM out | Winch, ±100% | D5 |
| E | Digital out | Fire 1 (armed) | D6 |
| F | Digital out | Fire 2 (armed) | D7 |
| G | Digital out | Horn | D8 |
| H | Analog in | Pressure transducer, 0.5–4.5 V = 0–300 psi | – |

### Arduino → drivetrain and relay board

| Arduino pin | Goes to |
|---|---|
| D9 | Left Jaguar PWM |
| D10 | Right Jaguar PWM |
| D11 | Relay IN1, barrel 1 solenoid valve |
| D12 | Relay IN2, barrel 2 solenoid valve |
| A0 | Relay IN3, 12 V horn |
| A1 | Relay IN4, 12 V feed to both Jaguars |
| A2 / A3 | Relays IN5 / IN6, turntable window motor (reversing pair) |
| A4 / A5 | Relays IN7 / IN8, winch motor (reversing pair) |

The relay inputs are active-low. Each reversing pair wires the motor leads to the two relays' COM terminals, with NC to ground and NO to +12 V. With neither relay on, or both on, both leads sit at the same potential and the motor brakes.

## Safety behaviour

- **Link watchdog.** The Arduino measures every drive pulse from the brain. If either drive channel goes 100 ms without a valid 0.8–2.2 ms pulse (radio loss, brain off, unplugged cable), it opens the Jaguar power relay, centres both Jaguars and releases every relay.
- **Re-arm.** After the link returns, firing stays locked until both fire lines read idle.
- **Arming.** A shot needs L2 held together with A or X.
- **One shot per press.** Each press opens the dump valve for 200 ms, and a barrel can fire at most once every 1.5 s.
- **Boot state.** Relay pins are driven high before they are set as outputs, so no valve opens at power-up.

## Build

- **V5:** create a VEXcode V5 C++ project and replace `src/main.cpp` and `include/vex.h` with the files in `vex/`.
- **Arduino:** open `arduino/cannon/cannon.ino` in the Arduino IDE, board *Arduino Uno*, and upload. It uses the bundled `Servo` library.

## Raspberry Pi version

`pi/` holds the earlier control code, which ran the cannon from Jan 2025 to Feb 2026 with an Xbox Wireless Controller over Bluetooth. It was written by Sushrut: [XboxBedrock/t-shirt-cannon](https://github.com/XboxBedrock/t-shirt-cannon). Dependencies: `evdev`, `RPi.GPIO`, `rpi-hardware-pwm`, `pigpio`, `pyserial`.
