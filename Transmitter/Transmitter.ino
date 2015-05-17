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
#define SMS_SEND_PERIOD 8                                   // in seconds, this will be 3600 = 1 hour
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

struct quantity {
  unsigned long min;
  unsigned long max;
  unsigned long rms;
  unsigned long mrrz; // most recent raising over zero
  unsigned long freq;
  unsigned long samp;
  int port;
  quantity(int p) {
    port = p;
    min = 1024;
    max = 0;
  }
} v1(A3), v2(A4), v3(A5), i1(A0), i2(A1), i3(A2);

quantity *sensorInputs[6] = {&i1, &i2, &i3, &v1, &v2, &v3};
unsigned long testsample[6] = {0};

void setup() {
//  cli();
  Serial.begin(BAUD_RATE);
  shieldGSM.begin(BAUD_RATE);    // the GPRS baud rate
//  Serial.println("Powering down...");
//  struct ATcommand powerDown[1] = {ATcommand("AT+CPOWD=1\r","NORMAL POWER DOWN")};
//  executeATCommands(powerDown,1);
//  powerUp();
  Serial.println("Rebooting...");
  Serial.println("ESW RMS Transmitter initializing...");
//  synchronizeLocalTime();
  initializeTimerInterrupts();
//  ADCsetup();
  
  i1.port = A0;
  i2.port = A1;
  i3.port = A2;
  v1.port = A3;
  v2.port = A4;
  v3.port = A5;
    
  Serial.println("Initialization complete!");
  // TMRArd_InitTimer(0, PRINT_TIME);
//  sei();
}

void loop() {
  pollUserCommand();
  while (shieldGSM.available()) Serial.write(shieldGSM.read());

  if(flagAutoSMS) {
//    if (readFlag == 1){
//      for(register int i=0;i<NUM_QUANTITY;i++) {
//        cli();
////        Serial.println("Phase "+String(i+1));
////        Serial.println(sensorInputs[i+ANALOG_PIN_OFFSET]->rms);
////        Serial.println(sensorInputs[i]->rms);
////        Serial.println(sensorInputs[i]->freq);
//        Serial.println(testsample[i]);
//        sei();
//      }
//      readFlag = 0;
//    }

      for(register int i=0;i< NUM_QUANTITY;i++) {
        quantity *q = sensorInputs[i];
        Serial.println(q->samp);
        Serial.println(q->min);
        Serial.println(q->max);
        Serial.println(q->rms);
        Serial.println("---");
      }

//    sendSMSMessage("message from loop");
    Serial.println("message from loop");
    flagAutoSMS=false;
  }

    pinMode(4,OUTPUT);
    digitalWrite(4,HIGH);
    for(register int i=0;i< NUM_QUANTITY;i++) {
      quantity *q = sensorInputs[i];
      q->samp = analogRead(q->port);
      if (q->samp > q->max) {
        q->max = q->samp;
        q->rms = (q->max - 511)/sqrt(2); //need to set bounds for under/overflow if this yields a negative result
      }
      if (q->samp < q->min) {
        q->min = q->samp;
      }
    }
    pinMode(4,OUTPUT);
    digitalWrite(4,LOW);

}

// Interrupt service routine for the ADC completion
ISR(ADC_vect){
  int bin = ADMUX & B00000111; 
//  ADMUX = (ADMUX >= B01000101) ? B01000000 : ADMUX+1;
  ADMUX = (ADMUX >= B01000101) ? B01000100 : ADMUX+1;

  char adm = ADMUX;
//  sensorInputs[ADMUX & B00000111]->samp = ADCL | (ADCH << 8);
  testsample[bin] = ADCL | (ADCH << 8);
  readFlag = 1;  

}

boolean toggle2;
unsigned int v1sample = 0;
unsigned int v1sample_prev = 1024;
unsigned int v1max = 0;
unsigned int v1min = 1024;
int sensorval=0;
ISR(TIMER2_COMPA_vect){//timer0 interrupt 2kHz toggles pin 8    
//  for(register int i=0;i<1;i++) {
//    quantity *q = sensorInputs[0];
//    unsigned long sensorReading = analogRead(q->port);
//    if (sensorReading > q->max) {
//      q->max = sensorReading;
//    }
//    if (sensorReading < q->min) {
//      q->min = sensorReading;
//    }
//  }
    
  if ((v1sample > 511) && (v1sample_prev < 511)) {
    v1.freq = micros() - v1.mrrz;
    v1.mrrz = micros();
  }
  v1sample_prev = v1sample;

//generates pulse wave of frequency interrupt/2 (takes two cycles for full wave- toggle high then toggle low)
//  if (toggle2){
//    digitalWrite(13,HIGH);
//    toggle2 = 0;
//  }
//  else{
//    digitalWrite(13,LOW);
//    toggle2 = 1;
//  }
}

char count = 0;
unsigned long prev1 = 0;
unsigned long curr1 = 0;
ISR (TIMER1_COMPA_vect) { // timer one interrupt function
//  verifying the time between interrupts ~ 4 seconds
//  prev1 = curr1;
//  curr1 = millis();
//  Serial.println(curr1 - prev1);

  count++;
  if (count >=SMS_INTERRUPT_CYCLES) {
//    digitalWrite(OUTPUT_PIN,digitalRead(OUTPUT_PIN) == LOW ? HIGH : LOW);
//    Serial.println(v1max);
//    Serial.println(v1min);
//    Serial.println(analogRead(3));
//    Serial.println(v1.freq); //frequency

//    for(register int i = A0; i < A3; i++){
//      sensorDataMessage(i);
//    }

    //sendSMSMessage("timer interrupt message");
    flagAutoSMS=true;
//    Serial.println("timer interrupt message");
    count = 0;
  }
}

void sensorDataMessage(int i) {
  Serial.print("P");
  Serial.print(i-PHASE_NUMBER_OFFSET);
  Serial.print("-V:");
  Serial.print(analogRead(i + ANALOG_PIN_OFFSET));
  Serial.print(",I:");
  Serial.println(analogRead(i));
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
//    buffer.trim();
//    if (!flagSendSMS) buffer.replace(" ","");
//    Serial.println(buffer);
//    shieldGSM.println(buffer);
//    delay(100);
//    
//    if (flagSendSMS) {
//      shieldGSM.println((char)26);
//      delay(100);
//      flagSendSMS = false;
//    }
//    
//    if (buffer.startsWith("AT+CMGS")) flagSendSMS = true;
//    
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

  TCCR2A = 0;// clear registers
  TCCR2B = 0;
  TCNT2  = 0;//initialize counter value to 0
  OCR2A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256) for 1khz
  TCCR2A |= (1 << WGM21); // turn on CTC mode
  TCCR2B |= (1 << CS21) | (1 << CS20);    // Set CS21 bit for 64 prescaler
  TIMSK2 |= (1 << OCIE2A);   // enable timer compare interrupt

  sei();         // set enable interrupt reallow interrupts
  
//  pinMode(OUTPUT_PIN,OUTPUT); 
  Serial.println("Timer interrupts initialized.");
}

void ADCsetup() {  
  ADMUX &= B01011111;
  ADMUX |= B01000000;
  ADMUX &= B11110000; //starting with analog pin 0
  ADMUX |= B00000101; //starting with analog pin 5
  ADCSRA |= B10000000;
  ADCSRA |= B00100000;
  ADCSRB &= B11111000;
  ADCSRA |= B00000111;
  ADCSRA |= B00001000;
  readFlag = 0;
  ADCSRA |=B01000000;
  Serial.println("ADC timer initialized.");
}


