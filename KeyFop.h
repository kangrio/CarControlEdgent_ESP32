#ifndef _KEY_FOP
#define _KEY_FOP

#define KEY_FOP_LOCK_BUTTON_PIN 32
#define KEY_FOP_UNLOCK_BUTTON_PIN 33
#define KEY_FOP_TRUNK_BUTTON_PIN 25
#define KEY_FOP_START_STOP_BUTTON_PIN 12
#define KEY_FOP_VCC_PIN 26
#define KEY_FOP_GND_PIN 27

#define PRESS_BUTTON_STATE LOW
#define RELEASE_BUTTON_STATE HIGH

#define DELAY_BEFORE_KEY_FOP_TURNEDON 300

#if DELAY_BEFORE_KEY_FOP_TURNEDON == 0
  #define KEY_FOP_ALWAYS_TURNED_ON true
#else
  #define KEY_FOP_ALWAYS_TURNED_ON false
#endif

class KeyFopClass {
public:
  uint8_t ButtonPressedState = LOW;
  uint8_t ButtonReleasedState = HIGH;

  void begin() {
    setupPinMode();
    // (KEY_FOP_ALWAYS_TURNED_ON) ? turnOnKeyFop() : turnOffKeyFop();
    if (KEY_FOP_ALWAYS_TURNED_ON) {
      turnOnKeyFop();
    } else {
      turnOffKeyFop();
    }
    releaseAllButton();
  }

  void turnOnKeyFop() {
    #if defined(DEBUG_MODE)
      return;
    #endif
    digitalWrite(KEY_FOP_VCC_PIN, HIGH);
    digitalWrite(KEY_FOP_GND_PIN, LOW);
  }

  void turnOffKeyFop() {
    digitalWrite(KEY_FOP_VCC_PIN, HIGH);
    digitalWrite(KEY_FOP_GND_PIN, HIGH);
  }

  void releaseAllButton() {
    digitalWrite(KEY_FOP_LOCK_BUTTON_PIN, HIGH);
    digitalWrite(KEY_FOP_UNLOCK_BUTTON_PIN, HIGH);
    digitalWrite(KEY_FOP_TRUNK_BUTTON_PIN, HIGH);
    digitalWrite(KEY_FOP_START_STOP_BUTTON_PIN, HIGH);
  }

  void simulateLock() {
    xTaskCreate(
      simulateLockTask,   // Function that should be called
      "simulateLockTask",     // Name of the task (for debugging)
      1024,               // Stack size (bytes)
      this,               // Parameter to pass
      1,                  // Task priority
      NULL                // Task handle
    );
  }

  void simulateUnlock() {
    xTaskCreate(
      simulateUnlockTask,   // Function that should be called
      "simulateUnlockTask",     // Name of the task (for debugging)
      1024,               // Stack size (bytes)
      this,               // Parameter to pass
      1,                  // Task priority
      NULL                // Task handle
    );
  }

  void simulateToggleTrunk() {
    xTaskCreate(
      simulateToggleTrunkTask,   // Function that should be called
      "simulateToggleTrunkTask",     // Name of the task (for debugging)
      1024,               // Stack size (bytes)
      this,               // Parameter to pass
      1,                  // Task priority
      NULL                // Task handle
    );
  }

  void simulateStartStop() {
    xTaskCreate(
      simulateStartStopTask,   // Function that should be called
      "simulateStartStopTask",     // Name of the task (for debugging)
      1024,               // Stack size (bytes)
      this,               // Parameter to pass
      1,                  // Task priority
      NULL                // Task handle
    );
  }

  uint8_t getLockButtonState() {
    return digitalRead(KEY_FOP_LOCK_BUTTON_PIN);
  }

  void pressLockButton() {
    digitalWrite(KEY_FOP_LOCK_BUTTON_PIN, LOW);
  }

  void releaseLockButton() {
    digitalWrite(KEY_FOP_LOCK_BUTTON_PIN, HIGH);
  }

  uint8_t getUnlockButtonState() {
    return digitalRead(KEY_FOP_UNLOCK_BUTTON_PIN);
  }

