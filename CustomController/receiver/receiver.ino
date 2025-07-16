#include <WiFi.h>
#include <esp_now.h>

enum Command : uint8_t {
  CMD_NONE = 0,
  CMD_SET_GAIT,
  CMD_SET_MODE,
  CMD_ENTER_CONFIG,
  CMD_SET_CONFIG,
  CMD_HOME_STANCE
};

struct ControlPacket {
  int16_t joystick1X;
  int16_t joystick1Y;
  Command command;
  int16_t commandArgs[3];
};

ControlPacket packet;

void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  memcpy(&packet, data, sizeof(packet));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  commandFSM();
}

void commandFSM() {
  switch (packet.command) {
    case CMD_SET_GAIT:
      Serial.print("Gait: ");
      Serial.println(packet.commandArgs[0]);
      break;
    case CMD_SET_MODE:
      Serial.print("Mode: ");
      Serial.println(packet.commandArgs[0]);
      break;
    case CMD_ENTER_CONFIG:
      Serial.println("Moving to config stance");
      break;
    case CMD_SET_CONFIG:
      Serial.print("Leg: ");
      Serial.print(packet.commandArgs[0]);
      Serial.print(" Joint: ");
      Serial.print(packet.commandArgs[1]);
      Serial.print(" Offset: ");
      Serial.println(packet.commandArgs[2]);
      break;
    case CMD_HOME_STANCE:
      Serial.println("Moving to home stance");
      break;
    default:
      break;
  }
}