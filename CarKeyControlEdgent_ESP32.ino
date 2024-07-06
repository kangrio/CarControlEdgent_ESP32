// ESP32

/* Uncomment below for use in production */
// #define DEBUG_MODE

/* Uncomment for use Serial */
// #define USE_REMOTE_SERIAL

#define BLYNK_TEMPLATE_ID "TMPL6BR474jmt"
#define BLYNK_TEMPLATE_NAME "Engine"

/* use for test device */
// #define BLYNK_TEMPLATE_ID "TMPL6p6MOW2KD"
// #define BLYNK_TEMPLATE_NAME "EngineCopy"

#define BLYNK_FIRMWARE_VERSION "1.0.13"

#define BLYNK_PRINT Serial

#ifdef USE_REMOTE_SERIAL
#define LOG_PRINT RemoteSerial
#else
#define LOG_PRINT Serial
#endif

// #define LOG_PRINT RemoteSerial
//#define BLYNK_DEBUG


#define APP_DEBUG

// #define LED_BUILTIN 2

#define VCC_STATE_PIN 36

#define CLOSE_DOOR_PIN 32
#define OPEN_DOOR_PIN 33
#define TOGGLE_TRUNK_PIN 25
#define VCC_BUTTON 12
#define POWER_PIN 27
#define GND_PIN 26

// Uncomment your board, or configure a custom board in Settings.h
#define USE_ESP32_DEV_MODULE

// #include <ESPAsyncWebServer.h>
// #include <RemoteSerial.h>
#include <ArduinoOTA.h>
#include <ezTime.h>
#include "BlynkEdgent.h"
#include "Secrets.h"


long sleepTimeForTurnOnKey = 300L;

BlynkTimer timer;
BlynkTimer timer2;
Timezone local;

int sleepTime = 5 * 60 * 1000;

bool isVccOn = false;

bool isLockButtonPressed = false;
bool isUnLockButtonPressed = false;

bool isCarVccTurnedOn = false;
bool isCarDoorLocked = false;


