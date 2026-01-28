#include <Arduino.h>
#include <esp32_smartdisplay.h>
#include <ui/ui.h>
#include <ModbusMaster.h>
#include <WiFi.h> // Add the Wi-Fi library
#include <time.h>

#include "secrets.h"
#include <WiFiClientSecure.h>
#define MQTT_MAX_PACKET_SIZE 100000
#define MQTT_KEEPALIVE 200
#define MQTT_SOCKET_TIMEOUT 200
#define ARDUINOJSON_USE_LONG_LONG 1
#include <PubSubClient.h>
#include <ArduinoJson.h>

ModbusMaster node;// Wi-Fi credentials

// The MQTT topics that this device should publish/subscribe
#define AWS_IOT_PUBLISH_TOPIC   "iot/firealarm/vtgfgMM" // "iot/firealarm/vtgdrTH" "iot/firealarm/vtgfgTH"
#define PUBLISH_INTERVAL 30000  // 30 seconds
#define LINE_TOKEN "jlEuC15y3q0H9amIaRrr25AEvVFnnzS1U78B2mhLPfwTgBoIohh023LEAC+coo80dIgSmWYWhOGII0JLRJHvadyotfinQo1vHzzKYwDx5d7Yy4eb++9OULo1Uco4vH6TjAqMulSiGe068San6VK2AgdB04t89/1O/w1cDnyilFU=" // Channel access token
#define RESTART_INTERVAL 10800000UL  // 3 hours in milliseconds

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Your sensor values (replace these with actual values)
float temperature = 0.0;
float humidity = 0.0;

const char* getModbusErrorDescription(uint8_t code) {
    switch(code) {
        case 0x00: return "None";                      // No error
        case 0xE0: return "Timeout";                   // Modbus Timeout
        case 0xE1: return "Invalid Response";          // Invalid response from slave
        case 0xE2: return "CRC Error | Wiring Error";  // CRC Error or wiring issue
        case 0xE3: return "Modbus Exception";          // Exception from Modbus slave
        case 0x01: return "Illegal Function";          // Modbus exception: Illegal function
        case 0x02: return "Illegal Data Address";      // Modbus exception: Illegal data address
        case 0x03: return "Illegal Data Value";        // Modbus exception: Illegal data value
        case 0x04: return "Slave Device Failure";      // Modbus exception: Slave device failure
        case 0x05: return "Acknowledge";               // Modbus exception: Acknowledge
        case 0x06: return "Slave Device Busy";         // Modbus exception: Slave device busy
        case 0x07: return "Negative Acknowledge";      // Modbus exception: Negative Acknowledge
        case 0x08: return "Memory Parity Error";       // Modbus exception: Memory parity error
        case 0x10: return "Gateway Path Unavailable";  // Modbus exception: Gateway path unavailable
        default: return "Unknown Modbus Error";        // Unknown Modbus error code
    }
}

// Function to get Wi-Fi status descriptions
const char* getWiFiStatusDescription() {
    uint8_t code = WiFi.status();
    switch (code) {
        case WL_IDLE_STATUS:            return "Idle, not connected";
        case WL_NO_SSID_AVAIL:          return "No SSID available";
        case WL_CONNECT_FAILED:         return "Wi-Fi connection failed";
        case WL_CONNECTION_LOST:        return "Wi-Fi connection lost";
        case WL_CONNECTED:              return "Connected to Wi-Fi";
        case WL_DISCONNECTED:           return "Disconnected";
        // Replace WL_WRONG_PASSWORD and WL_CONNECTING with a fallback description
        default:                        return "Unknown Wi-Fi status";
    }
}

const char* getMqttErrorDescription(int state) {
    switch (state) {
        case -4: return "Connection Timeout";
        case -3: return "Connection Lost";
        case -2: return "Connect Failed";
        case -1: return "Disconnected";
        case 0:  return "Connected";
        case 1:  return "Bad Protocol";
        case 2:  return "Bad Client ID";
        case 3:  return "Unavailable";
        case 4:  return "Bad Credentials";
        case 5:  return "Unauthorized";
        default: return "Unknown Error";
    }
}
// Use a large enough buffer size based on JSON structure
StaticJsonDocument<200> doc;

void setup() {
#ifdef ARDUINO_USB_CDC_ON_BOOT
    delay(5000);
#endif
    Serial.begin(4800, SERIAL_8N1, 1, 3);
    Serial.flush();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    client.setServer(AWS_IOT_ENDPOINT, 8883);

    // Configure time (NTP + UTC+7)
    configTzTime("ICT-7", "pool.ntp.org", "time.nist.gov");

    // Initialize your display, etc.
    smartdisplay_init();

    __attribute__((unused)) auto disp = lv_disp_get_default();
    ui_init();
}


ulong next_millis = 0;
auto lv_last_tick = millis();
uint8_t result;
ulong lastPublishTime = 0;
ulong startTime = millis();
unsigned long startAttemptTime = millis();
const unsigned long timeout = 10000; // 10 seconds
unsigned long startAttemptTimeAWS = millis();
const unsigned long timeoutAWS = 10000; // 10 seconds
unsigned long systemStartTime = millis();


