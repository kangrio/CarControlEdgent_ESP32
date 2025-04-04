// ESP32

/* Blynk 1.3.2 */
/* ArduinoIDE 2.3.2 */

/* 
## Library
- [**Blynk** : 1.3.2](https://github.com/blynkkk/blynk-library)
- [**RemoteSerial** : 0.0.1](https://github.com/kangrio/RemoteSerial)
- [**ESP32-TWAI-CAN** : 1.0](https://github.com/handmade0octopus/ESP32-TWAI-CAN)
- [**AntiDelay version** : 1.1.3](https://github.com/martinvichnal/AntiDelay)
- [**pubsubclient** : 2.8](https://github.com/knolleary/pubsubclient)
- [**sinricpro** : 3.3.1](https://github.com/sinricpro/esp8266-esp32-sdk)
*/

/* Uncomment below for use in production */
// #define DEBUG_MODE

/* Uncomment for use Serial */
#define USE_REMOTE_SERIAL

#define DEBUG_OBD

// Uncomment your board, or configure a custom board in Settings.h
#define USE_ESP32_DEV_MODULE
//#define USE_ESP32C3_DEV_MODULE
//#define USE_ESP32S2_DEV_KIT
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7
//#define USE_TTGO_T_OI

#define BLYNK_TEMPLATE_ID "TMPL6BR474jmt"
#define BLYNK_TEMPLATE_NAME "Engine"

/* use for test device */
// #define BLYNK_TEMPLATE_ID "TMPL6p6MOW2KD"
// #define BLYNK_TEMPLATE_NAME "EngineCopy"

#define BLYNK_FIRMWARE_VERSION "1.1.0"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG


#ifdef USE_REMOTE_SERIAL
#define LOG_PRINT RemoteSerial
#else
#define LOG_PRINT Serial
#endif



#define APP_DEBUG

// #define LED_BUILTIN 2

#if defined(USE_ESP32_DEV_MODULE)

#define CLOSE_DOOR_PIN 32
#define OPEN_DOOR_PIN 33
#define TOGGLE_TRUNK_PIN 25
#define VCC_BUTTON 12
#define POWER_PIN 27
#define GND_PIN 26

#define CAN_TX 22
#define CAN_RX 21

#elif defined(USE_ESP32C3_DEV_MODULE)

#define CLOSE_DOOR_PIN 21
#define OPEN_DOOR_PIN 20
#define TOGGLE_TRUNK_PIN 10
#define VCC_BUTTON 5
#define POWER_PIN 6
#define GND_PIN 7

#define CAN_TX 0
#define CAN_RX 1

#endif

#include <ArduinoOTA.h>
#include <ezTime.h>
#include "BlynkEdgent.h"
#include <PubSubClient.h>
#include "MqttClient.h"

String client_id = "esp32-client-";

bool isSetupComplete = false;

Timezone local;

int sleepTimeForCheckingReboot = 5 * 60 * 1000;
uint32_t offlineTimestamp = 0;

/* Car Status infomation */
bool isCarVccTurnedOn = false;
bool isCarDoorLocked = false;
bool isCarTrunkClosed = false;
bool isCarCharging = false;
uint8_t carBatterySoc = 0;
uint32_t carOdoMeter = 0;
uint16_t carRangeLeft = 0;




/* Key press Var */
long sleepTimeForTurnOnKey = 300L;

long startPressedTime = 0;
bool isLockButtonPressed = false;
bool isUnLockButtonPressed = false;

bool isRemotePowerOn = false;


/* TimerId */
int vccButtonTimerId, lockButtonTimerId, unlockButtonTimerId, trunkButtonTimerId = BlynkTimer::MAX_TIMERS;

void RESET_ALL_KEY() {
  digitalWrite(CLOSE_DOOR_PIN, HIGH);
  digitalWrite(OPEN_DOOR_PIN, HIGH);
  digitalWrite(TOGGLE_TRUNK_PIN, HIGH);
  digitalWrite(VCC_BUTTON, HIGH);
}

void POWER_ON_REMOTE() {
#if defined(DEBUG_MODE)
  return;
#endif
  isRemotePowerOn = true;
  digitalWrite(POWER_PIN, LOW);
}

void POWER_OFF_REMOTE() {
  isRemotePowerOn = false;
  digitalWrite(POWER_PIN, HIGH);
}

