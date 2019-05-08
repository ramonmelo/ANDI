//  █████╗ ███╗   ██╗██████╗ ██╗
// ██╔══██╗████╗  ██║██╔══██╗██║
// ███████║██╔██╗ ██║██║  ██║██║
// ██╔══██║██║╚██╗██║██║  ██║██║
// ██║  ██║██║ ╚████║██████╔╝██║
// ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>

#include "musik.h"
#include "config.h"

// Control Members
int loops_length = 60;
int beats[32];

int current_instrument = 0;
int current_beat = 0;
unsigned long next_beat = 0;

int curr_music_length = 0;
int music_length[] = {2, 4, 8, 16, 32};
int size_music_length = (sizeof(music_length) / sizeof(music_length[0]));

float delayTime = BASE_DELAY;

TMRpcm tmrpcm;

// MUSIC

void initMusic()
{
	for (int i = 0; i < 32; i++)
	{
		beats[i] = random(1, 10);
	}
}

void playMusic()
{
	if (millis() > next_beat)
	{
		int musik = beats[current_beat] + current_instrument;

		char wavFile[8];
		strcpy_P(wavFile, wav_table[musik]);

		tmrpcm.play(wavFile);

		current_beat++;
		current_beat = current_beat % music_length[curr_music_length];

		next_beat = millis() + delayTime;
	}
}

void setup()
{
	Serial.begin(9600);
	while (!Serial)
	{
	}

	Serial.println("Starting");

	if (!SD.begin(SD_ChipSelectPin))
	{ // see if the card is present and can be initialized:
		Serial.println("SD fail");
		return; // don't do anything more if not
	}

	Serial.println("Ready");

	// Init input
	pinMode(INPUT_RANDOM, INPUT_PULLUP);

	// Init music
	tmrpcm.speakerPin = 9;
	randomSeed(analogRead(A0));
	initMusic();
}

void loop()
{
	// Read random initlizer
	int inputValue = digitalRead(INPUT_RANDOM);
	if (inputValue == LOW)
	{
		initMusic();
	}

	// Settings

	delayTime = BASE_DELAY + map(analogRead(INPUT_SPEED), 20, 1024, 0, 1200);
	curr_music_length = min(4, max(0, analogRead(INPUT_LENGTH) / 220));
	current_instrument = 10 * min(5, max(0, analogRead(INPUT_INSTRUMENT) / 190));;

	Serial.print(delayTime);
	Serial.print("  ");
	Serial.print(music_length[curr_music_length]);
	Serial.print("  ");
	Serial.println(current_instrument);

	// Play Music
	playMusic();

	delay(10);
}