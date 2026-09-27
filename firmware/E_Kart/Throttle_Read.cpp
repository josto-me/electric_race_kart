// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Throttle_Read.cpp
 *
 *  Author: Johannes Stockhammer
 *
 * Converts the ADC value of the throttle pedal into a PWM value 0..PWM_MAX.
 * Below THROTTLE_ADC_MIN the pedal counts as released (dead band at rest).
 */
#include "Throttle_Read.h"

uint16_t Throttle_Read(int32_t adc)
{
	if(adc<=THROTTLE_ADC_MIN) return 0;				//Pedal released
	if(adc>=THROTTLE_ADC_MAX) return PWM_MAX;		//Pedal fully pressed -> power limit
	return (adc-THROTTLE_ADC_MIN)*PWM_MAX/(THROTTLE_ADC_MAX-THROTTLE_ADC_MIN);	//Linear in between
}
