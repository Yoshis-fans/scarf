#include <iostream>
#include "json.hpp"
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>

using json = nlohmann::json;

// Simulate device data
json generateDeviceData() {
    std::vector<json> devices = {
        {{"name", "Device 1"}, {"status", "connected"}, {"type", "USB"}},
        {{"name", "Device 2"}, {"status", "disconnected"}, {"type", "Bluetooth"}},
        {{"name", "Device 3"}, {"status", "connected"}, {"type", "WiFi"}},
        {{"name", "Device 4"}, {"status", "connected"}, {"type", "Ethernet"}}
    };

    // Randomly change device status
    for (auto& device : devices) {
        if (rand() % 2 == 0) {
            device["status"] = "disconnected";
        } else {
            device["status"] = "connected";
        }
    }

    return json{{"type", "devices"}, {"data", devices}};
}

// Simulate error data
json generateErrorData() {
    std::vector<json> errors = {
        {{"code", 500}, {"message", "Internal server error"}, {"details", "The server encountered an unexpected condition."}},
        {{"code", 401}, {"message", "Unauthorized access"}, {"details", "Access requires authentication."}}
    };

    return json{{"type", "errors"}, {"data", errors[rand() % errors.size()]}};
}

// Simulate progress data
json generateProgressData() {
    int percentage = rand() % 101;
    std::string description = (percentage < 50) ? "Processing data for Device 3" : "Updating firmware for Device 1";

    return json{
        {"type", "progress"},
        {"data", {{"percentage", percentage}, {"description", description}}}
    };
}

// Simulate status data
json generateStatusData() {
    std::vector<std::string> statuses = {
        "Device scanning initiated",
        "Firmware update in progress"
    };

    time_t now = time(0);
    std::string timestamp = std::to_string(now);

    return json{
        {"type", "status"},
        {"data", {{"message", statuses[rand() % statuses.size()]}, {"timestamp", timestamp}}}
    };
}

// Simulate unknown data
json generateUnknownData() {
    return json{
        {"type", "unknown"},
        {"data", {{"message", "Received data does not conform to expected format"}, {"rawData", "0xABCD1234"}, {"details", "Could not be parsed."}}}
    };
}

int main() {
    srand(time(0));

    while (true) {
        // Generate random JSON data based on event type
        int eventType = rand() % 5;
        json output;

        switch (eventType) {
            case 0: output = generateDeviceData(); break;
            case 1: output = generateErrorData(); break;
            case 2: output = generateProgressData(); break;
            case 3: output = generateStatusData(); break;
            case 4: output = generateUnknownData(); break;
        }

        // Output the JSON to stdout
        std::cout << output.dump(4) << std::endl;

        // Wait for 1 second before the next update
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
