#include <RemoteSerial.h>

class RemoteSerialMonitorTest {
public:
  void Test1() {
    Serial.println("Test1");
  }

  void startRemoteSerialMonitor() {
    RemoteSerial.setIncomingMessageHandler(messageReceived);
    RemoteSerial.begin(&remoteSerialServer);  // You can also add a custom url, e.g. /remotedebug
    remoteSerialServer.begin();
  }

} RemoteSerialMonitor1;