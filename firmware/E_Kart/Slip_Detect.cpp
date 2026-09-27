// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Slip_Detect.cpp
 *
 *  Author: Johannes Stockhammer
 *
 * Compares the periods of rear axle and front wheel. A shorter period means a higher
 * speed. Slip: rear axle faster than front wheel by SLIP_PERCENT (after the conversion
 * CALIB_PERCENT). Period 0 = wheel stands still -> no slip, the ASR does not intervene.
 */
#include "Slip_Detect.h"

bool Slip_Detect(uint32_t period_front, uint32_t period_rear)
{
	if((period_rear==0)||(period_front==0)) return 0;		//No signal -> no intervention
	return (uint64_t)period_rear*SLIP_PERCENT<(uint64_t)period_front*CALIB_PERCENT;	//64 bit: no overflow
}
