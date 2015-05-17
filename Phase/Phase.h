#ifndef PHASE_H
#define PHASE_H

#include "Arduino.h"
#include "Quantity.h"
#include "String.h"

class Phase{
public:
	Phase(String n, unsigned int v, unsigned int i);
	void clear();
	
	void sampleSignal();
	void getValues();

	void getName();
	void getPort();
	void getRMS();
	void getFreq();
	void getPowerFactor();
private:
	String name;
	Quantity voltage;
	Quantity current;
	// double pf; // power factor
	signed long pf; // temporary; will be double eventually
	signed long pfsum;
	unsigned int numsamples;
};

#endif