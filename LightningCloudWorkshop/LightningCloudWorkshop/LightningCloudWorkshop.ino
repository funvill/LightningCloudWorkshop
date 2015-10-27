#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

// Lightning constants 
#define LIGHTNING_STRIKE_FREQUENCY		 100  
#define LIGHTNING_STRIKE_FLASH_COUNT       4  
#define LIGHTNING_STRIKE_FLASH_DELAY      20  
#define LIGHTNING_STRIKE_SIZE_MIN		   2 
#define LIGHTNING_STRIKE_SIZE_MAX		   2 

// Lightning State machine 
#define LIGHTNING_STRIKE_STATE_READY				0  // Nothing is happening, We can do another LightingStrike if needed. 
#define LIGHTNING_STRIKE_STATE_BRIGHT_FLASH			1  // A bright blinding flash of light, full intensity
#define LIGHTNING_STRIKE_STATE_FLASH			    2  // A smaller less intence flash of light. 
#define LIGHTNING_STRIKE_STATE_DARK					3  // Everything is dark.
#define LIGHTNING_STRIKE_STATE_FLASH_WAIT		    4  // Wating for people to notice the change  

// Rainbow constants
#define RAINBOW_SPEED						10 

// Rolling 
#define ROLLING_SPEED_MIN					50
#define ROLLING_SPEED_MAX					200

// Crack 
#define CRACK_SPEED							50

#define CLOUD_STATECHANGE_FREQUENCY			1000 * 10
#define CLOUD_STATE_RAINBOW					1 
#define CLOUD_STATE_ROLLING					2
#define CLOUD_STATE_LIGHTING				3 
#define CLOUD_STATE_CRACK					4

/**
 * This class MUST never block. No delays of any type! Because of this we are going to use a stat machine. 
 * This class MUST be thread save. No static or global variables. Don't be a bad programer! 
 */
class Cloud {
private:

	unsigned int		m_cloudState;
	unsigned long		m_cloudStateTimer;
	unsigned long		m_timer;
	uint32_t			m_darkColor;

	// Start the lightning strike in the ready state. 
	unsigned int		m_lightingStrikeState; 
	unsigned int		m_lightingStrikeFlashCount;
	uint32_t			m_lightingStrikeBrightColor;

	unsigned long		lastLightingStrike = 0;
	
	
	bool			    lightingStrikeFlashing = true;
	int					lightingStrikeSize;
	int					lightingStrikeCenter;

	// Rainbow 
	unsigned int		m_cursor; 

public:
	Cloud() {
		this->m_cloudStateTimer = millis(); 
		this->m_cloudState = CLOUD_STATE_CRACK;
		this->Reset(); 
	}
	void Reset( ) {
		this->m_timer						= 0; // Each state will use this timer differently. 

		// Lighting specific. 
		this->m_lightingStrikeState			= LIGHTNING_STRIKE_STATE_READY;
		this->m_lightingStrikeFlashCount	= LIGHTNING_STRIKE_FLASH_COUNT;		
		this->m_darkColor					= strip.Color(0, 0, 0);


		unsigned long		lastLightingStrike = 0;
		
		bool			    lightingStrikeFlashing = true;
		int					lightingStrikeSize;
		int					lightingStrikeCenter;

		// Rainbow 
		this->m_cursor = 0;

		// Set everything to the dark color. 
		SetColor(m_darkColor);
	}

	void debug() {
		Serial.println("cloudState=" + String(m_cloudState) );
	}

	void loop() {
		switch (this->m_cloudState) 
		{
			default:
			case CLOUD_STATE_RAINBOW:
			{
				this->m_cloudState = CLOUD_STATE_RAINBOW;
				Rainbow();
				break;
			}
			/*
			case CLOUD_STATE_LIGHTING:
			{
				LightningStrike(); 
				break;
			}
			*/

			case CLOUD_STATE_ROLLING:
			{
				Rolling();
				break;
			}
			/*
			case CLOUD_STATE_CRACK:
			{
				Crack();
				break;
			}
			*/
		}

		// Every two mins, switch modes 
		if (this->m_cloudStateTimer + CLOUD_STATECHANGE_FREQUENCY < millis()) {
			this->m_cloudStateTimer = millis();
			this->m_cloudState++; 
			this->Reset(); 
		}

	}


	
	/* Sets the entire string to a certin color */
	void SetColor(uint32_t c) {
		for (uint16_t i = 0; i<strip.numPixels(); i++) {
			strip.setPixelColor(i, c);
		}
	}

	uint32_t GetBrightColor() {
		return Wheel(random(0, 255)); // Random color 
		// return strip.Color(128, 128, 200); // Blueish gray.  
	}

	// Input a value 0 to 255 to get a color value.
	// The colours are a transition r - g - b - back to r.
	uint32_t Wheel(byte WheelPos) {
		WheelPos = 255 - WheelPos;
		if (WheelPos < 85) {
			return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
		}
		else if (WheelPos < 170) {
			WheelPos -= 85;
			return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
		}
		else {
			WheelPos -= 170;
			return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
		}
	}

	void Rainbow() {
		if (this->m_timer > millis()) {
			return; // Wait some more time. 
		}
		this->m_timer = RAINBOW_SPEED + millis();
		this->m_cursor++;
		
		for (uint16_t pixel = 0; pixel<strip.numPixels(); pixel++) {
			strip.setPixelColor(pixel, Wheel((pixel + this->m_cursor) & 255));
		}
	}


