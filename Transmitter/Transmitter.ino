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
 * Version: V3, 2015.05.22
 * 
 * Timer Interrupt code modified from http://www.instructables.com/id/Arduino-Timer-Interrupts/?ALLSTEPS
 * Note, many macro values are defined in <avr/io.h> and
 * <avr/interrupts.h>, which are included automatically by
 * the Arduino interface
 */

#include <SoftwareSerial.h>
#include <String.h>
#include <Timers.h>

#include <Quantity.h>
#include <Phase.h>
#include <ShieldGSM.h>

#define BAUD_RATE 19200

#define STANDARD_DELAY 1500

#define ANALOG_PIN_OFFSET 3
#define NUM_QUANTITY 6
#define NUM_PHASES NUM_QUANTITY/2
#define PHASE_NUMBER_OFFSET 13

#define HEARTBEAT 4
#define LED 13

#define COMPARE_MATCH_REGISTER 62499 // [16 MHz / (1024 * 1/4 Hz) ] - 1
#define SMS_SEND_PERIOD 32 // in seconds, this will be 3600 = 1 hour
#define INTERRUPT_PERIOD 4 // highest achievable number of seconds with 16MHz clock and 1024 pre scale factor
#define SMS_INTERRUPT_CYCLES SMS_SEND_PERIOD/INTERRUPT_PERIOD // remove this when testing is done
#define PHONE_NUMBER "\"+15594929868\""

boolean flagSendSMS;
boolean flagAutoSMS;

Phase *phases[3];
ShieldGSM *transmitter;
SoftwareSerial mySerial(7,8);

void setup() {
  Serial.begin(BAUD_RATE);

  transmitter = new ShieldGSM(7,8,BAUD_RATE);
//  transmitter->verifyGSMOn();

  initializeTimerInterrupts();

  phases[0] = new Phase("P1",A3,A0);
  phases[1] = new Phase("P2",A4,A1);
  phases[2] = new Phase("P3",A5,A2);
  Serial.println("Sensor quantities initialized.");
      
  pinMode(HEARTBEAT,OUTPUT);
  pinMode(LED,OUTPUT);    
  Serial.println("Initialization complete!");
}

void loop() {
//  transmitter->pollUserCommand();
  while (transmitter->avail()) Serial.write(transmitter->read());

//  if(flagAutoSMS) {
  if(Serial.available()) {
//    while(Serial.available()){
//      Serial.read();
//    }
    String message;
    for(register int i=0;i<NUM_PHASES;i++) {
      Phase *p = phases[i];
      p->printMessage();
      message += p->getMessage();
      p->clear();
    }
    Serial.println(message);
    //implement messagelong to prevent overflow
    transmitter->sendSMSMessage("abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", PHONE_NUMBER);
    flagAutoSMS=false;
  }
  transmitter->pollUserCommand();

    digitalWrite(HEARTBEAT,HIGH);
    for(register int i=0;i<NUM_PHASES;i++){
      Phase *p = phases[i];
      p->sampleSignal();
    }
    digitalWrite(HEARTBEAT,LOW);
}

char count = 0;
ISR (TIMER1_COMPA_vect) {
  count++;
  if (count >= SMS_INTERRUPT_CYCLES) {
    flagAutoSMS=true;
    count = 0;
  }
  digitalWrite(LED, (digitalRead(LED) == HIGH) ? LOW : HIGH);
}

void initializeTimerInterrupts() {
  cli();        // clear interrupt: stop interrupts from messing with setup
  
  TCCR1A = 0;   // clearing registers 
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = COMPARE_MATCH_REGISTER; // for 0.25 Hz (16e6)/(0.25*1024) - 1= clk_spd/(des_frq*pre_scale) - 1
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // set 1024 prescale factor
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  sei();         // set enable interrupt reallow interrupts
  
  Serial.println("Timer interrupts initialized.");
}

