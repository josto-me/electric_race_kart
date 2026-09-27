// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Traction_Control.h
 *
 *  Author: Johannes Stockhammer
 */


#ifndef TRACTION_CONTROL_H_
#define TRACTION_CONTROL_H_

#include "Kart.h"
void Traction_Control(uint16_t throttle_target, uint16_t&throttle_asr, bool slip);

#endif /* TRACTION_CONTROL_H_ */
