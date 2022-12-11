#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stdbool.h>

#include "display_gui.h"

//defoult setting display
#define DEFOULT_AUTO_CHANGE_DISPAY          true
#define DEFOULT_USE_MMHG                    false
#define DEFOULT_DARK_THEME                  false
#define DEFOULT_STYLE                       0
#define DEFOULT_TIME_ON_DISPLAY             3



bool initFlash();
void saveGuiSittings(SettignDisplay settings);
bool getGuiSittings(SettignDisplay *settings);
bool saveExtDeviceSize(uint8_t connections);
uint8_t getExtDeviceSize();

#endif // FLASH_H
