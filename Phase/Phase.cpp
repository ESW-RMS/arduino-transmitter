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
	//overflows = 0;
}

void Phase::sampleSignal() {
	voltage.sampleSignal();
	current.sampleSignal(); 
	if(!voltage.getReset()&&!current.getReset()){
		signed long delaytemp = (signed long) voltage.getMRRZ() - current.getMRRZ();	
		signed long delaysumtemp = delaysum;
		unsigned long voltageperiod = voltage.getPeriod();
		delaytemp %= voltageperiod;
		if(voltageperiod!=0) {

			// verified to work, but may not be a reliable proxy
			// for power factor
/* 			if(delaytemp<0) {
				delaysum += (delaytemp + voltageperiod);
			}
			else {
				delaysum += delaytemp;
			} */
			
			// proposed as more reliable measure of power factor
			// delaytemp is closer to 0 than to P
			if(2*labs(delaytemp) < voltageperiod) {
				delaysum += labs(delaytemp);
			}
			// delaytemp is closer to P than to 0
			else { 
				delaysum += (voltageperiod - labs(delaytemp));
			}

			//check if the sample has overflowed
/* 			if(delaysumtemp > delaysum) {
				Serial.println("Overflow on sample: ");
				Serial.println(numsamples);
				overflows++;
			} */
			numsamples++;
		}
			
/* 		if(delaytemp < 0) {
			if (-delaytemp > voltage.getPeriod()) {
				Serial.println(delaytemp);
				Serial.println(voltage.getPeriod());
			}
			delaysum += (delaytemp + voltage.getPeriod());
		}
		else {
			delaysum += delaytemp;
		}
		numsamples++;
 */	}
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

signed long Phase::getDelay() {
	// true delaysum = overflows * (2^31-1) + delaysum
	delay = (signed long) ( (double) delaysum / numsamples);
	//delay += (signed long) ( (double) 2147482647 / numsamples) * overflows;
	//delay %= (signed long) voltage.getPeriod();
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
	Serial.print("Delay: ");
	Serial.println(getDelay());	
}