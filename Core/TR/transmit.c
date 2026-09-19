#include "transmit.h"
#include "usart.h"

#include <stdarg.h>
#include <stdio.h>

#define UART_TX_BUFFER_SIZE 256

//格式化后的字符串暂存在这里，函数返回前已经发送完成
static char uart_tx_buffer[UART_TX_BUFFER_SIZE];

static void UART_vprintf(UART_HandleTypeDef *huart, const char *format, va_list args)
{
	int length;

	length = vsnprintf(uart_tx_buffer, UART_TX_BUFFER_SIZE, format, args);

	if(length < 0)
	{
		return;
	}
	if(length >= UART_TX_BUFFER_SIZE)
	{
		length = UART_TX_BUFFER_SIZE - 1;
	}

	HAL_UART_Transmit(huart, (uint8_t *)uart_tx_buffer, (uint16_t)length, HAL_MAX_DELAY);
}

void USART1_printf(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	UART_vprintf(&huart1, format, args);
	va_end(args);
}

//void USART2_printf(const char *format, ...)
//{
//	va_list args;

//	va_start(args, format);
//	UART_vprintf(&huart2, format, args);
//	va_end(args);
//}

//void USART3_printf(const char *format, ...)
//{
//	va_list args;

//	va_start(args, format);
//	UART_vprintf(&huart3, format, args);
//	va_end(args);
//}
