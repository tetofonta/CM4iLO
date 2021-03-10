//
// Created by stefano on 3/10/21.
//
#include <stm32f4xx_hal.h>
#include "error_handler.h"
#include "main.h"

uint8_t func_to_code(const char * function_name, uint8_t variant){
    uint8_t r = 0xBA;
    while(*function_name != 0x00)
        r ^= *(function_name++);
    return r ^ variant;
}

void error_handler_code (int code){
    __disable_irq();
    volatile int q = 0;
    while (1) {
        _no_delay_write_code(code);
        for (uint32_t i = 0; i < 300000 * 96 / 4; ++i) { q++; }
        CLEAR_CODE();
        for (uint32_t i = 0; i < 100000 * 96 / 4; ++i) { q++; }
    }
}