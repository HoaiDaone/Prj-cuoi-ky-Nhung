#ifndef __LCD_I2C_H__
#define __LCD_I2C_H__

#include "stm32f4xx.h"

void I2C1_Init(void);
void LCD_Init(void); // <-- Sửa lại không có tham số
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(char *str);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);

#endif
