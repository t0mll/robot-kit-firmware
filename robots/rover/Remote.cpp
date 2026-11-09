#include "Remote.h"
#include <WebServer.h>
#include <WiFi.h>
#include <RobotShell.h>
#include "RemotePage.h"

extern const char* currentModeName();
extern int currentDistanceMm();

namespace {
WebServer server(80);
constexpr uint32_t COMMAND_TTL_MS = 500;
constexpr uint32_t JOIN_TIMEOUT_MS = 10000;
Remote* self = nullptr;
const char* name = "Rover";

/// A hotspot password per Rover, made once and kept in NVS, shown on the Face.
String hotspotPassword() {
  String pw = RobotShell.settings.getString("ap_pass", "");
  if (pw.length() != 8) {
    char buf[9];
    for (int i = 0; i < 8; i++) buf[i] = '0' + (esp_random() % 10);
    buf[8] = 0;
    pw = buf;
    RobotShell.settings.putString("ap_pass", pw);
  }
  return pw;
}
}  // namespace

void Remote::begin(const char* robotName) {
  self = this;
  name = robotName;
  const String ssid = RobotShell.settings.getString("wifi_ssid", "");
  bool joined = false;
  if (ssid.length()) {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("rover");
    WiFi.begin(ssid.c_str(), RobotShell.settings.getString("wifi_pass", "").c_str());
    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < JOIN_TIMEOUT_MS) delay(100);
    joined = WiFi.status() == WL_CONNECTED;
  }
  if (joined) {
    snprintf(hint_, sizeof(hint_), "http://%s", WiFi.localIP().toString().c_str());
  } else {
    // Hotspot fallback. Espressif rates hotspot + phone + BLE gamepad as unstable, so the gamepad
    // stays off while the hotspot is up (#92).
    char apName[24];
    snprintf(apName, sizeof(apName), "%s-%04X", robotName, (uint16_t)(ESP.getEfuseMac() & 0xFFFF));
    const String pw = hotspotPassword();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apName, pw.c_str());
    snprintf(hint_, sizeof(hint_), "%s / %s", apName, pw.c_str());
  }

  server.on("/", HTTP_GET, [] { server.send_P(200, "text/html", REMOTE_PAGE); });
  server.on("/drive", HTTP_GET, [] {
    self->left_ = constrain(server.arg("l").toInt(), -100, 100);
    self->right_ = constrain(server.arg("r").toInt(), -100, 100);
    self->commandMs_ = millis();
    server.send(204);
  });
  server.on("/status", HTTP_GET, [] {
    char json[160];
    snprintf(json, sizeof(json), "{\"name\":\"%s\",\"mode\":\"%s\",\"battery\":%.2f,\"distance\":%d}", name,
             currentModeName(), RobotShell.batteryVolts(), currentDistanceMm());
    server.send(200, "application/json", json);
  });
  server.on("/wifi", HTTP_GET, [] { server.send_P(200, "text/html", WIFI_PAGE); });
  server.on("/wifi", HTTP_POST, [] {
    if (server.hasArg("forget")) {
      RobotShell.settings.remove("wifi_ssid");
      RobotShell.settings.remove("wifi_pass");
    } else {
      RobotShell.settings.putString("wifi_ssid", server.arg("ssid"));
      RobotShell.settings.putString("wifi_pass", server.arg("pass"));
    }
    server.send(200, "text/plain", "Saved. The Rover restarts now.");
    delay(500);
    ESP.restart();
  });
  server.begin();
}

void Remote::update() { server.handleClient(); }

bool Remote::command(int& left, int& right) const {
  if (!commandMs_ || millis() - commandMs_ > COMMAND_TTL_MS) return false;
  left = left_;
  right = right_;
  return true;
}
