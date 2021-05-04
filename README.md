# arduino-unleash

![CI Status](https://github.com/ktos/arduino-unleash/actions/workflows/ci.yml/badge.svg)

This is an unofficial Unleash Client SDK for Arduino. It is compatible with the
[Unleash Open-Source](https://github.com/unleash/unleash). It should also work
with [GitLab Feature
Flags](https://docs.gitlab.com/ee/user/project/operations/feature_flags.html)
and [Unleash-hosted.com SaaS](https://www.unleash-hosted.com/) offering.

At the moment only ESP32 is supported.

## Usage

You have to just include the main file somewhere in the beginning of your
Arduino sketch:

```cpp
#include <Unleash.h>
```

Then in `setup()` you can configure your instance, while in `loop()` you have to
use `Unleash::loop()` method which is updating feature flags from the server.

```cpp
Unleash u;

void setup() {
    Serial.begin(115200);

    // Connect to Wi-Fi
    Serial.print("Connecting to Wi-Fi");    
    WiFi.begin("my_ssid", "my password");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    
    // configure Unleash, no trailing "/"!
    u.begin("http://example.com:4242", "application name", "instance id");
}

void loop() {
    u.loop();
}
```

In every part of your code you will be able to use the `Unleash::isEnabled()`
method, returning if the provided feature flag is enabled.

The default buffer for the toggle list is 2048 bytes, so it may be too small if
you are using very large list of feature toggles - it is controled by the
`UNLEASHJSON_SIZE` define, which you may override.

### Dependencies
The framework is dependent on the following libraries:

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson), 6.17.3.
