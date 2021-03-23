#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)


#Compile image file into the resulting firmware binary
COMPONENT_EMBED_FILES := imager.jpg humidity0.jpg humidity1.jpg temperature0.jpg temperature1.jpg co2.jpg im1.jpg im2.jpg

COMPONENT_ADD_INCLUDEDIRS = wifi_manager
COMPONENT_SRCDIRS = wifi_manager
COMPONENT_DEPENDS = log esp_http_server
COMPONENT_EMBED_FILES := wifi_manager/style.css wifi_manager/code.js wifi_manager/index.html
