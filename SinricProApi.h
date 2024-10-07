#include "SinricPro.h"
#include "SinricProBlinds.h"

typedef void (*CallbackFunction)(bool state);
CallbackFunction myCallback = NULL;

void registerCallbackSinricPro(CallbackFunction callback) {
  myCallback = callback;
}

bool onTrunkState(const String &deviceId, int &position) {
  Serial.printf("Device %s set position to %d\r\n", deviceId.c_str(), position);

  if (myCallback != NULL && (position == 0 || position == 100)) {
    bool state = true;
    if (position == 100) {
      state = false;
    }
    timer2.setTimeout(0, [state]() {
      myCallback(state);
    });
  }

  return true;  // request handled properly
}

void setupSinricPro() {
  SinricProBlinds &myTrunk = SinricPro[TRUNK_ID];
  myTrunk.onRangeValue(onTrunkState);

  // setup SinricPro
  SinricPro.onConnected([]() {
    Serial.printf("Connected to SinricPro\r\n");
  });
  SinricPro.onDisconnected([]() {
    Serial.printf("Disconnected from SinricPro\r\n");
  });
  SinricPro.begin(APP_KEY, APP_SECRET);
}