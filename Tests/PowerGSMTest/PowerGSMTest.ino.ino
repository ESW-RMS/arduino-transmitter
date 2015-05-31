#include <SoftwareSerial.h>
#include <String.h>
#include <Timers.h>

SoftwareSerial shieldGSM(7,8);
boolean flagSendSMS = false;

#define BAUD_RATE 19200
#define PRINT_TIME 1000
#define STANDARD_DELAY 1500
#define NUM_INIT_COMMANDS 5
#define INIT_SUCCESS_CODE 0
#define INIT_WAIT_CODE -1
#define INIT_ERROR_CODE -2
#define INIT_TIMEOUT_CODE -3

void setup() {
  // put your setup code here, to run once:
  powerUp();
  Serial.begin(BAUD_RATE);
  shieldGSM.begin(BAUD_RATE);    // the GPRS baud rate
}

void loop() {
  // put your main code here, to run repeatedly:
  pollUserCommand();
  printShieldGSMResponse();
}

void powerUp()
{
 pinMode(9, OUTPUT); 
 digitalWrite(9,LOW);
 delay(1000);
 digitalWrite(9,HIGH);
 delay(2000);
 digitalWrite(9,LOW);
 delay(3000);
}

void powerDown()
{
 pinMode(9, OUTPUT); 
 digitalWrite(9,LOW);
 delay(1000);
 digitalWrite(9,HIGH);
 delay(2000);
 digitalWrite(9,LOW);
 delay(3000);
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
