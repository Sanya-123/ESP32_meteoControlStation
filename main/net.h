#ifndef NET_H
#define NET_H

/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define PORT CONFIG_PORT_SERVER

//wifi TODO to enaouther file
/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      /*CONFIG_ESP_WIFI_SSID*/"TOPOVIY_WIFI"
#define EXAMPLE_ESP_WIFI_PASS      /*CONFIG_ESP_WIFI_PASSWORD*/"05101962"
#define EXAMPLE_ESP_MAXIMUM_RETRY  /*CONFIG_ESP_MAXIMUM_RETRY*/5

void startNet();
void wifi_init_sta(void *p);
void tcp_server_task(void *pvParameters);

#endif
