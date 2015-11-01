/*
Lighting Cloud Mood Lamp By James Bruce
View the full tutorial and build guide at http://www.makeuseof.com/
Sound sampling code originally by Adafruit Industries.  Distributed under the BSD license. 
This paragraph must be included in any redistribution.
*/

/**
* Modified by Steven Smethurst
* Orginal: https://github.com/jamesabruce/cloudlamp/blob/master/thundercloud/thundercloud.ino
*
* Changes 
* - Added rainbow mode
* - Removed sound activation (don't have a mic) 
* 
* More information and pictures can be found here 
* https://talk.hackspace.ca/t/lightning-cloud-hat-halloween-2015-project/3256?u=funvill
*/

#include <Wire.h>
#include "FastLED.h"

// How many leds in your strip?
#define NUM_LEDS 30
#define DATA_PIN 6



// Define the array of leds
CRGB leds[NUM_LEDS];

bool randomColor; 

CHSV getBright() {
	if (randomColor ) {
		// Random 
		return CHSV(random(0, 255), 255, 255 / 3);
	}
	else {
		return CHSV(145, 125, 255/2); // Light blue 
	}
}

void setup() {
	randomColor = true; 
	// this line sets the LED strip type - refer fastLED documeantion for more details https://github.com/FastLED/FastLED
	FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

#define TIME_MODE_CHANGE		(1000 * 60)
#define TIME_COLOR_MODE_CHANGE	(1000 * 60 * 5 )
void loop() {

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
	}
	else {
		constant_lightning();
	}

	return;
}
// used to make basic mood lamp colour fading feature


void colour_fade() {
	static int fade_h;
	static int fade_direction = 1;

	// mood mood lamp that cycles through colours
	for (int i = 0; i<NUM_LEDS; i++) {
		leds[i] = CHSV((fade_h + (i* (255 / NUM_LEDS))) % 255, 255, 255 / 2);
	}
	if (fade_h >254) {
		fade_direction = -1; //reverse once we get to 254
	}
	else if (fade_h < 0) {
		fade_direction = 1;
	}

	fade_h += fade_direction;
	FastLED.show();
	delay(10);
}




// utility function to turn all the lights off.  
void reset() {
	for (int i = 0; i<NUM_LEDS; i++) {
		leds[i] = CHSV(0, 0, 0);
	}
	FastLED.show();

}

void acid_cloud() {
	// a modification of the rolling lightning which adds random colour. trippy. 
	//iterate through every LED
	for (int i = 0; i<NUM_LEDS; i++) {
		if (random(0, 100)>90) {
			leds[i] = getBright();
		}
		else {
			leds[i] = CHSV(0, 0, 0);
		}
	}
	FastLED.show();
	delay(random(5, 100));
	reset();
}

void rolling() {
	// a simple method where we go through every LED with 1/10 chance
	// of being turned on, up to 10 times, with a random delay wbetween each time
	for (int r = 0; r<random(2, 10); r++) {
		//iterate through every LED
		for (int i = 0; i<NUM_LEDS; i++) {
			if (random(0, 100)>90) {
				leds[i] = getBright();
			}
			else {
				//dont need reset as we're blacking out other LEDs her 
				leds[i] = CHSV(0, 0, 0);
			}
		}
		FastLED.show();
		delay(random(5, 100));
		reset();
	}
}

void crack() {
	//turn everything white briefly
	for (int i = 0; i<NUM_LEDS; i++) {
		leds[i] = getBright();
	}
	FastLED.show();
	delay(random(10, 100));
	reset();
}

void thunderburst() {

	// this thunder works by lighting two random lengths
	// of the strand from 10-20 pixels. 
	int rs1 = random(0, NUM_LEDS / 2);
	int rl1 = random(5, 10);
	int rs2 = random(rs1 + rl1, NUM_LEDS);
	int rl2 = random(10, 20);

	CHSV on = getBright();

	//repeat this chosen strands a few times, adds a bit of realism
	for (int r = 0; r<random(3, 6); r++) {

		for (int i = 0; i< rl1; i++) {
			leds[i + rs1] = on;
		}

		if (rs2 + rl2 < NUM_LEDS) {
			for (int i = 0; i< rl2; i++) {
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
void constant_lightning() {
	switch (random(1, 100)) {
	case 1:
		thunderburst();
		delay(random(10, 500));
		Serial.println("Thunderburst");
		break;

	case 2:
	case 3:
	case 4:
		rolling();
		Serial.println("Rolling");
		break;

	case 5:
		crack();
		delay(random(50, 250));
		Serial.println("Crack");
		break;
	}
}

