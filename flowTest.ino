/*
Code: Jan Barten
April 2025
*/

unsigned int pulseCount = 0;
unsigned int treshold = 640;

int startSwitch = 14;
int relayPin = 33;
int sensorPin = 32;


void setup() {
  Serial.begin(9600);  // Seriële communicatie opzetten
  pinMode(relayPin, OUTPUT);
  pinMode(sensorPin, INPUT);
  pinMode(startSwitch, INPUT);

  attachInterrupt(sensorPin, ISR, RISING);  // interrupt aanzetten
attachInterrupt(sensorPin, ISR, RISING);  // interrupt aanzetten
  while (pulseCount < treshold) {           // zolang het het aantal pulsen nog niet gehaald is
    digitalWrite(relayPin, LOW);           // kraan open
    Serial.println(pulseCount);             // pulse teller laten zien
  }                                         // pas als het aantal pulsen is behaald gaat de code hier verder
  digitalWrite(relayPin, HIGH);              // kraan uit
  Serial.println(pulseCount);               // print ter controle het aantal getelde pulsen
  detachInterrupt(sensorPin);               // interrupt uitzetten
  Serial.println("Teller compleet");
  delay(5000);
  pulseCount = 0;  // pulse teller weer naar 0
  
}

void loop() {
  
}

void ISR() {     // Interrupt Service Routine
  pulseCount++;  // teller ophogen met 1 als er een puls komt
}
