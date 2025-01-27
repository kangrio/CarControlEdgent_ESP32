#include "WString.h"
#include <RemoteSerial.h>
#include <ESP32-TWAI-CAN.hpp>

#define ANSI_RED "\x1B[0;91m"
#define ANSI_GREEN "\x1B[0;92m"
#define ANSI_YELLOW "\x1B[0;93m"
#define ANSI_BLUE "\x1B[0;94m"
#define ANSI_MAGENTA "\x1B[0;95m"
#define ANSI_CYAN "\x1B[0;96m"
#define ANSI_WHITE "\x1B[0;97m"

AsyncWebServer remoteSerialServer(8080);

uint16_t pidMonitor = 0xF;

// Handle any incoming messages
void messageReceived(const uint8_t *data, size_t len) {
  char str[len + 1];  // +1 to accommodate the null-terminator

  for (size_t i = 0; i < len; i++) {
    str[i] = data[i];
  }
  str[len] = '\0';  // Null-terminate the string

  if (strcmp(str, "reboot") == 0) {  // Compare strings
    LOG_PRINT.println("Rebooting...");
    ESP.restart();
    for (;;)
      ;
  } else if (strcmp(str, "refresh") == 0) {
    LOG_PRINT.println("Refreshing...");

    CanFrame obdFrame = { 0 };

    obdFrame.identifier = 0x7DF;  // OBD2 address;
    obdFrame.extd = 0;
    obdFrame.data_length_code = 8;
    obdFrame.data[0] = 2;
    obdFrame.data[1] = 0x01;
    obdFrame.data[2] = 0x00;
    obdFrame.data[3] = 0xAA;
    obdFrame.data[4] = 0xAA;
    obdFrame.data[5] = 0xAA;
    obdFrame.data[6] = 0xAA;
    obdFrame.data[7] = 0xAA;
    ESP32Can.writeFrame(obdFrame, 5);
  } else {
    uint16_t pid = strtoul(str, NULL, 16);
    pidMonitor = pid;
    LOG_PRINT.println(pidMonitor, HEX);
  }


  // ESP.restart();

  // String strData = String(str);

  // if (strData.equalsIgnoreCase("stopobd")) {
  //   TwaiStop();
  // } else if (strData.equalsIgnoreCase("startobd")) {
  //   TwaiBegin();
  // } else if (strData.equalsIgnoreCase("refreshobd")) {
  //   sendDefaultObdFrame(0x0);
  // }
}


void startRemoteSerialMonitor() {
  RemoteSerial.setIncomingMessageHandler(messageReceived);
  RemoteSerial.setHttpAuth(Secrets.remoteSerialUsername, Secrets.remoteSerialPassword);
  RemoteSerial.begin(&remoteSerialServer);  // You can also add a custom url, e.g. /remotedebug
  remoteSerialServer.begin();
}