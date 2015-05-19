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

String Quantity::getName() {
	return name;
}

unsigned int Quantity::getPort(){
	return port;
}

unsigned int Quantity::getSamp(){
	return samp;
}

unsigned int Quantity::getPrevSamp(){
	return prevsamp;
}

unsigned int Quantity::getMax(){
	return max;
}

unsigned int Quantity::getRMS(){
	rms = (unsigned int) (0.707 * ( (double) maxsum/numcycles - ZERO));
	return rms;
}

unsigned long Quantity::getMRRZ(){
	return mrrz;
}

unsigned long Quantity::getPeriod(){
	period = (unsigned long) ( (double) periodsum/numcycles);
	return period;
}

unsigned long Quantity::getMaxSum(){
	return maxsum;
}

unsigned long Quantity::getPeriodSum(){
	return periodsum;
}

unsigned long Quantity::getNumCycles(){
	return numcycles;
}

bool Quantity::getReset() {
	return reset;
}

void Quantity::printValues() {
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

void Quantity::printName(){
	Serial.println(name);	
}

void Quantity::printPort(){
	Serial.print("port: ");
	Serial.println(port);	
}

void Quantity::printSamp(){
	Serial.print("samp: ");
	Serial.println(samp);	
}

void Quantity::printPrevSamp(){
	Serial.print("prevsamp: ");
	Serial.println(prevsamp);	
}

void Quantity::printMax(){
	Serial.print("max: ");
	Serial.println(max);	
}

void Quantity::printRMS() {
	Serial.print("rms: ");
	Serial.println(getRMS());	
}

void Quantity::printMRRZ(){
	Serial.print("mrrz: ");
	Serial.println(mrrz);	
}

void Quantity::printPeriod() {
	Serial.print("period: ");
	Serial.println(period);	
	
}

void Quantity::printMaxSum(){
	Serial.print("maxsum: ");
	Serial.println(maxsum);	
}

void Quantity::printPeriodSum(){
	Serial.print("periodsum: ");
	Serial.println(periodsum);
}

void Quantity::printNumCycles(){
	Serial.print("numcycles: ");
	Serial.println(numcycles);
}

void Quantity::printReset() {
	Serial.print("reset: ");
	Serial.println(reset ? "true" : "false");
}