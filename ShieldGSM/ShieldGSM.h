#ifndef SHIELDGSM_H
#define SHIELDGSM_H

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "String.h"
#include "Timers.h"

#define INIT_SUCCESS_CODE 0
#define INIT_WAIT_CODE -1
#define INIT_ERROR_CODE -2
#define INIT_TIMEOUT_CODE -3

#define INIT_TIME 5000

#define NUM_INIT_COMMANDS 5
#define NUM_SMS_COMMANDS 4

#define POWER_PIN 9

// see http://www.geeetech.com/wiki/index.php/Arduino_GPRS_Shield

struct ATcommand;

class ShieldGSM{
public:
	ShieldGSM(int RX, int TX, int baud);
	
	void powerUp();
	
	int avail();
	char read();
		
	void executeUserCommand(String buffer);
	void pollUserCommand();
	int printShieldGSMResponse(String resp);
	
	void executeATCommands(struct ATcommand *commandslist, int numCommands);
	void verifyGSMOn();
	void synchronizeLocalTime();
	void sendSMSMessage(String message, String phoneNumber);
	void sendMessageLong(String message, String phoneNumber);

private:
	SoftwareSerial GSMShield;
};

#endif