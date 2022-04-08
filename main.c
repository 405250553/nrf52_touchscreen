#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
#include "bsp.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_spi.h"

#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif

#include "ili9341.h"
#include "XPT2046_touch.h"

#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

extern const nrf_lcd_t nrf_lcd_ili9341;

void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[20];
    //static uint8_t index = 0;
    //uint32_t       err_code;
 
    switch (p_event->evt_type)
    {
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(data_array));
              //for (uint32_t i = 0; i < 20; i++)
              //printf("input: %c",data_array[i]);              
            break;
 
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;
 
        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;
 
        default:
            break;
    }
}

void uart_init()
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
          RX_PIN_NUMBER,
          TX_PIN_NUMBER,
          NULL,
          NULL,
          APP_UART_FLOW_CONTROL_ENABLED,
          false,
#if defined (UART_PRESENT)
          NRF_UART_BAUDRATE_115200
#else
          NRF_UARTE_BAUDRATE_115200
#endif
      };

    APP_UART_FIFO_INIT(&comm_params,
                         UART_RX_BUF_SIZE,
                         UART_TX_BUF_SIZE,
                         uart_event_handle,
                         APP_IRQ_PRIORITY_LOWEST,
                         err_code);

    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for main application entry.
 */
int main(void)
{
    uart_init();
   
    nrf_lcd_ili9341.lcd_init();

    //nrf_lcd_ili9341.lcd_rect_draw(0,0,20,20,512);
    //nrf_lcd_ili9341.lcd_rect_draw(0,ILI9341_WIDTH-20,20,20,512);
    //nrf_lcd_ili9341.lcd_rect_draw(ILI9341_HEIGHT-20,0,20,20,512);
    //nrf_lcd_ili9341.lcd_rect_draw(ILI9341_HEIGHT-20,ILI9341_WIDTH-20,20,20,512);

    xpt2046_init();
    nrf_delay_ms(20);
    uint16_t data[2] = {0};

    /* Configure LED-pins as outputs */
    //bsp_board_init(BSP_INIT_LEDS);

    while(1){
        //printf("print!!!!!!\r\n");
        bool data_flag = XPT2046_TouchGetCoordinates_Average(&data[0], &data[1]);
        //bool data_flag = XPT2046_TouchGetCoordinates_Average_threshold(&data[0], &data[1]);
        if(data_flag)
          nrf_lcd_ili9341.lcd_rect_draw(ILI9341_HEIGHT-data[0],ILI9341_WIDTH-data[1],5,5,512);
        nrf_delay_ms(1);
    }
}


/** @} */
