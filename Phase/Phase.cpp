#include "Phase.h"

Phase::Phase(String n, unsigned int v, unsigned int i) :
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
	numsamples = 0;
	pfsum = 0;
}

void Phase::sampleSignal() {
	voltage.sampleSignal();
	current.sampleSignal(); 
	if(!voltage.getReset()&&!current.getReset()){
		unsigned long voltageperiod = voltage.getPeriod();
		if(voltageperiod!=0) {
			signed long delaytemp = (signed long) voltage.getMRRZ() - current.getMRRZ();

            pfsum += cos( (double) delaytemp / voltageperiod * 2*PI);
			numsamples++;
		}
	}
}

String Phase::getMessage() { //Vrms Irms period pf
	String message = String(voltage.getMaxAvg());
	message += ",";
	message += String(current.getMaxAvg());
	message += ",";
	message += String(voltage.getPeriod());
	message += ",";
	message += String(getPF());
	message += ",";
	return message;
}

String Phase::getName() {
	return name;
}

unsigned long Phase::getPeriod() {
	return voltage.getPeriod();
}

signed int Phase::getPF(){
	pf = 100* ( (double) pfsum / numsamples);
	return pf;	
}

void Phase::printMessage() {
	printName();
	printRMS();
	printPeriod();
	printPF();
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

void Phase::printPF() {
	Serial.print("Power Factor: ");
	Serial.println(getPF());	
}