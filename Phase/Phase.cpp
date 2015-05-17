#include "Phase.h"

Phase::Phase(String n, unsigned int v, unsigned int i):
	voltage("V",v),
	current("I",i)
{
	name = n;
	pfsum = 0;
	numsamples = 0;
}

void Phase::clear() {
	voltage.clear();
	current.clear();
	pfsum = 0;
	numsamples=0;
}

void Phase::sampleSignal() {
	voltage.sampleSignal();
	current.sampleSignal(); 
	if(!voltage.getReset()&&!current.getReset()){
		signed long pftemp = (signed long) voltage.getMRRZ() - current.getMRRZ();	
		if(pftemp < 0) {
			pfsum += (pftemp + voltage.getFreq());
		}
		else {
			pfsum += pftemp;
		}
		numsamples++;
	}
}

void Phase::getValues() {
	getName();
	getRMS();
	getFreq();
	getPowerFactor();
	Serial.println(pfsum);
	Serial.println(numsamples);
}

void Phase::getName() {
	Serial.println(name);
}

void Phase::getPort() {
	Serial.print(voltage.getName());
	voltage.getPort();
	Serial.print(current.getName());
	current.getPort();
}
void Phase::getRMS(){
	Serial.print(voltage.getName());
	voltage.getRMS();
	Serial.print(current.getName());
	current.getRMS();
}

void Phase::getFreq() {
	voltage.getFreq();
}

void Phase::getPowerFactor() {
	pf = (signed long) ( (double) pfsum / numsamples);
	Serial.print("Power factor: ");
	Serial.println(pf);
	// difference, need to turn into angle and call cosine
}