void VCC_STATE() {
  digitalWrite(VCC_BUTTON, LOW);

  if (isCarVccTurnedOn) {
    Blynk.virtualWrite(V1, "Stopping...");
    LOG_PRINT.println("Stopping...");

  } else {
    Blynk.virtualWrite(V1, "Starting...");
    LOG_PRINT.println("Starting...");
  }
}


void CLOSE_DOOR() {
  LOG_PRINT.println("Door close");

  digitalWrite(CLOSE_DOOR_PIN, LOW);
}

void OPEN_DOOR() {
  LOG_PRINT.println("Door open");

  digitalWrite(OPEN_DOOR_PIN, LOW);
}

void pressTrunkButton() {
  digitalWrite(TOGGLE_TRUNK_PIN, LOW);
}

void releaseTrunkButton() {
  digitalWrite(TOGGLE_TRUNK_PIN, HIGH);
}

void TOGGLE_TRUNK() {
  LOG_PRINT.println("Toggle Trunk");

  pressTrunkButton();
  timer.setTimeout(100L, []() {
    releaseTrunkButton();
    timer.setTimeout(100L, []() {
      pressTrunkButton();
      timer.setTimeout(100L, []() {
        releaseTrunkButton();
      });
    });
  });
}


void TOGGLE_TRUNK_FROM_SINRIC() {
  LOG_PRINT.println("Toggle Trunk From Sinric");

  BlynkState::set(MODE_KEYPRESS);
  onBoardLedOn();
  POWER_ON_REMOTE();

  timer.setTimeout(sleepTimeForTurnOnKey, []() {
    pressTrunkButton();
    timer.setTimeout(100L, []() {
      releaseTrunkButton();
      timer.setTimeout(100L, []() {
        pressTrunkButton();
        timer.setTimeout(100L, []() {
          releaseTrunkButton();
          timer.setTimeout(100L, []() {
            releaseTrunkButton();
            timer.setTimeout(100, []() {
              BlynkState::set(MODE_RUNNING);
              onBoardLedOff();
              RESET_ALL_KEY();
              POWER_OFF_REMOTE();
            });
          });
        });
      });
    });
  });
}

void printText(String str) {
  LOG_PRINT.println(str);
}

void onBoardLedOn() {
  ledcWrite(BOARD_LEDC_CHANNEL_1, TO_PWM(255));
}

void onBoardLedOff() {
  ledcWrite(BOARD_LEDC_CHANNEL_1, TO_PWM(0));
}

void deleteTimer(int *timerId) {
  timer.deleteTimer(*timerId);
  *timerId = BlynkTimer::MAX_TIMERS;
  LOG_PRINT.println(timer.getNumTimers());
}

void vccButtonAction(int state) {
  int pinValue = state % 2;

  if (pinValue > 0) {
    BlynkState::set(MODE_KEYPRESS);
    onBoardLedOn();
    POWER_ON_REMOTE();
    RESET_ALL_KEY();
    vccButtonTimerId = timer.setTimeout(sleepTimeForTurnOnKey, VCC_STATE);
  } else {
    sendObd0100();
    ObdDevice.sendObdFrame(0x05);
    BlynkState::set(MODE_RUNNING);
    onBoardLedOff();
    deleteTimer(&vccButtonTimerId);
    RESET_ALL_KEY();
    POWER_OFF_REMOTE();
    updateCarStatus();
  }
}

void lockButtonAction(int state) {
  int pinValue = state % 2;

  if (pinValue > 0) {
    startPressedTime = millis();
    BlynkState::set(MODE_KEYPRESS);
    isLockButtonPressed = true;
    onBoardLedOn();
    POWER_ON_REMOTE();
    RESET_ALL_KEY();
    timer.setTimeout(sleepTimeForTurnOnKey, CLOSE_DOOR);
  } else {
    onBoardLedOff();
    if (millis() - startPressedTime > sleepTimeForTurnOnKey) {
      LOG_PRINT.println("released rapid Lock BUtton");
      RESET_ALL_KEY();
      timer.setTimeout(500, POWER_OFF_REMOTE);
    } else {
      LOG_PRINT.println("released Lock BUtton");
      timer.setTimeout(sleepTimeForTurnOnKey + 50, RESET_ALL_KEY);
      timer.setTimeout(sleepTimeForTurnOnKey + 300, POWER_OFF_REMOTE);
    }
    BlynkState::set(MODE_RUNNING);
    isLockButtonPressed = false;
  }
}

