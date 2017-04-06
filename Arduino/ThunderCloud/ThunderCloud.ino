/**
 * Thunder Cloud 
 * ==================================
 * A thunder cloud workshop. Todo add more info
 * 
 * More info: https://github.com/funvill/LightningCloudWorkshop
 * 
 * Requirements 
 * - https://github.com/tzapu/WiFiManager
 * - https://github.com/FastLED/FastLED 
 * - https://github.com/bblanchon/ArduinoJson
 * 
 * Created by: Steven Smethurst
 * Last updated: April 04, 2017 
 * Version: 0.0.1 
 * 
 * This project is based on Lighting Cloud Mood Lamp By James Bruce
 * 
 * Lighting Cloud Mood Lamp By James Bruce
 * View the full tutorial and build guide at http://www.makeuseof.com/
 * Sound sampling code originally by Adafruit Industries. Distributed under the BSD license. 
 * This paragraph must be included in any redistribution. More info: https://github.com/jamesabruce/cloudlamp
 */

// Captive portal
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <ESP8266WiFi.h> //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

// LEDs
#include "FastLED.h"
#include <Wire.h>

// Setting file
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

// Settings
// =====================================
// How many leds in your strip?
#define SETTING_NUM_LEDS 30
// What pin is the LED data pin connected to.
#define SETTING_DATA_PIN D4

// Constants
// =====================================
#define VERSION_MAJOR "0"
#define VERSION_MINOR "0"
#define VERSION_PATH "1"

// Globals 
// =====================================
// Define the array of leds
CRGB leds[SETTING_NUM_LEDS];
ESP8266WebServer webServer(80);


bool randomColor;

CHSV getBright()
{
    if (randomColor) {
        // Random
        return CHSV(random(0, 255), 255, 255 / 3);
    } else {
        return CHSV(145, 125, 255 / 2); // Light blue
    }
}

void UnconfiguredWiFi(WiFiManager* wifi)
{
	// WiFi is not configured. 
	// Setting the cloud into a known default state

	Serial.println("FYI: WiFi is not configured.");
    colour_fade();
	
}

void handleRoot() {
	webServer.send(200, "text/html", "hello world");
}

void setup()
{
    // Set up serial
    Serial.begin(115200);
    Serial.println();

    // Print version information
    Serial.println("FYI: Thunder Cloud version: " + String(VERSION_MAJOR) + "." + String(VERSION_MINOR) + "." + String(VERSION_PATH));
    Serial.println("FYI: More info: https://github.com/funvill/LightningCloudWorkshop");
    Serial.println();
    
    // Define the LEDs, how many, what type and what order. 
    FastLED.addLeds<WS2812B, SETTING_DATA_PIN, GRB>(leds, SETTING_NUM_LEDS);

    // Note, Connecting to the Wifi Manager is a blocking function.
    WiFiManager wifiManager;	
    // wifiManager.resetSettings(); // Debug: Reset saved settings
    wifiManager.setAPCallback(UnconfiguredWiFi);
	String ssid = "ThunderCloud-" + String(ESP.getChipId());
    wifiManager.autoConnect(ssid.c_str());

    // Connected to WiFi
    Serial.println("FYI: Successfully connected to wifi");
    Serial.print("FYI: Local IP address: ");
    Serial.println(WiFi.localIP());

	// Set up web server routes 
	webServer.on("/", handleRoot);
	webServer.begin();

	randomColor = true;
}

#define TIME_MODE_CHANGE (1000 * 5)
#define TIME_COLOR_MODE_CHANGE (1000 * 1 * 5)
void loop()
{
	webServer.handleClient();
	yield(); 

    constant_lightning();
    return;

    static bool mode = true;
    static unsigned long lastModeChange = millis();
    if (millis() > lastModeChange + TIME_MODE_CHANGE) {
        lastModeChange = millis();
        mode = !mode;
    }

    static unsigned long lastColourModeChange = millis();
    if (millis() > lastColourModeChange + TIME_COLOR_MODE_CHANGE) {
        lastColourModeChange = millis();
        randomColor = !randomColor;
    }

    if (mode) {
        colour_fade();
    } else {
        constant_lightning();
    }

    return;
}
// used to make basic mood lamp colour fading feature

void colour_fade()
{
    static int fade_h;
    static int fade_direction = 1;

    // mood mood lamp that cycles through colours
    for (int i = 0; i < SETTING_NUM_LEDS; i++) {
        leds[i] = CHSV((fade_h + (i * (255 / SETTING_NUM_LEDS))) % 255, 255, 255 / 2);
    }
    if (fade_h > 254) {
        fade_direction = -1; //reverse once we get to 254
    } else if (fade_h < 0) {
        fade_direction = 1;
    }

    fade_h += fade_direction;
    FastLED.show();
    delay(10);
}

// utility function to turn all the lights off.
void reset()
{
    for (int i = 0; i < SETTING_NUM_LEDS; i++) {
        leds[i] = CHSV(0, 0, 0);
    }
    FastLED.show();
}

void acid_cloud()
{
    // a modification of the rolling lightning which adds random colour. trippy.
    //iterate through every LED
    for (int i = 0; i < SETTING_NUM_LEDS; i++) {
        if (random(0, 100) > 90) {
            leds[i] = getBright();
        } else {
            leds[i] = CHSV(0, 0, 0);
        }
    }
    FastLED.show();
    delay(random(5, 100));
    reset();
}

void rolling()
{
    // a simple method where we go through every LED with 1/10 chance
    // of being turned on, up to 10 times, with a random delay wbetween each time
    for (int r = 0; r < random(2, 10); r++) {
        //iterate through every LED
        for (int i = 0; i < SETTING_NUM_LEDS; i++) {
            if (random(0, 100) > 90) {
                leds[i] = getBright();
            } else {
                //dont need reset as we're blacking out other LEDs her
                leds[i] = CHSV(0, 0, 0);
            }
        }
        FastLED.show();
        delay(random(5, 100));
        reset();
    }
}

void crack()
{
    //turn everything white briefly
    for (int i = 0; i < SETTING_NUM_LEDS; i++) {
        leds[i] = getBright();
    }
    FastLED.show();
    delay(random(10, 100));
    reset();
}

void thunderburst()
{

    // this thunder works by lighting two random lengths
    // of the strand from 10-20 pixels.
    int rs1 = random(0, SETTING_NUM_LEDS / 2);
    int rl1 = random(5, 10);
    int rs2 = random(rs1 + rl1, SETTING_NUM_LEDS);
    int rl2 = random(10, 20);

    CHSV on = getBright();

    //repeat this chosen strands a few times, adds a bit of realism
    for (int r = 0; r < random(3, 6); r++) {

        for (int i = 0; i < rl1; i++) {
            leds[i + rs1] = on;
        }

        if (rs2 + rl2 < SETTING_NUM_LEDS) {
            for (int i = 0; i < rl2; i++) {
                leds[i + rs2] = on;
            }
        }

        FastLED.show();
        //stay illuminated for a set time
        delay(random(10, 50));

        reset();
        delay(random(10, 50));
    }
}

// basically just a debug mode to show off the lightning in all its glory, no sound reactivity.
void constant_lightning()
{
    switch (random(1, 100)) {
        case 1:
            thunderburst();
            delay(random(10, 500));
            Serial.println("FYI: Thunderburst");
            break;

        case 2:
        case 3:
        case 4:
            rolling();
            Serial.println("FYI: Rolling");
            break;

        case 5:
            crack();
            delay(random(50, 250));
            Serial.println("FYI: Crack");
            break;
    }
}
