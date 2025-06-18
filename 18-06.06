#include "main.h"
#include <stdio.h>
#include <string.h>
#include "lcd_i2c.h"

void SystemClock_Config(void);
void Error_Handler(void);
void Buzzer_GPIO_Init(void);
void MQ2_GPIO_Init(void);
void LED_GPIO_Init(void);
void Button_GPIO_Init(void);
void Relay_GPIO_Init(void);
void ADC1_Init(void);

ADC_HandleTypeDef hadc1;
uint32_t lastToggle = 0;
uint32_t blinkInterval = 1000;


int main(void) {
    HAL_Init();
    SystemClock_Config();
    Buzzer_GPIO_Init();
    MQ2_GPIO_Init();
    LED_GPIO_Init();
    Button_GPIO_Init();
    Relay_GPIO_Init();
    I2C1_Init();
    LCD_Init();
    ADC1_Init();


    uint8_t gasLevel = 0;
    uint8_t system_running = 1;
    GPIO_PinState prevSW1 = GPIO_PIN_SET;
    GPIO_PinState prevSW2 = GPIO_PIN_SET;
    static uint16_t ppm_last = 0;
    static uint8_t reset_flag = 0;

    while (1) {
        uint16_t ppm = 0;  // <-- Khai báo ở đây để dùng được bên ngoài if

        // Nút SW1: bật/tắt hệ thống
        GPIO_PinState sw1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
        if (sw1 == GPIO_PIN_RESET && prevSW1 == GPIO_PIN_SET) {
            system_running = !system_running;
            HAL_Delay(200);
        }
        prevSW1 = sw1;

        // Nút SW2: reset
        GPIO_PinState sw2 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7);
        if (sw2 == GPIO_PIN_RESET && prevSW2 == GPIO_PIN_SET) {
            gasLevel = 0;
            ppm_last = 0;
            system_running = 1;
            reset_flag = 1;
            HAL_Delay(200);
        }
        prevSW2 = sw2;

        if (system_running) {
            // Đọc cảm biến MQ2
            uint16_t adc_value = 0;
            HAL_ADC_Start(&hadc1);
            if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
                adc_value = HAL_ADC_GetValue(&hadc1);
            }
            HAL_ADC_Stop(&hadc1);

            // Tính ppm từ giá trị ADC
            ppm = adc_value * 1000 / 4095;

            // Xác định mức gas
            if (ppm < 200) gasLevel = 0;
            else if (ppm < 400) gasLevel = 1;
            else if (ppm < 700) gasLevel = 2;
            else gasLevel = 3;

            // Phản ứng theo mức gas
            switch (gasLevel) {
                case 0:
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
                    break;
                case 1:
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
                    break;
                case 2:
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
                    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
                    break;
                case 3:
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
                    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
                    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // ✅ Bật còi
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
                    break;
            }
        } else {
            // Hệ thống dừng
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
        }

        // Cập nhật ppm_last nếu cần
        if (reset_flag) {
            ppm_last = 0;
            reset_flag = 0;
        } else if (system_running && ppm > 0) {
            ppm_last = ppm;
        }

        // Hiển thị LCD
        char line1[17], line2[17];
        snprintf(line1, sizeof(line1), "%-6s %4dppm", system_running ? "WORK" : "STOP", ppm_last);

        if (!system_running) {
            snprintf(line2, sizeof(line2), "System stopped  ");
        } else if (ppm == 0) {
            snprintf(line2, sizeof(line2), "No Gas          ");
        } else if (ppm < 300) {
            snprintf(line2, sizeof(line2), "Gas Low         ");
        } else if (ppm < 1000) {
            snprintf(line2, sizeof(line2), "Gas High        ");
        } else {
            snprintf(line2, sizeof(line2), "DANGER Level!   ");
        }

        LCD_SetCursor(0, 0); LCD_Print(line1);
        LCD_SetCursor(1, 0); LCD_Print(line2);

        HAL_Delay(500);
    }
}
// ==== GPIO Init Functions ====

void Buzzer_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void MQ2_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void LED_GPIO_Init(void) {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Button_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Relay_GPIO_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // Tắt rơ-le ban đầu
}

// ==== Clock & Error Handler ====

void Error_Handler(void) {
    while (1);
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}
void ADC1_Init(void) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    ADC_ChannelConfTypeDef sConfig = {0};

    hadc1.Instance = ADC1;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;

    HAL_ADC_Init(&hadc1);

    sConfig.Channel = ADC_CHANNEL_1;  // PA0 = MQ2
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}
