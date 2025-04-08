#ifndef _LOCAL_CONTROL
#define _LOCAL_CONTROL

#if defined(ESP8266)
#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#elif defined(ESP32)
#include "WiFi.h"
#include "AsyncTCP.h"
#include "esp_random.h"
#else
#error Platform not supported
#endif
#include "ESPAsyncWebServer.h"
#include <Preferences.h>
#include "LocalRemoteControlPage.h"


class LocalRemoteControlClass {
public:
  LocalRemoteControlClass() {
  }

  void begin(const char *username, const char *passwordHash, int portNumber = 8088) {
    _validUsername = username;
    _storedPasswordHash = passwordHash;
    _server = new AsyncWebServer(portNumber);
    load_token();
    setupLogin();
    setupControl();
    _server->begin();
  }

private:
  void load_token(){
    Preferences prefs;
    if (prefs.begin("localremote", true)) { // read-only
      _clientToken = prefs.getString("token");
      prefs.end();
    }
  }

  void save_token(String token){
    Preferences prefs;
    if (prefs.begin("localremote", false)) { // writeable
      prefs.putString("token", token);
      prefs.end();
    }
  }

  bool isAuthenticated(AsyncWebServerRequest *request) {
    if (request->hasHeader("Cookie")) {
      String cookie = request->header("Cookie");
      int tokenIndex = cookie.indexOf("auth_token=");
      if (tokenIndex >= 0) {
        String token = cookie.substring(tokenIndex + 11);
        token = token.substring(0, token.indexOf(";"));
        if (token == _clientToken) {
          return true;
        }
      }
    }

    return false;
  }

  String generateToken() {
    String token = "";
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < 32; i++) {
      #if defined(ESP8266)
      uint32_t r = RANDOM_REG32; // ESP8266's hardware RNG register
      #elif defined(ESP32)
      uint32_t r = esp_random(); // ESP32's hardware random number generator
      #endif
      token += charset[r % (sizeof(charset) - 1)];
    }
    return token;
  }

  void setupLogin() {
    _server->on("/login", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if (isAuthenticated(request)) {
        request->redirect("/");
        return;
      }
      request->send(200, "text/html", loginPage);
    });

    _server->on("/login", HTTP_POST, [this](AsyncWebServerRequest *request) {
      if (millis() - lastAttemptTime < 10000 && failedAttempts > 3) {
        request->send(429, "text/plain", "Too many attempts. Try again later in " + String((10000 - (millis() - lastAttemptTime))/1000) + " second");
        return;
      }

      String username = request->arg("username");
      String passwordHash = request->arg("password");

      if (username == _validUsername && passwordHash == _storedPasswordHash) {
        String token = generateToken();
        _clientToken = token;
        save_token(token);
        String cookieHeader = "Set-Cookie: auth_token=" + token + "; HttpOnly; SameSite=Strict";

        AsyncWebServerResponse* response = request->beginResponse(302, "text/html", controlPage);
        response->addHeader("Location", "/");
        response->addHeader("Set-Cookie", cookieHeader);
        request->send(response);
      } else {
        lastAttemptTime = millis();
        failedAttempts++;
        request->redirect("/login");
      }
    });
  }

  void setupControl() {
    _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
      if (isAuthenticated(request)) {
        request->send(200, "text/html", controlPage);
        return;
      }
      request->redirect("/login");
    });

    _server->on("/action", HTTP_POST, [this](AsyncWebServerRequest *request) {
      String authHeader = request->header("Authorization");
      if (!isAuthenticated(request)) {
        request->redirect("/login");
        return;
      }

      String button = request->arg("button");

      if (button.equals("lock")) {
        KeyFop.simulateLock();
      } else if (button.equals("unlock")) {
        KeyFop.simulateUnlock();
      } else if (button.equals("trunk")) {
        KeyFop.simulateToggleTrunk();
      } else if (button.equals("startstop")) {
        KeyFop.simulateStartStop();
      } else {
        request->send(501);
        return;
      }

      request->send(200);
    });
  }

  String _validUsername;
  String _storedPasswordHash;
  String _clientToken;
  AsyncWebServer *_server;
  uint8_t failedAttempts = 0;
  uint32_t lastAttemptTime = 0;
};

LocalRemoteControlClass LocalRemoteControl;

#endif