void unlockButtonAction(int state) {
  int pinValue = state % 2;

  if (pinValue > 0) {
    startPressedTime = millis();
    BlynkState::set(MODE_KEYPRESS);
    isUnLockButtonPressed = true;
    onBoardLedOn();
    POWER_ON_REMOTE();
    RESET_ALL_KEY();
    timer.setTimeout(sleepTimeForTurnOnKey, OPEN_DOOR);
  } else {
    onBoardLedOff();
    if (millis() - startPressedTime > sleepTimeForTurnOnKey) {
      LOG_PRINT.println("released rapid unLock BUtton");
      RESET_ALL_KEY();
      timer.setTimeout(500, POWER_OFF_REMOTE);
    } else {
      LOG_PRINT.println("released unLock BUtton");
      timer.setTimeout(sleepTimeForTurnOnKey + 50, RESET_ALL_KEY);
      timer.setTimeout(sleepTimeForTurnOnKey + 300, POWER_OFF_REMOTE);
    }
    BlynkState::set(MODE_RUNNING);
    isUnLockButtonPressed = false;
  }
}

void trunkButtonAction(int state) {
  int pinValue = state % 2;

  if (pinValue > 0) {
    BlynkState::set(MODE_KEYPRESS);
    onBoardLedOn();
    POWER_ON_REMOTE();
    RESET_ALL_KEY();
    trunkButtonTimerId = timer.setTimeout(sleepTimeForTurnOnKey, TOGGLE_TRUNK);
  } else {
    BlynkState::set(MODE_RUNNING);
    onBoardLedOff();
    deleteTimer(&trunkButtonTimerId);
    RESET_ALL_KEY();
    POWER_OFF_REMOTE();
  }
}

void powerButtonAction(int state) {
  int pinValue = state % 2;

  if (pinValue > 0) {
    POWER_ON_REMOTE();
  } else {
    onBoardLedOff();
  }
}

BLYNK_WRITE(V4) {
  int pinValue = param.asInt();

  switch(pinValue) {
    case 0:
    case 1:
        vccButtonAction(pinValue);
      break;
    case 2:
    case 3:
        lockButtonAction(pinValue);
      break;
    case 4:
    case 5:
        unlockButtonAction(pinValue);
      break;
    case 6:
    case 7:
        trunkButtonAction(pinValue);
      break;
    case 8:
    case 9:
        powerButtonAction(pinValue);
      break;
    default:
      break;
  }
}

void sinricProCallback(bool state) {
  if ((isCarTrunkClosed == true && state == false) || (isCarTrunkClosed == false && state == true)) {
    TOGGLE_TRUNK_FROM_SINRIC();
  }
}

void resetMCU() {
#if defined(ARDUINO_ARCH_MEGAAVR)
  wdt_enable(WDT_PERIOD_8CLK_gc);
#elif defined(__AVR__)
  wdt_enable(WDTO_15MS);
#elif defined(__arm__)
  NVIC_SystemReset();
#elif defined(ESP8266) || defined(ESP32)
  ESP.restart();
#else
#error "MCU reset procedure not implemented"
#endif
  for (;;) {}
}

void notifyDeviceStatus(String status) {
  LOG_PRINT.println("Sending notifyDeviceStatus");
  Blynk.logEvent("device_status", status);
}

void notifyCarAccStarted() {
  LOG_PRINT.println("Sending notifyCarAccStarted");
  Blynk.logEvent("atto3_started");
}

void notifyCarAccStopped() {
  LOG_PRINT.println("Sending notifyCarAccStopped");
  Blynk.logEvent("atto3_stopped");
}

void notifyCarDoorLocked() {
  LOG_PRINT.println("Sending notifyCarDoorLocked");
  Blynk.logEvent("atto3_door_locked");
}

void notifyCarDoorUnLocked() {
  LOG_PRINT.println("Sending notifyCarDoorUnLocked");
  Blynk.logEvent("atto3_door_unlocked");
}

void notifyCarTrunkClosed() {
  LOG_PRINT.println("Sending notifyCarTrunkClosed");
  Blynk.logEvent("device_status", "Trunk Closed");
}

