#ifndef _UART2_H_
#define _UART2_H_

#include <stdint.h>


void uart2_init();
uint8_t uart2_getChar(void);     // прием данных
void uart2_putChar(uint8_t c) ;  // вывод символа
void uart2_flush() ;  //
//uint8_t uart2_rxBufSize() ;  //
//bool uart2_rxBufOverflow() ;  //

#endif // _UART2_H_
