set(SOURCE "main.c" "net.c" "bme280_ok.c" "ST7735.c" "display.c" "uart2.c" "mh-z19.c"
"./esp_request/esp_request.c" "./esp_request/req_list.c" "./esp_request/uri_parser.c" "./nrf24l01.c"
"./cJSON/cJSON_Utils.c" "./cJSON/cJSON.c" "openWeather.c" "tjpgd.c" "decode_image.c"
)


#"./tft2/spi_master.c" "./tft2/tftfunc.c"
#"./tft/comic24.c" "./tft/DefaultFont.c" "./tft/DejaVuSans18.c" "./tft/DejaVuSans24.c" "./tft/spi_master_nodma.c" "./tft/tft.c" "./tft/minya24.c" "./tft/tftfunc.c" "./tft/tooney32.c" "./tft/Ubuntu16.c"
#            "./spidriver/spi_master_lobo.c"
#            "./tft/comic24.c" "./tft/def_small.c" "./tft/DefaultFont.c""./tft/DejaVuSans18.c""./tft/DejaVuSans24.c" "./tft/minya24.c"
#            "./tft/SmallFont.c" "./tft/tft.c" "./tft/tftspi.c" "./tft/tooney32.c" "./tft/Ubuntu16.c"
#            "./spiffs/esp_spiffs.c" "./spiffs/list.c" "./spiffs/mutex.c" "./spiffs/spiffs_cache.c" "./spiffs/spiffs_check.c"
#            "./spiffs/spiffs_gc.c" "./spiffs/spiffs_hydrogen.c" "./spiffs/spiffs_nucleus.c" "./spiffs/spiffs_vfs.c"

set(img images/imager.jpg images/humidity0.jpg images/humidity1.jpg images/temperature0.jpg images/temperature1.jpg images/co2.jpg)

idf_component_register(SRCS ${SOURCE}
                    INCLUDE_DIRS "." "./esp_request/include"
                    EMBED_FILES ${img})
