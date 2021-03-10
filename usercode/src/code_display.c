//
// Created by stefano on 3/10/21.
//
#include <stm32f4xx_hal.h>
#include "main.h"

uint16_t positions_a[] = {
        0b10110111,
        0b00000101,
        0b10101110,
        0b10001111,
        0b00011101,
        0b10011011,
        0b10111011,
        0b00000111,
        0b10111111,
        0b10011111,
        0b00111111,
        0b10111001,
        0b10110010,
        0b10101101,
        0b10111010,
        0b00111010
};
uint16_t positions_b[] = {
        0b01110111,
        0b00010001,
        0b10110110,
        0b10110011,
        0b11010001,
        0b11100011,
        0b11100111,
        0b00110001,
        0b11110111,
        0b11110011,
        0b11110101,
        0b11000111,
        0b01100110,
        0b10010111,
        0b11100110,
        0b11100100
};

void _no_delay_write_code(uint8_t code) {
    HAL_GPIO_WritePin(CODE_DAT_GPIO_Port, CODE_DAT_Pin, 0);
    HAL_GPIO_WritePin(CODE_CLK_GPIO_Port, CODE_CLK_Pin, 0);

    for (int i = 0; i < 8; ++i)
        SEND_CODE_BIT((positions_a[code & 0x0F] >> i) & 1);
    for (int i = 0; i < 8; ++i)
        SEND_CODE_BIT((positions_b[code >> 4] >> i) & 1);
}

void write_code(uint8_t code) {
    _no_delay_write_code(code);
    HAL_Delay(2);
}