#include <Wire.h>

#define I2C_SLAVE_ADDR 0x08 // I2C address of the Raspberry Pi Pico

// Define the data structure with no padding
#pragma pack(push, 1)
struct DataPacket {
    int temperature;
    float humidity;
    char message[20];
};
#pragma pack(pop)

// Create an instance of the data structure
DataPacket data = {25, 60.5, "Hello, Pico!"};

void setup() {
    Wire.begin(I2C_SLAVE_ADDR); // Initialize I2C as slave with specified address
    Wire.onRequest(requestEvent); // Set callback function to handle request event
    Serial.begin(115200); // Initialize serial communication for debugging
}

void loop() {
    // No need for continuous sending in loop, as the Pico will request data
}

// Function to handle I2C request event
void requestEvent() {
    // Send the data structure to the master (Raspberry Pi Pico)
    Wire.write((uint8_t*)&data, sizeof(data));
}