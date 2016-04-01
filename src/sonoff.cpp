#include <Arduino.h>
#include "Espanol.h"

#define DEBUG

#define RELAY_PIN 4
#define TOPIC     "foo/bar"

const char *ssid = "ssid";
const char *password = "password";

const char *broker = "broker";
const int port = 1883;

void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    pinMode(RELAY_PIN, OUTPUT);

    Espanol.begin(ssid, password, broker, port);
    Espanol.setCallback([](char *topic, uint8_t *bytes, unsigned int length) {
        char *str = new char[length + 1];
        memcpy(str, bytes, length);
        str[length] = '\0';

        if (strcmp(topic, TOPIC) == 0)
        {
            if (strcmp(str, "on") == 0)
            {
                digitalWrite(RELAY_PIN, HIGH);
            }
            else if (strcmp(str, "off") == 0)
            {
                digitalWrite(RELAY_PIN, LOW);
            }
        }
    });

#ifdef DEBUG
    Espanol.setDebug(true);
#endif
}

void loop()
{
    Espanol.loop();
}
