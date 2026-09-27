// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * E_Kart.ino
 *
 *  Author: Johannes Stockhammer
 *
 * Version:      3.0
 * Hardware:     Arduino Due (SAM3X8E, 84 MHz), ignition relay, throttle pedal (potentiometer),
 *               VEC500 motor controller (PWM input), hall sensors on sensor rings
 *               front wheel and rear axle, 3.5" TFT (HX8357D, SPI)
 * Software:     Arduino IDE, board package Arduino SAM Boards, own main()
 * Description:  Throttle pedal, ignition and traction control (ASR) of the E-Kart.
 *               The wheel periods are measured with the timer capture units TC0 and TC1,
 *               TC2 gives a 1ms tick. The ignition only switches on when the pedal is
 *               released at power up. If the rear axle turns faster than the front wheel
 *               by SLIP_PERCENT, the power is ramped down.
 */

//Includes
#include "Kart.h"
#include "Throttle_Read.h"
#include "Slip_Detect.h"
#include "Traction_Control.h"
#include "Speed_Calc.h"
#if DISPLAY_ACTIVE
#include "Display.h"
#endif

//Defines
	//Sequence
#define WAIT_RELEASE 0							//Ignition off, wait for released pedal
#define DRIVE 1									//Ignition on, throttle and ASR active

//Variables
	//Written in the ISRs
volatile uint32_t period_front=0;				//Front wheel period in capture ticks (42 MHz)
volatile uint32_t period_rear=0;				//Rear axle period in capture ticks (42 MHz)
volatile uint16_t timeout_front=0;				//>0 = front wheel pulse within TIME_RPM_TIMEOUT
volatile uint16_t timeout_rear=0;				//>0 = rear axle pulse within TIME_RPM_TIMEOUT
volatile uint16_t display_timer=0;				//Dashboard refresh
volatile uint8_t ticks_pending=0;				//1ms ticks not yet processed by the main loop
	//Main loop
uint8_t sequence=WAIT_RELEASE;
uint16_t throttle_target=0;						//Throttle from the pedal (PWM value)
uint16_t throttle_asr=0;						//Throttle after traction control (PWM value)
bool message_shown=0;

