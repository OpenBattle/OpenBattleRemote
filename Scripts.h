void ledWhite() {
	pixels.setPixelColor(0, 0x10, 0x10, 0x10);
}

void ledRed() {
	pixels.setPixelColor(0, 0x10, 0x00, 0x00);
}

void ledGreen() {
	pixels.setPixelColor(0, 0x00, 0x10, 0x00);
}

void ledPurple() {
	pixels.setPixelColor(0, 0x10, 0x00, 0x10);
}

void ledOff() {
	pixels.setPixelColor(0, 0x00, 0x00, 0x00);
}

Script script(new Step*[6] {
	new Step(10, &ledRed),
		new Step(100, &ledOff),
		new Step(10, &ledRed),
		new Step(380, &ledOff),
		new Step(10, &ledWhite),
		new Step(990, &ledOff)
}, 6);
