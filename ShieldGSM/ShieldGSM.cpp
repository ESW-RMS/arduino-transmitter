#include "ShieldGSM.h"

struct ATcommand {
  String cmd;
  String resp; 
  ATcommand(String c, String r) {
    cmd = c;
    resp = r;
  }
};

ShieldGSM::ShieldGSM(int RX, int TX, int baud):
	GSMShield(RX,TX)
{
	GSMShield.begin(baud);
	Serial.println("ESW RMS Transmitter initializing...");
}

void ShieldGSM::powerUp() {
 pinMode(POWER_PIN, OUTPUT); 
 digitalWrite(POWER_PIN,LOW);
 delay(1000);
 digitalWrite(POWER_PIN,HIGH);
 delay(2000);
 digitalWrite(POWER_PIN,LOW);
 delay(3000);
}

int ShieldGSM::avail() {
	return GSMShield.available();
}

char ShieldGSM::read() {
	return GSMShield.read();
}

void ShieldGSM::executeUserCommand(String buffer) {
  buffer.trim();
  Serial.println(buffer);
  GSMShield.println(buffer);
}

void ShieldGSM::pollUserCommand() {
  char avail = Serial.available();
  if (avail) {
    Serial.println(millis());
    String buffer = Serial.readString();
    executeUserCommand(buffer);  
  }
}

int ShieldGSM::printShieldGSMResponse(String resp) {
  int result = INIT_WAIT_CODE;
  String serialOutput;
  String okString = "\r\nOK\r\n";
  String errorString = "ERROR";
  while(GSMShield.available()) {
    char c = GSMShield.read();
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

void ShieldGSM::executeATCommands(struct ATcommand *commandsList, int numCommands) {
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

void ShieldGSM::verifyGSMOn() {
  Serial.println("Powering down...");
  struct ATcommand powerDown[1] = {ATcommand("AT+CPOWD=1\r","NORMAL POWER DOWN")};
  executeATCommands(powerDown,1);
  Serial.println("Rebooting...");
  powerUp();
  Serial.println("GSM Shield is on.");
}

// void synchronizeLocalTime() {
  // struct ATcommand setupCommands[NUM_INIT_COMMANDS] = { ATcommand("AT","\r\nOK\r\n") , ATcommand("AT+CLTS=1","\r\nOK\r\n") , ATcommand("AT+CFUN=0","\r\nOK\r\n") , ATcommand("AT+CFUN=1","\r\nOK\r\n") , ATcommand("AT+CCLK?","\r\nOK\r\n") };
  // executeATCommands(setupCommands, NUM_INIT_COMMANDS);
  // Serial.println("Synchronized to local time.");
// }

void ShieldGSM::sendSMSMessage(String message, String phoneNumber) {
  String phoneNumberSet = "AT+CMGS=";
  phoneNumberSet += phoneNumber;  
  struct ATcommand sendSMSCommands[NUM_SMS_COMMANDS] = { ATcommand("AT+CMGF=1\r","\r\nOK\r\n") , ATcommand(phoneNumberSet,"\r\n> ") , ATcommand(message,"\r\n> ") , ATcommand(String((char)26),"\r\nOK\r\n") };
  executeATCommands(sendSMSCommands,NUM_SMS_COMMANDS);
}