	void Rolling() {

		if (this->m_timer > millis()) {
			return;
		}
		this->m_timer = millis() + random(ROLLING_SPEED_MIN, ROLLING_SPEED_MAX);


		for (uint16_t pixel = 0; pixel<strip.numPixels(); pixel++) {
			if (random(0, 100) > 90) {
				strip.setPixelColor(pixel, GetBrightColor() );
			}
			else {
				strip.setPixelColor(pixel, this->m_darkColor);
			}
		}
	}

	void Crack() {
		if (this->m_timer > millis()) {
			return;
		}
		this->m_timer = millis() + CRACK_SPEED ;
		this->m_cursor++; 

		if (this->m_cursor % 2 == 0) {
			SetColor(GetBrightColor());
		}
		else {
			SetColor(this->m_darkColor);
		}
	}

	void LightningStrike()
	{
		switch (m_lightingStrikeState)
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
				m_lightingStrikeFlashCount = 0;

				// Choose a random size of the lightning strike 
				lightingStrikeSize = random(LIGHTNING_STRIKE_SIZE_MIN, LIGHTNING_STRIKE_SIZE_MAX);

				// Choose a random section of the strip.  
				// Ensure that the strike is shot cut off by the end of the strip length. 
				lightingStrikeCenter = random((lightingStrikeSize / 2), strip.numPixels() - (lightingStrikeSize / 2));

				this->m_lightingStrikeBrightColor = GetBrightColor();


				SetColor(m_darkColor);
				lightingStrikeFlashing = false;

				// Start the lightning sequence. 
				m_lightingStrikeState = LIGHTNING_STRIKE_STATE_BRIGHT_FLASH;
				break;
			}
			case LIGHTNING_STRIKE_STATE_DARK:
			{
				for (int pixel = lightingStrikeCenter - (lightingStrikeSize / 2); pixel < lightingStrikeCenter + (lightingStrikeSize / 2); pixel++) {
					strip.setPixelColor(pixel, m_darkColor);
				}
				lightingStrikeFlashing = false;
				m_lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH_WAIT;
				break;
			}

			case LIGHTNING_STRIKE_STATE_BRIGHT_FLASH:
			{
				// Flash cursor part of the LED strip 
				for (int pixel = lightingStrikeCenter - (lightingStrikeSize / 2); pixel < lightingStrikeCenter + (lightingStrikeSize / 2); pixel++) {
					strip.setPixelColor(pixel, m_lightingStrikeBrightColor);
				}
				lightingStrikeFlashing = true;
				m_lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH_WAIT;
				break;
			}

			case LIGHTNING_STRIKE_STATE_FLASH_WAIT:
			{
				if (lightingStrikeFlashing) {
					if (lastLightingStrike + LIGHTNING_STRIKE_FLASH_DELAY + (LIGHTNING_STRIKE_FLASH_DELAY * m_lightingStrikeFlashCount) > millis()) {
						// We are waiting on the flash to be noticed before continuning. 
						break;
					}

					m_lightingStrikeState = LIGHTNING_STRIKE_STATE_DARK;
				}
				else {

					if (lastLightingStrike + LIGHTNING_STRIKE_FLASH_DELAY + (LIGHTNING_STRIKE_FLASH_DELAY * (m_lightingStrikeFlashCount + 1)) > millis()) {
						// We are waiting on the flash to be noticed before continuning. 
						break;
					}

					m_lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH;
					m_lightingStrikeFlashCount++;
				}

				break;
			}

			case LIGHTNING_STRIKE_STATE_FLASH:
			{
				if (m_lightingStrikeFlashCount > LIGHTNING_STRIKE_FLASH_COUNT) {
					// We are all done 
					for (int pixel = lightingStrikeCenter - (lightingStrikeSize / 2); pixel < lightingStrikeCenter + (lightingStrikeSize / 2); pixel++) {
						strip.setPixelColor(pixel, m_darkColor);
					}
					m_lightingStrikeState = LIGHTNING_STRIKE_STATE_READY;
					break;
				}


				// Make a less intence flash 
				// Flash cursor part of the LED strip 
				for (int pixel = lightingStrikeCenter - (lightingStrikeSize / 2); pixel < lightingStrikeCenter + (lightingStrikeSize / 2); pixel++) {
					strip.setPixelColor(pixel, m_lightingStrikeBrightColor);
				}
				lightingStrikeFlashing = true;
				m_lightingStrikeState = LIGHTNING_STRIKE_STATE_FLASH_WAIT;
				break;
			}
		}
	}
};



















#define FLUFFY_CLOUD_MAX	10 

Cloud fluffyCloud[FLUFFY_CLOUD_MAX ];

void setup() {
	// strip.setBrightness(255/6);
	strip.begin();
	strip.show(); // Initialize all pixels to 'off'

	Serial.begin(115200);
	Serial.println("Lightning test");
}

void loop() {

	for (int offset = 0; offset < FLUFFY_CLOUD_MAX; offset++) {
		fluffyCloud[offset].loop();
	}

	strip.show();
	delay(10); // Save some batteries and sleep for a bit. 
}
