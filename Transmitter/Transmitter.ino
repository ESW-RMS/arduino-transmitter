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
 * Version: V2, 2015.05.08
 * 
 * Timer Interrupt code modified from http://www.instructables.com/id/Arduino-Timer-Interrupts/?ALLSTEPS
 * Free Running Mode Test code from http://www.glennsweeney.com/tutorials/interrupt-driven-analog-conversion-with-an-atmega328p
 * ATMega328p
 * Note, many macro values are defined in <avr/io.h> and
 * <avr/interrupts.h>, which are included automatically by
 * the Arduino interface
 */

#include <SoftwareSerial.h>
#include <String.h>
#include <Quantity.h>
#include <Phase.h>
#include <Timers.h>

#define BAUD_RATE 19200
#define PRINT_TIME 1000
#define STANDARD_DELAY 1500
#define NUM_INIT_COMMANDS 5
#define INIT_SUCCESS_CODE 0
#define INIT_WAIT_CODE -1
#define INIT_ERROR_CODE -2
#define INIT_TIMEOUT_CODE -3
#define INIT_TIME 1000
#define COMPARE_MATCH_REGISTER 62499 // [16 MHz / (1024 * 1/4 Hz) ] - 1
#define ANALOG_PIN_OFFSET 3
#define NUM_QUANTITY 6
#define PHASE_NUMBER_OFFSET 13
#define OUTPUT_PIN 13
#define SMS_SEND_PERIOD 12                                   // in seconds, this will be 3600 = 1 hour
#define INTERRUPT_PERIOD 4 // highest integer numbers of second a timer interrupt is achievable with 16MHz clock and 1024 pre scale factor
#define SMS_INTERRUPT_CYCLES SMS_SEND_PERIOD/INTERRUPT_PERIOD // remove this when testing is done
#define NUM_SMS_COMMANDS 4
#define PHONE_NUMBER "\"+15594929868\""

SoftwareSerial shieldGSM(7,8);
boolean flagSendSMS;
boolean flagAutoSMS;
// High when a value is ready to be read
volatile int readFlag;

// Value to store analog result
volatile int analogVal;

struct ATcommand {
  String cmd;
  String resp; 
  ATcommand(String c, String r) {
    cmd = c;
    resp = r;
  }
};

Quantity *sensorInputs[6];

void setup() {
//  cli();
  Serial.begin(BAUD_RATE);
  shieldGSM.begin(BAUD_RATE);    // the GPRS baud rate
//  verifyGSMOn();
  Serial.println("ESW RMS Transmitter initializing...");
  initializeTimerInterrupts();

  sensorInputs[0] = new Quantity("I1",A0);
  sensorInputs[1] = new Quantity("I2",A1);
  sensorInputs[2] = new Quantity("I3",A2);
  sensorInputs[3] = new Quantity("V1",A3);
  sensorInputs[4] = new Quantity("V2",A4);
  sensorInputs[5] = new Quantity("V3",A5);
  

  Serial.println("Sensor quantities initialized.");
      
  Serial.println("Initialization complete!");
  // TMRArd_InitTimer(0, PRINT_TIME);
//  sei();
}

void loop() {
  pollUserCommand();
  while (shieldGSM.available()) Serial.write(shieldGSM.read());

  if(flagAutoSMS) {
//    for(register int i=0;i< NUM_QUANTITY;i++) {
//      Quantity *q = sensorInputs[i];
//      q->getValues();
//      q->clear();
//    }

      sensorInputs[0]->getValues();
      sensorInputs[0]->clear();
      sensorInputs[3]->getValues();
      sensorInputs[3]->clear();
      
//    sendSMSMessage("message from loop");
    Serial.println("message from loop");
    flagAutoSMS=false;
  }

    pinMode(4,OUTPUT);
    digitalWrite(4,HIGH);
    for(register int i=0;i< NUM_QUANTITY;i++) {
      Quantity *q = sensorInputs[i];
      q->sampleSignal();
    }
    digitalWrite(4,LOW);
}

char count = 0;
ISR (TIMER1_COMPA_vect) { // timer one interrupt function
  count++;
  if (count >=SMS_INTERRUPT_CYCLES) {
    flagAutoSMS=true;
    count = 0;
  }
}

void executeATCommands(struct ATcommand *commandsList, int numCommands) {
    for (int i = 0; i < numCommands; i++) {
      struct ATcommand atc = commandsList[i];
      int tocount=0;
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
            Serial.println("COMMAND TIMEOUT: "+String(++tocount));  // intentionally no break
          case INIT_ERROR_CODE:
            executeUserCommand(atc.cmd);
            break;
          default:
            break;
        }   
        if(tocount>=3)return;
      } while(state != INIT_SUCCESS_CODE);
    }
}

// see http://www.geeetech.com/wiki/index.php/Arduino_GPRS_Shield
void sendSMSMessage(String message) {
  String phoneNumberSet = "AT+CMGS=";
  phoneNumberSet += PHONE_NUMBER;
  
  struct ATcommand sendSMSCommands[NUM_SMS_COMMANDS] = { ATcommand("AT+CMGF=1\r","\r\nOK\r\n") , ATcommand(phoneNumberSet,"\r\n> ") , ATcommand(message,"\r\n> ") , ATcommand(String((char)26),"\r\nOK\r\n") };
  executeATCommands(sendSMSCommands,NUM_SMS_COMMANDS);
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
    Serial.println(millis());
    String buffer = Serial.readString();
    executeUserCommand(buffer);  
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

void powerUp() {
 pinMode(9, OUTPUT); 
 digitalWrite(9,LOW);
 delay(1000);
 digitalWrite(9,HIGH);
 delay(2000);
 digitalWrite(9,LOW);
 delay(3000);
}

void verifyGSMOn() {
  Serial.println("Powering down...");
  struct ATcommand powerDown[1] = {ATcommand("AT+CPOWD=1\r","NORMAL POWER DOWN")};
  executeATCommands(powerDown,1);
  Serial.println("Rebooting...");
  powerUp();
}

void synchronizeLocalTime() {
  struct ATcommand setupCommands[NUM_INIT_COMMANDS] = { ATcommand("AT","\r\nOK\r\n") , ATcommand("AT+CLTS=1","\r\nOK\r\n") , ATcommand("AT+CFUN=0","\r\nOK\r\n") , ATcommand("AT+CFUN=1","\r\nOK\r\n") , ATcommand("AT+CCLK?","\r\nOK\r\n") };
  executeATCommands(setupCommands, NUM_INIT_COMMANDS);
  Serial.println("Synchronized to local time.");
}

void initializeTimerInterrupts() {
  cli();        // clear interrupt stop interrupts from messing with setup
  
  TCCR1A = 0;   // clearing registers 
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = COMPARE_MATCH_REGISTER; // for 0.25 Hz (16e6)/(0.25*1024) - 1= clk_spd/(des_frq*pre_scale) - 1
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // set 1024 prescale factor
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  sei();         // set enable interrupt reallow interrupts
  
//  pinMode(OUTPUT_PIN,OUTPUT); 
  Serial.println("Timer interrupts initialized.");
}

