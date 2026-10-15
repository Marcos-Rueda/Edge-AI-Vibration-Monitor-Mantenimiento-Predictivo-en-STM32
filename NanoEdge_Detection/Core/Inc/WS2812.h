/*
 * ARGB.h
 *
 *  Created on: Feb 18, 2025
 *      Author: Angel Jimenez
 */

#ifndef INC_WS2812_H_
#define INC_WS2812_H_

void WS2812_Set_LED (int LEDnum, int Red, int Green, int Blue);

void WS2812_Clear_LED();

void WS2812_Set_All_LED( int Red, int Green, int Blue);

void WS2812_Set_Brightness (int brightness);  // 0-45

void WS2812_Update (void);

#endif /* INC_WS2812_H_ */
