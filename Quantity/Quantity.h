#ifndef QUANTITY_H
#define QUANTITY_H

#include "Arduino.h"
#include "String.h"

#define ZERO 511

class Quantity {
public:
	Quantity(String n, unsigned int p);
	
	void clear();
	void sampleSignal();
	
	//String getValues();
	String getName();
	unsigned int getPort();
	unsigned int getSamp();
	unsigned int getPrevSamp();
	unsigned int getMax();
	unsigned int getRMS();
	unsigned long getMRRZ();
	unsigned long getPeriod();
	unsigned long getMaxSum();
	unsigned long getPeriodSum();
	unsigned long getNumCycles();
	bool getReset();
	
	void printValues();
	void printName();
	void printPort();
	void printSamp();
	void printPrevSamp();
	void printMax();
	void printRMS();
	void printMRRZ();
	void printPeriod();
	void printMaxSum();
	void printPeriodSum();
	void printNumCycles();
	void printReset();

private:
	// object properties
	String name;
	unsigned int port; // may be able to change to char afterward

	// variables per cycle
	unsigned int samp;
	unsigned int prevsamp;
	unsigned int max;

	unsigned long mrrz; // micro/milli second value
	unsigned long period;

	// variables per hour
	unsigned int rms;  // need to set bounds for under/overflow if this yields a negative result
	unsigned long maxsum; 
	unsigned long periodsum;
	unsigned long  numcycles;
	bool reset;
};

#endif