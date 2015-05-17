#include "Quantity.h"

Quantity::Quantity (String n, unsigned int p) {
	name = n;
	port = p;
	clear();
}

void Quantity::clear() {
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

void Quantity::sampleSignal() {
	samp = analogRead(port);
	if (samp > max) {
    	max = samp;
  	}

	if ((samp > 511) && (prevsamp < 511)) {  // check if signal crosses zero => 511
    unsigned long crosstime = micros();
    
    if (reset) {
      reset=false;
   	}
    else {
      freqsum += crosstime - mrrz;
      numcycles++;
    }

    mrrz = crosstime;

    maxsum += max;
    max = 0;
  }

	prevsamp = samp;
}

void Quantity::getName(){
	Serial.println(name);	
}

void Quantity::getPort(){
	Serial.print("port: ");
	Serial.println(port);	
}

void Quantity::getSamp(){
	Serial.print("samp: ");
	Serial.println(samp);	
}

void Quantity::getPrevSamp(){
	Serial.print("prevsamp: ");
	Serial.println(prevsamp);	
}

void Quantity::getMax(){
	Serial.print("max: ");
	Serial.println(max);	
}

void Quantity::getRMS(){
	rms = (unsigned int) (0.707 * ( (double) maxsum/numcycles));
	Serial.println("rms: ");
	Serial.println(rms);
}

void Quantity::getMRRZ(){
	Serial.print("mrrz: ");
	Serial.println(mrrz);	
}

void Quantity::getFreq(){
	freq = (unsigned int) ( (double) freqsum/numcycles);
	Serial.print("freq: ");
	Serial.println(freq);	
}

void Quantity::getMaxSum(){
	Serial.print("maxsum: ");
	Serial.println(maxsum);	
}

void Quantity::getFreqSum(){
	Serial.print("freqsum: ");
	Serial.println(freqsum);
}

void Quantity::getNumCycles(){
	Serial.print("numcycles: ");
	Serial.println(numcycles);
}

void Quantity::getValues() {
	Serial.println(name);
	Serial.print("port: ");
	Serial.println(port);
	
	Serial.print("samp: ");
	Serial.println(samp);
	Serial.print("prevsamp: ");
	Serial.println(prevsamp);

	Serial.print("max: ");
	Serial.println(max);
	getRMS();

	Serial.print("mrrz: ");
	Serial.println(mrrz);
	getFreq();

	Serial.print("maxsum: ");
	Serial.println(maxsum);
	Serial.print("freqsum: ");
	Serial.println(freqsum);

	Serial.print("numcycles: ");
	Serial.println(numcycles);
	Serial.print("reset: ");
	Serial.println(reset ? "true" : "false");
}