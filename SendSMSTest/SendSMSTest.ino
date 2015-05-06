/**
 * Transmitter
 * Transmitter code for Stanford Engineers for a Sustainable World
 * Remote Monitoring System for Micro-Hydro Plants Indonesia.
 * Our hope is to have this code base generalizable to other
 * renewable energy and remote monitoring projects.
 *
 * Platform: Arduino Uno
 * Authors: Evan Giarta, egiarta1671
 *          Evelyn Li, 0evelyn
 *
 * Version: V1, 2015.04.01
 * Maiden code to test Arduino's sampling capability
 * off the ESW Three Phase Sensor Board v2 with minimal
 * Geetech SIM 900 GPRS GSM Shield interfacing.
 * 
 * Timer Interrupt code modified from http://www.instructables.com/id/Arduino-Timer-Interrupts/?ALLSTEPS
 *
 */

#include <SoftwareSerial.h>
#include <String.h>
#include <Timers.h>

#define BAUD_RATE 19200
#define PRINT_TIME 1000
#define STANDARD_DELAY 1500
#define NUM_INIT_COMMANDS 5
#define INIT_SUCCESS_CODE 0
#define INIT_WAIT_CODE -1
#define INIT_ERROR_CODE -2
#define INIT_TIMEOUT_CODE -3
#define INIT_TIME 5000
#define COMPARE_MATCH_REGISTER 62499 // [16 MHz / (1024 * 1/4 Hz) ] - 1
#define ANALOG_PIN_OFFSET 3
#define PHASE_NUMBER_OFFSET 13
#define OUTPUT_PIN 13
#define SMS_SEND_PERIOD 4 // in seconds, this will be 3600 = 1 hour
#define INTERRUPT_PERIOD 4 // highest integer numbers of second a timer interrupt is achievable with 16MHz clock and 1024 pre scale factor
#define SMS_INTERRUPT_CYCLES SMS_SEND_PERIOD/INTERRUPT_PERIOD // remove this when testing is done
#define NUM_SMS_COMMANDS 4
#define PHONE_NUMBER "\"+15594929868\""

SoftwareSerial shieldGSM(7,8);
boolean flagSendSMS;

void setup() {;
  Serial.begin(BAUD_RATE);
  shieldGSM.begin(BAUD_RATE);    // the GPRS baud rate
}

void loop() {
//  pollUserCommand();
//  printShieldGSMResponse();
  if(Serial.available()) {
    //Serial.println(String((char)26));
    sendSMSMessage("hello world yeee");
    Serial.println(millis());
    Serial.read();
  }
}

struct ATcommand {
  String cmd;
  String resp; 
  ATcommand(String c, String r) {
    cmd = c;
    resp = r;
  }
};

// see http://www.geeetech.com/wiki/index.php/Arduino_GPRS_Shield
void sendSMSMessage(String message) {
  
  
  
  
  String phoneNumberSet = "AT+CMGS=";
  phoneNumberSet += PHONE_NUMBER;
//  String sendSMSCommands[NUM_SMS_COMMANDS] = {"AT+CMGF=1\r",phoneNumberSet,message,String((char)26)};
  
  struct ATcommand sendSMSCommands[NUM_SMS_COMMANDS] = { ATcommand("AT+CMGF=1\r","\r\nOK\r\n") , ATcommand(phoneNumberSet,"\r\n> ") , ATcommand(message,"\r\n> ") , ATcommand(String((char)26),"\r\nOK\r\n") };

//  for (register int i = 0; i < NUM_SMS_COMMANDS; ++i) {
//    struct ATcommand atc = sendSMSCommands[i];
//    Serial.println(atc.cmd);
//    Serial.println(atc.resp); 
//  }
  
  executeATCommands(sendSMSCommands,NUM_SMS_COMMANDS);

// old hard-coded version
//  delay(100);
//  executeUserCommand("AT+CMGS = "+PHONE_NUMBER);
//  delay(100);
//  executeUserCommand(message);
//  delay(100);
//  executeUserCommand((char)26); //the ASCII code of the ctrl+z is 26
//  delay(100);
//  Serial.println();
}

void executeATCommands(struct ATcommand *commandsList, int numCommands) {
    for (int i = 0; i < numCommands; i++) {
      struct ATcommand atc = commandsList[i];
      int state = INIT_WAIT_CODE; 
      TMRArd_InitTimer(0, INIT_TIME);
      Serial.print(i);
      executeUserCommand(atc.cmd);
      do {
        state = printShieldGSMResponse(atc.resp);
        if (state == INIT_WAIT_CODE && TMRArd_IsTimerExpired(0)) {
          state = INIT_TIMEOUT_CODE;
          TMRArd_InitTimer(0, INIT_TIME);        
        }
        else if (state != INIT_WAIT_CODE) {
          TMRArd_InitTimer(0, INIT_TIME);
        }
        switch(state) {
          case INIT_TIMEOUT_CODE:
            Serial.println("COMMAND TIMEOUT");  // intentionally no break
          case INIT_ERROR_CODE:
            executeUserCommand("A/");
            break;
          default:
            break;
        }   
      } while(state != INIT_SUCCESS_CODE);
    }
}

void executeUserCommand(String buffer) {
  buffer.trim();
  Serial.println(buffer);
  shieldGSM.println(buffer);
  // delay(100);
}

void pollUserCommand() {
  char avail = Serial.available();
  if (avail) {
    String buffer = Serial.readString();
    buffer.trim();
    if (!flagSendSMS) buffer.replace(" ","");
    Serial.println(buffer);
    shieldGSM.println(buffer);
    delay(100);
    
    if (flagSendSMS) {
      shieldGSM.println((char)26);
      delay(100);
      flagSendSMS = false;
    }
    
    if (buffer.startsWith("AT+CMGS")) flagSendSMS = true;
    
  }
}

int printShieldGSMResponse(String resp) {
//  while (shieldGSM.available()) Serial.write(shieldGSM.read());
  int result = INIT_WAIT_CODE;
  String serialOutput;
  String okString = "\r\nOK\r\n";
  String errorString = "ERROR";
  while(shieldGSM.available()) {
    char c = shieldGSM.read();
//    Serial.print((int)c);
//    Serial.write(':');
    Serial.write(c);
//    Serial.write(',');
    serialOutput += c;
    if(serialOutput.endsWith(errorString)) {
      result = INIT_ERROR_CODE;
    }
    if(serialOutput.endsWith(resp)) {
      result = INIT_SUCCESS_CODE;
    }
  }
  Serial.flush();
  return result;
}

