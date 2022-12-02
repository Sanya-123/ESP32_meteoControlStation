#ifndef IPGEOLOCATION_H
#define IPGEOLOCATION_H

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct
{
    //location
    char country_code[2];
    char city[128];
    float lat, lon;
    //time
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minutes;
}IpLocation;

int getLocation(IpLocation *location);

#endif // IPGEOLOCATION_H
