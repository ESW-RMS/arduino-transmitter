/* InterruptTest
 * Code modified from http://www.instructables.com/id/Arduino-Timer-Interrupts/?ALLSTEPS
 */
void setup() {
  Serial.begin(19200);
  cli();        // clear interrupt stop interrupts from messing with setup
  TCCR1A = 0;   // clearing registers 
  TCCR1B = 0;
  TCNT1 = 0;
  
  OCR1A = 62499; // for 0.25 Hz (16e6)/(0.25*1024) - 1= clk_spd/(des_frq*pre_scale) - 1
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // set 1024 prescale factor
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();         // set enable interrupt reallow interrupts
  
  pinMode(13,OUTPUT);
}

char count = 0;
unsigned long prev = 0;
unsigned long curr = 0;
ISR (TIMER1_COMPA_vect) { // timer one interrupt function
  prev = curr;
  curr = millis();
  Serial.println(curr - prev);
  count++;
  if (count >=8) {
    digitalWrite(13,digitalRead(13) == LOW ? HIGH : LOW);
    Serial.println("Send text here.");
    count = 0;
  }
}

void loop() {}
