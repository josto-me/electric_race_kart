# IO list

Pin and IO assignment of the Arduino Due (SAM3X8E) control unit, as used by the firmware
(`firmware/E_Kart/Kart.h`). The rear axle and front wheel speed signals are read with the
timer capture units (TIOA0 / TIOA1).

| Port / Pin | Function | Analog / Digital |
|---|---|---|
| A0 | throttle pedal (potentiometer) | analog in |
| D2 (PB25, TIOA0) | front wheel speed, timer capture | digital in |
| A7 (PA2, TIOA1) | rear axle speed, timer capture | digital in |
| D7 | ignition relay (LOW = ignition on) | digital out |
| D8 | PWM to motor controller | digital out (PWM) |
| D10 / D11 / D12 | TFT CS / DC / RST (dashboard) | digital out |
| SPI header (MOSI, MISO, SCK) | TFT data | SPI |
