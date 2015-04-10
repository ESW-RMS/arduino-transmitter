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


SoftwareSerial shieldGSM(7,8);

boolean flagSendSMS;

void setup() {
  Serial.begin(19200);
  //shieldGSM = new SoftwareSerial(7,8);
  shieldGSM.begin(19200);    // the GPRS baud rate
  Serial.println("ESW RMS Transmitter initializing...");
  flagSendSMS = false;
  delay(500);
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

void printShieldGSMResponse() {
  while (shieldGSM.available()) Serial.write(shieldGSM.read());
  Serial.flush();
}
