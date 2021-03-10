//
// Created by stefano on 3/10/21.
//

#ifndef CMRACK_CODE_DISPLAY_H
#define CMRACK_CODE_DISPLAY_H

#define SEND_CODE_BIT(b) do{HAL_GPIO_WritePin(CODE_DAT_GPIO_Port, CODE_DAT_Pin, b);HAL_GPIO_WritePin(CODE_CLK_GPIO_Port, CODE_CLK_Pin, 1);HAL_GPIO_WritePin(CODE_CLK_GPIO_Port, CODE_CLK_Pin, 0);}while(0)
#define CLEAR_CODE() do{uint8_t i = 16; while(i--)SEND_CODE_BIT(i == 9 || i == 4);}while(0)

void _no_delay_write_code(uint8_t code);
void write_code(uint8_t code);

#endif //CMRACK_CODE_DISPLAY_H
