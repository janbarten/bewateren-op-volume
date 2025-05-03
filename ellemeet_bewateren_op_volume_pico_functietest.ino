// Code door J. Barten
// mei 2025

/*
V3.0
- code aangepast voor RaspBerry Pi Pico
- afhandeling van de trays herschreven met een functie
- alle relays in ruststand niet actief/lampje uit

V2.2
- pinbezetting gewijzigd. P2 mag niet aangesloten zijn bij upload
- getest met relais blok
- overbodige tray statussen verwijderd

V2.1
- uitgebreid naar 6 trays

V2.0
- code herzien. Werkt nu met een gesimuleerde flowsensor
- pinbeztting aangepast i.v.m. het ontbreken van pin 24

V1.3
- pinbezetting volgens voorstel toegevoegd
- afhandeling tray 2 aangepast
- de alarm ISR aangepast. Alarm mededeling naar de loop
*/
// includes voor rgb led op pin ....
#include <Adafruit_NeoPixel.h>

#define RGB 23 // pin 23 is de onboard rgb led
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, RGB, NEO_GRB + NEO_KHZ800);
/* 
Hardware aansluitingen
Goed kijken naar de pinbezetting op het ESP niet alle pinnen zijn beschikbaar voor elke taak.
Hieronder de pinbezetting volgens voorstel!!
*/

// hoofdkraan toekennen aan een pin
int kraan0 = 5;

// 6 kogelkranen toekennen aan een pin
int kraan1 = 6;
int kraan2 = 7;
int kraan3 = 8;
int kraan4 = 9;
int kraan5 = 10;
int kraan6 = 11;

// startknop toekennen aan een pin
int startButton = 15;

// tray schakelaars toekennen aan een pin
int switch1 = 16;
int switch2 = 17;
int switch3 = 18;
int switch4 = 19;
int switch5 = 20;
int switch6 = 21;

// pin voor de pulsen naar de interrupt
int flowSensor = 26;
int overFlow = 27;

/*
Variabelen
*/

// status van het hele rek, bij opstart in wachtstand
bool rekStatus = 0;  // 0 = het rek is in rust/wachtstand, 1 = het rek in actief met bewateren  3 = ALARM

// bij het opstarten starten we in alle trays in de wachtstand
int tray1Status = 0;  // 0 = wachten, 1 = vullen
int tray2Status = 0;
int tray3Status = 0;
int tray4Status = 0;
int tray5Status = 0;
int tray6Status = 0;

// detectie of de start en resetknop zijn ingedrukt (om te debouncen)
bool startButtonStatus;
bool resetSwitchStatus;
bool laststartButtonStatus = 0;
bool lastresetSwitchStatus = 0;

// 6 tray schakelaarstanden kunnen opslaan in een boolse waarde
bool switch1Status = 0;
bool switch2Status = 0;
bool switch3Status = 0;
bool switch4Status = 0;
bool switch5Status = 0;
bool switch6Status = 0;

// puls teller voor de flowmeter
unsigned int pulseCount = 0;

// aantal pulsen per liter
unsigned int pulseWaarde = 630;

// inwatering en leeglOPEN duur in minuten
int inwateren = 2;
int leegloop = 2;
int kraanPauze = 17000;  // tijd om de kraan naar de eindpositie te laten gaan
bool OPEN = HIGH;
bool DICHT = LOW;

bool HKOPEN = LOW;
bool HKDICHT = HIGH;

unsigned long previousMillis = 0;  // gebruikt om niet blokkerend wachten

