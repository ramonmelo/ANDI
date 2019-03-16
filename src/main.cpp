//  █████╗ ███╗   ██╗██████╗ ██╗
// ██╔══██╗████╗  ██║██╔══██╗██║
// ███████║██╔██╗ ██║██║  ██║██║
// ██╔══██║██║╚██╗██║██║  ██║██║
// ██║  ██║██║ ╚████║██████╔╝██║
// ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝

#define SD_ChipSelectPin 4
#define BOUNCE_WITH_PROMPT_DETECTION

#define ROT_SW 2
#define ROT_DT 3
#define ROT_CLK 5

#define BASE_DELAY 300.0
#define MAX_ITEMS_IDX 3

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <TMRpcm.h>
#include <Encoder.h>
#include <Adafruit_SSD1306.h>
// #include <Bounce2.h>

// MEMBERS

// 'page1', 32x32px
const unsigned char myBitmap[] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
	0x00, 0x40, 0x78, 0x00, 0x00, 0xf8, 0xec, 0x00, 0x01, 0x9f, 0x86, 0x00, 0x01, 0x87, 0x06, 0x00,
	0x03, 0x0f, 0x03, 0x00, 0x03, 0x19, 0x81, 0x80, 0x06, 0x30, 0xc0, 0xc0, 0x0c, 0x60, 0x70, 0xc0,
	0x0c, 0xc0, 0x38, 0x60, 0x19, 0x80, 0x0c, 0x30, 0x1b, 0x00, 0x06, 0x30, 0x31, 0x80, 0x03, 0x30,
	0x30, 0xc0, 0x01, 0x60, 0x18, 0x60, 0x02, 0x60, 0x0c, 0x60, 0x04, 0x60, 0x06, 0x30, 0x08, 0xc0,
	0x03, 0x18, 0x10, 0xc0, 0x01, 0x98, 0x21, 0x80, 0x00, 0xcc, 0x43, 0x00, 0x00, 0x66, 0x8e, 0x00,
	0x00, 0x33, 0x1c, 0x00, 0x00, 0x30, 0x70, 0x00, 0x00, 0x18, 0xe0, 0x00, 0x00, 0x0d, 0x80, 0x00,
	0x00, 0x07, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char loops[][8] = {
	"00.wav", "01.wav", "02.wav", "03.wav", "04.wav", "05.wav", "06.wav", "07.wav", "08.wav", "09.wav",
	"10.wav", "11.wav", "12.wav", "13.wav", "14.wav", "15.wav", "16.wav", "17.wav", "18.wav", "19.wav",
	"20.wav", "21.wav", "22.wav", "23.wav", "24.wav", "25.wav", "26.wav", "27.wav", "28.wav", "29.wav",
	"30.wav", "31.wav", "32.wav", "33.wav", "34.wav", "35.wav", "36.wav", "37.wav", "38.wav", "39.wav",
	"40.wav", "41.wav", "42.wav", "43.wav", "44.wav", "45.wav", "46.wav", "47.wav", "48.wav", "49.wav",
	"50.wav", "51.wav", "52.wav", "53.wav", "54.wav", "55.wav", "56.wav", "57.wav", "58.wav", "59.wav"};

Encoder selector(ROT_DT, ROT_CLK);
// Bounce buttonSW = Bounce();

int16_t oldEncPos, encPos;
uint8_t buttonState;

Adafruit_SSD1306 display;

// Control Members
volatile bool active = false;
int curr_item = 0;

int loops_length = 60;
int beats[32];

int current_instrument = 0;
int current_beat = 0;
unsigned long next_beat = 0;

int music_length[] = {2, 4, 8, 16, 32};
int size_music_length = (sizeof(music_length) / sizeof(music_length[0]));
int curr_music_length = 0;

float delayTime = BASE_DELAY;

TMRpcm tmrpcm;

#pragma region Utils

// DISPLAY

void write(int x, int y, int size, String text)
{
	display.setCursor(x, y);
	display.setTextSize(size);
	display.setTextColor(WHITE);
	display.print(text);
}

void point(int x, int y) { display.drawPixel(x, y, WHITE); }
void line(int x1, int y1, int x2, int y2) { display.drawLine(x1, y1, x2, y2, WHITE); }
void circle(int x, int y, int r) { display.drawCircle(x, y, r, WHITE); }
void circleFill(int x, int y, int r) { display.fillCircle(x, y, r, WHITE); }

