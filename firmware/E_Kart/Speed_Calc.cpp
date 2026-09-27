// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Speed_Calc.cpp
 *
 *  Author: Johannes Stockhammer
 *
 * Speed in 0.1 km/h from the period of the front wheel (not driven, so no slip).
 * v = (FRONT_WHEEL_MM / FRONT_TEETH) mm per pulse / (period / CAPTURE_HZ) s
 * km/h * 10 = mm/s * 0.036
 */
#include "Speed_Calc.h"

uint16_t Speed_Calc(uint32_t period_front)
{
	uint64_t v;

	if(period_front==0) return 0;							//Wheel stands still
	v=(uint64_t)FRONT_WHEEL_MM*CAPTURE_HZ*36/((uint64_t)FRONT_TEETH*period_front*1000);
	if(v>65535) v=65535;									//Limit to 16 bit
	return (uint16_t)v;
}
