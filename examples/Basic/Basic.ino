#include <Unleash.h>
#include <WiFi.h>

const char* ssid = "PUT YOUR SSID HERE";
const char* password = "THE PASSWORD IS HERE";

Unleash u;

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");

    u.begin("THE UNLEASH SERVER URL IS HERE", "appName", "instanceId");
}

void loop() {
    if (u.isEnabled("feature-1")) {
        Serial.println("Feature is enabled :)");
    } else {
        Serial.println("Feature is disabled :(");
    }    
    delay(1000);

    u.loop();
}