void quad(bool selected, int x, int y, int w, int h)
{
	if (selected)
	{
		display.drawRoundRect(x, y, w, h, 4, WHITE);
	}
	else
	{
		display.drawRoundRect(x, y, w, h, 0, WHITE);
	}
}

void circleArc(int x, int y, int r, float percent)
{
	circle(x, y, r);
	circleFill(x, y, (uint16_t)r * percent);
}

void dots(int total)
{
	int diff_x = 3;
	int diff_y = 3;
	int space = 5;
	int x = 0;
	int y = 0;

	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{

			if ((i == 0 && j == 0) ||
				(i == 0 && j == 5) ||
				(i == 5 && j == 0) ||
				(i == 5 && j == 5))
			{
				continue;
			}

			x = diff_x + (i * space);
			y = diff_y + (j * space);

			point(x, y);

			if (total > 0)
			{
				circleFill(x, y, 1);
				total--;
			}
		}
	}
}

void updateView(bool selected, int item, int size, float velocity)
{
	display.clearDisplay();

	// Show quad around the selected item
	quad(selected, item * 32, 0, 32, 32);

	dots(size);
	circleArc(display.width(), display.height() / 2, 30, velocity);

	display.drawBitmap(30, 0, myBitmap, 32, 32, WHITE);
	display.drawBitmap(30 + 32, 0, myBitmap, 32, 32, WHITE);

	// write(0, 0, 1, "# " + String(current) + "/" + String(total));
	// write(60, 0, 1, "AVG: " + String(avg));
	// write(38, 9, 3, String(value));

	// display.drawLine(50, display.height() - 1, display.width() - 1, display.height() - 1, WHITE);
	display.display();
}

// MUSIC

void initMusic()
{
	for (int i = 0; i < 32; i++)
	{
		beats[i] = random(10);
	}
}

void playMusic()
{
	if (millis() > next_beat)
	{
		int musik = beats[current_beat] + current_instrument;

		tmrpcm.play(loops[musik]);

		current_beat++;
		current_beat = current_beat % music_length[curr_music_length];

		next_beat = millis() + delayTime;
	}
}

// Control

volatile unsigned long last_interrupt_time = 0;
void changeSelect()
{
	unsigned long interrupt_time = millis();
	// If interrupts come faster than 200ms, assume it's a bounce and ignore
	if (interrupt_time - last_interrupt_time > 200)
	{
		active = !active;
	}
	last_interrupt_time = interrupt_time;
}

long newPosition = 0;
long oldPosition = 0;
int res = 0;

int readSelector(int curr, int max)
{
	newPosition = selector.read();
	res = 0;

	if (newPosition != oldPosition)
	{
		res = newPosition > oldPosition ? 1 : -1;
		oldPosition = newPosition;
	}

	curr += res;
	return curr < 0 ? 0 : curr > max ? max : curr;
}

#pragma endregion

void setup()
{
	Serial.begin(9600);

	if (!SD.begin(SD_ChipSelectPin))
	{ // see if the card is present and can be initialized:
		Serial.println("SD fail");
		return; // don't do anything more if not
	}

	Serial.println("Ready");

	// Init control
	pinMode(ROT_SW, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(ROT_SW), changeSelect, CHANGE);

	// buttonSW.attach(ROT_SW, INPUT_PULLUP);
	// buttonSW.interval(50); // interval in ms

	// Init display
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

	// Init music
	tmrpcm.speakerPin = 9;
	randomSeed(analogRead(A0));
	initMusic();
}

void loop()
{
	// Update view
	updateView(active, curr_item, music_length[curr_music_length], 0.5);

	// Control
	// curr_item = readSelector(curr_item, MAX_ITEMS_IDX);
	// Serial.println(curr_item);

	if (active)
	{
		switch (curr_item)
		{
		case 0:
			curr_music_length = readSelector(curr_music_length, size_music_length);
			break;
		}
	}
	else
	{
		curr_item = readSelector(curr_item, MAX_ITEMS_IDX);
	}

	// Read random initlizer
	// int inputValue = digitalRead(INPUT_BUTTON);
	// if (inputValue == LOW)
	// {
	// 	initMusic();
	// }

	// Settings

	// delayTime = BASE_DELAY + map(analogRead(INPUT_DELAY), 0, 850, -200, 600);
	// music_length = (int) ceil(pow(2, map(analogRead(INPUT_LENGTH), 0, 850, 1, 6)));
	// current_instrument = 10 * (int) map(analogRead(INPUT_INSTRUMENT), 0, 850, 0, 6);

	// Play Music
	// playMusic();

	delay(10);
}