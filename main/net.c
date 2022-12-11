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
#include "esp_system.h"
//#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
//#include "nvs_flash.h"
//#include "esp_netif.h"
//#include "protocol_examples_common.h"

//#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "lwip/netdb.h"
#include "lwip/ip4_addr.h"

#include "esp_attr.h"
#include "soc/rtc.h"
#include "net.h"
#include "esp_dpp.h"

#include "wifi_manager.h"
#include "display_gui.h"

#include "csevent.h"

#define CURVE_SEC256R1_PKEY_HEX_DIGITS      64

extern const char *TAG;

extern int reciveSMD(char *rx, char *tx, int n);

char qrcodeAP[256];
char qrcodeURL[256];
char str_ip[IP4ADDR_STRLEN_MAX];

void wifi_fail_connect(void *arg);

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];
    char tx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
            int to_write = reciveSMD(rx_buffer, tx_buffer, len);
            len = to_write;
            while (to_write > 0) {
                int written = send(sock, tx_buffer + (len - to_write), to_write, 0);
                if (written < 0) {
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                }
                to_write -= written;
            }
             
        }
    } while (len > 0);
}

void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    } else if (addr_family == AF_INET6) {
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

//static int s_retry_num = 0;

void dpp_enrollee_event_cb(esp_supp_dpp_event_t event, void *data)
{
    wifi_config_t s_dpp_wifi_config;
    wifi_config_t *s_wifiManager_config = wifi_manager_get_wifi_sta_config();
    switch (event) {
    case ESP_SUPP_DPP_URI_READY:
        if (data != NULL) {
            char *qrData = (char *)data;
            ESP_LOGI("DPP", "Generate QRcode:%s", (const char *)data);
            setDPPQrData(qrData, strlen(qrData));
            if(esp_supp_dpp_start_listen() != ESP_OK)
                ESP_LOGE("DPP", "Error listen dpp");
        }
        break;
    case ESP_SUPP_DPP_CFG_RECVD:
        memcpy(&s_dpp_wifi_config, data, sizeof(s_dpp_wifi_config));
        memset(data, 0, sizeof(s_dpp_wifi_config));//clean old data
        ESP_LOGI("DPP", "DPP Authentication successful, connecting to AP : %s;%s",
                 s_dpp_wifi_config.sta.ssid, s_dpp_wifi_config.sta.password);
        wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED, wifi_fail_connect);
        if(s_wifiManager_config != NULL)
        {
            s_wifiManager_config->sta = s_dpp_wifi_config.sta;
            wifi_manager_connect_async();
        }
//        else
//            if(esp_supp_dpp_start_listen() != ESP_OK)
//                ESP_LOGE("DPP", "Error listen dpp");

        break;
    case ESP_SUPP_DPP_FAIL:
        ESP_LOGE("DPP", "DPP Auth failed (Reason: %s), retry...", esp_err_to_name((int)data));
        if(esp_supp_dpp_start_listen() != ESP_OK)
            ESP_LOGE("DPP", "Error listen dpp");
        break;
    default:
        break;
    }
}

void wifi_fail_connect(void *arg)
{
    (void)arg;
    ESP_LOGI("WIFI", "**************************Fail connect to AP**************************");

    if(esp_supp_dpp_start_listen() != ESP_OK)
        ESP_LOGE("DPP", "Error listen dpp");
}

void wifi_connect_to_ap(void *arg)
{
    esp_ip4_addr_t ip4 = ((ip_event_got_ip_t*)arg)->ip_info.ip;
    ESP_LOGI("WIFI", "**************************Connect To AP**************************");
    wifi_manager_set_callback(WM_EVENT_STA_DISCONNECTED, NULL);

    esp_ip4addr_ntoa(&ip4, str_ip, IP4ADDR_STRLEN_MAX);
    ESP_LOGI("WIFI", "IP:%s", str_ip);

    sendEvent(EVENT_WIFI_CONNECT_STA);
    setWifiConnect(true);
    setEnableQrPage(false);
    setIPdev(str_ip);

    sprintf(qrcodeURL, "http://%s/", str_ip);
    seturlQrData(qrcodeURL, strlen(qrcodeURL));

    //deinit dpp
    esp_supp_dpp_stop_listen();
    esp_supp_dpp_deinit();
}

void wifi_start_ap(void *arg)
{
    (void)arg;
    ESP_LOGI("WIFI", "**************************Start AP**************************");

    //generate qrcode
    sprintf(qrcodeAP, "WIFI:T:WPA;S:%s;P:%s;H:%s;;", DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD, DEFAULT_AP_SSID_HIDDEN ? "true" : "");
    ESP_LOGI("WIFI", "QRcode AP:%s", qrcodeAP);


    setWifiConnect(false);
    setEnableQrPage(true);
    setIPdev(DEFAULT_AP_IP);
    setCurentAPQrData(qrcodeAP, strlen(qrcodeAP));

    sprintf(qrcodeURL, "http://%s/", DEFAULT_AP_IP);
    seturlQrData(qrcodeURL, strlen(qrcodeURL));



    //start DPP
    esp_supp_dpp_init(dpp_enrollee_event_cb);
    esp_supp_dpp_bootstrap_gen(CONFIG_ESP_DPP_LISTEN_CHANNEL_LIST, DPP_BOOTSTRAP_QR_CODE,
                                         CONFIG_ESP_DPP_BOOTSTRAPPING_KEY, CONFIG_ESP_DPP_DEVICE_INFO);
}

void startNet()
{
    wifi_manager_start();
    wifi_manager_set_callback(WM_EVENT_STA_GOT_IP, wifi_connect_to_ap);
    wifi_manager_set_callback(WM_ORDER_START_AP, wifi_start_ap);
}


