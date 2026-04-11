# T-Shirt Cannon

Control code for the Cal High Robotics pneumatic T-shirt cannon.

The cannon is driven from a VEX V5 controller. The V5 brain receives the radio link and drives eight 3-wire ports. An Arduino Uno reads those ports and drives the two Jaguar motor controllers on the CIM drivetrain and a 16-channel 12 V relay board for the valves, horn, Jaguar power, turntable and winch. Two AS5600 encoders on an I2C multiplexer give the turntable and elevation angles.

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
| Hold L2 + A | Fire |
| Hold X | Fill the chamber from the regulator |
| L1 | Horn |
| B / Y | Turntable one way / the other |
| R1 / R2 | Barrel elevation up / down (winch) |

The controller screen shows chamber pressure and whether firing is armed. The brain screen shows chamber pressure, or says the sensor isn't read.

## Wiring

The wiring diagram is `docs/wiring.png`.

### V5 brain 3-wire ports to the Arduino

Each port uses the white signal wire and the black ground. The red 5 V wire is left unconnected so the brain and Arduino supplies stay separate.

| V5 port | Mode | Signal | Arduino pin |
|---|---|---|---|
| A | PWM out | Left drive, 1 to 2 ms | D2 |
| B | PWM out | Right drive, 1 to 2 ms | D3 |
| C | PWM out | Turntable, ±100% | D4 |
| D | PWM out | Winch, ±100% | D5 |
| E | Digital out | Fire (armed) | D6 |
| F | Digital out | Fill | D7 |
| G | Digital out | Horn | D8 |
| H | Analog in | Pressure transducer, 0.5 to 4.5 V = 0 to 300 psi | none |

### Arduino to the drivetrain, relay board and encoders

| Arduino pin | Goes to |
|---|---|
| D9 | Left Jaguar PWM |
| D10 | Right Jaguar PWM |
| D11 | Relay K1, U.S. Solid 1 in fire valve |
| D12 | Relay K2, U.S. Solid USS2-00051 1/4 in fill valve |
| D13 | Relay K3, 12 V horn |
| A0 | Relay K4, 12 V feed to both Jaguars |
| A1 / A2 | Relays K5 / K6, turntable window motor (reversing pair) |
| A3 / D1 | Relays K7 / K8, winch window motor (reversing pair) |
| A4 SDA, A5 SCL | Level shifter, then HW-617 (TCA9548A) channel 0 turntable AS5600, channel 1 elevation AS5600 |

The relay inputs are active low. Each reversing pair wires the motor leads to the two relays' COM terminals, with NC to ground and NO to +12 V. With neither relay on, or both on, the motor brakes. D1 is the serial TX pin, so the sketch doesn't use `Serial`.

## Safety behaviour

- **Link watchdog.** The Arduino measures every drive pulse from the brain. If either drive channel goes 100 ms without a valid 0.8 to 2.2 ms pulse, it opens the Jaguar power relay, centres both Jaguars and releases every relay.
- **Re-arm.** After the link returns, firing and filling stay locked until both lines read idle.
- **Arming.** A shot needs L2 held together with A.
- **One shot per press.** Each press opens the fire valve for 200 ms, at most once every 1.5 s. The fill valve stays shut while firing and for 300 ms after.
- **Soft limits.** The AS5600 angles are zeroed at power up. The turntable stops at ±150° and the elevation at 0° and 60°. If an encoder doesn't answer, that axis runs without limits.
- **Reversing dead time.** A motor pair is off for 50 ms before it changes direction.
- **Boot state.** Relay pins are driven high before they are set as outputs, so no valve opens at power up.

## Build

- **V5:** create a VEXcode V5 C++ project and replace `src/main.cpp` and `include/vex.h` with the files in `vex/`.
- **Arduino:** open `arduino/cannon/cannon.ino` in the Arduino IDE, board *Arduino Uno*, and upload. It uses the bundled `Servo` and `Wire` libraries.

## Raspberry Pi version

`pi/` holds the earlier control code, which ran the cannon from Jan 2025 to Feb 2026 with an Xbox Wireless Controller over Bluetooth. It was written by Sushrut: [XboxBedrock/t-shirt-cannon](https://github.com/XboxBedrock/t-shirt-cannon). Dependencies: `evdev`, `RPi.GPIO`, `rpi-hardware-pwm`, `pigpio`, `pyserial`.
