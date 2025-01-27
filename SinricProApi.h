#include "SinricPro.h"
#include "SinricProBlinds.h"
#include "Secrets.h"

typedef void (*CallbackFunction)(bool state);
CallbackFunction myCallback = NULL;

void registerCallbackSinricPro(CallbackFunction callback) {
  myCallback = callback;
}

bool onTrunkState(const String &deviceId, int &position) {
  LOG_PRINT.printf("Device %s set position to %d\r\n", deviceId.c_str(), position);

  if (myCallback != NULL && (position == 0 || position == 100)) {
    bool state = true;
    if (position == 100) {
      state = false;
    }
    timer2.setTimeout(50, [state]() {
      myCallback(state);
    });
  }

  return true;  // request handled properly
}

SinricProBlinds &myTrunk = SinricPro[Secrets.TRUNK_ID];
void setupSinricPro() {
  // &myTrunk = SinricPro[TRUNK_ID];
  myTrunk.onRangeValue(onTrunkState);

  // setup SinricPro
  SinricPro.onConnected([]() {
    LOG_PRINT.printf("Connected to SinricPro\r\n");
  });
  SinricPro.onDisconnected([]() {
    LOG_PRINT.printf("Disconnected from SinricPro\r\n");
  });
  SinricPro.begin(Secrets.APP_KEY, Secrets.APP_SECRET);
}