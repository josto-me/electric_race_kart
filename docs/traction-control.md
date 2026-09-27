# Traction control flow chart

```mermaid
flowchart TD
    start([Program start]) --> ign{Ignition on?}
    ign -- No --> ign
    ign -- Yes --> read[Read requested motor power]
    read --> asr{ASR active?}
    asr -- No --> keep[Keep motor power]
    asr -- Yes --> slip{Slip of the driven axle?}
    slip -- No --> keep
    slip -- Yes --> reduce[Reduce motor power]
    keep --> ign
    reduce --> ign
```

In the firmware, "Keep motor power" means that the power follows the pedal with a limited
ramp; "Reduce motor power" ramps the PWM down by `ASR_RAMP_DOWN` per millisecond
(see `firmware/E_Kart/Traction_Control.cpp`).
