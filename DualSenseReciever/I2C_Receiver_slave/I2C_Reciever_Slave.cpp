#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C address and port of the ESP32
#define ESP32_SLAVE_ADDR 0x08
#define I2C_PORT i2c0
#define SDA_PIN 0
#define SCL_PIN 1

// Define the data structure with no padding
#pragma pack(push, 1)
struct DataPacket {
    int temperature;
    float humidity;
    char message[20];
};
#pragma pack(pop)

int main() {
    stdio_init_all();

    // Initialize I2C as master
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Data structure to store received data from ESP32
    DataPacket receivedData;

    while (true) {
        // Request data from ESP32
        i2c_write_blocking(I2C_PORT, ESP32_SLAVE_ADDR, NULL, 0, true);

        // Receive data from ESP32
        int bytesRead = i2c_read_blocking(I2C_PORT, ESP32_SLAVE_ADDR, (uint8_t*)&receivedData, sizeof(receivedData), false);

        if (bytesRead == sizeof(receivedData)) {
            // Print received data
            printf("Bytes read: %d\n", bytesRead);
            printf("Temperature: %d\n", receivedData.temperature);
            printf("Humidity: %.2f\n", receivedData.humidity);
            printf("Message: %s\n", receivedData.message);
        } else {
            // Print error if data size does not match expected size
            printf("Failed to read data from ESP32. Bytes read: %d\n", bytesRead);
        }

        sleep_ms(1000); // Wait for a second before requesting data again
    }

    return 0;
}