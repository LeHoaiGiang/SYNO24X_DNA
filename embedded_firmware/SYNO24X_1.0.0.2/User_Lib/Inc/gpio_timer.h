/*
 * gpio_timer.h
 *
 *  Created on: Jun 9, 2025
 *      Author: LHGiang
 */

#ifndef GPIO_TIMER_H_
#define GPIO_TIMER_H_
#include "main.h"
// Định nghĩa cấu trúc cho một GPIO có hẹn giờ
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t is_on;
    uint32_t start_time;
    uint32_t duration_ms;
} GPIO_Timer_Object;

// Hàm khởi tạo đối tượng GPIO_Timer
void GPIO_Timer_init(GPIO_Timer_Object* obj, GPIO_TypeDef* port, uint16_t pin);

// Bật GPIO và đặt thời gian tắt sau delay_ms mili giây
void GPIO_Timer_turnOn(GPIO_Timer_Object* obj, uint32_t delay_ms);

// Tat GPIO
void GPIO_Timer_turnOff(GPIO_Timer_Object* obj);
// Kiểm tra xem đã đến lúc tắt chưa - gọi liên tục trong vòng lặp
void GPIO_Timer_update(GPIO_Timer_Object* obj);



#endif /* GPIO_TIMER_H_ */
