// #include "HardwareSerial.h"
// #include <ESPAsyncWebServer.h>
#include <RemoteSerial.h>
// #include "ObdReadingHelper.h"


#define ANSI_RED "\x1B[0;91m"
#define ANSI_GREEN "\x1B[0;92m"
#define ANSI_YELLOW "\x1B[0;93m"
#define ANSI_BLUE "\x1B[0;94m"
#define ANSI_MAGENTA "\x1B[0;95m"
#define ANSI_CYAN "\x1B[0;96m"
#define ANSI_WHITE "\x1B[0;97m"

AsyncWebServer remoteSerialServer(8080);

// Handle any incoming messages
void messageReceived(const uint8_t *data, size_t len) {
  char str[len];

  for (uint16_t i = 0; i < len; i++) {
    str[i] = data[i];
  }
  str[len] = 0;

  Serial.print("Received: ");
  Serial.println(str);


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
  RemoteSerial.begin(&remoteSerialServer);  // You can also add a custom url, e.g. /remotedebug
  remoteSerialServer.begin();
}