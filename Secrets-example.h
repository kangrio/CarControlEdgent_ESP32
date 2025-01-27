#ifndef _SECRETS
#define _SECRETS

class SecretsClass {
public:
  // password for arduinoOTA hashed with md5
  const char *passWordHashed = "827ccb0eea8a706c4c34a16891f84e7b";

  // password for RemoteSerial
  const char *remoteSerialUsername = "username";
  const char *remoteSerialPassword = "12345";

  // Sinric Pro
  const char *APP_KEY = "APP_KEY";
  const char *APP_SECRET = "APP_SECRET";
  const char *TRUNK_ID = "TRUNK_ID";

  // MQTT Client
  const char *mqtt_server = "localhost";
  const char *topic = "mycar/trunk";
  const char *mqtt_username = "mymqtt";
  const char *mqtt_password = "12345";
  const int mqtt_port = 1883;
};

SecretsClass Secrets;
#endif