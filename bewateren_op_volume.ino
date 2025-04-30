// Code door J. Barten
// april 2025

/*
V1.3
- pinbeztting volgens voorstel toegevoegd
- afhandeling tray 2 aangepast
- de alarm ISR aangepast. Alarm mededeling naar de loop
*/


/* 
Hardware aansluitingen
Goed kijken naar de pinbezetting op het ESP niet alle pinnen zijn beschikbaar voor elke taak.
Hieronder de pinbezetting volgens voorstel!!
*/

// hoofdkraan toekennen aan een pin
int kraan0 = 12;

// 6 kogelkranen toekennen aan een pin
int kraan1 = 20;
int kraan2 = 24;
int kraan3 = 25;
int kraan4 = 26;
int kraan5 = 27;
int kraan6 = 32;

// reset- en een startknop toekennen aan een pin
int resetButton = 33;  // om uit de alarm stand te komen en naar rust stand
int startSwitch = 36;

// tray schakelaars toekennen aan een pin
int switch1 = 4;
int switch2 = 13;
int switch3 = 14;
int switch4 = 15;
int switch5 = 16;
int switch6 = 17;

// pin voor de pulsen naar de interrupt
int flowSensor = 34;
int overFlow = 35;

/*
Variabelen
*/

// status van het hele rek, bij opstart in wachtstand
bool rekState = 0;  // 0 = het rek is in rust/wachtstand, 1 = het rek in actief met bewateren 3 = ALARM

// bij het opstarten starten we in alle trays in de wachtstand
int tray1Status = 0;  // 0 = wachten, 1 = vullen, 2 = inwateren, 3 = leeglopen
int tray2Status = 0;
int tray3Status = 0;
int tray4Status = 0;
int tray5Status = 0;
int tray6Status = 0;

// 6 tray schakelaarstanden kunnen opslaan in een boolse waarde
bool switch1State = 0;
bool switch2State = 0;
bool switch3State = 0;
bool switch4State = 0;
bool switch5State = 0;
bool switch6State = 0;

// puls teller voor de flowmeter
unsigned int pulseCount = 0;

// aantal pulsen per liter
unsigned int pulseWaarde = 630;

// inwatering en leeglopen duur in minuten
int inwateren = 20;
int leegloop = 2;
int kraanPauze = 20000;  // 20 seconden voor openen en sluiten

unsigned long previousMillis = 0;  // gebruikt om niet blokkerend wachten


void setup() {
  Serial.begin(9600);  // instellen snelheid seriële communicatie

  /*
  Ter overweging, bij het minste geringste gaat het rek in alarmstand. Je zou ook hier een aantal pulsen kunnen tellen voor een alarm.
  */
  attachInterrupt(overFlow, ISRalarm, CHANGE);  // interrupt aanzetten op de overflow

  pinMode(kraan0, OUTPUT);  // pins definiëren als uitgaande poorten
  pinMode(kraan1, OUTPUT);
  pinMode(kraan2, OUTPUT);
  pinMode(kraan3, OUTPUT);
  pinMode(kraan4, OUTPUT);
  pinMode(kraan5, OUTPUT);
  pinMode(kraan6, OUTPUT);

  pinMode(resetButton, INPUT_PULLUP);  // pins definiëren als ingaande poorten. Buttons zijn standaard HIGH
  pinMode(startSwitch, INPUT_PULLUP);
  pinMode(switch1, INPUT_PULLUP);
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);
  pinMode(switch5, INPUT_PULLUP);
  pinMode(switch6, INPUT_PULLUP);

  // hoofdkraan dicht
  digitalWrite(kraan0, LOW);

  // kogelkranen open zettem
  digitalWrite(kraan1, HIGH);  // nog onderzoeken of dit niet LOW moet zijn
  digitalWrite(kraan2, HIGH);  // is afhanklijk van het relaisblok
  digitalWrite(kraan3, HIGH);
  digitalWrite(kraan4, HIGH);
  digitalWrite(kraan5, HIGH);
  digitalWrite(kraan6, HIGH);

  rekState = 0;  // startten in wachtstand van het rek
}

