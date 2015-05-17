#include "Quantity.h"

Quantity::Quantity (String n, unsigned int p) {
	port = p;
	reset();
}

Quantity::reset() {
	samp = 0;
	prevsamp = 0;
	max = 0;
	rms = 0;

	mrrz = 0; // micro/milli second value
	freq = 0;

	// variables per hour
	maxsum = 0; 
	freqsum = 0;
	numcycles = 0;
	reset = true;
}

Quantity::getValues() {
	Serial.println(name);
	Serial.print("port: ");
	Serial.println(port);
	
	Serial.print("samp: ");
	Serial.println(samp);
	Serial.print("prevsamp: ");
	Serial.println(prevsamp);

	Serial.print("max: ");
	Serial.println(max);
	Serial.print("rms: ");
	Serial.println(rms);

	Serial.print("mrrz: ");
	Serial.println(mrrz);
	Serial.print("freq: ");
	Serial.println(freq);

	Serial.print("maxsum: ");
	Serial.println(maxsum);
	Serial.print("freqsum: ");
	Serial.println(freqsum);

	Serial.print("numcycles: ");
	Serial.println(numcycles);
	Serial.print("reset: ");
	Serial.println(reset ? "true" : "false");
}