#ifndef _OBD_DEVICE
#define _OBD_DEVICE

#include "driver/twai.h"
#include "esp32-hal.h"
#include <ESP32-TWAI-CAN.hpp>
#include "AntiDelay.h"

#ifndef CAN_TX
#define CAN_TX 22
#endif

#ifndef CAN_RX
#define CAN_RX 21
#endif

class ObdDeviceClass {
private:
  AntiDelay updateChargingDetailDelay;
  AntiDelay updateChargingColorDelay;
  AntiDelay printFrameDelay;

  CanFrame rxFrame;

  uint32_t doorId = 0x407;
  uint32_t engineId = 0x055;
  uint32_t chargingId = 0x323;
  uint32_t odoMeterId = 0x3D9;

  float timeCounter = 0.0;

  String Obd_Fram = "";
public:
  typedef struct {
    bool carVccTurnedOnState = false;
    bool carDoorLockedState = false;
    bool carTrunkClosedState = false;
    bool carChargingState = false;
    uint32_t carOdoMeter = 0;
    uint16_t carRangeLeft = 0;
    uint8_t carBatterySoc = 0;
    uint8_t car12VBatteryVoltage = 0;
  } carState;

  carState myCarState;

  ObdDeviceClass()
    : updateChargingDetailDelay(1000),
      updateChargingColorDelay(100),
      printFrameDelay(500) {
  }

  void printFrame(CanFrame *message) {
    String data = String(millis()) + ": ";
    String lenghtTAG = (message->extd) ? " X " : " S ";
    data = data + "0x" + String(message->identifier, HEX) + lenghtTAG + String(message->data_length_code) + " ";
    for (int i = 0; i < message->data_length_code; i++) {
      String bitData = String(message->data[i], HEX);
      bitData.toUpperCase();
      data = data + bitData + " ";
    }

    Obd_Fram = Obd_Fram + data + "\n";
    if (printFrameDelay) {
      LOG_PRINT.print(Obd_Fram);
      Obd_Fram = "";
    }
  }

  String getFadeCustomGreenColor(float factor) {
    // Dark green color (#0F624A)
    int redMin = 15;
    int greenMin = 98;
    int blueMin = 74;

    // Bright green color (#24C48E)
    int redMax = 36;
    int greenMax = 196;
    int blueMax = 142;

    // Calculate the interpolation factor (from 0 to 1 based on the sine wave)
    float interpolation = (sin(factor) + 1.0) / 2.0;  // Map sine wave (-1 to 1) to (0 to 1)

    // Calculate RGB values by interpolation
    int red = redMin + (redMax - redMin) * interpolation;
    int green = greenMin + (greenMax - greenMin) * interpolation;
    int blue = blueMin + (blueMax - blueMin) * interpolation;

    // Convert RGB to a hex color string
    char hexColor[7];
    sprintf(hexColor, "#%02X%02X%02X", red, green, blue);

    return String(hexColor);
  }

  void resetWidgetColor() {
    Blynk.setProperty(V0, V1, V2, V3, V4, "color", "#24c48e");
  }

  void updateChargingDetail(CanFrame frame) {
    if (!updateChargingDetailDelay) {
      return;
    }
    uint8_t d[8];

    for (int i = 0; i < 8; i++) {
      d[i] = frame.data[i];
    }

    uint8_t charge_remaining_hours = d[4];
    if (charge_remaining_hours == 0xff) {
      charge_remaining_hours = 0;
    }

    uint8_t charge_remaining_mins = d[5];
    if (charge_remaining_mins == 0xff) {
      charge_remaining_mins = 0;
    }

    uint8_t charge_soc = d[1];

    char charge_remaining[6];
    sprintf(charge_remaining, "%02d:%02d", charge_remaining_hours, charge_remaining_mins);

    int16_t lsb = d[2];
    int16_t msb = (d[3] & 0x1f) * 256;

    bool ac_charging = (d[3] & 0x20) > 0;

    myCarState.carBatterySoc = charge_soc;

    String textVccState = (!myCarState.carVccTurnedOnState) ? "✅" : "🚫";
    String textDoorState = (myCarState.carDoorLockedState) ? "✅" : "🚫";
    String textTrunkState = (myCarState.carTrunkClosedState) ? "✅" : "🚫";

    String chargingSoc = String(charge_soc);
    String chargingPower = String((msb + lsb - 500) / 10.0);
    String chargingTime = String(charge_remaining);
    String chargingMode = ac_charging ? "AC" : "DC";

    String textState = String("Stopped: ") + textVccState + String(", Locked: ") + textDoorState + ", Trunk: " + textTrunkState + ", Soc: " + String(myCarState.carBatterySoc) + "%";
    String chargingDetail = "Soc: " + chargingSoc + "%" + ", Left: " + chargingTime + ", Mode: " + chargingMode + ", Power: " + chargingPower + " Kw";

    Blynk.setProperty(V1, "label", chargingDetail);
    Blynk.virtualWrite(V1, textState);
  }

