set(SOURCE "main.c" "net.c" "bme280_ok.c" "tft.c" "display.c" "uart2.c" "mh-z19.c" "bme_i2c.c"
"./esp_request/esp_request.c" "./esp_request/req_list.c" "./esp_request/uri_parser.c" "./nrf24l01.c"
"openWeather.c" "tjpgd.c" "decode_image.c" "ipgeolocation.c" "flash.c"
"display_gui.c" "display_logo.c" "images.c"
)


#"./tft2/spi_master.c" "./tft2/tftfunc.c"
#"./tft/comic24.c" "./tft/DefaultFont.c" "./tft/DejaVuSans18.c" "./tft/DejaVuSans24.c" "./tft/spi_master_nodma.c" "./tft/tft.c" "./tft/minya24.c" "./tft/tftfunc.c" "./tft/tooney32.c" "./tft/Ubuntu16.c"
#            "./spidriver/spi_master_lobo.c"
#            "./tft/comic24.c" "./tft/def_small.c" "./tft/DefaultFont.c""./tft/DejaVuSans18.c""./tft/DejaVuSans24.c" "./tft/minya24.c"
#            "./tft/SmallFont.c" "./tft/tft.c" "./tft/tftspi.c" "./tft/tooney32.c" "./tft/Ubuntu16.c"
#            "./spiffs/esp_spiffs.c" "./spiffs/list.c" "./spiffs/mutex.c" "./spiffs/spiffs_cache.c" "./spiffs/spiffs_check.c"
#            "./spiffs/spiffs_gc.c" "./spiffs/spiffs_hydrogen.c" "./spiffs/spiffs_nucleus.c" "./spiffs/spiffs_vfs.c"

file(GLOB_RECURSE IMAGERS images/wind/*.jpg images/wheather/*.jpg images/wheatherB/*.jpg images/moon/*.jpg images/wheather_white_x1/*.png)

file(GLOB_RECURSE IMAGERS_C images_c/*.bin)

set(IMG ${IMAGERS} images/display_logo.png)

#images/wind/E.jpg images/wind/N.jpg images/wind/NE.jpg images/wind/NW.jpg images/wind/S.jpg images/wind/SE.jpg images/wind/SW.jpg images/wind/W.jpg
#images/wheather/clear-day.jpg images/wheather/clear-night.jpg images/wheather/cloudy.jpg images/wheather/drizzle.jpg images/wheather/fog.jpg images/wheather/hail.jpg
#images/wheather/lightRain.jpg images/wheather/partly-cloudy-day.jpg images/wheather/partly-cloudy-night.jpg images/wheather/rain.jpg images/wheather/sleet.jpg
#images/wheather/snow.jpg images/wheather/thunderstorm.jpg images/wheather/unknown.jpg images/wheather/wind.jpg

idf_component_register(SRCS ${SOURCE}
                    INCLUDE_DIRS "." "./esp_request/include"
                    EMBED_FILES ${IMG} ${IMAGERS_C})
