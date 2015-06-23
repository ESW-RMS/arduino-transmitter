#ifndef PHASE_H
#define PHASE_H

#include "Arduino.h"
#include "Quantity.h"
#include "String.h"

class Phase {
public:
	Phase(String n, unsigned int v, unsigned int i);
	void clear();
	
	void sampleSignal();
	
	String getMessage();
	String getName();
	//unsigned int getPort();
	//unsigned int getRMS();
	unsigned long getPeriod();
	signed long getDelay();

	void printMessage();
	void printName();
	//void printPort();
	void printRMS();
	void printPeriod();
	void printDelay();
	
private:
	String name;
	Quantity voltage;
	Quantity current;
	// double delay; // delay between voltage and current
	signed long delay; // temporary; will be double eventually
	unsigned long prevdelay;
	signed long delaysum;
	int overflows;
	unsigned long numsamples;
};

#endif