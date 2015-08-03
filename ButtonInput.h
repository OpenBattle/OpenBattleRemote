#ifndef _BUTTONINPUT
#define _BUTTONINPUT

#include "Arduino.h"

class ButtonInput {
	class buttonInfo {
	public:
		int id;
		int value;
		uint8_t flag;
		buttonInfo(int id, int value, uint8_t flag) : id(id), value(value), flag(flag) {}
	};

	uint8_t pin;
	uint8_t threshold;
	buttonInfo** buttons;
	uint8_t num = 0;
public:
	uint8_t flags = 0;
	ButtonInput(uint8_t inputPin, uint8_t numButtons, uint8_t threshold = 10) :
		pin(inputPin), threshold(threshold)
	{
		this->buttons = new buttonInfo*[numButtons];
	}
	void begin() {
		pinMode(this->pin, INPUT);
		digitalWrite(this->pin, HIGH);
	}
	int value() {
		return analogRead(this->pin);
	}
	int button() {
		int val = this->value();
		for (uint8_t i = 0; i < this->num; i++) {
			if (val >= this->buttons[i]->value - this->threshold && val <= this->buttons[i]->value + this->threshold) {
				if ((this->flags & this->buttons[i]->flag) != this->buttons[i]->flag) {
					this->flags |= this->buttons[i]->flag;
					return this->buttons[i]->id;
				}
			}
			else {
				this->flags &= ~this->buttons[i]->flag;
			}
		}
		return -1;
	}
	void addButton(int id, int value) {
		this->buttons[this->num] = new buttonInfo(id, value, 1U << this->num);
		this->num++;
	}
};

#endif
