#include <ArduinoOTA.h>
#include "Secrets.h"


void setupArduinoOTA() {
  ArduinoOTA.setPasswordHash(Secrets.passWordHashed);
  ArduinoOTA.onStart([]() {
    BlynkState::set(MODE_OTA_UPGRADE);
    Blynk.disconnect();

    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    LOG_PRINT.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    LOG_PRINT.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    LOG_PRINT.printf("Progress: %u%%\r\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    LOG_PRINT.printf("Error[%u]: ", error);

    if (error == OTA_AUTH_ERROR) {
      LOG_PRINT.println("Auth Failed");

    } else if (error == OTA_BEGIN_ERROR) {
      LOG_PRINT.println("Begin Failed");

    } else if (error == OTA_CONNECT_ERROR) {
      LOG_PRINT.println("Connect Failed");

    } else if (error == OTA_RECEIVE_ERROR) {
      LOG_PRINT.println("Receive Failed");

    } else if (error == OTA_END_ERROR) {
      LOG_PRINT.println("End Failed");
    }
    restartMCU();
  });
  ArduinoOTA.begin();
}

void OTAHandle() {
  ArduinoOTA.handle();
}

void enterOTA() {
}