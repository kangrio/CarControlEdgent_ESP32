#include <ESP32-TWAI-CAN.hpp>



#ifndef CAN_TX
#define CAN_TX 22
#endif

#ifndef CAN_RX
#define CAN_RX 21
#endif

#ifndef MAXBUFFER
#define MAXBUFFER 4095
#endif

CanFrame rxFrame;


typedef struct {
  bool carVccTurnedOnState = false;
  bool carDoorLockedState = false;
  uint8_t carBatterySoc = 0;
} carState;

carState myCarState;

unsigned long buffer2unsignedLong(int startByte = 0, int bytesCount = 0, CanFrame *message = nullptr) {
  unsigned long result = 0;
  int maxBytes;
  byte g_dataBuffer[MAXBUFFER];
  long g_dataBufferLength = message->data[0] - 3;

  for (int i = 0; i < g_dataBufferLength; i++) {
    g_dataBuffer[i] = message->data[4 + i];
  }

  if (startByte >= g_dataBufferLength) {
    Serial.print("Start byte out of range ");
    Serial.print(startByte);
    Serial.print(">=");
    Serial.println(g_dataBufferLength);
    return 0;
  }
  if ((bytesCount > 0) && (startByte + bytesCount > g_dataBufferLength)) {
    Serial.print("Start byte and lenght out of range ");
    Serial.print(startByte);
    Serial.print("+");
    Serial.print(bytesCount);
    Serial.print(">");
    Serial.println(g_dataBufferLength);
    return 0;
  }

  if (bytesCount == 0) maxBytes = g_dataBufferLength;
  else maxBytes = bytesCount;
  for (int i = 0; i < maxBytes; i++) {
    result += (g_dataBuffer[i + startByte] << ((maxBytes - i - 1) * 8));
  }
  return result;
}

void setCarDoorState(uint8_t state) {
  if (state == 0xFC) {
    if (myCarState.carDoorLockedState == false) {
      myCarState.carDoorLockedState = true;
    }
  } else {
    if (myCarState.carDoorLockedState == true) {
      myCarState.carDoorLockedState = false;
    }
  }
}

void setCarVccTurnedOnState(uint8_t state) {
  if (state == 0x82) {
    if (myCarState.carVccTurnedOnState == false) {
      myCarState.carVccTurnedOnState = true;
    }
  } else {
    if (myCarState.carVccTurnedOnState == true) {
      myCarState.carVccTurnedOnState = false;
    }
  }
}

void setCarBatterySoc(CanFrame frame) {
  if (frame.data[3] != 0x05) {
    return;
  }

  // Serial.println(frame.data[4]);
  // uint8_t batterySoc = (buffer2unsignedLong(0, 1, &frame));
  myCarState.carBatterySoc = frame.data[4];
}

void Obd2Run() {
  twai_status_info_t status_info;
  twai_get_status_info(&status_info);

  // switch (status_info.state) {
  //   case TWAI_STATE_RUNNING:
  //     Serial.println("TWAI_STATE_RUNNING");
  //     break;
  //   case TWAI_STATE_STOPPED:
  //     Serial.println("TWAI_STATE_STOPPED");
  //     break;
  //   case TWAI_STATE_BUS_OFF:
  //     Serial.println("TWAI_STATE_BUS_OFF");
  //     break;
  //   case TWAI_STATE_RECOVERING:
  //     Serial.println("TWAI_STATE_RECOVERING");
  //     break;
  //   default:
  //     break;
  // }

  if (status_info.state != TWAI_STATE_RUNNING) {
    return;
  }

  if (ESP32Can.readFrame(rxFrame, 50)) {
    switch (rxFrame.identifier) {
      case 0x32C:
        // LOG_PRINT.print("Door Status=> ");
        // for (int i = 0; i < 8; i++) {
        //   LOG_PRINT.print(rxFrame.data[i]);
        //   LOG_PRINT.print(" ");
        // }
        // LOG_PRINT.println();
        setCarDoorState(rxFrame.data[0]);
        break;
      case 0x055:
        setCarVccTurnedOnState(rxFrame.data[4]);
        break;
      case 0x7EF:
        LOG_PRINT.print("Data Polling=> ");
        for (int i = 0; i < 8; i++) {
          LOG_PRINT.print(rxFrame.data[i], HEX);
          LOG_PRINT.print(" ");
        }
        LOG_PRINT.println();
        setCarBatterySoc(rxFrame);
        break;
      default:
        break;
    }
  }
}

int TwaiBegin() {
  ESP32Can.setPins(CAN_TX, CAN_RX);
  ESP32Can.setRxQueueSize(5);
  ESP32Can.setTxQueueSize(5);
  ESP32Can.setSpeed(ESP32Can.convertSpeed(500));

  if (ESP32Can.begin(ESP32Can.convertSpeed(500), CAN_TX, CAN_RX, 10, 10)) {
    Serial.println("CAN bus started!");
  } else {
    Serial.println("CAN bus failed!");
    return 0;
  }

  return 1;
}

int TwaiStop() {
  if (ESP32Can.end()) {
    Serial.println("CAN bus Ended!");
  } else {
    Serial.println("CAN bus End failed!");
    return 0;
  }

  return 1;
}

int readFrame() {
  // long remainingTimeoutMS;
  // remainingTimeoutMS = CAN_RECEIVE_TIMEOUT - (millis() - lastSentMS);
  // if (remainingTimeoutMS <= 0) {
  //   Serial.println("Timeout receiving message");
  //   return false;  // Abort
  // }
}

int obdCheckCarDoorLockState() {
  if (ESP32Can.readFrame(rxFrame, 100)) {
    Serial.println(rxFrame.data[2], HEX);
    if (rxFrame.identifier == 0x32C && rxFrame.data[2] == 0x0) {
      return 1;
    }
  }

  return 0;
}

void sendAndReceiveObdFrame(uint32_t txmoduleid, uint16_t rxmoduleid, uint8_t txData[], uint8_t txDataSize) {
  CanFrame obdFrame = { 0 };
  uint8_t dataSize = txDataSize;
  // Serial.println(dataSize);

  obdFrame.identifier = txmoduleid;  // OBD2 address;
  obdFrame.extd = 0;
  obdFrame.data_length_code = 8;
  obdFrame.data[0] = dataSize;
  obdFrame.data[1] = 0xAA;
  obdFrame.data[2] = 0xAA;
  obdFrame.data[3] = 0xAA;
  obdFrame.data[4] = 0xAA;
  obdFrame.data[5] = 0xAA;
  obdFrame.data[6] = 0xAA;
  obdFrame.data[7] = 0xAA;

  for (int i = 0; i < dataSize; i++) {
    obdFrame.data[i + 1] = txData[i];
  }

  // Accepts both pointers and references
  ESP32Can.writeFrame(obdFrame);  // timeout defaults to 1 ms
}


void sendDefaultObdFrame(uint8_t obdId) {
  uint8_t data[] = { 0x01, obdId };
  sendAndReceiveObdFrame(0x7DF, 0x7E8, data, sizeof(data) / sizeof(data[0]));
}

void sendObdFrame(uint8_t obdId) {
  uint8_t data[] = { 0x22, 0x00, obdId };
  sendAndReceiveObdFrame(0x7E7, 0x7EF, data, sizeof(data) / sizeof(data[0]));
}