#ifndef BASIC_OTA_H_
#define BASIC_OTA_H_
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <lvgl.h>
#include <TFT_eSPI.h>
#include "ui.h"
const char* ssid = "between2williams";
const char* password = "tuxisacunt";

void OTA_setup() {

  Serial.println("Starting WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int i = 0;

  while (WiFi.waitForConnectResult() != WL_CONNECTED && i++ < 50) {
    
    //lv_bar_set_value(ui_barLoading, ++i, LV_ANIM_OFF);
    Serial.print(".");
    delay(10);
    
    /*    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
    */
  }
  if (i == 50) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  } else {
    /* for (int j = i; j < 50; j++) {
      lv_bar_set_value(ui_barLoading, j, LV_ANIM_OFF);
      delay(1);
    }
    */
  }


  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  ArduinoOTA.setPassword(password);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void OTA_loop() {
  ArduinoOTA.handle();
}
#endif