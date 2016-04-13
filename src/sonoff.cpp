#include <Updater.h>
#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include "Espanol.h"

#define DEBUG

#define SWITCH_NUMBER_STRING "3"

#define RELAY_PIN  12
#define BUTTON_PIN 0
#define LED_PIN    13

#define DEBOUNCE_COUNTER_START 150

#define HOSTNAME "switch-" SWITCH_NUMBER_STRING

#define WIFI_SSID     "HaSi-Kein-Internet-Legacy"
#define WIFI_PASSWORD "bugsbunny"

#define OTA_PASSWORD "bugsbunny"

#define MQTT_BROKER "atlas.hasi"
#define MQTT_PORT   1883

#define RESPONSE_SUFFIX              "/response"
#define SWITCH_TOPIC                 "hasi/switches/" SWITCH_NUMBER_STRING
#define SWITCH_TOPICS_TOPIC          SWITCH_TOPIC "/topics"
#define SWITCH_TOPICS_RESPONSE_TOPIC SWITCH_TOPICS_TOPIC RESPONSE_SUFFIX
#define SWITCH_HELP_TOPIC            SWITCH_TOPIC "/help"
#define SWITCH_HELP_RESPONSE_TOPIC   SWITCH_HELP_TOPIC RESPONSE_SUFFIX
#define SWITCH_STATE_TOPIC           SWITCH_TOPIC "/state"
#define SWITCH_STATE_RESPONSE_TOPIC  SWITCH_STATE_TOPIC RESPONSE_SUFFIX

#define TOPICS_LIST "topics:string,help:string,state:string"

#define HELP_MESSAGE "Switch (hasi/switches/" SWITCH_NUMBER_STRING ")\n" \
                     "\n" \
                     "This switch is a Sonoff switch switchable over MQTT.\n" \
                     "\n" \
                     "TOPICS\n" \
                     "    state: \"on\" or \"off\"\n" \
                     "        - Set switch on or off"

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

    Espanol.subscribe(SWITCH_TOPICS_TOPIC);
    Espanol.subscribe(SWITCH_HELP_TOPIC);
    Espanol.subscribe(SWITCH_STATE_TOPIC);
    Espanol.begin(WIFI_SSID, WIFI_PASSWORD, HOSTNAME, MQTT_BROKER, MQTT_PORT);

    Espanol.setCallback([](char *cTopicStr, uint8_t *bytes, unsigned int length) {
        String content, topic;

        topic = String(cTopicStr);

        char *cContentStr = new char[length + 1];
        memcpy(cContentStr, bytes, length);
        cContentStr[length] = '\0';
        content = String(cContentStr);
        free(cContentStr);

        if (topic.equals(SWITCH_TOPICS_TOPIC))
        {
            if (content.equals("get"))
            {
                Espanol.publish(SWITCH_TOPICS_RESPONSE_TOPIC, TOPICS_LIST);
            }
        }
        else if (topic.equals(SWITCH_HELP_TOPIC))
        {
            if (content.equals("get"))
            {
                Espanol.publish(SWITCH_HELP_RESPONSE_TOPIC, HELP_MESSAGE);
            }
        }
        else if (topic.equals(SWITCH_STATE_TOPIC))
        {
            if (content.equals("get"))
            {
                Espanol.publish(SWITCH_STATE_RESPONSE_TOPIC, relayOn ? "on" : "off");
            }
            else if (content.equals("on"))
            {
                switchRelayOn();
            }
            else if (content.equals("off"))
            {
                switchRelayOff();
            }
        }
    });

    switchRelayOff();

    while (WiFi.status() != WL_CONNECTED)
    {
        Espanol.loop();
        delay(100);
    }

    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.begin();
}

void loop()
{
    static int lastButtonState = HIGH;
    static int debounceCounter = 0;

    ArduinoOTA.handle();
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
