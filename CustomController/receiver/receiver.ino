#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

// Define esp32 I2C slave address
#define I2C_SLAVE_ADDR 0x08

// Mac address for hexapod esp32
uint8_t controllerMAC[] = {0x80, 0x65, 0x99, 0xE9, 0x6F, 0x56};

// Enum for availble commands
enum Command : uint8_t {
  CMD_NONE = 0,
  CMD_SET_GAIT,
  CMD_SET_MODE,
  CMD_SET_CONFIG,
  CMD_HOME_STANCE,
  CMD_REQUEST_CONFIG
};

// Define the data structure with no padding
#pragma pack(push, 1)
// Define ControlPacket struct
struct ControlPacket {
  int16_t joystick1X;
  int16_t joystick1Y;
  int16_t currentHeight;
  Command command;
  int16_t commandArgs[3];
};

// Define HexPacket struct
struct HexPacket {
  int16_t legConfigs[3];
  int16_t currentHeight;
};
#pragma pack(pop)

// Instances of packets
ControlPacket controlPacket = {};
HexPacket hexPacket = {};

//_______________________________________________________________________OnDataRecv__________________________________________________________________

// Function to handle espNOW receive event
void receiveEventEspNOW(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (len != sizeof(ControlPacket)) {
    Serial.println("Invalid packet size");
    return;
  }

  // Temporary holder for the incoming packet
  ControlPacket incomingPacket;
  memcpy(&incomingPacket, data, sizeof(ControlPacket));
  
  if (packetChanged(incomingPacket, controlPacket)) {controlPacket = incomingPacket;}

  esp_now_send(controllerMAC, (uint8_t*)&hexPacket, sizeof(HexPacket));
}

//_______________________________________________________________________requestEvent__________________________________________________________________

// Function to handle I2C request event
void requestEventI2C() {
  // Send the data structure to the master (Raspberry Pi Pico)
  Wire.write((uint8_t*)&controlPacket, sizeof(ControlPacket));
}

//_______________________________________________________________________receiveEvent__________________________________________________________________

// Function to handle I2C receive event (ESP32 receives updated HexPacket from Pico)
void receiveEventI2C(int numBytes) {
  if (numBytes != sizeof(HexPacket)) {
    Serial.printf("Warning: Expected %d bytes, received %d\n", sizeof(HexPacket), numBytes);
  }

  Wire.readBytes((char*)&hexPacket, sizeof(HexPacket));
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

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, controllerMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(controllerMAC)) {esp_now_add_peer(&peerInfo);}

  esp_now_register_recv_cb(receiveEventEspNOW); // Set callback function to handle espNOW receive event
  Wire.begin(I2C_SLAVE_ADDR); // Initialize I2C as slave with specified address
  Wire.onRequest(requestEventI2C); // Set callback function to handle I2C request event
  Wire.onReceive(receiveEventI2C); // Set callback function to handle I2C receive event
}

//_______________________________________________________________________loop__________________________________________________________________
void loop() {
  // Do nothing
}