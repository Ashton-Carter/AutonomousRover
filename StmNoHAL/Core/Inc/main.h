#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g4xx.h"
#include <stdint.h>

void Error_Handler(void);

#define LPUART1_TX_PIN     2
#define LPUART1_TX_PORT    GPIOA

#define LPUART1_RX_PIN     3
#define LPUART1_RX_PORT    GPIOA

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
