#ifndef CONSTANTS_H_
#define CONSTANTS_H_
#define CALIBRATION_FILE "/TouchCalData"
#define REPEAT_CAL false
// DEFINE WHICH DISPLAY WE ARE USING
#define ESP32_3248S035 1
#define ESP32_2432S028R 0
#define USE_LOVYANGFX 1

#if USE_LOVYANGFX
#define LGFX_USE_V1
#endif

#if ESP32_3248S035
#define SCREEN_W 320
#define SCREEN_H 480
#define SCREEN_ORIENTATION 1
#else
#define SCREEN_ORIENTATION 3
#define SCREEN_W 320
#define SCREEN_H 240
#endif

#define TOUCH_W SCREEN_W - 1
#define TOUCH_H SCREEN_H - 1

#endif