  void updateChargingColor() {
    if (!updateChargingColorDelay) {
      return;
    }

    // Calculate the color using the sine wave factor
    String colorCode = getFadeCustomGreenColor(timeCounter);
    // Set the widget color on V1 based on the sine wave
    Blynk.setProperty(V0, V1, V2, V3, V4, "color", colorCode);

    // Update the time counter for the sine wave (adjust the speed by modifying the value added)
    timeCounter += 0.2;  // Smaller value for slower fade, larger for faster
  }

  void setCarChargingState(CanFrame frame) {
    if (frame.data[0] == 0x03 || frame.data[0] == 0x43) {
      if (myCarState.carChargingState == false) {
        myCarState.carChargingState = true;
      }
      updateChargingDetail(frame);
      updateChargingColor();
    } else {
      if (myCarState.carChargingState == true) {
        Blynk.setProperty(V1, "label", "Car Status");
        myCarState.carChargingState = false;
        resetWidgetColor();
      }
    }
  }

  void setCarDoorState(CanFrame frame) {
    if (frame.data[0] == 0xAA) {
      /* All Door Locked */
      if (myCarState.carDoorLockedState == false) {
        myCarState.carDoorLockedState = true;
      }
    } else {
      if (myCarState.carDoorLockedState == true) {
        myCarState.carDoorLockedState = false;
      }
    }

    /**
    Closed:
    0001 1010 1A
    0001 0100 14
    0001 0110 16
    0001 1000 18

    Opened:
    0001 0101 15
    0001 1001 19
    */

    /* Trunk Closed*/
    if (frame.data[1] & (1 << 0)) {
      if (myCarState.carTrunkClosedState == true) {
        myCarState.carTrunkClosedState = false;
      }
    } else {
      if (myCarState.carTrunkClosedState == false) {
        myCarState.carTrunkClosedState = true;
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

    myCarState.carBatterySoc = frame.data[4];
  }

  void setCarOdoMeterValue(CanFrame frame) {
    switch (frame.data[0]) {
      case 0x4:
        {
          uint16_t lsb = frame.data[1];
          uint16_t msb = frame.data[2] & 0x0f;
          myCarState.carRangeLeft = lsb + (msb * 256);
          break;
        }
      case 0x5:
        {
          myCarState.carOdoMeter = (frame.data[1] | (frame.data[2] << 8) | (frame.data[3] << 16)) / 10;
          break;
        }
      default:
        break;
    }
  }

  int begin() {
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

  void run() {
    twai_status_info_t status_info;
    twai_get_status_info(&status_info);

    if (status_info.state != TWAI_STATE_RUNNING) {
      return;
    }

    if (ESP32Can.readFrame(rxFrame, 5)) {
#ifdef DEBUG_OBD
      if (pidMonitor != 0xF) {
        if (pidMonitor == 0x0 || pidMonitor == rxFrame.identifier) {
          printFrame(&rxFrame);
        }
      }
#endif
      switch (rxFrame.identifier) {
        case 0x407:
          setCarDoorState(rxFrame);
          break;
        case 0x055:
          setCarVccTurnedOnState(rxFrame.data[4]);
          break;
        case 0x323:
          setCarChargingState(rxFrame);
          break;
        case 0x3D9:
          setCarOdoMeterValue(rxFrame);
          break;
        default:
          break;
      }
    }
  }

  bool sendAndReceiveObdFrame(uint32_t txmoduleid, uint16_t rxmoduleid, uint8_t txData[], uint8_t txDataSize) {
    CanFrame obdFrame = { 0 };
    uint8_t dataSize = txDataSize;

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
    // return ESP32Can.writeFrame(obdFrame, 10);  // timeout defaults to 1 ms


    int maxWriteRetry = 10;
    int maxReadRetry = 20;
    for (int i = 0; i < maxWriteRetry; i++) {
      if (ESP32Can.writeFrame(obdFrame, 5)) {
        for (int j = 0; j < maxReadRetry; j++) {
          if (ESP32Can.readFrame(rxFrame, 5)) {
            if (rxFrame.identifier == rxmoduleid) {
              LOG_PRINT.print("Data Polling=> ");
              LOG_PRINT.print(rxFrame.identifier, HEX);
              LOG_PRINT.print(": ");
              String data = "";
              for (int i = 0; i < 8; i++) {
                data = data + String(rxFrame.data[i], HEX) + ":";
              }
              LOG_PRINT.println(data);
              setCarBatterySoc(rxFrame);
              return true;
            }
          }
        }
      }
    }
    return false;
  }

  bool sendDefaultObdFrame(uint8_t obdId) {
    uint8_t data[] = { 0x01, obdId };
    return sendAndReceiveObdFrame(0x7DF, 0x7E8, data, sizeof(data) / sizeof(data[0]));
  }

  bool sendObdFrame(uint8_t obdId) {
    uint8_t data[] = { 0x22, 0x00, obdId };
    return sendAndReceiveObdFrame(0x7E7, 0x7EF, data, sizeof(data) / sizeof(data[0]));
  }
};

ObdDeviceClass ObdDevice;
#endif