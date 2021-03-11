//
// Created by stefano on 3/10/21.
//

#ifndef CMRACK_ERROR_HANDLER_H
#define CMRACK_ERROR_HANDLER_H

#include <stdint.h>
#include "code_display.h"
uint8_t func_to_code(const char *, uint32_t);
void error_handler_code(int) __attribute__((noreturn));

#define Error_Handler() error_handler_code(func_to_code(__func__, __LINE__))

#endif //CMRACK_ERROR_HANDLER_H
