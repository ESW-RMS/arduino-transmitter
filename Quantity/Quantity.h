#ifndef QUANTITY_H
#define QUANTITY_H

#include "Arduino.h"
#include "String.h"

class Quantity {
public:
	Quantity(String n, unsigned int p);
	reset();
	getValues();

private:
	// object properties
	String name;
	unsigned int port; // may be able to change to char afterward

	// variables per cycle
	unsigned int samp;
	unsigned int prevsamp;
	unsigned int max;
	unsigned int rms;

	unsigned long mrrz; // micro/milli second value
	unsigned int freq;

	// variables per hour
	unsigned long maxsum; 
	unsigned long freqsum;
	unsigned int  numcycles;
	unsigned bool reset;
};

#endif

/*
struct quantity {
  unsigned long samp;
  unsigned long prevsamp;
  unsigned long min;
  unsigned long max;
  unsigned long rms;
  unsigned long maxsum;
  
  unsigned long mrrz; // most recent raising over zero, need to consider the case right after we send a text since mrrz would create a freq to high
  unsigned long freq;
  int cycl;  
  
  int port;
  quantity(int p) {
    port = p;
    min = 1024;
    max = 0;
  }
} v1(A3), v2(A4), v3(A5), i1(A0), i2(A1), i3(A2);
*/