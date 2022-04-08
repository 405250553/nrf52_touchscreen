/*
 * XPT2046_touch.h
 *
 *  Created on: 20 sep. 2019.
 *      Author: Andriy Honcharenko
 */

#ifndef XPT2046_TOUCH_H
#define XPT2046_TOUCH_H

#include <stdbool.h>
#include "nrf_gpio.h"
#include "nrf_drv_spi.h"

/*** Redefine if necessary ***/
#define	SOFTWARE_SPI

#define XPT2046_IRQ_Pin       	26
#define XPT2046_MISO_Pin        25
#define XPT2046_MOSI_Pin        24
#define XPT2046_SS_Pin 	        23
#define XPT2046_SCK_Pin         22

#define TOUCH_ORIENTATION_PORTRAIT 		(0U)
#define TOUCH_ORIENTATION_LANDSCAPE 		(1U)
#define TOUCH_ORIENTATION_PORTRAIT_MIRROR 	(2U)
#define TOUCH_ORIENTATION_LANDSCAPE_MIRROR 	(3U)

#define ORIENTATION	(TOUCH_ORIENTATION_PORTRAIT)

#define XPT2046_SPI_INSTANCE 0

// change depending on screen orientation
#if (ORIENTATION == 0)
#define XPT2046_SCALE_X 240
#define XPT2046_SCALE_Y 320
#elif (ORIENTATION == 1)
#define XPT2046_SCALE_X 320
#define XPT2046_SCALE_Y 240
#elif (ORIENTATION == 2)
#define XPT2046_SCALE_X 240
#define XPT2046_SCALE_Y 320
#elif (ORIENTATION == 3)
#define XPT2046_SCALE_X 320
#define XPT2046_SCALE_Y 240
#endif

// to calibrate uncomment UART_Printf line in ili9341_touch.c
#define XPT2046_MIN_RAW_X 3400
#define XPT2046_MAX_RAW_X 14500
#define XPT2046_MIN_RAW_Y 3400
#define XPT2046_MAX_RAW_Y 14500

// call before initializing any SPI devices
void xpt2046_init(void);
bool XPT2046_TouchPressed(void);
bool XPT2046_TouchGetCoordinates_Average(uint16_t* x, uint16_t* y);
bool XPT2046_TouchGetCoordinates_Average_threshold(uint16_t* x, uint16_t* y);

#endif /* XPT2046_TOUCH_H_ */