#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

// Define esp32 I2C slave address
#define I2C_SLAVE_ADDR 0x08

// Enum for availble commands
enum Command : uint8_t {
  CMD_NONE = 0,
  CMD_SET_GAIT,
  CMD_SET_MODE,
  CMD_ENTER_CONFIG,
  CMD_SET_CONFIG,
  CMD_HOME_STANCE
};

// Define ControlPacket struct
struct ControlPacket {
  int16_t joystick1X;
  int16_t joystick1Y;
  int16_t currentHeight;
  Command command;
  int16_t commandArgs[3];
};

// Instance of ControlPacket
ControlPacket packet;

//_______________________________________________________________________OnDataRecv__________________________________________________________________

// Function to handle espNOW receive event
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (len != sizeof(ControlPacket)) {
    Serial.println("Invalid packet size");
    return;
  }

  // Temporary holder for the incoming packet
  ControlPacket incomingPacket;
  memcpy(&incomingPacket, data, sizeof(ControlPacket));

  if (packetChanged(incomingPacket, packet)) {packet = incomingPacket;}
}

//_______________________________________________________________________requestEvent__________________________________________________________________

// Function to handle I2C request event
void requestEvent() {
    // Send the data structure to the master (Raspberry Pi Pico)
    Wire.write((uint8_t*)&packet, sizeof(ControlPacket));
}

//_______________________________________________________________________packetChanged__________________________________________________________________

// Compare if two ControlPackets are different
bool packetChanged(const ControlPacket& a, const ControlPacket& b) {
  if (a.joystick1X != b.joystick1X) return true;
  if (a.joystick1Y != b.joystick1Y) return true;
  if (a.currentHeight != b.currentHeight) return true;
  if (a.command != b.command) return true;

  for (int i = 0; i < 3; i++) {
    if (a.commandArgs[i] != b.commandArgs[i]) return true;
  }
  return false;
}


//_______________________________________________________________________setup__________________________________________________________________
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv); // Set callback function to handle espNOW receive event

  Wire.begin(I2C_SLAVE_ADDR); // Initialize I2C as slave with specified address
  Wire.onRequest(requestEvent); // Set callback function to handle I2C request event
}

//_______________________________________________________________________loop__________________________________________________________________
void loop() {
  // Do nothing
}