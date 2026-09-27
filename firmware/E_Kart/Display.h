// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Display.h
 *
 *  Author: Johannes Stockhammer
 */


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "Kart.h"
void Display_Init(void);
void Display_Message(const char *text);
void Display_Update(uint16_t speed_x10, uint8_t throttle_percent, bool asr_limiting, volatile uint16_t&display_timer);

#endif /* DISPLAY_H_ */
