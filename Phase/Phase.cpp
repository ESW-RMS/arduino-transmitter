#include "Phase.h"

Phase::Phase(String n, unsigned int v, unsigned int i):
	voltage("V",v),
	current("I",i)
{
	name = n;
}

void Phase::clear() {
	voltage.clear();
	current.clear();
}

void Phase::sampleSignal() {
	voltage.sampleSignal();
	current.sampleSignal();
}

void Phase::getValues() {
	getName();
	getRMS();
	getFreq();
	
}

void Phase::getName() {
	Serial.println(name);
}

void Phase::getPort() {
	voltage.getName();
	Serial.print(" ");
	voltage.getPort();
	current.getName();
	Serial.print(" ");
	current.getPort();
}
void Phase::getRMS(){
	voltage.getName();
	voltage.getRMS();
	current.getName();
	current.getRMS();
}

void Phase::getFreq() {
	voltage.getFreq();
}

void Phase::getPowerFactor() {
	pf = voltage.getMRRZ() - current.getMRRZ(); 
	// difference, need to turn into angle and call cosine
}