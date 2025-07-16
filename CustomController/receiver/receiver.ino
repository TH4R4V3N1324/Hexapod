#include <WiFi.h>
#include <esp_now.h>

enum Command : uint8_t {
  CMD_NONE = 0,
  CMD_SET_GAIT,
  CMD_SET_MODE,
  CMD_ENTER_CONFIG,
  CMD_SET_CONFIG
};

struct ControlPacket {
  int16_t joystick1X;
  int16_t joystick1Y;
  Command command;
  uint8_t commandValue;
  int16_t legConfig[3];
};

ControlPacket packet;

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  memcpy(&packet, data, sizeof(packet));
  Serial.print("leg config - leg: ");
  Serial.print(packet.legConfig[0]);
  Serial.print(", joint: ");
  Serial.print(packet.legConfig[1]);
  Serial.print(", offset: ");
  Serial.println(packet.legConfig[2]);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Nothing here
}