# nrf52_touchscreen

- SDK : nRF5_SDK_15.2.0_9412b96
- IDE : Segger
- project : nRF5_SDK_15.2.0_9412b96\examples\peripheral\ili9341_spi_test+xpt2046
- monitor : 2.4 吋 TFT+觸控螢幕 ILI9341 SPI液晶屏模塊 240*320，**使用此螢幕記得led pin和 reset pin 要pull high** (ili9341.c 參考 nRF5_SDK_15.2.0_9412b96\components\drivers_ext\ili9341 )
![image](https://user-images.githubusercontent.com/44420087/162472861-55d3612a-1763-4a74-acbe-10ca247e2c36.png)

schematic
![image](https://user-images.githubusercontent.com/44420087/162506381-ef62368d-6ece-4801-9b9f-d215620777d7.png)

- ili9341為螢幕驅動
- xpt2046為一個12 bits resolution ADC，負責採集觸控螢幕時電壓的變化，ADC data會透過spi訊號送出，每次讀取以24個clock為一個週期，前8個clk為mcu給的cmd，後16個clk為adc data (12 bit mode就是後4個bit 為空)
 
 ![image](https://user-images.githubusercontent.com/44420087/162508412-8754521e-742a-4add-a5ca-3905fb2fefd4.png)

- (P.S.) 此Project 有用到nrf_lcd.h，已搬移到project中

**- (P.S.) 在寫的時候，不知道為甚麼spi channel 1 不 work，所以xpt2046 Driver 的 spi 是用gpio模擬的** 

## 電阻式觸控螢幕
當手指觸摸屏幕時，兩層導電層在觸摸點位置就有了接觸，電阻發生變化，在X和Y兩個方向上的電壓發生變化，產生信號

## ADC TO pixel

由於電阻式觸摸屏的品質和讀數問題，在開始時首先要將觸摸屏校準（Calibration），**方法是通過觸摸液晶屏的4個角，採集 X 和 Y 座標**的12位值(我的code還是以16bit做計算，反正shift 4位也只是= /16)，再通過數學等比公式計算，就可以得到比例值。

```c
//XPT2046_touch.h
//ADC VALUE 的 4個角最大最小值
#define XPT2046_MIN_RAW_X 3400
#define XPT2046_MAX_RAW_X 14500
#define XPT2046_MIN_RAW_Y 3400
#define XPT2046_MAX_RAW_Y 14500
```

## Project 介紹
每隔1ms就執行函式看現在是否有東西觸控螢幕
```C
int main(void)
{
    uart_init();
   
    nrf_lcd_ili9341.lcd_init();

    xpt2046_init();
    nrf_delay_ms(20);
    uint16_t data[2] = {0};

    /* Configure LED-pins as outputs */
    //bsp_board_init(BSP_INIT_LEDS);

    while(1){
        bool data_flag = XPT2046_TouchGetCoordinates_Average(&data[0], &data[1]);
        //bool data_flag = XPT2046_TouchGetCoordinates_Average_threshold(&data[0], &data[1]);
        if(data_flag)
          nrf_lcd_ili9341.lcd_rect_draw(ILI9341_HEIGHT-data[0],ILI9341_WIDTH-data[1],5,5,512);
        nrf_delay_ms(1);
    }
}
```

當螢幕感到按壓時，**T_IRQ那隻腳會pull low**，此時再讀取data
```C
//return True,
bool XPT2046_TouchPressed(void)
{
    //return HAL_GPIO_ReadPin(XPT2046_IRQ_GPIO_Port, XPT2046_IRQ_Pin) == GPIO_PIN_RESET;
    return nrf_gpio_pin_read(XPT2046_IRQ_Pin) == 0;
}
```

由於是adc sample，為了減少誤差，使用多次sample來取平均

```C
bool XPT2046_TouchGetCoordinates_Average(uint16_t* x, uint16_t* y)
{
#ifndef SOFTWARE_SPI

    static const uint8_t cmd_read_x[] = { READ_X };
    static const uint8_t cmd_read_y[] = { READ_Y };
    static const uint8_t zeroes_tx[] = { 0x00, 0x00 };

#endif /* SOFTWARE_SPI */
    
    //SPI SS pin pull low
    XPT2046_TouchSelect();

    uint32_t avg_x = 0;
    uint32_t avg_y = 0;
    uint8_t nsamples = 0;

    for(uint8_t i = 0; i < AVER_TIME; i++)
    {
        if(!XPT2046_TouchPressed())//touchpad is pressed or not
            break;

        nsamples++;

        uint8_t y_raw[2]={0};
        uint8_t x_raw[2]={0};


        spi_write_byte(READ_Y);

        y_raw[0] = spi_read_byte();
        y_raw[1] = spi_read_byte();

        spi_write_byte(READ_X);

        x_raw[0] = spi_read_byte();
        x_raw[1] = spi_read_byte();


        avg_x += (((uint16_t)x_raw[0]) << 8) | ((uint16_t)x_raw[1]);
        avg_y += (((uint16_t)y_raw[0]) << 8) | ((uint16_t)y_raw[1]);
        
        //if(i==0)
        //printf("raw_x = %d, raw_y = %d\r\n", (int) avg_x, (int) avg_y);
    }
    
    //SPI SS pin pull high
    XPT2046_TouchUnselect();
    
    //not all nsample is press
    if(nsamples < AVER_TIME)
        return false;
    
    //average and detect limit
    uint32_t raw_x = (avg_x / AVER_TIME);
    if(raw_x < XPT2046_MIN_RAW_X) raw_x = XPT2046_MIN_RAW_X;
    if(raw_x > XPT2046_MAX_RAW_X) raw_x = XPT2046_MAX_RAW_X;
    
    //average and detect limit
    uint32_t raw_y = (avg_y / AVER_TIME);
    if(raw_y < XPT2046_MIN_RAW_Y) raw_y = XPT2046_MIN_RAW_Y;
    if(raw_y > XPT2046_MAX_RAW_Y) raw_y = XPT2046_MAX_RAW_Y;

    // Uncomment this line to calibrate touchscreen:
    //printf("raw_x = %d, raw_y = %d\r\n", (int) raw_x, (int) raw_y);
    
    //raw_x,raw_y is adc val coordinate，nedd to transfor to pixel coordinate
    *x = (raw_x - XPT2046_MIN_RAW_X) * XPT2046_SCALE_X / (XPT2046_MAX_RAW_X - XPT2046_MIN_RAW_X);
    *y = (raw_y - XPT2046_MIN_RAW_Y) * XPT2046_SCALE_Y / (XPT2046_MAX_RAW_Y - XPT2046_MIN_RAW_Y);

    //printf("raw_x = %d, raw_y = %d\r\n", *x, *y);
    return true;
}
```
## 成果
![touchpad](https://user-images.githubusercontent.com/44420087/162516532-90c6522d-7615-44a1-9209-ba2c22a6f310.gif)


## 參考

http://www.51hei.com/bbs/dpj-92932-1.html

http://bugworkshop.blogspot.com/2016/07/diy-pic24-xpt2046-xy.html

https://github.com/taburyak/STM32-touchscreen-XPT2046-HAL-SPI
