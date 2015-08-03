#include <MemoryFree.h>

#include <IRremote.h>
#include <IRremoteInt.h>

#include <U8glib.h>

#include <VirtualWire.h>

#define SERIAL
#define RADIO
#define IR
#define REMOTE

#include <OpenBattleCore.h>
#include <Generated.h>
#include <RadioRxTx.h>
#include <InfraredRxTx.h>

#include "ButtonInput.h"

#include <Adafruit_NeoPixel.h>

#define BUTTON_LEFT		1
#define BUTTON_MID		2
#define BUTTON_RIGHT	3

#define RADIO_TX		2
#define RADIO_RX		4
#if defined(__AVR_ATmega2560__)
#define IR_TX			9
#else
#define IR_TX			3 // Hardcoded in library
#endif
#define IR_RX			5 // Not used
#define LED_PIN			6

#define CHANNEL_SERIAL	0
#define CHANNEL_RADIO	1

using namespace OpenBattle;

U8GLIB_SSD1306_128X64 screen(U8G_I2C_OPT_NONE | U8G_I2C_OPT_DEV_0); // I2C / TWI
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, LED_PIN, NEO_GRB + NEO_KHZ800);
ButtonInput input(A3, 3);

//IRsend irsend;
InfraredRxTx infrared;
RadioRxTx radio;

Protocol protocol(2, 4);

String status = "READY";
bool dirty = true;

GameParameters gameParams;

#include "Scripts.h"
ScriptRunner runner(&script);

#include "Menus.h"

class MenuInterface : public IMenuInterface {
public:
	MenuInterface(Menu* menu) :
		IMenuInterface(menu)
	{
	}

	void drawMenu(U8GLIB_SSD1306_128X64* display, uint16_t y, uint8_t lines) {
		display->setFont(u8g_font_tpss);
		display->setFontRefHeightText();
		display->setFontPosTop();

		Menu *menu = this->current();
		uint8_t lineHeight = display->getFontLineSpacing() + 2;
		uint8_t w = display->getWidth();
		uint8_t offset = max(0, (int)this->selectedIndex - lines + 1);
		for (uint8_t i = offset; i < menu->numSubmenus && (i - offset) < lines; i++) {
			display->setDefaultForegroundColor();
			if (i == this->selectedIndex) {
				display->drawBox(0, y + (i - offset) * lineHeight, w, lineHeight);
				display->setDefaultBackgroundColor();
			}
			display->drawStr(1, y + (i - offset) * lineHeight + 1, menu->submenus[i]->title);
		}
	}
};

MenuInterface navigator(&menu);

unsigned long time = 0;
unsigned long lastRefresh = 0;

class SerialStream : public OpenBattle::Stream {
public:
	void writeByte(unsigned char c) {
		Serial.write(c);
	}
};

void setup() {
	Serial.begin(9600);

	radio.begin(RADIO_RX, RADIO_TX);

#if defined(__AVR_ATmega2560__)
	pinMode(9, OUTPUT);
	digitalWrite(9, LOW);
#endif
	infrared.begin(IR_RX, 38);

	// Configure input buttons
	input.addButton(BUTTON_LEFT, 240);
	input.addButton(BUTTON_MID, 378);
	input.addButton(BUTTON_RIGHT, 475);
	input.begin();

	pixels.begin();
	pixels.show();

	screen.begin();

	// Register all core message factories
	REGISTER(protocol);

	InformationMessage msg;
	msg.message = "STARTED";
	Serializer serializer;
	SerialStream stream;
	msg.serialize(&serializer, &stream);
}

void loop() {
	unsigned long now = millis();

	runner.update(now);
	pixels.show();

	int val = input.button();

	switch (val)
	{
	case BUTTON_LEFT:
		navigator.prev();
		dirty = true;
		break;
	case BUTTON_MID:
		navigator.select();
		dirty = true;
		break;
	case BUTTON_RIGHT:
		navigator.next();
		dirty = true;
		break;
	default:
		break;
	}

	status = String(val) + " / " + String(input.value()) + " / " + String(input.flags, 2);

	if (mode == MODE_TEST_INFRARED && now - lastRefresh > 3000) {
		pixels.setPixelColor(1, 0x20, 0x00, 0x20); // Flash a light to show transmitting
		pixels.show();

		unsigned char data[7] = { 0x01, 0xff, 0x04, 0x32, 0x10, 0x00, 0x10 };
		infrared.write(data, 7);

		pixels.setPixelColor(1, 0x00, 0x00, 0x00);
		pixels.show();

		lastRefresh = now;
		//dirty = true;
	}
	if (mode == MODE_TEST_RADIO && now - lastRefresh > 3000) {
		pixels.setPixelColor(1, 0x20, 0x10, 0x00); // Flash a light to show transmitting
		pixels.show();

		unsigned char data[7] = { 0x01, 0xff, 0x04, 0x32, 0x10, 0x05, 0x00 };
		radio.write(data, 7);

		pixels.setPixelColor(1, 0x00, 0x00, 0x00);
		pixels.show();

		lastRefresh = now;
		//dirty = true;
	}

	while (Serial.available() > 0)	{
		int data = Serial.read();
		if (data >= 0) {
			protocol.consume(data, CHANNEL_SERIAL);
			//Serial.println(data, HEX);
		}
	}
	while (radio.available() > 0) {
		int data = radio.read();
		if (data >= 0) {
			protocol.consume(data, CHANNEL_RADIO);
		}
	}

	if (dirty) {
		screen.firstPage();
		do {
			screen.setFont(u8g_font_04b_03b);
			screen.setFontPosTop();
			screen.setDefaultForegroundColor();
			//screen.drawStr(0, 0, "OpenBattle Remote v0.01.002");

			//screen.setFontPosBottom();
			screen.drawStr(0, 0, ("MEM: " + String(freeMemory())).c_str());
			//screen.drawStr(50, 0, String(script.step()).c_str());
			screen.drawStr(112, 0, (String(readVcc() / 1000.0, 1) + "V").c_str());

			//screen.setFont(u8g_font_tpss);
			//screen.setFontPosTop();
			//screen.drawStr(0, 15, status.c_str());
			navigator.drawMenu(&screen, 16, 3);

		} while (screen.nextPage());
		dirty = false;
	}
	time = now;
}

long readVcc() {
	// Read 1.1V reference against AVcc
	// set the reference to Vcc and the measurement to the internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
	ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
	ADMUX = _BV(MUX3) | _BV(MUX2);
#else
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif  

	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Start conversion
	while (bit_is_set(ADCSRA, ADSC)); // measuring

	uint8_t low = ADCL; // must read ADCL first - it then locks ADCH  
	uint8_t high = ADCH; // unlocks both

	long result = (high << 8) | low;

	result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
	return result; // Vcc in millivolts
}