void notifyCarTrunkOpenned() {
  LOG_PRINT.println("Sending notifyCarTrunkOpenned");
  Blynk.logEvent("device_status", "Trunk Opened");
}

BLYNK_DISCONNECTED() {
  POWER_OFF_REMOTE();
  RESET_ALL_KEY();
  offlineTimestamp = millis();
}

BLYNK_CONNECTED() {
  Blynk.sendInternal("utc", "time");     // Unix timestamp (with msecs)
  Blynk.sendInternal("utc", "tz_rule");  // POSIX TZ rule
  Blynk.syncAll();


  if (!isSetupComplete) {
    // registerCallbackSinricPro(sinricProCallback);
    // setupSinricPro();
    setupArduinoOTA();

    timer.setTimeout(1000L, []() {
      sendObd0100();
      ObdDevice.sendObdFrame(0x05);

      timer.setTimeout(2000L, []() {
        isCarVccTurnedOn = ObdDevice.myCarState.carVccTurnedOnState;
        isCarDoorLocked = ObdDevice.myCarState.carDoorLockedState;
        isCarTrunkClosed = ObdDevice.myCarState.carTrunkClosedState;
        carBatterySoc = ObdDevice.myCarState.carBatterySoc;

        updateCarStatus();
        timer.setInterval(1000L, checkCarStatus);
      });
    });
    isSetupComplete = true;
    // startRemoteSerialMonitor();
  }
}

BLYNK_WRITE(InternalPinUTC) {
  String cmd = param[0].asStr();
  if (cmd == "time") {
    const uint64_t utc_time = param[1].asLongLong();
    UTC.setTime(utc_time / 1000, utc_time % 1000);
    LOG_PRINT.print("Unix time (UTC): ");
    LOG_PRINT.println(utc_time);
  } else if (cmd == "tz_rule") {
    String tz_rule = param[1].asStr();
    local.setPosix(tz_rule);
    LOG_PRINT.print("POSIX TZ rule:   ");
    LOG_PRINT.println(tz_rule);
  }
}

void printClock() {
  LOG_PRINT.println(local.minute());

  LOG_PRINT.println("Time: " + local.dateTime());
}

void restartEverydayAt3AM() {
  LOG_PRINT.println("Checking Reboot Time");
  int hour = local.hour();
  int minute = local.minute();
  LOG_PRINT.print(hour);
  LOG_PRINT.print(":");
  LOG_PRINT.println(minute);
  if (hour == 3 && (minute >= 0 && minute <= 5)) {
    LOG_PRINT.println("Rebooting...");
    timer.setTimeout(2000L, resetMCU);
  }
}

void requestObdStaySendDoorLockData() {
  // if (ObdDevice.myCarState.carDoorLockedState) {
  // LOG_PRINT.println("requestObdStaySendDoorLockData()");
  sendObd0100();
  // }
}

void checkCarStatus() {
  checkCarVccTurnedOnState();
  checkCarDoorLockState();
  checkCarTrunkClosedState();
  checkCarBatterySoc();
  checkCarBatteryStatus();
  checkCarOdoMeterValue();
  checkCarRangeLeftValue();
}

void checkCarBatterySoc() {
  if (ObdDevice.myCarState.carBatterySoc != carBatterySoc) {
    carBatterySoc = ObdDevice.myCarState.carBatterySoc;
    updateCarStatus();
  }
}

void checkCarDoorLockState() {
  if (isCarDoorLocked != ObdDevice.myCarState.carDoorLockedState) {
    isCarDoorLocked = ObdDevice.myCarState.carDoorLockedState;
    if (isCarDoorLocked) {
      notifyCarDoorLocked();
      updateCarStatus();
      LOG_PRINT.println("Car Door is Locked");
    } else {
      notifyCarDoorUnLocked();
      updateCarStatus();
      LOG_PRINT.println("Car Door is UnLocked");
    }
  }
}

void checkCarTrunkClosedState() {
  if (isCarTrunkClosed != ObdDevice.myCarState.carTrunkClosedState) {
    isCarTrunkClosed = ObdDevice.myCarState.carTrunkClosedState;
    if (isCarTrunkClosed) {
      notifyCarTrunkClosed();
      MqttClient.updateTrunkState("close");
      updateCarStatus();
      myTrunk.sendRangeValueEvent(0, "Normal");
      LOG_PRINT.println("Car Trunk is Closed");
    } else {
      notifyCarTrunkOpenned();
      MqttClient.updateTrunkState("open");
      updateCarStatus();
      myTrunk.sendRangeValueEvent(100, "Normal");
      LOG_PRINT.println("Car Trunk is Openned");
    }
  }
}



