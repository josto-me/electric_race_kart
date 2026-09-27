# Electric Race Kart

[![DOI](https://img.shields.io/badge/DOI-10.5281%2Fzenodo.22990338-blue.svg)](https://doi.org/10.5281/zenodo.22990338) [![Build](https://github.com/josto-me/electric_race_kart/actions/workflows/build.yml/badge.svg)](https://github.com/josto-me/electric_race_kart/actions/workflows/build.yml) [![Code: Apache-2.0](https://img.shields.io/badge/code-Apache--2.0-blue.svg)](LICENSE) [![Docs: CC BY 4.0](https://img.shields.io/badge/docs-CC%20BY%204.0-lightgrey.svg)](LICENSE-CC-BY-4.0.txt) [![Cite](https://img.shields.io/badge/cite-CITATION.cff-green.svg)](CITATION.cff)

Electric kart with traction control (ASR), diploma project at HTL Braunau.

Elektro-Kart mit Antischlupfregelung, Diplomprojekt HTL Braunau.

## About

The Electric Race Kart is a three-person diploma project at HTL Braunau. The aim is an electric kart that stays competitive with combustion karts through an efficient drivetrain and sufficient battery run time. The kart uses a 10 kW BLDC motor, a VEC500 motor controller and a 76.8 V LiFePO4 battery built from 24 individual cells; splitting the pack into single cells helps with weight distribution and allows high charging currents for energy recovery. An Arduino Due reads the throttle pedal, switches the ignition relay and sets the power demand as PWM to the motor controller. A traction control (ASR) developed for this application measures the wheel speeds and ramps the power down when the driven axle turns faster than the front wheels, so that little energy is wasted as slip. Multi-stage energy recuperation and a cell-level battery management system are further parts of the project concept. Within the team, Johannes Stockhammer is responsible for the drive, the energy storage and the traction control; this repository contains these parts.

## Safety and disclaimer

This is a school project, not a certified product. The kart runs on a 76.8 V battery with a 10 kW motor; high currents, stored energy and moving parts can cause fire, injury and damage. The traction control is not a safety function. Anyone building on this work does so at their own risk and is responsible for the safety of the vehicle, for electrical safety and for local regulations. No warranty, see the licenses.

## Vehicle

| | |
|---|---|
| Motor | 10 kW BLDC, 72 V, air cooled (HPM-10KW) |
| Controller | VEC500 FOC motor controller, throttle via PWM |
| Battery | 24 LiFePO4 cells, 3.2 V / 60 Ah, 76.8 V nominal, approx. 4.6 kWh |
| Control unit | Arduino Due (SAM3X8E) |
| Wheel speed | sensors on a toothed sensor ring, front wheel and rear axle |

## Contents

```
firmware/E_Kart/               Arduino Due: throttle, ignition relay, traction control, TFT dashboard
hardware/pdf/                  schematics and board layouts as PDF
hardware/bom/                  bills of material
calculations/                  motor, battery, charging curve (Excel) and chain sizing
docs/                          traction control flow chart, IO list
```

## Firmware

The control unit reads the throttle pedal, switches the ignition relay and sends the power demand as PWM to the motor controller. The wheel speeds are measured with the timer capture units of the SAM3X8E (TC0 channel 0 and 1, direct register access), a third timer channel gives a 1 ms tick that counts down all software timers. The firmware has its own `main()` with a state machine (wait for released pedal, drive) and one module per function: `Throttle_Read`, `Slip_Detect`, `Traction_Control`, `Speed_Calc` and `Display`.

Traction control: if the rear axle turns more than 20 % faster than the front wheel, the PWM is ramped down, otherwise it follows the pedal with a limited ramp. Without front wheel signal the ASR does not intervene. The ignition only switches on when the pedal is released at power up.

Dashboard: a 3.5" TFT (480×320, HX8357D, SPI) shows the speed in km/h, the max speed, a throttle bar and an **ASR** lamp that lights while traction control holds the throttle back. The speed comes from the front wheel, which is not driven and so does not slip. The dashboard has **not been tested on the kart**. `FRONT_TEETH` is 20 (teeth on the sensor ring); the tyre circumference `FRONT_WHEEL_MM` (800 mm, 10×4.50-5 tyre) must be checked on the kart. Drawing is split into small jobs so the 1 ms control loop keeps running; `DISPLAY_ACTIVE 0` switches the dashboard off.

Build with the Arduino IDE (board package *Arduino SAM Boards*, board *Arduino Due (Programming Port)*), libraries *Adafruit GFX*, *Adafruit HX8357* and *Adafruit BusIO* from the Library Manager. All parameters are in `Kart.h`; `ASR_ACTIVE 0` gives the version without traction control. `CALIB_PERCENT` has to be set to the ratio of the sensor rings and wheel diameters front/rear.

## Dependencies

Required to build the firmware: Arduino SAM core with SPI (LGPL-2.1-or-later; SPI GPL-2.0 or LGPL-2.1), Adafruit GFX Library (BSD), Adafruit HX8357 Library (MIT), Adafruit BusIO (MIT). Install them with the Arduino IDE.

## License

- Code in `firmware/`: **Apache License 2.0**, see [`LICENSE`](LICENSE) and [`NOTICE`](NOTICE).
- `hardware/`, `calculations/`, `docs/` and the README: **CC BY 4.0**, see [`LICENSE-CC-BY-4.0.txt`](LICENSE-CC-BY-4.0.txt).

You may use, change and share everything, also commercially. When you pass it on or
publish something based on it, credit it as:

> Johannes Stockhammer, "Electric Race Kart", version 1.0.0, Zenodo, https://doi.org/10.5281/zenodo.22990338

GitHub shows the same citation under "Cite this repository" (from [`CITATION.cff`](CITATION.cff)).

## Trademarks

Arduino, Adafruit and Golden Motor are trademarks of their respective owners; HPM-10KW and VEC500 are product names of their manufacturer. They are used here only to identify the parts used, with no affiliation or endorsement.

## Author

Johannes Stockhammer

Concept, hardware, calculations and original firmware by Johannes Stockhammer. The current firmware (based on the original), the TFT dashboard code, the translation and the documentation were written with the help of AI tools and reviewed by the author.
