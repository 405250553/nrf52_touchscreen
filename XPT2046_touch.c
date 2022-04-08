/*
 * XPT2046_touch.c
 *
 *  Created on: 20 sep. 2019.
 *      Author: Andriy Honcharenko
 */

#include <stdio.h>
#include <stdlib.h>
#include "XPT2046_touch.h"

#define AVER_TIME 10
#define XY_threshold 50

#if (ORIENTATION == 0)
#define READ_X 0xD0
#define READ_Y 0x90
#elif (ORIENTATION == 1)
#define READ_Y 0xD0
#define READ_X 0x90
#elif (ORIENTATION == 2)
#define READ_X 0xD0
#define READ_Y 0x90
#elif (ORIENTATION == 3)
#define READ_Y 0xD0
#define READ_X 0x90
#endif

#ifdef SOFTWARE_SPI

#define SPI_CLK_L()		nrf_gpio_pin_clear(XPT2046_SCK_Pin)
#define SPI_CLK_H()		nrf_gpio_pin_set(XPT2046_SCK_Pin)
#define SPI_MOSI_L()            nrf_gpio_pin_clear(XPT2046_MOSI_Pin)
#define SPI_MOSI_H()            nrf_gpio_pin_set(XPT2046_MOSI_Pin)
#define SPI_MISO()		nrf_gpio_pin_read(XPT2046_MISO_Pin)

static void spi_write_byte(uint8_t data)
{
	for(size_t i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			SPI_MOSI_H();
		}
		else
		{
			SPI_MOSI_L();
		}
		data = data << 1;
		SPI_CLK_L();
		SPI_CLK_H();
	}
}

static uint8_t spi_read_byte(void)
{
	uint8_t i, ret , value;
	ret = 0;
	i = 8;

	do {
		i--;
		SPI_CLK_L();
		value = SPI_MISO();
		if (value)
		{
			//set the bit to 0 no matter what
			ret |= (1 << i);
		}

		SPI_CLK_H();
	} while (i > 0);

	return ret;
}