int main(void)
{
	uint8_t ticks;
	uint32_t front, rear;

	watchdogSetup();							//Watchdog off (on after reset)
	init();										//Arduino init: clock, SysTick, ADC, PWM

	//Inputs/Outputs
		//Outputs
	pinMode(PIN_IGNITION,OUTPUT);				//D7 as output ignition relay
	digitalWrite(PIN_IGNITION,HIGH);			//HIGH -> ignition off
	pinMode(PIN_THROTTLE_OUT,OUTPUT);			//D8 as output PWM motor controller
	analogWrite(PIN_THROTTLE_OUT,0);			//No power
		//Inputs
			//Front wheel - PB25 - digital pin 2
	REG_PIOB_PDR=(1 << BIT_FRONT_WHEEL);		//Disable PIO control (pin to peripheral)
	REG_PIOB_PUDR=(1 << BIT_FRONT_WHEEL);		//Pull-up off
	REG_PIOB_ABSR |= (1 << BIT_FRONT_WHEEL);	//Peripheral B = TIOA0
			//Rear axle - PA2 - analog pin 7
	REG_PIOA_PDR=(1 << BIT_REAR_AXLE);			//Disable PIO control (pin to peripheral)
	REG_PIOA_PUDR=(1 << BIT_REAR_AXLE);			//Pull-up off
	REG_PIOA_ABSR &= ~(1 << BIT_REAR_AXLE);		//Peripheral A = TIOA1

	//Timer
		//Clock
	REG_PMC_PCER0=(1 << ID_TC0)|(1 << ID_TC1)|(1 << ID_TC2);		//Clock TC0, TC1, TC2 on
		//TC0 - front wheel period (capture mode)
	REG_TC0_CMR0=(0 << 0)						//TCCLKS: TIMER_CLOCK1 = MCK/2 = 42 MHz
				|(2 << 8)						//ETRGEDG: counter reset on falling edge
				|(1 << 10)						//ABETRG: TIOA is the trigger
				|(2 << 16);						//LDRA: RA loaded on falling edge of TIOA -> RA = period
	REG_TC0_IER0=(1 << 5);						//Interrupt on RA loading (LDRAS)
	REG_TC0_CCR0=(1 << 0)|(1 << 2);				//Clock enable, software trigger (start)
		//TC1 - rear axle period (capture mode)
	REG_TC0_CMR1=(0 << 0)						//TCCLKS: TIMER_CLOCK1 = MCK/2 = 42 MHz
				|(2 << 8)						//ETRGEDG: counter reset on falling edge
				|(1 << 10)						//ABETRG: TIOA is the trigger
				|(2 << 16);						//LDRA: RA loaded on falling edge of TIOA -> RA = period
	REG_TC0_IER1=(1 << 5);						//Interrupt on RA loading (LDRAS)
	REG_TC0_CCR1=(1 << 0)|(1 << 2);				//Clock enable, software trigger (start)
		//TC2 - periodic interrupt 1ms (waveform mode)
	REG_TC0_CMR2=(0 << 0)						//TCCLKS: TIMER_CLOCK1 = MCK/2 = 42 MHz
				|(1 << 15)						//WAVE: waveform mode
				|(2 << 13);						//WAVSEL: up to RC, then restart
	REG_TC0_RC2=CAPTURE_HZ/TICK_HZ;				//42 MHz / 1000 = 42000 -> 1ms
	REG_TC0_IER2=(1 << 4);						//Interrupt on RC compare (CPCS)
	REG_TC0_CCR2=(1 << 0)|(1 << 2);				//Clock enable, software trigger (start)

	//Interrupt
	NVIC_SetPriority(TC1_IRQn,0);				//Rear axle highest priority
	NVIC_SetPriority(TC0_IRQn,1);				//Front wheel
	NVIC_SetPriority(TC2_IRQn,2);				//Tick
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_EnableIRQ(TC1_IRQn);
	NVIC_EnableIRQ(TC2_IRQn);

#if DISPLAY_ACTIVE
	Display_Init();
#endif

	while(1)		//Main loop
	{
		//Snapshot of the ISR values
		noInterrupts();
		ticks=ticks_pending;
		ticks_pending=0;
		front=timeout_front ? period_front : 0;	//0 = front wheel stands still
		rear=timeout_rear ? period_rear : 0;	//0 = rear axle stands still
		interrupts();

		throttle_target=Throttle_Read(analogRead(PIN_THROTTLE_IN));

		switch(sequence)
		{
			case WAIT_RELEASE:	if(throttle_target)							//Pedal pressed at power up
								{
#if DISPLAY_ACTIVE
									if(!message_shown) Display_Message("Release throttle");
#endif
									message_shown=1;
								}
								else										//Pedal released
								{
#if DISPLAY_ACTIVE
									Display_Message("Ready");
#endif
									digitalWrite(PIN_IGNITION,LOW);			//LOW -> ignition on
									sequence=DRIVE;
								}
								break;

			case DRIVE:			if(ticks)									//At least one 1ms tick elapsed
								{
									while(ticks--)							//One ramp step per elapsed ms, also after a display job
									{
#if ASR_ACTIVE
										Traction_Control(throttle_target, throttle_asr, Slip_Detect(front, rear));
#else
										throttle_asr=throttle_target;
#endif
									}
									analogWrite(PIN_THROTTLE_OUT,throttle_asr);	//Power demand to the motor controller
								}
#if DISPLAY_ACTIVE
								Display_Update(Speed_Calc(front), throttle_asr*100/PWM_MAX, throttle_asr<throttle_target, display_timer);
#endif
								break;
		}//end switch sequence
	}//end while
}//end main

void TC0_Handler(void)		//Front wheel: RA loaded
{
	REG_TC0_SR0;								//Read status once -> clear interrupt flag
	period_front=REG_TC0_RA0;					//Period of the signal in capture ticks
	timeout_front=TIME_RPM_TIMEOUT;				//Wheel turns
}

void TC1_Handler(void)		//Rear axle: RA loaded
{
	REG_TC0_SR1;								//Read status once -> clear interrupt flag
	period_rear=REG_TC0_RA1;					//Period of the signal in capture ticks
	timeout_rear=TIME_RPM_TIMEOUT;				//Axle turns
}

void TC2_Handler(void)		//Periodic interrupt every 1ms
{
	REG_TC0_SR2;								//Read status once -> clear interrupt flag
	if(timeout_front) timeout_front--;
	if(timeout_rear) timeout_rear--;
	if(display_timer) display_timer--;
	if(ticks_pending<255) ticks_pending++;
}
