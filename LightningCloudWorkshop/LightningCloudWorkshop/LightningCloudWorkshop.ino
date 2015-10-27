#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(30, PIN, NEO_GRB + NEO_KHZ800);


void setup() {
	// strip.setBrightness(255/6);
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'

	Serial.begin(115200);
	Serial.println("Lightning test");


}

void loop() {


	LightningStrike();
	strip.show();
	delay(10); // Save some batteries and sleep for a bit. 
}






#define LIGHTNING_STRIKE_FREQUENCY		 200  // How often a lighting strike will occure in ms 
#define LIGHTNING_STRIKE_FLASH_COUNT       4  
#define LIGHTNING_STRIKE_FLASH_DELAY      50  
#define LIGHTNING_STRIKE_SIZE_MIN		   5 
#define LIGHTNING_STRIKE_SIZE_MAX		  10 

// State machine 
#define LIGHTNING_STRIKE_STATE_READY				0  // Nothing is happening, We can do another LightingStrike if needed. 
#define LIGHTNING_STRIKE_STATE_BRIGHT_FLASH			1  // A bright blinding flash of light, full intensity
#define LIGHTNING_STRIKE_STATE_FLASH			    2  // A smaller less intence flash of light. 
#define LIGHTNING_STRIKE_STATE_DARK					3  // Everything is dark.
#define LIGHTNING_STRIKE_STATE_FLASH_WAIT		    4  // Wating for people to notice the change  

void LightningStrike()
{
	// Start the lightning strike in the ready state. 
	static unsigned int		lightingStrikeState = LIGHTNING_STRIKE_STATE_READY;
	static unsigned long	lastLightingStrike = 0;
	static unsigned int		lightingStrikeFlashCount = LIGHTNING_STRIKE_FLASH_COUNT;
	static bool			    lightingStrikeFlashing = true;
	static uint32_t			brightColor = strip.Color(128, 128, 200);
	static uint32_t			darkColor = strip.Color(0, 0, 0);
	static int				lightingStrikeSize;
	static int				lightingStrikeCenter;

	Serial.println("State=" + String(lightingStrikeState) + " Count=" + String(lightingStrikeFlashCount));

	switch (lightingStrikeState)
	{
	case LIGHTNING_STRIKE_STATE_READY:
	{
		// We only want to make lighting every once in a while. 
		if (lastLightingStrike + LIGHTNING_STRIKE_FREQUENCY > millis()) {
			// We had a Lighting Strike recently, lets wait a bit. 
			break;
		}
		lastLightingStrike = millis();

		// reset the flash count. 
		lightingStrikeFlashCount = 0;

		// Choose a random size of the lightning strike 
		lightingStrikeSize = random(LIGHTNING_STRIKE_SIZE_MIN, LIGHTNING_STRIKE_SIZE_MAX);

		// Choose a random section of the strip.  
		// Ensure that the strike is shot cut off by the end of the strip length. 
		lightingStrikeCenter = random((lightingStrikeSize / 2), strip.numPixels() - (lightingStrikeSize / 2));

		SetColor(darkColor);
		lightingStrikeFlashing = false;

		// Start the lightning sequence. 
		lightingStrikeState = LIGHTNING_STRIKE_STATE_BRIGHT_FLASH;
		break;
	}
	case LIGHTNING_STRIKE_STATE_DARK:
	{
		SetColor(darkColor);
		lightingStrikeFlashing = false;
		lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH_WAIT;
		break;
	}

	case LIGHTNING_STRIKE_STATE_BRIGHT_FLASH:
	{
		// Flash cursor part of the LED strip 
		for (int pixel = lightingStrikeCenter - (lightingStrikeSize / 2); pixel < lightingStrikeCenter + (lightingStrikeSize / 2); pixel++) {
			strip.setPixelColor(pixel, brightColor);
		}
		lightingStrikeFlashing = true;
		lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH_WAIT;
		break;
	}

	case LIGHTNING_STRIKE_STATE_FLASH_WAIT:
	{
		if (lightingStrikeFlashing) {
			if (lastLightingStrike + LIGHTNING_STRIKE_FLASH_DELAY + (LIGHTNING_STRIKE_FLASH_DELAY * lightingStrikeFlashCount) > millis()) {
				// We are waiting on the flash to be noticed before continuning. 
				break;
			}

			lightingStrikeState = LIGHTNING_STRIKE_STATE_DARK;
		}
		else {

			if (lastLightingStrike + LIGHTNING_STRIKE_FLASH_DELAY + (LIGHTNING_STRIKE_FLASH_DELAY * (lightingStrikeFlashCount + 1)) > millis()) {
				// We are waiting on the flash to be noticed before continuning. 
				break;
			}

			lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH;
			lightingStrikeFlashCount++;
		}

		break;
	}

	case LIGHTNING_STRIKE_STATE_FLASH:
	{
		if (lightingStrikeFlashCount > LIGHTNING_STRIKE_FLASH_COUNT) {
			// We are all done 
			SetColor(darkColor);
			lightingStrikeState = LIGHTNING_STRIKE_STATE_READY;
			break;
		}


		// Make a less intence flash 
		// Flash cursor part of the LED strip 
		for (int pixel = lightingStrikeCenter - (lightingStrikeSize / 2); pixel < lightingStrikeCenter + (lightingStrikeSize / 2); pixel++) {
			strip.setPixelColor(pixel, brightColor);
		}
		lightingStrikeFlashing = true;
		lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH_WAIT;
		break;
	}
	}
}

void SetColor(uint32_t c) {
	for (uint16_t i = 0; i<strip.numPixels(); i++) {
		strip.setPixelColor(i, c);
	}
}