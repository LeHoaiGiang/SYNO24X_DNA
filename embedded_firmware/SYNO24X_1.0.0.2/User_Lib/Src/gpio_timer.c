/*
 * gpio_timer.c
 *
 *  Created on: Jun 9, 2025
 *      Author: LHGiang
 */

#include "gpio_timer.h"

// Khởi tạo đối tượng GPIO_Timer
void GPIO_Timer_init(GPIO_Timer_Object* obj, GPIO_TypeDef* port, uint16_t pin)
{
    obj->port = port;
    obj->pin = pin;
    obj->is_on = 0;
}

// Bật chân GPIO và bắt đầu đếm thời gian
void GPIO_Timer_turnOn(GPIO_Timer_Object* obj, uint32_t delay_ms)
{
    HAL_GPIO_WritePin(obj->port, obj->pin, GPIO_PIN_SET);
    obj->is_on = 1;
    obj->start_time = HAL_GetTick();
    obj->duration_ms = delay_ms;
}

// Tat GPIO Ngay lap tuc
void GPIO_Timer_turnOff(GPIO_Timer_Object* obj)
{
	HAL_GPIO_WritePin(obj->port, obj->pin, GPIO_PIN_RESET);
}
// Kiểm tra nếu hết thời gian thì tắt
void GPIO_Timer_update(GPIO_Timer_Object* obj)
{
    if (obj->is_on && (HAL_GetTick() - obj->start_time >= obj->duration_ms)) {
        HAL_GPIO_WritePin(obj->port, obj->pin, GPIO_PIN_RESET);
        obj->is_on = 0;
    }
}


