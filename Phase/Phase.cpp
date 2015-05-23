#include "Phase.h"

Phase::Phase(String n, unsigned int v, unsigned int i) :
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

String Phase::getMessage() { //Vrms Irms period delay
	String message = name;
	message += " ";
	message += String(voltage.getRMS());
	message += " ";
	message += String(current.getRMS());
	message += " ";
	message += String(voltage.getPeriod());
	message += " ";
	message += String(getDelay());
	message += ";\n";
	return message;
}

String Phase::getName() {
	return name;
}

unsigned long Phase::getPeriod() {
	return voltage.getPeriod();
}

signed long Phase::getDelay() {
	delay = (signed long) ( (double) delaysum / numsamples);
	return delay;
}

void Phase::printMessage() {
	printName();
	printRMS();
	printPeriod();
	printDelay();
	Serial.println(delaysum);
	Serial.println(numsamples);
}

void Phase::printName() {
	Serial.println(name);
}

void Phase::printRMS() {
	Serial.print(voltage.getName());
	Serial.print(": ");
	Serial.println(voltage.getRMS());
	Serial.print(current.getName());
	Serial.print(": ");
	Serial.println(current.getRMS());
}

void Phase::printPeriod() {
	Serial.print("T: ");
	Serial.println(voltage.getPeriod());
}

void Phase::printDelay() {
	Serial.print("Delay: ");
	Serial.println(getDelay());	
}