void checkCarVccTurnedOnState() {
  if (isCarVccTurnedOn != ObdDevice.myCarState.carVccTurnedOnState) {
    isCarVccTurnedOn = ObdDevice.myCarState.carVccTurnedOnState;
    if (isCarVccTurnedOn) {
      updateCarStatus();
      notifyCarAccStarted();
      LOG_PRINT.println("Car is Started");
    } else {
      updateCarStatus();
      notifyCarAccStopped();
      LOG_PRINT.println("Car is Stopped");
    }
  }
}

void checkCarBatteryStatus() {
  if (isCarCharging != ObdDevice.myCarState.carChargingState) {
    isCarCharging = ObdDevice.myCarState.carChargingState;
    if (isCarCharging) {
      updateCarStatus();
      notifyDeviceStatus("Charging Started");
      LOG_PRINT.println("Charging Started");
    } else {
      updateCarStatus();
      notifyDeviceStatus("Charging Stoped");
      LOG_PRINT.println("Charging Stoped");
    }
  }
}

void checkCarOdoMeterValue() {
  if (carOdoMeter != ObdDevice.myCarState.carOdoMeter) {
    carOdoMeter = ObdDevice.myCarState.carOdoMeter;
    updateCarStatus();
  }
}

void checkCarRangeLeftValue() {
  if (carRangeLeft != ObdDevice.myCarState.carRangeLeft) {
    carRangeLeft = ObdDevice.myCarState.carRangeLeft;
    updateCarStatus();
  }
}

void updateCarStatus() {
  if (ObdDevice.myCarState.carChargingState) return;
  // if (isCarVccTurnedOn == ObdDevice.myCarState.carVccTurnedOnState && isCarDoorLocked == ObdDevice.myCarState.carDoorLockedState && carBatterySoc == ObdDevice.myCarState.carBatterySoc) return;

  String textVccState = (!isCarVccTurnedOn) ? "✅" : "🚫";
  String textDoorState = (isCarDoorLocked) ? "✅" : "🚫";
  String textTrunkState = (isCarTrunkClosed) ? "✅" : "🚫";
  String textState = String("Stopped: ") + textVccState + String(", Locked: ") + textDoorState + ", Trunk: " + textTrunkState + ", Soc: " + String(carBatterySoc) + "%";

  String textCarDetail = "ODO: " + String(carOdoMeter) + " Km" + ", Range: " + String(carRangeLeft) + " Km";

  Blynk.setProperty(V1, "label", textCarDetail);
  Blynk.virtualWrite(V1, textState);
}

void sendObd0100() {
  ObdDevice.sendDefaultObdFrame(0x0);
}

void setup() {
  Serial.begin(9600);

  pinMode(CLOSE_DOOR_PIN, OUTPUT);
  pinMode(OPEN_DOOR_PIN, OUTPUT);
  pinMode(TOGGLE_TRUNK_PIN, OUTPUT);
  pinMode(VCC_BUTTON, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(GND_PIN, OUTPUT);

  digitalWrite(CLOSE_DOOR_PIN, HIGH);
  digitalWrite(OPEN_DOOR_PIN, HIGH);
  digitalWrite(TOGGLE_TRUNK_PIN, HIGH);
  digitalWrite(VCC_BUTTON, HIGH);
  digitalWrite(POWER_PIN, HIGH);
  digitalWrite(GND_PIN, HIGH);


  delay(1000);

  ObdDevice.begin();
  client_id += String(WiFi.macAddress());
  MqttClient.begin(client_id);

  BlynkEdgent.begin();
  timer.setInterval(sleepTimeForCheckingReboot, restartEverydayAt3AM);
}

void serialReading() {
  if (Serial.available()) {
    String str = Serial.readString();
    Serial.println(str);
    if (str.equalsIgnoreCase("reboot")) {
      restartMCU();
    }
  }
}

void loop() {
  BlynkEdgent.run();
  ObdDevice.run();
  ArduinoOTA.handle();
  MqttClient.mqttHandle();
  // SinricPro.handle();
  // serialReading();
}
