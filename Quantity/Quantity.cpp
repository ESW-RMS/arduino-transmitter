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
	period = 0;

	// variables per hour
	maxsum = 0; 
	periodsum = 0;
	numcycles = 0;
	reset = true;
}

void Quantity::sampleSignal() {
	samp = analogRead(port);
	if (samp > max) {
    	max = samp;
  	}

	if ((samp >= ZERO) && (prevsamp < ZERO)) {  // check if signal crosses zero => 511
    unsigned long crosstime = micros();
    
    if (reset) {
      reset=false;
   	}
    else {
      periodsum += crosstime - mrrz;
      numcycles++;
    }

    mrrz = crosstime;

    maxsum += max;
    max = 0;
  }

	prevsamp = samp;
}

void Quantity::printName(){
	Serial.println(name);	
}

String Quantity::getName() {
	return name;
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

unsigned int Quantity::getRMS(){
	rms = (unsigned int) (0.707 * ( (double) maxsum/numcycles - ZERO));
	return rms;
}

void Quantity::printRMS() {
	Serial.print("rms: ");
	Serial.println(rms);	
}

unsigned long Quantity::getMRRZ(){
	return mrrz;
}

void Quantity::printMRRZ(){
	Serial.print("mrrz: ");
	Serial.println(mrrz);	
}

unsigned int Quantity::getPeriod(){
	period = (unsigned int) ( (double) periodsum/numcycles);
	return period;
}

void Quantity::printPeriod() {
	Serial.print("period: ");
	Serial.println(period);	
	
}

void Quantity::getMaxSum(){
	Serial.print("maxsum: ");
	Serial.println(maxsum);	
}

void Quantity::getPeriodSum(){
	Serial.print("periodsum: ");
	Serial.println(periodsum);
}

unsigned int Quantity::getNumCycles(){
	Serial.print("numcycles: ");
	Serial.println(numcycles);
	return numcycles;
}

bool Quantity::getReset() {
	return reset;
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
	getPeriod();

	Serial.print("maxsum: ");
	Serial.println(maxsum);
	Serial.print("periodsum: ");
	Serial.println(periodsum);

	Serial.print("numcycles: ");
	Serial.println(numcycles);
	Serial.print("reset: ");
	Serial.println(reset ? "true" : "false");
}