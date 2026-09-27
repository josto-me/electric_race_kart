// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Traction_Control.cpp
 *
 *  Author: Johannes Stockhammer
 *
 * One step of the traction control (ASR), called once per 1ms tick.
 * Slip: ramp the power down by ASR_RAMP_DOWN. No slip: follow the pedal upwards with
 * ASR_RAMP_UP, downwards immediately.
 */
#include "Traction_Control.h"

void Traction_Control(uint16_t throttle_target, uint16_t&throttle_asr, bool slip)
{
	if(slip)												//Rear axle spins
	{
		if(throttle_asr>ASR_RAMP_DOWN) throttle_asr-=ASR_RAMP_DOWN;		//Reduce power
		else throttle_asr=0;
	}
	else
	{
		if(throttle_asr+ASR_RAMP_UP<throttle_target) throttle_asr+=ASR_RAMP_UP;	//Limited ramp up
		else throttle_asr=throttle_target;					//Pedal reached or released -> follow directly
	}
}