void setup() {
  Serial.begin(9600);  // instellen snelheid seriële communicatie
  /*
  Ter overweging, bij het minste geringste gaat het rek in alarmstand. Je zou ook hier een aantal pulsen kunnen tellen voor een alarm.
  */

  pixels.begin();
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(200, 100, 0));  // oranje kleur voor opstarten tellen begint bij 0
  pixels.show();
  pinMode(overFlow, INPUT_PULLUP);
  pinMode(flowSensor, INPUT);

  attachInterrupt(digitalPinToInterrupt(27), ISRalarm, FALLING);  // interrupt aanzetten op de overflow

  pinMode(kraan0, OUTPUT);  // pins definiëren als uitgaande poorten

  pinMode(kraan1, OUTPUT);
  pinMode(kraan2, OUTPUT);
  pinMode(kraan3, OUTPUT);
  pinMode(kraan4, OUTPUT);
  pinMode(kraan5, OUTPUT);
  pinMode(kraan6, OUTPUT);

  pinMode(startButton, INPUT_PULLUP);

  pinMode(switch1, INPUT_PULLUP);  // schakelaars om de trays te selecteren die ingewaterd moeten worden
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);
  pinMode(switch5, INPUT_PULLUP);
  pinMode(switch6, INPUT_PULLUP);

  // kogelkranen OPEN zettem
  digitalWrite(kraan1, OPEN);  // nog onderzoeken of dit niet LOW moet zijn
  digitalWrite(kraan2, OPEN);  // is afhanklijk van het relaisblok
  digitalWrite(kraan3, OPEN);
  digitalWrite(kraan4, OPEN);
  digitalWrite(kraan5, OPEN);
  digitalWrite(kraan6, OPEN);

  digitalWrite(kraan0, HKDICHT);
  Serial.println("Voorzorg wacht na reset");
  delay(kraanPauze);  // voor de zekerheid wachten op de eindpositie van de kranen i.v.m. mogelijke reset

  rekStatus = 0;  // startten in wachtstand van het rek
}

void loop() {
  pixels.clear();
  if (rekStatus == 0) {
    digitalWrite(kraan0, HKDICHT);
    pixels.setPixelColor(0, pixels.Color(0, 150, 0));
    pixels.show();
  }
  // lezen van de buttons
  startButtonStatus = digitalRead(startButton);
  Serial.print("Startknop: ");
  Serial.println(startButtonStatus);
  Serial.print("Rekstatus: ");
  Serial.println(rekStatus);
  if (startButtonStatus == 0) {
    rekStatus = 1;
  }

  if (rekStatus == 1) {
    attachInterrupt(digitalPinToInterrupt(26), ISRpulsen, RISING);  // interrupt voor het tellen van de pulsen aanzetten, kan ook in de setup
    // als we begonnen zijn
    switch1Status = digitalRead(switch1);  // lezen van de schakelaars
    switch2Status = digitalRead(switch2);
    switch3Status = digitalRead(switch3);
    switch4Status = digitalRead(switch4);
    switch5Status = digitalRead(switch5);
    switch6Status = digitalRead(switch6);

    Serial.print("switch1Status ");
    Serial.println(switch1Status);
    Serial.print("switch2Status ");
    Serial.println(switch2Status);
    Serial.print("switch3Status ");
    Serial.println(switch3Status);
    Serial.print("switch4Status ");
    Serial.println(switch4Status);
    Serial.print("switch5Status ");
    Serial.println(switch5Status);
    Serial.print("switch6Status ");
    Serial.println(switch6Status);
    pixels.setPixelColor(0, pixels.Color(0, 0, 150));
    pixels.show();
    delay(5000);


    if (switch1Status == 0) {
      tray1Status = 1;
    }
    if (switch2Status == 0) {
      tray2Status = 1;
    }
    if (switch3Status == 0) {
      tray3Status = 1;
    }
    if (switch4Status == 0) {
      tray4Status = 1;
    }
    if (switch5Status == 0) {
      tray5Status = 1;
    }
    if (switch6Status == 0) {
      tray6Status = 1;
    }
  }

  //afhandelen standen van de schakelaars
  // Tray 1
  if (tray1Status == 1 && rekStatus == 1) {  // schakelaar 1 stond dus geselecteerd
    trayAfhandeling(kraan1, 1, tray1Status, switch1Status);
  }

  // Tray 2
  if (tray2Status == 1 && rekStatus == 1) {  //staat schakelaar 2 aan en is tray 1 klaar?
    trayAfhandeling(kraan2, 2, tray2Status, switch2Status);
  }

  //Tray 3
  if (tray3Status == 1 && rekStatus == 1) {  //staat schakelaar 2 aan?
    trayAfhandeling(kraan3, 3, tray3Status, switch3Status);
  }

  // Tray 4
  if (tray4Status == 1 && rekStatus == 1) {  //staat schakelaar 2
    trayAfhandeling(kraan4, 4, tray4Status, switch4Status);
  }

  // Tray 5
  if (tray5Status == 1 && rekStatus == 1) {  //staat schakelaar 2 aan en is tray 5 klaar?
    trayAfhandeling(kraan5, 5, tray5Status, switch5Status);
  }

  // Tray 6
  if (tray6Status == 1 && rekStatus == 1) {  //staat schakelaar 2 aan en is tray 5 klaar?
    trayAfhandeling(kraan6, 6, tray6Status, switch6Status);
  }

  rekStatus = 0;  // rek in de ruststand

  while (rekStatus == 3) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      Serial.println("Alarm");  //Zolang de alarmstand duurt blijft deze booschap herhalen en gebeurt er niets
      previousMillis = currentMillis;
    }
  }
}

