#include <Wire.h>
#include <ps5.h>
#include <ps5Controller.h>
#include <ps5_int.h>

#define I2C_SLAVE_ADDR 0x08 // I2C address of the Raspberry Pi Pico

// Define the data structure with no padding
#pragma pack(push, 1)
struct DataPacket {
    bool Right;
    bool Left;
    bool Up;
    bool Down;

    bool Square;
    bool Cross;
    bool Circle;
    bool Triangle;

    int LStickX;
    int LStickY;

    int RStickX;
    int RStickY;
};
#pragma pack(pop)

// Create an instance of the data structure
//DataPacket data = {25, 60.5, "Hello, Pico!", true};
DataPacket data;

void setup() {
    Wire.begin(I2C_SLAVE_ADDR); // Initialize I2C as slave with specified address
    Wire.onRequest(requestEvent); // Set callback function to handle request event
    Serial.begin(115200); // Initialize serial communication for debugging

    ps5.begin("4c:b9:9b:43:04:49"); //replace with MAC address of your controller
    Serial.println("Ready.");
}

void loop() {
    while (ps5.isConnected() == true) {
      data.Right = ps5.Right();
      data.Left = ps5.Left();
      data.Up = ps5.Up();
      data.Down = ps5.Down();

      data.Square = ps5.Square();
      data.Cross = ps5.Cross();
      data.Circle = ps5.Circle();
      data.Triangle = ps5.Triangle();

      data.LStickX = ps5.LStickX();
      data.LStickY = ps5.LStickY();

      data.RStickX = ps5.RStickX();
      data.RStickY = ps5.RStickY();
    }
}

// Function to handle I2C request event
void requestEvent() {
    // Send the data structure to the master (Raspberry Pi Pico)
    Wire.write((uint8_t*)&data, sizeof(data));
}