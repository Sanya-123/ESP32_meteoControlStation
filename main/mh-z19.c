#include "mh-z19.h"
#include "uart2.h"

/*
Запрос данных: на скорости 9600(8N1) нужно отправить девять байт:
 0xFF — начало любой команды
 0x01 — первый сенсор (он всего один)
 0x86 — команда
 0x00, 0x00, 0x00, 0x00, 0x00 — данные
 0x79 — контрольная сумма.

В ответ придет что-то такое:
 0xFF — начало любого ответа
 0x86 — команда
 0x01, 0xC1 — старшее и младшее значение (256 * 0x01 + 0xC1 = 449)
 0x3C, 0x04, 0x3C, 0xC1 — в документации сказано, что должно приходить что-то типа 0x47, 0x00, 0x00, 0x00, но на деле приходит непонятно что.
 0x7B — контрольная сумма.

Контрольная сумма считается следующим образом: берутся 7 байт ответа (все кроме первого и последнего), складываются, инвертируются, увеличиваются на 1: 0x86 + 0x01… + 0xC1 = 0x85, 0x85 xor 0xFF = 0x7A, 0x7A + 1 = 0x7B.
*/

void co2_init()
{
    uart2_init();
}

//#define ERROR_BAD_HANDLER   -1
//#define ERROR_BAD_CRC       -2

#define PACKET_SIZE 9
static const uint8_t zapros_co2[PACKET_SIZE] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};


bool checkCrc(uint8_t *packet)
{
    uint8_t checksum = 0;

    for (uint8_t i = 1; i < 8; i++)
    {
        checksum += packet[i];
    }
    checksum = 0xFF - checksum;
    checksum += 1;

    return checksum == packet[8];
}

uint16_t co2_read()
{
    static uint16_t previousMeas = 0;

    // Запрос данных:
    uart2_flush();
    for (uint8_t i = 0; i < PACKET_SIZE; i++)
    {
        uart2_putChar(zapros_co2[i]);
    }

    // Чтение ответа
    uint8_t otvet_co2[PACKET_SIZE] ;
    for (uint8_t i = 0; i < PACKET_SIZE; i++) // FIXME
    {
        otvet_co2[i] = uart2_getChar();
    }

    // проверки на правильность пакета и CRC
    if (otvet_co2[0] != 0xFF || !checkCrc(otvet_co2) )
    {
        return previousMeas;
    }

    // проверки на выход за пределы датчика (400-5000)
    uint16_t measure = 256 * otvet_co2[2] + otvet_co2[3];
    if (measure < 300 || measure > 5500)
    {
        return previousMeas;
    }

    previousMeas = measure;
    return measure;
}
