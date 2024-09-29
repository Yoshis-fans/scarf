#include <iostream>
#include "json.hpp"
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>

using json = nlohmann::json;

// Device struct to store device info
struct Device {
    std::string name;
    std::string status;  // "available" or "in use"
    std::string type;
    int progress;        // Progress in percentage
};

// Global list of devices
std::vector<Device> devices = {
    {"Device 1", "available", "USB", 0},
    {"Device 2", "available", "Bluetooth", 0}
};

// Function to simulate device usage and increase progress until it reaches 100%
void updateDeviceProgress(Device &device) {
    if (device.status == "in use" && device.progress < 100) {
        // Increase progress by a random amount (between 5% and 20%)
        device.progress += rand() % 16 + 5;
        if (device.progress >= 100) {
            device.progress = 100;
            device.status = "available";  // Reset device to available after reaching 100%
        }
    }
}

// Simulate device competition by selecting a random device and using it
void simulateDeviceCompetition() {
    for (auto& device : devices) {
        if (device.status == "available" && rand() % 2 == 0) {
            // Randomly set the device "in use" and start the process
            device.status = "in use";
            device.progress = 0;  // Reset progress
        }

        // Update progress if the device is in use
        updateDeviceProgress(device);
    }
}

// Generate the current device data as a JSON object
json generateDeviceData() {
    std::vector<json> deviceList;

    // Convert devices into JSON format
    for (const auto& device : devices) {
        deviceList.push_back({
            {"name", device.name},
            {"status", device.status},
            {"type", device.type},
            {"progress", device.progress}
        });
    }

    return json{{"type", "devices"}, {"data", deviceList}};
}

int main() {
    srand(time(0));

    while (true) {
        // Simulate competition for devices and update progress
        simulateDeviceCompetition();

        // Generate device data with the current statuses and progress
        json deviceData = generateDeviceData();

        // Output the device data to stdout as a JSON string
        std::cout << deviceData.dump(4) << std::endl;

        // Wait for 1 second before the next update
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
