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
 */

#include <SoftwareSerial.h>
#include <String.h>
#include <Timers.h>

#define PRINT_TIME 1000
#define ANALOG_PIN_OFFSET 3
#define PHASE_NUMBER_OFFSET 13
#define STANDARD_DELAY 1500
#define NUM_INIT_COMMANDS 5
#define INIT_SUCCESS_CODE 0
#define INIT_WAIT_CODE -1
#define INIT_ERROR_CODE -2
#define INIT_TIMEOUT_CODE -3
#define INIT_TIME 2000

SoftwareSerial shieldGSM(7,8);

boolean flagSendSMS;

void setup() {
  Serial.begin(19200);
  //shieldGSM = new SoftwareSerial(7,8);
  shieldGSM.begin(19200);    // the GPRS baud rate
  Serial.println("ESW RMS Transmitter initializing...");
  flagSendSMS = false;
  delay(STANDARD_DELAY);
//  shieldGSM.println("AT");
//  delay(STANDARD_DELAY);
//  printShieldGSMResponse();
//  shieldGSM.println("AT+CLTS=1");
//  delay(STANDARD_DELAY);
//  printShieldGSMResponse();
//  shieldGSM.println("AT+CFUN=0");
//  delay(STANDARD_DELAY << 3);
//  printShieldGSMResponse();
//  shieldGSM.println("AT+CFUN=1");
//  delay(STANDARD_DELAY << 3);
//  printShieldGSMResponse();
//  shieldGSM.println("AT+CCLK?");
//  delay(STANDARD_DELAY);
//  printShieldGSMResponse();
//  delay(STANDARD_DELAY);
  String setupCommands[NUM_INIT_COMMANDS] = {"AT","AT+CLTS=1","AT+CFUN=0","AT+CFUN=1","AT+CCLK?"};
  
  for (int i = 0; i < NUM_INIT_COMMANDS; i++) {
    int state = INIT_WAIT_CODE; 
    TMRArd_InitTimer(0, INIT_TIME);
    Serial.print(i);
    executeUserCommand(setupCommands[i]);
    do {
      state = printShieldGSMResponse();
      if (state == INIT_WAIT_CODE && TMRArd_IsTimerExpired(0)) {
        state = INIT_TIMEOUT_CODE;
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
  Serial.println("Initialization complete!");
  TMRArd_InitTimer(0, PRINT_TIME);
}

void loop() {
  pollUserCommand();
  printShieldGSMResponse();
}

void printSensorSample(){
  if (TMRArd_IsTimerExpired(0)) {
    //Serial.println(analogRead(A3));
    for (register int i = A0; i < A3; ++i) {
      Serial.print("PHASE ");
      Serial.println(i-13);
      Serial.print("V: ");
      Serial.print(analogRead(i + ANALOG_PIN_OFFSET));
      Serial.print(", I: ");
      Serial.println(analogRead(i));
    }
    
    Serial.println();
    TMRArd_InitTimer(0, PRINT_TIME);
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

int printShieldGSMResponse() {
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
    if(serialOutput.endsWith(okString)) {
      result = INIT_SUCCESS_CODE;
    }
  }
  Serial.flush();
  return result;
}
