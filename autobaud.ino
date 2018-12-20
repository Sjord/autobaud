const int RX_PIN = 0;
const int retries = 10;

unsigned long risingCycles = 0;
unsigned long fallingCycles = 0;
int changePin;

void onChange() {
  if (risingCycles) return;
  
  int newValue = digitalRead(changePin);
  if (newValue == LOW) {
    fallingCycles = ARM_DWT_CYCCNT;
  } else if (fallingCycles) {
    risingCycles = ARM_DWT_CYCCNT;
  }
}

unsigned long pulseInClockticks(int pin) {
  changePin = pin;
  risingCycles = fallingCycles = 0;
  attachInterrupt(digitalPinToInterrupt(pin), onChange, CHANGE);
  do { delay(100); } while (!fallingCycles && !risingCycles);
  detachInterrupt(digitalPinToInterrupt(pin));
  return risingCycles - fallingCycles;
}

unsigned long calibrateClockticks() {
  const int IN_PIN = 4;
  const int OUT_PIN = 5;
  pinMode(IN_PIN, INPUT);
  pinMode(OUT_PIN, OUTPUT);
  tone(OUT_PIN, 50);
  unsigned long result = pulseInClockticks(IN_PIN);
  noTone(OUT_PIN);
  return result;
}

unsigned long getPreciseBaudrate() {
  enableCycleCounter();
  Serial.println("Calibrating.");
  unsigned long cal = calibrateClockticks();
  Serial.print(cal * 100);
  Serial.println(" ticks per second.");
  
  unsigned long shortestTime = -1;
  for (int i = 0; i < retries; i++) {
    Serial.flush();
    noInterrupts();
    unsigned long pulse = pulseInClockticks(RX_PIN);
    interrupts();
    Serial.print("Low pulse of ");
    Serial.print(pulse);
    Serial.print(" ticks, which is ");
    unsigned long ptime = pulse * 1e7 / cal;
    Serial.print(ptime);
    Serial.println(" ns.");
    shortestTime = min(shortestTime, ptime);
  }
  Serial.print("Shortest pulse was ");
  Serial.print(shortestTime);
  Serial.print(" ns, corresponding to a baud rate of ");
  unsigned long baudRate = 1e9 / shortestTime;
  Serial.println(baudRate);
  return baudRate;
}

void enableCycleCounter() {
  ARM_DEMCR |= ARM_DEMCR_TRCENA;
  ARM_DWT_CTRL |= ARM_DWT_CTRL_CYCCNTENA;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Autobaud starting...");
  pinMode(RX_PIN, INPUT);
  int result = digitalRead(RX_PIN);
  if (result == HIGH) {
    Serial.println("Input pin is currently HIGH");
  } else {
    Serial.println("Input pin is currently LOW");
  }
    
  unsigned long minPulse = -1UL;
  unsigned long pulseDuration = 0UL;
  for (int i = 0; i < retries; pulseDuration && i++) {
    pulseDuration = pulseIn(RX_PIN, LOW);
    if (pulseDuration) {
      Serial.print("Low pulse of ");
      Serial.print(pulseDuration);
      Serial.println("µs.");
      minPulse = min(minPulse, pulseDuration);
    } else {
      Serial.println("No pulse detected.");
    }
  }
  
  unsigned long baudRate = 1e6 / minPulse;
  Serial.print("Shortest pulse was ");
  Serial.print(minPulse);
  Serial.print("µs, corresponding to a baud rate of ");
  Serial.println(baudRate);

  if (minPulse < 40) {
    Serial.println("Getting more precise measurement.");
    baudRate = getPreciseBaudrate();
  }
  
  Serial.println("Forwarding connection.");
  Serial1.begin(baudRate);
}

void loop() {
  if (Serial.available()) {
    Serial1.write(Serial.read());
  }
  if (Serial1.available()) {
    Serial.write(Serial1.read());
  }
}
