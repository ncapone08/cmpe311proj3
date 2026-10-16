#include "myMillis.h" // inject the file
#include "avr/io.h"
#include <avr/interrupt.h>
#include <stdint.h>

static volatile unsigned long milli_count = 0; // we will return this value

ISR(TIMER2_COMPA_vect) {
  milli_count++;
}

void myInit(void){

  // clear the registers entirely for reset
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  
  TCCR2A = (1 << WGM21); // Set to CTC

  OCR2A = 249; // 250 micro = 1 milli

  TCCR2B |= (1 << CS22); // Set prescaler to 64, frequency is 250MHz or 4 micro

  TIMSK2 |= (1 << OCIE2A); // compare match A

  sei();
}

unsigned long myMillis(void){
  unsigned long m; // create a copy to return

  // save state and disable interrupts
  uint8_t oldSREG = SREG;
  cli();

  // save elapsed time and return SREG
  m = milli_count;
  SREG = oldSREG; 

  return m;
}