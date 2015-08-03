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
	unsigned long getPeriod();
	signed int getPF();

	void printMessage();
	void printName();
	void printRMS();
	void printPeriod();
	void printPF();
	
private:
	String name;
	Quantity voltage;
	Quantity current;
	signed int pf;
	double pfsum;
	unsigned long numsamples;
};

#endif