void trayAfhandeling(int kraan, int nummer, int traystatus, int switchstatus) {
  traystatus = 1;
  Serial.print("Tray ");
  Serial.print(nummer);
  Serial.println(" is aan de beurt");

  delay(2000);  // we gaan tray 1 vullen
                // bij het opstarten / resetten stonden alle kranen OPEN

  digitalWrite(kraan, DICHT);  // kogelkraan tray 1 DICHT
  Serial.println("tray kraan DICHT");
  Serial.println("Wachten op sluiten");
  delay(kraanPauze);
  digitalWrite(kraan0, HKOPEN);  // hoofdkraan OPEN om te vullen
  Serial.println("Hoofdkraan OPEN");
  delay(2000);
  /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, OPEN);       // hoofdkraan OPEN om te vullen
      Serial.println("Hoofdkraan OPEN");
    }
    */
  for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
    Serial.print("Pulsen tellen: ");
    Serial.println(pulseCount);
    delay(6);
  }

  digitalWrite(kraan0, HKDICHT);  // als pulsen voldoende zijn dan hoofdkraan DICHT
  Serial.println("Hoofdkraan DICHT");
  Serial.println("Start inwateren");
  pulseCount = 0;           // pulsen terugzetten naar 0
  delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
  Serial.println("Inwateren klaar, tray kraan weer OPEN");
  digitalWrite(kraan, OPEN);  // kraan 1 OPEN (staan nu allemaal OPEN)
  delay(leegloop * 6000);     // tijd gunnen voor het leeglOPEN *60000 om aan minuten te komen
  Serial.println("Leeglopen klaar. Naar volgende tray, anders klaar");
  traystatus = 0;    // tray status 0
  switchstatus = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
  delay(2000);
}



void ISRpulsen() {
  pulseCount++;  // Interrupt routine voor het ophogen van de pulse teller
}

void ISRalarm() {
  rekStatus = 3;               // systeem komt in alarmstand na pulsen uit de overflow sensor
  digitalWrite(kraan0, OPEN);  // hoofdkraan DICHT
  digitalWrite(kraan1, OPEN);  // nog onderzoeken of dit niet LOW moet zijn
  digitalWrite(kraan2, OPEN);  // is afhanklijk van het relaisblok
  digitalWrite(kraan3, OPEN);
  digitalWrite(kraan4, OPEN);
  digitalWrite(kraan5, OPEN);
  digitalWrite(kraan6, OPEN);
}