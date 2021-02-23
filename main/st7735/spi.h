#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>
#include <stdbool.h>


void spi_init();
void spi_write(uint8_t *data, uint32_t size);


#endif // _SPI_H
