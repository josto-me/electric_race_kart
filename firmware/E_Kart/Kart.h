// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Kart.h
 *
 *  Author: Johannes Stockhammer
 *
 * Parameters and pin assignment of the E-Kart control unit.
 */


#ifndef KART_H_
#define KART_H_

#include <Arduino.h>

//Build variants
#define ASR_ACTIVE 1							//1 = traction control, 0 = throttle only
#define DISPLAY_ACTIVE 1						//1 = 3.5" TFT dashboard (HX8357D, SPI)

//Logic
#define ON 1
#define OFF 0

//Pins
#define PIN_IGNITION 7							//D7 relay ignition, LOW = ignition on
#define PIN_THROTTLE_IN A0						//A0 throttle pedal (potentiometer)
#define PIN_THROTTLE_OUT 8						//D8 PWM to the motor controller
#define BIT_FRONT_WHEEL 25						//PB25 = D2 = TIOA0, front wheel speed
#define BIT_REAR_AXLE 2							//PA2 = A7 = TIOA1, rear axle speed
#define PIN_TFT_CS 10							//D10 TFT chip select
#define PIN_TFT_DC 11							//D11 TFT data/command
#define PIN_TFT_RST 12							//D12 TFT reset

//Throttle
#define THROTTLE_ADC_MIN 173					//ADC value pedal at rest
#define THROTTLE_ADC_MAX 795					//ADC value pedal fully pressed
#define PWM_MAX 220								//Power limit (PWM 0..255)

//Timing
#define TICK_HZ 1000							//TC2 periodic interrupt 1000 Hz
#define TICK_MS (1000/TICK_HZ)					//1ms per tick
#define MS_TO_TICKS(ms) ((ms)/TICK_MS)
#define TIME_RPM_TIMEOUT MS_TO_TICKS(200)		//200ms without pulse = wheel stands still
#define TIME_DISPLAY MS_TO_TICKS(200)			//Dashboard refresh 5 times per second
#define CAPTURE_HZ (VARIANT_MCK/2)				//TIMER_CLOCK1 = MCK/2 = 42 MHz

//Traction control
#define CALIB_PERCENT 100						//Ratio sensor rings and wheel diameters front/rear
#define SLIP_PERCENT 120						//Rear axle 20 % faster than front wheel = slip
#define ASR_RAMP_DOWN 2							//PWM steps per ms while slip
#define ASR_RAMP_UP 1							//PWM steps per ms without slip

//Speed (front wheel, not driven -> no slip)
#define FRONT_TEETH 20							//Teeth on the sensor ring = pulses per revolution
#define FRONT_WHEEL_MM 800						//Circumference front tyre 10x4.50-5 in mm (check on the kart)

#endif /* KART_H_ */
