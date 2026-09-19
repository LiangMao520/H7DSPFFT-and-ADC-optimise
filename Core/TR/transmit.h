#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

/**
 * @brief  使用printf格式，通过指定的USART阻塞发送数据
 * @param  format printf格式字符串
 * @note   必须在对应的MX_USARTx_UART_Init()执行完成后调用，不建议在中断中调用
 */
void USART1_printf(const char *format, ...);
void USART2_printf(const char *format, ...);
void USART3_printf(const char *format, ...);

#endif