void setupArduinoOTA() {
  ArduinoOTA.setPasswordHash(passWordHashed);
  ArduinoOTA.onStart([]() {
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
  digitalWrite(POWER_PIN, LOW);
}

void POWER_OFF_REMOTE() {
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


  timer.setTimeout(50L, pressTrunkButton);
  timer.setTimeout(100L, releaseTrunkButton);
  timer.setTimeout(150L, pressTrunkButton);
  timer.setTimeout(200L, releaseTrunkButton);
}

void printText(String str) {
  LOG_PRINT.println(str);
}

void onBoardLedOn() {
  ledcWrite(BOARD_LEDC_CHANNEL_1, TO_PWM(255));
  // digitalWrite(LED_BUILTIN, HIGH);
}

void onBoardLedOff() {
  ledcWrite(BOARD_LEDC_CHANNEL_1, TO_PWM(0));
  // digitalWrite(LED_BUILTIN, LOW);
}

// ACC Button
BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  LOG_PRINT.println(pinValue);


  if (pinValue > 0) {
    BlynkState::set(MODE_KEYPRESS);
    onBoardLedOn();
    POWER_ON_REMOTE();
    timer.setTimeout(sleepTimeForTurnOnKey, VCC_STATE);
  } else {
    sendObd0100();
    sendObdFrame(0x05);
    BlynkState::set(MODE_RUNNING);
    onBoardLedOff();
    timer.disableAll();
    RESET_ALL_KEY();
    POWER_OFF_REMOTE();

    setCarStatus();
  }
}

long startPressedTime = 0;

// Lock BUtton
BLYNK_WRITE(V2) {
  int pinValue = param.asInt();
  LOG_PRINT.println("Lock BUtton: " + String(pinValue));

  if (pinValue > 0) {
    sendObd0100();
    startPressedTime = millis();
    BlynkState::set(MODE_KEYPRESS);
    isLockButtonPressed = true;
    onBoardLedOn();
    POWER_ON_REMOTE();
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
    // timer.setTimeout(200, RESET_ALL_KEY);
  }
}

// Unlock BUtton
BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  LOG_PRINT.println("Unlock BUtton: " + String(pinValue));

  if (pinValue > 0) {
    sendObd0100();
    startPressedTime = millis();
    BlynkState::set(MODE_KEYPRESS);
    isUnLockButtonPressed = true;
    onBoardLedOn();
    POWER_ON_REMOTE();
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

// Trunk Button
BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  LOG_PRINT.println("Trunk Button: " + String(pinValue));

  if (pinValue > 0) {
    BlynkState::set(MODE_KEYPRESS);
    onBoardLedOn();
    POWER_ON_REMOTE();
    timer.setTimeout(sleepTimeForTurnOnKey, TOGGLE_TRUNK);
  } else {
    BlynkState::set(MODE_RUNNING);
    onBoardLedOff();
    timer.disableAll();
    RESET_ALL_KEY();
    POWER_OFF_REMOTE();
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

void notifyDeviceOnline() {
  LOG_PRINT.println("Sending notifyDeviceOnline");

  Blynk.logEvent("device_online", local.dateTime());
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

BLYNK_CONNECTED() {
  Blynk.sendInternal("utc", "time");     // Unix timestamp (with msecs)
  Blynk.sendInternal("utc", "tz_rule");  // POSIX TZ rule
  Blynk.syncAll();

  // startRemoteSerialMonitor();


  setupArduinoOTA();

  timer2.setInterval(1000L, checkCarStatus);

  // timer2.setInterval(10000L, requestObdStaySendDoorLockData);

  timer2.setTimeout(200L, notifyDeviceOnline);
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
  if (hour == 3 && (minute >= 0 && minute <= 8)) {
    LOG_PRINT.println("Rebooting...");
    timer2.setTimeout(2000L, resetMCU);
  }
}

void requestObdStaySendDoorLockData() {
  // if (myCarState.carDoorLockedState) {
  // LOG_PRINT.println("requestObdStaySendDoorLockData()");
  sendObd0100();
  // }
}

void checkCarStatus() {
  checkCarVccTurnedOnState();
  checkCarDoorLockState();
  checkCarBatterySoc();
}

uint8_t carBatterySoc = 0;
void checkCarBatterySoc() {
  if (myCarState.carBatterySoc != carBatterySoc) {
    setCarStatus();
  }
}

void checkCarDoorLockState() {
  if (myCarState.carVccTurnedOnState) {
    setCarStatus();
    return;
  }


  if (myCarState.carDoorLockedState == true) {
    if (isCarDoorLocked == false) {
      isCarDoorLocked = true;
      notifyCarDoorLocked();
      LOG_PRINT.println("Car Door is Locked");
    }
  } else {
    if (isCarDoorLocked == true) {
      isCarDoorLocked = false;
      notifyCarDoorUnLocked();
      setCarStatus();
      LOG_PRINT.println("Car Door is UnLocked");
    }
  }
}



void checkCarVccTurnedOnState() {
  // LOG_PRINT.println(myCarState.carVccTurnedOnState);
  if (myCarState.carVccTurnedOnState == true) {
    if (isCarVccTurnedOn == false) {
      isCarVccTurnedOn = true;
      setCarStatus();
      notifyCarAccStarted();
      LOG_PRINT.println("Car is Started");
    }
  } else {
    if (isCarVccTurnedOn == true) {
      isCarVccTurnedOn = false;
      setCarStatus();
      notifyCarAccStopped();
      LOG_PRINT.println("Car is Stopped");
    }
  }
}

void setCarStatus() {
  if (isCarVccTurnedOn == myCarState.carVccTurnedOnState && isCarDoorLocked == myCarState.carDoorLockedState && carBatterySoc == myCarState.carBatterySoc) return;

  String textVccState = (isCarVccTurnedOn) ? "✅" : "🚫";
  String textDoorState = (isCarVccTurnedOn) ? "Err" : (isCarDoorLocked) ? "✅"
                                                                        : "🚫";
  String textState = String("Started: ") + textVccState + String(", Locked: ") + textDoorState + ", Soc: " + String(myCarState.carBatterySoc) + "%";
  Blynk.virtualWrite(V1, textState);
}

void readVoltPin() {
  LOG_PRINT.println(analogRead(VCC_STATE_PIN));
}

void sendObd0100() {
  sendDefaultObdFrame(0x0);
}

void setup() {
  Serial.begin(9600);

  // pinMode(VCC_STATE_PIN, INPUT_PULLDOWN);

  // pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CLOSE_DOOR_PIN, OUTPUT);
  pinMode(OPEN_DOOR_PIN, OUTPUT);
  pinMode(TOGGLE_TRUNK_PIN, OUTPUT);
  pinMode(VCC_BUTTON, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  pinMode(GND_PIN, OUTPUT);

  // digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(CLOSE_DOOR_PIN, HIGH);
  digitalWrite(OPEN_DOOR_PIN, HIGH);
  digitalWrite(TOGGLE_TRUNK_PIN, HIGH);
  digitalWrite(VCC_BUTTON, HIGH);
  digitalWrite(POWER_PIN, HIGH);
  digitalWrite(GND_PIN, HIGH);


  delay(1000);

  TwaiBegin();

  BlynkEdgent.begin();
  timer2.setInterval(sleepTime, restartEverydayAt3AM);
  // timer2.setInterval(1000, readVoltPin);
  // timer2.setInterval(1000, testObd);
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
  ArduinoOTA.handle();
  BlynkEdgent.run();
  timer.run();
  timer2.run();
  Obd2Run();

  // serialReading();
}
