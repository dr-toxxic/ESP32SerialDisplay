#include "Constants.h"
//#include <Vector.h>

#if USE_LOVYANGFX
#if ESP32_3248S035
#include "ESP32_3248S035_Display.h"

#elif ESP32_2432S028R
#include "ESP32_2432S028R_Display.h"
#endif
#else
#include <TFT_eSPI.h>
#include <SPI.h>
#endif

#include <lvgl.h>
#include "ui.h"
//#include "BasicOTA.h"

String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

/*Change to your screen resolution*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 480;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

#if USE_LOVYANGFX && (ESP32_2432S028R || ESP32_3248S035)
LGFX tft;
#else
TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
#endif


#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char *buf) {
  Serial.printf(buf);
  Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  uint16_t touchX = 0, touchY = 0;

  bool touched = tft.getTouch(&touchX, &touchY, 600);

  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.printf("(%d, %d)\n ", touchX, touchY);
  }
}

#ifndef LGFX_USE_V1
void touch_calibrate() {
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formatting file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    } else {
      fs::File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    fs::File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}
#endif


void ui_setup() {
  lv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

  tft.begin();        /* TFT init */
  tft.setRotation(0); /* Landscape orientation, flipped */


#ifdef LGFX_USE_V1
  uint16_t calData[] = { 239, 3926, 233, 265, 3856, 3896, 3714, 308 };
  tft.setTouchCalibrate(calData);
#else

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  touch_calibrate();
#endif

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);


  ui_init();


  static uint32_t user_data = 10;
  lv_timer_t *timer = lv_timer_create(serial_timer, 50, &user_data);

  Serial.println("UI Initialized");
}
uint32_t lineCount = 0;
const uint32_t maxLineCount = 30;

void serial_timer(lv_timer_t *timer) {
  serialEvent();
  /*Use the user_data*/
  // print the string when a newline arrives:
  if (stringComplete) {
    lineCount++;
    inputString = String(lineCount) + ": " + inputString;
    lv_textarea_add_text(ui_txtSerial, inputString.c_str());


    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;

    if (lineCount % maxLineCount == 0) {
      lv_textarea_set_text(ui_txtSerial, inputString.c_str());
    }
  }
}

void setup() {
  serial_setup();
  ui_setup();
  // OTA_setup();
}

void loop() {
  //OTA_loop();
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}

#define RXD2 21
#define TXD2 22

void serial_setup() {
  // initialize serial:
  Serial.begin(115200);
  //Serial2.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }

  while (Serial1.available()) {
    // get the new byte:
    char inChar = (char)Serial1.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }


  while (Serial2.available()) {
    // get the new byte:
    char inChar = (char)Serial2.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}