#ifndef _MQTT_CLIENT
#define _MQTT_CLIENT

#include <WiFiClient.h>
#include <PubSubClient.h>
#include "Secrets.h"

#define MAX_RETRY_CONNECT 5

class MqttClientClass {
private:
  WiFiClient MqttEspClient;  // Initialize WiFi client
  PubSubClient mqttClient;   // Declare the PubSubClient object
  String client_id;
  uint8_t retryConnect = 0;

public:
  MqttClientClass()
    : mqttClient(MqttEspClient) {}  // Initialize mqttClient with MqttEspClient

  void begin(String clientId) {
    client_id = clientId;
    mqttClient.setServer(Secrets.mqtt_server, Secrets.mqtt_port);
  }

  void updateTrunkState(String state) {
    mqttClient.publish(Secrets.topic, state.c_str());
  }

  void mqttHandle() {
    // Loop until we're reconnected
    // Attempt to connect
    if (!mqttClient.connected() && WiFi.isConnected()) {
      if (retryConnect > MAX_RETRY_CONNECT) return;
      retryConnect++;
      if (mqttClient.connect(client_id.c_str(), Secrets.mqtt_username, Secrets.mqtt_password)) {
        Serial.println("connected");
        mqttClient.publish(Secrets.topic, "Connected!");
      }
    } else {
      mqttClient.loop();
    }
  }
};

MqttClientClass MqttClient;
#endif