void loop() {
    auto const now = millis();
    if (millis() - systemStartTime >= RESTART_INTERVAL) {
        ESP.restart();  // Restart the ESP32 after 3 hours
    }
    if (now > next_millis){
        next_millis = now + 100;

        // Read 2 holding registers starting at 0 (humidity and temperature)
        Serial.flush();
        node.begin(1, Serial);
        result = node.readHoldingRegisters(0, 2);
        delay(100); // Wait for the response

        String msgerror = String(result, HEX) + " (" + getModbusErrorDescription(result) + ")";

        if (msgerror != "0 (None)") {
#ifdef BOARD_HAS_RGB_LED
            smartdisplay_led_set_rgb(255, 0, 0); // Set RGB LED to red
#endif
            lv_label_set_text(ui_sensorerrorValue, msgerror.c_str());
            delay(1000);
        }
        else{
#ifdef BOARD_HAS_RGB_LED
            smartdisplay_led_set_rgb(0, 255, 0); // Set LED to green when connected
#endif
            lv_label_set_text(ui_sensorerrorValue, "Connected");
        }

        humidity = node.getResponseBuffer(0) / 10.0;
        String humidity_str = String(humidity, 2);
        if (humidity > 60.0) {
            // Set the text to red
            lv_label_set_text(ui_humidValue, "");
            lv_label_set_text(ui_humidValue1, humidity_str.c_str());
        } else {
            // Set the text to default color (optional)
            lv_label_set_text(ui_humidValue1, "");
            lv_label_set_text(ui_humidValue, humidity_str.c_str());
        }

        
        temperature = node.getResponseBuffer(1) / 10.0;
        String temp_str = String(temperature, 2);
        lv_label_set_text(ui_tempValue, temp_str.c_str());

         // Check Wi-Fi status periodically (e.g., once every 5 seconds)
        if (millis() - lastPublishTime > PUBLISH_INTERVAL) {
            if (WiFi.status() != WL_CONNECTED) {
                WiFi.disconnect();
                WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
                String msgerror = String(WiFi.status(), HEX) + " (" + getWiFiStatusDescription() + ")";
                // Set RGB LED to blue
#ifdef BOARD_HAS_RGB_LED
                smartdisplay_led_set_rgb(0, 0, 255); // Set LED to blue for Wi-Fi error
#endif
                lv_label_set_text(ui_inteneterrorValue, msgerror.c_str());
                lv_label_set_text(ui_dateValue, "Loss");
                lv_label_set_text(ui_timeValue, "Loss");
                delay(500);
            } else {
#ifdef BOARD_HAS_RGB_LED
                smartdisplay_led_set_rgb(0, 255, 0); // Set LED to green when connected
#endif
                String wifiInfo = "Connect -->" + String(WiFi.SSID()) + " (" + String(WiFi.RSSI()) + " dBm)";
                lv_label_set_text(ui_inteneterrorValue, wifiInfo.c_str());

                time_t rawtime;
                struct tm timeinfo;
                time(&rawtime);
                localtime_r(&rawtime, &timeinfo);

                // Format date and time strings
                char dateStr[16];
                char timeStr[16];
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);  // Format: 2025-05-01
                strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);  // Format: 14:30:45

                lv_label_set_text(ui_dateValue, dateStr);
                lv_label_set_text(ui_timeValue, timeStr);
                
                if (!client.connect(THINGNAME)){
                    }
#ifdef BOARD_HAS_RGB_LED
                    smartdisplay_led_set_rgb(0, 0, 0); // Set RGB LED to green
#endif
                    delay(500);
                }
                
                if (!client.connected()){
                    char msgerror[100];
                    snprintf(msgerror, sizeof(msgerror), "%X (%s)", client.state(), getMqttErrorDescription(client.state()));
                    lv_label_set_text(ui_awsValue, msgerror);
                    client.connect(THINGNAME);
                }
                else{
                    result = node.readHoldingRegisters(0, 2);
                    humidity = node.getResponseBuffer(0) / 10.0;
                    temperature = node.getResponseBuffer(1) / 10.0;
                    doc["humidity"] = String(humidity, 2);
                    doc["temperature"] = String(temperature, 2);
                    char jsonBuffer[512];
                    size_t len = serializeJson(doc, jsonBuffer);
                    bool published = client.publish(AWS_IOT_PUBLISH_TOPIC, (const uint8_t*)jsonBuffer, len);
                    if (published) {
                        char statusMsg[64];
                        snprintf(statusMsg, sizeof(statusMsg), "Published (Size: %d)", len);
                        lv_label_set_text(ui_awsValue, statusMsg);
                        // Clear the shared data buffer
                    } else {
                        int mqttState = client.state();
                        char statusMsg[64];
                        snprintf(statusMsg, sizeof(statusMsg), "Publish Failed (State: %d)", mqttState);
                        lv_label_set_text(ui_awsValue, statusMsg);
                    }
                    doc.clear();
                    node.clearResponseBuffer();
                    node.clearResponseBuffer();
                    lastPublishTime = millis();
                    delay(1000);
#ifdef BOARD_HAS_RGB_LED
                    smartdisplay_led_set_rgb(255, 255, 0); // Set LED to blue for Wi-Fi error
#endif
            }
        }

#ifdef BOARD_HAS_RGB_LED
        smartdisplay_led_set_rgb(0, 255, 0); // Set RGB LED to green
#endif
    }
    // Update the ticker
    lv_tick_inc(now - lv_last_tick);
    lv_last_tick = now;
    // Update the UI
    lv_timer_handler();
}
