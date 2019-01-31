#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>

#define SD_ChipSelectPin 4
#define INPUT_BUTTON 8
#define INPUT_DELAY A1
#define INPUT_LENGTH A2
#define INPUT_INSTRUMENT A3

#define BASE_DELAY 300.0

char loops[][8] = {
	"00.wav", "01.wav", "02.wav", "03.wav", "04.wav", "05.wav", "06.wav", "07.wav", "08.wav", "09.wav",
	"10.wav", "11.wav", "12.wav", "13.wav", "14.wav", "15.wav", "16.wav", "17.wav", "18.wav", "19.wav",
	"20.wav", "21.wav", "22.wav", "23.wav", "24.wav", "25.wav", "26.wav", "27.wav", "28.wav", "29.wav",
	"30.wav", "31.wav", "32.wav", "33.wav", "34.wav", "35.wav", "36.wav", "37.wav", "38.wav", "39.wav",
	"40.wav", "41.wav", "42.wav", "43.wav", "44.wav", "45.wav", "46.wav", "47.wav", "48.wav", "49.wav",
	"50.wav", "51.wav", "52.wav", "53.wav", "54.wav", "55.wav", "56.wav", "57.wav", "58.wav", "59.wav"};

int loops_length = 60;
int beats[32];

int current_instrument = 0;
int music_length = 8;
int current_beat = 0;
unsigned long next_beat = 0;

float delayTime = BASE_DELAY;

TMRpcm tmrpcm;

void initMusic()
{
	for (int i = 0; i < 32; i++)
	{
		beats[i] = random(10);
	}
}

void setup()
{
	tmrpcm.speakerPin = 9; //11 on Mega, 9 on Uno, Nano, etc

	Serial.begin(9600);

	if (!SD.begin(SD_ChipSelectPin))
	{ // see if the card is present and can be initialized:
		Serial.println("SD fail");
		return; // don't do anything more if not
	}

	Serial.println("Ready");

	analogReference(DEFAULT);

	randomSeed(analogRead(A0));

	pinMode(INPUT_BUTTON, INPUT_PULLUP);

	initMusic();
}

void loop()
{
	// Read random initlizer
	int inputValue = digitalRead(INPUT_BUTTON);
	if (inputValue == LOW)
	{
		initMusic();
	}

	// Settings

	delayTime = BASE_DELAY + map(analogRead(INPUT_DELAY), 0, 850, -200, 600);
	music_length = (int) ceil(pow(2, map(analogRead(INPUT_LENGTH), 0, 850, 1, 6)));
	current_instrument = 10 * (int) map(analogRead(INPUT_INSTRUMENT), 0, 850, 0, 6);

	// Play Music

	if (millis() > next_beat)
	{
		int musik = beats[current_beat] + current_instrument;

		tmrpcm.play(loops[musik]);

		current_beat++;
		current_beat = current_beat % music_length;

		next_beat = millis() + delayTime;
	}
}