
// Frequency timer using input capture unit
// Author: Nick Gammon
// Date: 31 August 2013

// Input: Pin D8 

volatile boolean first;
volatile boolean triggered;
volatile unsigned long overflowCount;
volatile unsigned long startTime;
volatile unsigned long finishTime;

// timer overflows (every 65536 counts)
ISR (TIMER1_OVF_vect) 
{
  overflowCount++;
}  // end of TIMER1_OVF_vect

ISR (TIMER1_CAPT_vect)
  {
  // grab counter value before it changes any more
  unsigned int timer1CounterValue;
  timer1CounterValue = ICR1;  // see datasheet, page 117 (accessing 16-bit registers)
  unsigned long overflowCopy = overflowCount;
  
  // if just missed an overflow
  if ((TIFR1 & bit (TOV1)) && timer1CounterValue < 0x7FFF)
    overflowCopy++;
  
  // wait until we noticed last one
  if (triggered)
    return;

  if (first)
    {
    startTime = (overflowCopy << 16) + timer1CounterValue;
    first = false;
    return;  
    }
    
  finishTime = (overflowCopy << 16) + timer1CounterValue;
  triggered = true;
  TIMSK1 = 0;    // no more interrupts for now
  }  // end of TIMER1_CAPT_vect
  
void prepareForInterrupts ()
  {
  noInterrupts ();  // protected code
  first = true;
  triggered = false;  // re-arm for next time
  // reset Timer 1
  TCCR1A = 0;
  TCCR1B = 0;
  
  TIFR1 = bit (ICF1) | bit (TOV1);  // clear flags so we don't get a bogus interrupt
  TCNT1 = 0;          // Counter to zero
  overflowCount = 0;  // Therefore no overflows yet
  
  // Timer 1 - counts clock pulses
  TIMSK1 = bit (TOIE1) | bit (ICIE1);   // interrupt on Timer 1 overflow and input capture
  // start Timer 1, no prescaler
  TCCR1B =  bit (CS10) | bit (ICES1);  // plus Input Capture Edge Select (rising on D8)
  interrupts ();
  }  // end of prepareForInterrupts
  

void setup () 
  {
  Serial.begin(115200);       
  Serial.println("Frequency Counter");
  // set up for interrupts
  prepareForInterrupts ();   
  } // end of setup

void loop () 
  {
  // wait till we have a reading
  if (!triggered)
    return;
 
  // period is elapsed time
  unsigned long elapsedTime = finishTime - startTime;
  // frequency is inverse of period, adjusted for clock period
  float freq = F_CPU / float (elapsedTime);  // each tick is 62.5 ns at 16 MHz
  
  Serial.print ("Took: ");
  Serial.print (elapsedTime);
  Serial.print (" counts. ");

  Serial.print ("Frequency: ");
  Serial.print (freq);
  Serial.println (" Hz. ");

  // so we can read it  
  delay (500);

  prepareForInterrupts ();   
}   // end of loop
