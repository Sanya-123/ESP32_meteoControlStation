#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)


#Compile image file into the resulting firmware binary
#images/imager.jpg images/humidity0.jpg images/humidity1.jpg images/temperature0.jpg images/temperature1.jpg images/co2.jpg images/im1.jpg images/im2.jpg
COMPONENT_EMBED_FILES := images/*.jpg images/wind/*.jpg images/wheather/*.jpg images/wheatherB/*.jpg images/test/*.jpg

COMPONENT_ADD_INCLUDEDIRS = wifi_manager
COMPONENT_SRCDIRS = wifi_manager
COMPONENT_DEPENDS = log esp_http_server
COMPONENT_EMBED_FILES := wifi_manager/style.css wifi_manager/code.js wifi_manager/index.html
