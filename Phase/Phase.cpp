#include "Phase.h"

Phase::Phase(String n, unsigned int v, unsigned int i):
	voltage("V",v),
	current("I",i)
{
	name = n;
	delaysum = 0;
	numsamples = 0;
}

void Phase::clear() {
	voltage.clear();
	current.clear();
	delaysum = 0;
	numsamples = 0;
}

void Phase::sampleSignal() {
	voltage.sampleSignal();
	current.sampleSignal(); 
	if(!voltage.getReset()&&!current.getReset()){
		signed long delaytemp = (signed long) voltage.getMRRZ() - current.getMRRZ();	
		if(delaytemp < 0) {
			delaysum += (delaytemp + voltage.getPeriod());
		}
		else {
			delaysum += delaytemp;
		}
		numsamples++;
	}
}

void Phase::getValues() {
	getName();
	getRMS();
	getPeriod();
	getDelay();
	Serial.println(delaysum);
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

void Phase::getPeriod() {
	voltage.getPeriod();
}

signed long Phase::getDelay() {
	delay = (signed long) ( (double) delaysum / numsamples);
	return delay;
}

void Phase::printDelay() {
	Serial.print("Delay: ");
	Serial.println(delay);	
}