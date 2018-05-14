#ifndef OLED_H
#define OLED_H

#include "stm32l4xx_hal.h"

#define oled_rst_clr() HAL_GPIO_WritePin(SPI2_RST_GPIO_Port,SPI2_RST_Pin,GPIO_PIN_RESET)
#define oled_rst_set() HAL_GPIO_WritePin(SPI2_RST_GPIO_Port,SPI2_RST_Pin,GPIO_PIN_SET)

#define oled_rs_clr() HAL_GPIO_WritePin(SPI2_RS_GPIO_Port,SPI2_RS_Pin,GPIO_PIN_RESET)
#define oled_rs_set() HAL_GPIO_WritePin(SPI2_RS_GPIO_Port,SPI2_RS_Pin,GPIO_PIN_SET)

#define oled_cmd 0
#define oled_data 1
#define Max_Column	128
#define SIZE 16

void oled_refresh_gram(void);
void oled_wr_byte(uint8_t dat,uint8_t cmd);
void oled_set_pos(uint8_t x, uint8_t y);
void oled_display_on(void);
void oled_display_off(void);
void oled_clear(void);
void oled_showchar(uint8_t x,uint8_t y,uint8_t chr);
uint32_t oled_pow(uint8_t m,uint8_t n);
void oled_shownum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void oled_showstring(uint8_t x,uint8_t y,uint8_t *chr);
void OLED_ShowCh(uint8_t x,uint8_t y,uint8_t *dst);
void OLED_Show_Bat(uint8_t x,uint8_t y,uint8_t *dst);
void OLED_DrawBMP(uint8_t x0, uint8_t y0,uint8_t x1, uint8_t y1,uint8_t BMP[]);
void oled_init(void);

#endif
