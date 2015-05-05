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
#include <WProgram.h>
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
#define INIT_TIME 2000
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

struct quantity {
  unsigned long min;
  unsigned long max;
  unsigned long rms;
  unsigned long mrrz; // most recent raising over zero
  unsigned long freq;
  int port;
} v1, v2, v3, i1, i2, i3;

quantity *sensorInputs[6] = {&v1, &v2, &v3, &i1, &i2, &i3};

void setup() {
  //powerUp();
  Serial.begin(BAUD_RATE);
  shieldGSM.begin(BAUD_RATE);    // the GPRS baud rate
  Serial.println("ESW RMS Transmitter initializing...");
  synchronizeLocalTime();
  initializeTimerInterrupts();
  Serial.println("Initialization complete!");
  
  v1.port = A3;
  v2.port = A4;
  v3.port = A5;
  i1.port = A0;
  i2.port = A1;
  i3.port = A2;

  // TMRArd_InitTimer(0, PRINT_TIME);
}

void loop() {
  pollUserCommand();
  printShieldGSMResponse();
//  if(Serial.available()) {
 //   sendSMSMessage("hello world");
//    Serial.println(millis());
//    Serial.read();
//  }
}

boolean toggle2;
unsigned int v1sample = analogRead(3);
unsigned int v1sample_prev = 1024;
unsigned int v1max = 0;
unsigned int v1min = 1024;
ISR(TIMER2_COMPA_vect){//timer0 interrupt 2kHz toggles pin 8    
  for(register int i;i<6;i++) {
    quantity *q = sensorInputs[i];
    if (analogRead(q->port) > q->max) {
      q->max = analogRead(q->port);
    }
    if (analogRead(q->port) < q->min) {
      q->min = analogRead(q->port);
    }
  }
  
  if ((v1sample > 511) && (v1sample_prev < 511)) {
    v1.freq = micros() - v1.mrrz;
    v1.mrrz = micros();
  }
  v1sample_prev = v1sample;

//generates pulse wave of frequency 1kHz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  if (toggle2){
    digitalWrite(8,HIGH);
    toggle2 = 0;
  }
  else{
    digitalWrite(8,LOW);
    toggle2 = 1;
  }
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
    digitalWrite(OUTPUT_PIN,digitalRead(OUTPUT_PIN) == LOW ? HIGH : LOW);
//    Serial.println("Send text here."); //AT+CMGS to send SMS message
    Serial.println(v1max);
    Serial.println(v1min);
    Serial.println(analogRead(3));
    Serial.println(v1.freq); //frequency

//    for(register int i = A0; i < A3; i++){
//      sensorDataMessage(i);
//    }
    
    count = 0;
  }
}

void sensorDataMessage(int i) { //change to String if using dataMessage
  Serial.print("P");
  Serial.print(i-PHASE_NUMBER_OFFSET);
  Serial.print("-V:");
  Serial.print(analogRead(i + ANALOG_PIN_OFFSET));
  Serial.print(",I:");
  Serial.println(analogRead(i));
}

void executeATCommands(String *commandsList, int numCommands) {
    if(shieldGSM.available()){
    for (int i = 0; i < numCommands; i++) {
      int state = INIT_WAIT_CODE; 
      TMRArd_InitTimer(0, INIT_TIME);
      Serial.print(i);
      executeUserCommand(commandsList[i]);
      do {
        state = printShieldGSMResponse();
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
}

// see http://www.geeetech.com/wiki/index.php/Arduino_GPRS_Shield
void sendSMSMessage(String message) {
//  executeUserCommand("AT+CMGF=1\r");
  String phoneNumberSet = "AT+CMGS = ";
  phoneNumberSet += PHONE_NUMBER;
  String sendSMSCommands[NUM_SMS_COMMANDS] = {"AT+CMGF=1\r",phoneNumberSet,message,String((char)26)};
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
  String setupCommands[NUM_INIT_COMMANDS] = {"AT","AT+CLTS=1","AT+CFUN=0","AT+CFUN=1","AT+CCLK?"};
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
  OCR2A = 249;// = (16*10^6) / (1000*64) - 1 (must be <256) for 2khz
  TCCR2A |= (1 << WGM21); // turn on CTC mode
  TCCR2B |= (1 << CS21) | (1 << CS20);    // Set CS21 bit for 64 prescaler
  TIMSK2 |= (1 << OCIE2A);   // enable timer compare interrupt

  sei();         // set enable interrupt reallow interrupts
  
  pinMode(OUTPUT_PIN,OUTPUT); 
  Serial.println("Timer interrupts initialized.");
}
