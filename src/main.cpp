#include <Arduino.h>

#include "device.h"
#include "utils.h"
#include <string>

extern rpz::Device device;

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("init()"));

    device.init();
}

void loop() {
    device.update();

    delay(1000);
}