void xpt2046_init(void)
{
    nrf_gpio_cfg_output(XPT2046_MOSI_Pin);
    nrf_gpio_cfg_output(XPT2046_SCK_Pin);
    nrf_gpio_cfg_input(XPT2046_MISO_Pin,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(XPT2046_SS_Pin);

    nrf_gpio_cfg_input(XPT2046_IRQ_Pin,NRF_GPIO_PIN_NOPULL);
}

#else

static const nrf_drv_spi_t xpt2046_spi = NRF_DRV_SPI_INSTANCE(XPT2046_SPI_INSTANCE);

void xpt2046_init(void)
{
    ret_code_t err_code;
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

    spi_config.sck_pin  = XPT2046_SCK_Pin;
    spi_config.miso_pin = XPT2046_MISO_Pin;
    spi_config.mosi_pin = XPT2046_MOSI_Pin;
    //spi_config.ss_pin   = NULL;

    spi_config.frequency = NRF_DRV_SPI_FREQ_1M;

    err_code = nrf_drv_spi_init(&xpt2046_spi, &spi_config, NULL, NULL);

    if (err_code != NRF_SUCCESS)
    {
        printf("error!!!!!!\r\n");
    }

    nrf_gpio_cfg_input(XPT2046_IRQ_Pin,NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_output(XPT2046_SS_Pin);
}

#endif /* SOFTWARE_SPI */

static void XPT2046_TouchSelect()
{
    //HAL_GPIO_WritePin(XPT2046_CS_GPIO_Port, XPT2046_CS_Pin, GPIO_PIN_RESET);
    nrf_gpio_pin_clear(XPT2046_SS_Pin);
}

static void XPT2046_TouchUnselect()
{
    //HAL_GPIO_WritePin(XPT2046_CS_GPIO_Port, XPT2046_CS_Pin, GPIO_PIN_SET);
    nrf_gpio_pin_set(XPT2046_SS_Pin);
}

bool XPT2046_TouchPressed(void)
{
    //return HAL_GPIO_ReadPin(XPT2046_IRQ_GPIO_Port, XPT2046_IRQ_Pin) == GPIO_PIN_RESET;
    return nrf_gpio_pin_read(XPT2046_IRQ_Pin) == 0;
}

bool XPT2046_TouchGetCoordinates_Average(uint16_t* x, uint16_t* y)
{
#ifndef SOFTWARE_SPI

    static const uint8_t cmd_read_x[] = { READ_X };
    static const uint8_t cmd_read_y[] = { READ_Y };
    static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

#endif /* SOFTWARE_SPI */

    XPT2046_TouchSelect();

    uint32_t avg_x = 0;
    uint32_t avg_y = 0;
    uint8_t nsamples = 0;

    for(uint8_t i = 0; i < AVER_TIME; i++)
    {
        if(!XPT2046_TouchPressed())
            break;

        nsamples++;

        uint8_t y_raw[2]={0};
        uint8_t x_raw[2]={0};

#ifdef SOFTWARE_SPI

       spi_write_byte(READ_Y);

	   y_raw[0] = spi_read_byte();
	   y_raw[1] = spi_read_byte();

	   spi_write_byte(READ_X);

	   x_raw[0] = spi_read_byte();
	   x_raw[1] = spi_read_byte();

#else

        //HAL_SPI_Transmit(&XPT2046_SPI_PORT, (uint8_t*)cmd_read_y, sizeof(cmd_read_y), HAL_MAX_DELAY);
        //HAL_SPI_TransmitReceive(&XPT2046_SPI_PORT, (uint8_t*)zeroes_tx, y_raw, sizeof(y_raw), HAL_MAX_DELAY);

        //HAL_SPI_Transmit(&XPT2046_SPI_PORT, (uint8_t*)cmd_read_x, sizeof(cmd_read_x), HAL_MAX_DELAY);
        //HAL_SPI_TransmitReceive(&XPT2046_SPI_PORT, (uint8_t*)zeroes_tx, x_raw, sizeof(x_raw), HAL_MAX_DELAY);

         //APP_ERROR_CHECK(nrf_drv_spi_transfer(&xpt2046_spi,cmd_read_y,sizeof(cmd_read_y), NULL, 0));
         //APP_ERROR_CHECK(nrf_drv_spi_transfer(&xpt2046_spi,(uint8_t*)cmd_read_x,sizeof(cmd_read_x), x_raw, sizeof(x_raw)));

         //nrf_drv_spi_transfer(&xpt2046_spi,(uint8_t*)cmd_read_y,sizeof(cmd_read_y), NULL, 0);
         //nrf_drv_spi_transfer(&xpt2046_spi,(uint8_t*)zeroes_tx,sizeof(zeroes_tx),y_raw, sizeof(y_raw));
        

#endif /* SOFTWARE_SPI */

        avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
        
        //if(i==0)
        //printf("raw_x = %d, raw_y = %d\r\n", (int) avg_x, (int) avg_y);
    }

    XPT2046_TouchUnselect();

    if(nsamples < AVER_TIME)
        return false;

    uint32_t raw_x = (avg_x / AVER_TIME);
    if(raw_x < XPT2046_MIN_RAW_X) raw_x = XPT2046_MIN_RAW_X;
    if(raw_x > XPT2046_MAX_RAW_X) raw_x = XPT2046_MAX_RAW_X;

    uint32_t raw_y = (avg_y / AVER_TIME);
    if(raw_y < XPT2046_MIN_RAW_Y) raw_y = XPT2046_MIN_RAW_Y;
    if(raw_y > XPT2046_MAX_RAW_Y) raw_y = XPT2046_MAX_RAW_Y;

    // Uncomment this line to calibrate touchscreen:
    //printf("raw_x = %d, raw_y = %d\r\n", (int) raw_x, (int) raw_y);

    *x = (raw_x - XPT2046_MIN_RAW_X) * XPT2046_SCALE_X / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
    *y = (raw_y - XPT2046_MIN_RAW_Y) * XPT2046_SCALE_Y / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);

    //printf("raw_x = %d, raw_y = %d\r\n", *x, *y);
    return true;
}


bool XPT2046_TouchGetCoordinates_Average_threshold(uint16_t* x, uint16_t* y)
{
#ifndef SOFTWARE_SPI

    static const uint8_t cmd_read_x[] = { READ_X };
    static const uint8_t cmd_read_y[] = { READ_Y };
    static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

#endif /* SOFTWARE_SPI */

    XPT2046_TouchSelect();

    uint32_t avg_x = 0;
    uint32_t avg_y = 0;
    uint8_t nsamples = 0;

    for(uint8_t i = 0; i < AVER_TIME; i++)
    {
        if(!XPT2046_TouchPressed())
            break;

        nsamples++;

        uint8_t y_raw[2]={0};
        uint8_t x_raw[2]={0};

#ifdef SOFTWARE_SPI

       spi_write_byte(READ_Y);

	   y_raw[0] = spi_read_byte();
	   y_raw[1] = spi_read_byte();

	   spi_write_byte(READ_X);

	   x_raw[0] = spi_read_byte();
	   x_raw[1] = spi_read_byte();

#else

        //HAL_SPI_Transmit(&XPT2046_SPI_PORT, (uint8_t*)cmd_read_y, sizeof(cmd_read_y), HAL_MAX_DELAY);
        //HAL_SPI_TransmitReceive(&XPT2046_SPI_PORT, (uint8_t*)zeroes_tx, y_raw, sizeof(y_raw), HAL_MAX_DELAY);

        //HAL_SPI_Transmit(&XPT2046_SPI_PORT, (uint8_t*)cmd_read_x, sizeof(cmd_read_x), HAL_MAX_DELAY);
        //HAL_SPI_TransmitReceive(&XPT2046_SPI_PORT, (uint8_t*)zeroes_tx, x_raw, sizeof(x_raw), HAL_MAX_DELAY);

         //APP_ERROR_CHECK(nrf_drv_spi_transfer(&xpt2046_spi,cmd_read_y,sizeof(cmd_read_y), NULL, 0));
         //APP_ERROR_CHECK(nrf_drv_spi_transfer(&xpt2046_spi,(uint8_t*)cmd_read_x,sizeof(cmd_read_x), x_raw, sizeof(x_raw)));

         //nrf_drv_spi_transfer(&xpt2046_spi,(uint8_t*)cmd_read_y,sizeof(cmd_read_y), NULL, 0);
         //nrf_drv_spi_transfer(&xpt2046_spi,(uint8_t*)zeroes_tx,sizeof(zeroes_tx),y_raw, sizeof(y_raw));
        

#endif /* SOFTWARE_SPI */

        avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
        
        //if(i==0)
        //printf("raw_x = %d, raw_y = %d\r\n", (int) avg_x, (int) avg_y);
    }

    XPT2046_TouchUnselect();

    if(nsamples < AVER_TIME)
        return false;

    uint32_t raw_x = (avg_x / AVER_TIME);
    uint32_t raw_y = (avg_y / AVER_TIME);
    
    static uint32_t old_raw_x,old_raw_y,count=0;

    if(count==0){
        old_raw_x = raw_x;
        old_raw_y = raw_y;
        count++;
    }

    if( abs(old_raw_x - raw_x) > XY_threshold || abs(old_raw_y - raw_y) > XY_threshold )
        false;
    
    if(raw_x < XPT2046_MIN_RAW_X) raw_x = XPT2046_MIN_RAW_X;
    if(raw_x > XPT2046_MAX_RAW_X) raw_x = XPT2046_MAX_RAW_X;

    if(raw_y < XPT2046_MIN_RAW_Y) raw_y = XPT2046_MIN_RAW_Y;
    if(raw_y > XPT2046_MAX_RAW_Y) raw_y = XPT2046_MAX_RAW_Y;

    old_raw_x = raw_x;
    old_raw_y = raw_y;

    // Uncomment this line to calibrate touchscreen:
    //printf("raw_x = %d, raw_y = %d\r\n", (int) raw_x, (int) raw_y);

    *x = (raw_x - XPT2046_MIN_RAW_X) * XPT2046_SCALE_X / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
    *y = (raw_y - XPT2046_MIN_RAW_Y) * XPT2046_SCALE_Y / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);

    //printf("raw_x = %d, raw_y = %d\r\n", *x, *y);
    return true;
}