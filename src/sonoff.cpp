#include <Updater.h>
#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include "PubSubClient.h"
#include "Espanol.h"

#define DEBUG

#define SWITCH_NUMBER_STRING "2"

#define RELAY_PIN  12
#define BUTTON_PIN 0
#define LED_PIN    13

#define DEBOUNCE_COUNTER_START 150

#define HOSTNAME "switch-" SWITCH_NUMBER_STRING

#define WIFI_SSID     "HaSi-Kein-Internet-Legacy"
#define WIFI_PASSWORD "bugsbunny"

#define MQTT_BROKER "atlas.hasi"
#define MQTT_PORT   1883

#define SWITCH_TOPIC "hasi/switches/" SWITCH_NUMBER_STRING

bool relayOn = false;

void switchRelayOn()
{
#ifdef DEBUG
    Serial.println("Turning the relay on");
#endif

    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);

    relayOn = true;
}

void switchRelayOff()
{
#ifdef DEBUG
    Serial.println("Turning the relay off");
#endif

    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

    relayOn = false;
}

void toggleRelay()
{
    if (relayOn)
    {
        switchRelayOff();
    }
    else
    {
        switchRelayOn();
    }
}

void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
    Espanol.setDebug(true);

    Serial.println("Booting...");
#endif

    delay(10);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    Espanol.subscribe(SWITCH_TOPIC);
    Espanol.begin(WIFI_SSID, WIFI_PASSWORD, HOSTNAME, MQTT_BROKER, MQTT_PORT);

    Espanol.setCallback([](char *topic, uint8_t *bytes, unsigned int length) {
        char *str = new char[length + 1];
        memcpy(str, bytes, length);
        str[length] = '\0';

        if (strcmp(topic, SWITCH_TOPIC) == 0)
        {
            if (strcmp(str, "on") == 0)
            {
                switchRelayOn();
            }
            else if (strcmp(str, "off") == 0)
            {
                switchRelayOff();
            }
        }
    });

    switchRelayOff();
}

void loop()
{
    static int lastButtonState = HIGH;
    static int debounceCounter = 0;

    Espanol.loop();

    if (debounceCounter > 0)
    {
        if (debounceCounter == 1)
        {
            int newButtonState = lastButtonState == HIGH ? LOW : HIGH;

            if (newButtonState == LOW)
            {
                toggleRelay();
            }

            lastButtonState = newButtonState;
        }

        debounceCounter--;
    }
    else if (lastButtonState != digitalRead(BUTTON_PIN))
    {
        debounceCounter = DEBOUNCE_COUNTER_START;
    }
}