void loop() {
  if (digitalRead(resetButton) == LOW) {  // is de reset button ingedrukt?
    abort();                              // Esp herstart en komt door de void setup() in de rust stand met hoofdkraan dicht en kogelkranen open
  }

  if (digitalRead(startSwitch == LOW)) {  // lees de startschakelaar. Dit moet een drukchakelaar zijn
    rekState = 1;                         //het rek wordt actief

    attachInterrupt(flowSensor, ISRpulsen, RISING);  // we zetten de interruptPin aan zodat we de pulsen van van de flowsensor kunnen tellen
    if (digitalRead(switch1 == LOW)) {               // lees trayschakelaar 1
      switch1State = 1;                              // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch2 == LOW)) {  // lees trayschakelaar 2
      switch2State = 1;                 // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch3 == LOW)) {  // lees trayschakelaar 3
      switch3State = 1;                 // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch4 == LOW)) {  // lees trayschakelaar 4
      switch4State = 1;                 // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch5 == LOW)) {  // lees trayschakelaar 5
      switch5State = 1;                 // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch6 == LOW)) {  // lees trayschakelaar 6
      bool switch6State = 1;            // sla de stand op in een boolse waarde
    }
  }

  //afhandelen standen van de schakelaars

  if (switch1State == 1 && rekState == 1) {  // schakelaar 1 staat aan
    tray1Status = 1;                         // we gaan tray 1 vullen
                                             // bij het opstarten stonden alle kranen open
    digitalWrite(kraan1, LOW);               // kogelkraan tray 1 dicht
    delay(kraanPauze);                       // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {       // pulsen tellen
      digitalWrite(kraan0, HIGH);            // hoofdkraan open om te vullen
    }
    digitalWrite(kraan0, LOW);   // als pulsen voldoende zijn dankraan 0 dicht
    pulseCount = 0;              // pulsen terugzetten naar 0
    tray1Status = 2;             // traystatus 2 = inwateren
    delay(inwateren * 60000);    // timer starten voor het inwateren *60000 om aan minuten te komen
    tray1Status = 3;             // we gaan tray 1 laten leeglopen
    digitalWrite(kraan1, HIGH);  // kogelkraan tray 1 dicht// kraan 1 open (staan nu allemaal open)
    delay(leegloop * 60000);     // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    tray1Status = 0;             // tray status 0
    switch1State = 0;            // leesstatus van trayschakelaar van 1 terug naar 0
  }

  if (switch2State == 1 && tray1Status == 0 && rekState == 1) {  //staat schakelaar 2 aan en is tray 1 klaar?
    tray2Status = 1;                                             // we gaan tray 2 vullen
                                                                 // bij het opstarten stonden alle kranen open
    digitalWrite(kraan2, LOW);                                   // kogelkraan tray 1 dicht
    delay(kraanPauze);                                           // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {                           // pulsen tellen
      digitalWrite(kraan0, HIGH);                                // hoofdkraan open om te vullen
    }
    digitalWrite(kraan0, LOW);   // als pulsen voldoende zijn dankraan 0 dicht
    pulseCount = 0;              // pulsen terugzetten naar 0
    tray2Status = 2;             // traystatus 2 = inwateren
    delay(inwateren * 60000);    // timer starten voor het inwateren *60000 om aan minuten te komen
    tray2Status = 3;             // we gaan tray 1 laten leeglopen
    digitalWrite(kraan2, HIGH);  // kogelkraan tray 2 dicht open (staan nu allemaal open)
    delay(leegloop * 60000);     // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    tray2Status = 0;             // tray status 0, tray is leeg
    switch2State = 0;            // leesstatus van trayschakelaar van 1 terug naar 0
  }
  // nog 4 trays afhandelen

  rekState = 0;                             // rek terug zetten in de ruststand
  
  while (rekState == 3) {                   // is het rek in de alarm stand??
    if (digitalRead(resetButton) == LOW) {  // is de reset button ingedrukt?
      abort();                              // Esp herstart
    }
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      Serial.println("Alarm");  //Zolang de alarmstand duurt blijft deze booschap herhalen en gebeurt er niets
      previousMillis = currentMillis;
    }
  }
}

void ISRpulsen() {
  pulseCount++;  // Interrupt routine voor het ophogen van de pulse teller
}

void ISRalarm() {
  rekState = 3;                // systeem komt in alarmstand na pulsen uit de overflow sensor
  digitalWrite(kraan0, LOW);   // hoofdkraan dicht
  digitalWrite(kraan1, HIGH);  // nog onderzoeken of dit niet LOW moet zijn
  digitalWrite(kraan2, HIGH);  // is afhanklijk van het relaisblok
  digitalWrite(kraan3, HIGH);
  digitalWrite(kraan4, HIGH);
  digitalWrite(kraan5, HIGH);
  digitalWrite(kraan6, HIGH);
}