  void pressUnlockButton() {
    digitalWrite(KEY_FOP_UNLOCK_BUTTON_PIN, LOW);
  }

  void releaseUnlockButton() {
    digitalWrite(KEY_FOP_UNLOCK_BUTTON_PIN, HIGH);
  }

  uint8_t getTrunkButtonState() {
    return digitalRead(KEY_FOP_TRUNK_BUTTON_PIN);
  }

  void pressTrunkButton() {
    digitalWrite(KEY_FOP_TRUNK_BUTTON_PIN, LOW);
  }

  void releaseTrunkButton() {
    digitalWrite(KEY_FOP_TRUNK_BUTTON_PIN, HIGH);
  }

  uint8_t getStartStopButtonState() {
    return digitalRead(KEY_FOP_START_STOP_BUTTON_PIN);
  }

  void pressStartStopButton() {
    digitalWrite(KEY_FOP_START_STOP_BUTTON_PIN, LOW);
  }

  void releaseStartStopButton() {
    digitalWrite(KEY_FOP_START_STOP_BUTTON_PIN, HIGH);
  }

private:
  void setupPinMode() {
    pinMode(KEY_FOP_LOCK_BUTTON_PIN, OUTPUT);
    pinMode(KEY_FOP_UNLOCK_BUTTON_PIN, OUTPUT);
    pinMode(KEY_FOP_TRUNK_BUTTON_PIN, OUTPUT);
    pinMode(KEY_FOP_START_STOP_BUTTON_PIN, OUTPUT);
    pinMode(KEY_FOP_VCC_PIN, OUTPUT);
    pinMode(KEY_FOP_GND_PIN, OUTPUT);
  }
  
  static void simulateLockTask(void *pvParameters) {
    KeyFopClass *self = static_cast<KeyFopClass*>(pvParameters);

    self->turnOnKeyFop();
    vTaskDelay(DELAY_BEFORE_KEY_FOP_TURNEDON / portTICK_PERIOD_MS);
    self->pressLockButton();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    self->releaseLockButton();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (!KEY_FOP_ALWAYS_TURNED_ON) {
      self->turnOffKeyFop();
    }
    vTaskDelete(NULL);
  }
  
  static void simulateUnlockTask(void *pvParameters) {
    KeyFopClass *self = static_cast<KeyFopClass*>(pvParameters);

    self->turnOnKeyFop();
    vTaskDelay(DELAY_BEFORE_KEY_FOP_TURNEDON / portTICK_PERIOD_MS);
    self->pressUnlockButton();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    self->releaseUnlockButton();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (!KEY_FOP_ALWAYS_TURNED_ON) {
      self->turnOffKeyFop();
    }
    vTaskDelete(NULL);
  }
    
  static void simulateToggleTrunkTask(void *pvParameters) {
    KeyFopClass *self = static_cast<KeyFopClass*>(pvParameters);

    self->turnOnKeyFop();
    vTaskDelay(DELAY_BEFORE_KEY_FOP_TURNEDON / portTICK_PERIOD_MS);
    self->pressTrunkButton();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    self->releaseTrunkButton();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    self->pressTrunkButton();
    vTaskDelay(100 / portTICK_PERIOD_MS);
    self->releaseTrunkButton();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (!KEY_FOP_ALWAYS_TURNED_ON) {
      self->turnOffKeyFop();
    }
    vTaskDelete(NULL);
  }
      
  static void simulateStartStopTask(void *pvParameters) {
    KeyFopClass *self = static_cast<KeyFopClass*>(pvParameters);

    self->turnOnKeyFop();
    vTaskDelay(DELAY_BEFORE_KEY_FOP_TURNEDON / portTICK_PERIOD_MS);
    self->pressStartStopButton();
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    self->releaseStartStopButton();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (!KEY_FOP_ALWAYS_TURNED_ON) {
      self->turnOffKeyFop();
    }
    vTaskDelete(NULL);
  }
};

KeyFopClass KeyFop;

#endif