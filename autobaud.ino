const int RX_PIN = 0;

void setup() {
  // put your setup code here, to run once:
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
  for (int i = 0; i < 5; pulseDuration && i++) {
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
  Serial.println("Done.");
}

void loop() {
  delay(100);
}
