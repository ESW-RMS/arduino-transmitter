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
	numsamples = 0;
	delaysum = 0;
}

void Phase::sampleSignal() {
	voltage.sampleSignal();
	current.sampleSignal(); 
	if(!voltage.getReset()&&!current.getReset()){
		unsigned long voltageperiod = voltage.getPeriod();
		if(voltageperiod!=0) {
            signed long delaytemp1 = (signed long) voltage.getMRRZ() - current.getMRRZ();
            signed long delaytemp2 = voltageperiod - labs(delaytemp1);

            signed long delaytemp = (labs(delaytemp1) < labs(delaytemp2)) ? labs(delaytemp1) : labs(delaytemp2);

            delaysum += delaytemp;
			numsamples++;
		}
	}
}

String Phase::getMessage() { //Vrms Irms period delay
	String message = String(voltage.getMaxAvg());
	message += ",";
	message += String(current.getMaxAvg());
	message += ",";
	message += String(voltage.getPeriod());
	message += ",";
	message += String(getDelay());
	message += ",";
	return message;
}

String Phase::getName() {
	return name;
}

unsigned long Phase::getPeriod() {
	return voltage.getPeriod();
}

signed long Phase::getDelay(){
	delay = (signed long) ( (double) delaysum / numsamples);
	return delay;
}

void Phase::printMessage() {
	printName();
	printRMS();
	printPeriod();
	printDelay();
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
	Serial.print("D: ");
	Serial.println(getDelay());	
}