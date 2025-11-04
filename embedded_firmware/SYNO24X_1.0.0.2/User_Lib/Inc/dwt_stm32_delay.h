#ifndef DWT_STM32_DELAY_H
#define DWT_STM32_DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal.h"

/**
 * @brief  Initializes DWT_Cycle_Count for DWT_Delay_us function
 * @return Error DWT counter
 *         1: DWT counter Error
 *         0: DWT counter works
 */
//__STATIC_INLINE void DWT_Delay_us(volatile uint32_t au32_microseconds);
uint32_t DWT_Delay_Init(void);
/**
 * @brief  This function provides a delay (in microseconds)
 * @param  microseconds: delay in microseconds
 */
__STATIC_INLINE void DWT_Delay_us(volatile uint32_t au32_microseconds)
{
	uint32_t au32_initial_ticks = DWT->CYCCNT;
	uint32_t au32_ticks = (HAL_RCC_GetHCLKFreq() / 1000000);
	au32_microseconds *= au32_ticks;
	while ((DWT->CYCCNT - au32_initial_ticks) < au32_microseconds-au32_ticks);
}
__STATIC_INLINE void DWT_Delay_ms(volatile uint32_t au32_milliseconds)
{
	uint32_t au32_initial_ticks = DWT->CYCCNT;
	uint32_t au32_ticks = (HAL_RCC_GetHCLKFreq() / 1000);
	au32_milliseconds *= au32_ticks;
	while ((DWT->CYCCNT - au32_initial_ticks) < au32_milliseconds);
}

#ifdef __cplusplus
}
#endif

#endif
