#include "lcd_i2c.h"

#define LCD_ADDR (0x27 << 1)
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_RS 0x01

void I2C1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER &= ~((3 << (8*2)) | (3 << (9*2)));
    GPIOB->MODER |= (2 << (8*2)) | (2 << (9*2));
    GPIOB->OTYPER |= (1 << 8) | (1 << 9);
    GPIOB->PUPDR |= (1 << (8*2)) | (1 << (9*2));
    GPIOB->AFR[1] |= (4 << ((8-8)*4)) | (4 << ((9-8)*4));

    I2C1->CR1 |= I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 = 16;
    I2C1->CCR = 80;
    I2C1->TRISE = 17;

    I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_Write(uint8_t addr, uint8_t data) {
    while (I2C1->SR2 & I2C_SR2_BUSY);

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));

    (void)I2C1->SR1;
    I2C1->DR = addr;
    while (!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2;

    while (!(I2C1->SR1 & I2C_SR1_TXE));
    I2C1->DR = data;

    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

void LCD_SendNibble(uint8_t nibble, uint8_t control) {
    uint8_t data = nibble | control | LCD_BACKLIGHT;
    I2C1_Write(LCD_ADDR, data | LCD_ENABLE);
    for (volatile int i = 0; i < 1000; i++);
    I2C1_Write(LCD_ADDR, data & ~LCD_ENABLE);
    for (volatile int i = 0; i < 1000; i++);
}

void LCD_SendByte(uint8_t byte, uint8_t mode) {
    LCD_SendNibble(byte & 0xF0, mode);
    LCD_SendNibble((byte << 4) & 0xF0, mode);
}

void LCD_SendCommand(uint8_t cmd) {
    LCD_SendByte(cmd, 0);
}

void LCD_SendData(uint8_t data) {
    LCD_SendByte(data, LCD_RS);
}

void LCD_Init(void) {
    for (volatile int i = 0; i < 500000; i++);
    LCD_SendNibble(0x30, 0); for (volatile int i = 0; i < 50000; i++);
    LCD_SendNibble(0x30, 0); for (volatile int i = 0; i < 50000; i++);
    LCD_SendNibble(0x30, 0); for (volatile int i = 0; i < 50000; i++);
    LCD_SendNibble(0x20, 0); for (volatile int i = 0; i < 50000; i++);

    LCD_SendCommand(0x28);
    LCD_SendCommand(0x0C);
    LCD_SendCommand(0x01);
    for (volatile int i = 0; i < 50000; i++);
    LCD_SendCommand(0x06);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t pos = (row == 0) ? 0x00 : 0x40;
    LCD_SendCommand(0x80 | (pos + col));
}

void LCD_Print(char *str) {
    while (*str) {
        LCD_SendData((uint8_t)*str++);
    }
}

