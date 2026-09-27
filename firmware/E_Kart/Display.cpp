// SPDX-FileCopyrightText: Johannes Stockhammer
// SPDX-License-Identifier: Apache-2.0
/*
 * Display.cpp
 *
 *  Author: Johannes Stockhammer
 *
 * Dashboard on the 3.5" TFT (480x320, HX8357D, hardware SPI): speed in km/h, max speed,
 * throttle bar and an ASR lamp that lights while the traction control holds the throttle back.
 * Drawing a large digit over SPI takes a few ms, longer than one control tick. So each call
 * of Display_Update() does at most one small drawing job and only redraws what changed;
 * the control loop keeps running in between.
 */

//Includes
#include "Display.h"
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>

//Defines
	//Colours
#define COLOR_BG HX8357_BLACK
#define COLOR_TEXT HX8357_WHITE
#define COLOR_BAR HX8357_GREEN
#define COLOR_ASR_ON HX8357_RED
#define COLOR_ASR_OFF 0x2104							//Dark grey
	//Layout (landscape 480 x 320)
#define SPEED_SIZE 14									//Text size: one digit = 84 x 112 px
#define SPEED_X 40
#define SPEED_Y 60
#define DIGIT_W (6*SPEED_SIZE)
#define NUM_DIGITS 3									//Speed 0..999 km/h
#define BAR_X 40
#define BAR_Y 262
#define BAR_W 400
#define BAR_H 40
#define ASR_X 378
#define ASR_Y 6
#define ASR_W 96
#define ASR_H 36
#define MAX_X 170
#define MSG_Y 216
	//Drawing jobs, one per call
#define JOB_IDLE 0
#define JOB_DIGIT_0 1
#define JOB_DIGIT_1 2
#define JOB_DIGIT_2 3
#define JOB_BAR 4
#define JOB_ASR 5
#define JOB_MAX 6

//Variables
static Adafruit_HX8357 tft=Adafruit_HX8357(PIN_TFT_CS, PIN_TFT_DC, PIN_TFT_RST);
static uint8_t job=JOB_IDLE;
	//Values of the running refresh
static char want_digit[NUM_DIGITS];
static int16_t want_bar=0;
static bool want_asr=OFF;
	//Content of the screen
static char shown_digit[NUM_DIGITS];
static int16_t shown_bar=0;
static int8_t shown_asr=-1;								//-1 = not drawn yet
static uint16_t shown_max=0xFFFF;						//0xFFFF = not drawn yet
static uint16_t max_kmh=0;

void Display_Init(void)
{
	uint8_t k;

	tft.begin();
	tft.setRotation(1);									//Landscape
	tft.fillScreen(COLOR_BG);

	tft.setTextColor(COLOR_TEXT, COLOR_BG);				//Title
	tft.setTextSize(3);
	tft.setCursor(8,12);
	tft.print("E-KART");

	tft.setTextSize(4);									//Unit next to the speed
	tft.setCursor(SPEED_X+NUM_DIGITS*DIGIT_W+16, SPEED_Y+8*SPEED_SIZE-32);
	tft.print("km/h");

	tft.drawRect(BAR_X-2, BAR_Y-2, BAR_W+4, BAR_H+4, COLOR_TEXT);	//Frame throttle bar

	for(k=0;k<NUM_DIGITS;k++) shown_digit[k]=0;			//Unknown -> draw on first refresh
	shown_bar=0;
	shown_asr=-1;
	shown_max=0xFFFF;
	max_kmh=0;
	job=JOB_IDLE;
}

void Display_Message(const char *text)
{
	tft.fillRect(0, MSG_Y, 480, 32, COLOR_BG);			//Clear message line
	tft.setTextColor(COLOR_TEXT, COLOR_BG);
	tft.setTextSize(3);
	tft.setCursor(BAR_X, MSG_Y+4);
	tft.print(text);
}

void Display_Update(uint16_t speed_x10, uint8_t throttle_percent, bool asr_limiting, volatile uint16_t&display_timer)
{
	uint16_t kmh, color;
	uint8_t k;
	char text[16];

	switch(job)
	{
		case JOB_IDLE:		if(display_timer) break;						//Refresh time not expired
							display_timer=TIME_DISPLAY;						//Start timer

							kmh=speed_x10/10;								//Snapshot of the values for this refresh
							if(kmh>999) kmh=999;
							if(kmh>max_kmh) max_kmh=kmh;
							snprintf(text, sizeof(text), "%3u", kmh);
							for(k=0;k<NUM_DIGITS;k++) want_digit[k]=text[k];
							if(throttle_percent>100) throttle_percent=100;
							want_bar=(int16_t)((uint32_t)throttle_percent*BAR_W/100);
							want_asr=asr_limiting;
							job=JOB_DIGIT_0;
							break;

		case JOB_DIGIT_0:
		case JOB_DIGIT_1:
		case JOB_DIGIT_2:	k=job-JOB_DIGIT_0;
							if(want_digit[k]!=shown_digit[k])				//Only draw changed digits
							{
								tft.drawChar(SPEED_X+k*DIGIT_W, SPEED_Y, want_digit[k], COLOR_TEXT, COLOR_BG, SPEED_SIZE);
								shown_digit[k]=want_digit[k];
							}
							job++;
							break;

		case JOB_BAR:		if(want_bar>shown_bar)							//Bar longer -> only draw the new part
								tft.fillRect(BAR_X+shown_bar, BAR_Y, want_bar-shown_bar, BAR_H, COLOR_BAR);
							else if(want_bar<shown_bar)						//Bar shorter -> only clear the rest
								tft.fillRect(BAR_X+want_bar, BAR_Y, shown_bar-want_bar, BAR_H, COLOR_BG);
							shown_bar=want_bar;
							job=JOB_ASR;
							break;

		case JOB_ASR:		if(want_asr!=shown_asr)							//ASR lamp changed
							{
								color=want_asr ? COLOR_ASR_ON : COLOR_ASR_OFF;
								tft.fillRect(ASR_X, ASR_Y, ASR_W, ASR_H, color);
								tft.setTextColor(COLOR_TEXT, color);
								tft.setTextSize(3);
								tft.setCursor(ASR_X+21, ASR_Y+7);
								tft.print("ASR");
								shown_asr=want_asr;
							}
							job=JOB_MAX;
							break;

		case JOB_MAX:		if(max_kmh!=shown_max)							//New max speed
							{
								snprintf(text, sizeof(text), "max %3u", max_kmh);
								tft.setTextColor(COLOR_TEXT, COLOR_BG);
								tft.setTextSize(3);
								tft.setCursor(MAX_X, 12);
								tft.print(text);
								shown_max=max_kmh;
							}
							job=JOB_IDLE;
							break;

		default:			job=JOB_IDLE;
							break;
	}//end switch job
}
