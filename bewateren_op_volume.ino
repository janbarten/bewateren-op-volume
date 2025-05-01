// Code door J. Barten
// april 2025

/*
V2.2
- pinbezetting gewijzigd. P2 mag niet aangesloten zijn bij upload
- getest met relais blok

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


/* 
Hardware aansluitingen
Goed kijken naar de pinbezetting op het ESP niet alle pinnen zijn beschikbaar voor elke taak.
Hieronder de pinbezetting volgens voorstel!!
*/

// hoofdkraan toekennen aan een pin
int kraan0 = 2;

// 6 kogelkranen toekennen aan een pin
int kraan1 = 19;
int kraan2 = 23;
int kraan3 = 25;
int kraan4 = 26;
int kraan5 = 27;
int kraan6 = 32;

// reset- en een startknop toekennen aan een pin
int resetButton = 33;  // om uit de alarm stand te komen en naar rust stand
int startButton = 18;

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
bool rekState = 0;  // 0 = het rek is in rust/wachtstand, 1 = het rek in actief met bewateren  3 = ALARM

// bij het opstarten starten we in alle trays in de wachtstand
int tray1Status = 0;  // 0 = wachten, 1 = vullen, 2 = inwateren, 3 = leeglopen
int tray2Status = 0;
int tray3Status = 0;
int tray4Status = 0;
int tray5Status = 0;
int tray6Status = 0;

// detectie of de start en resetknop zijn ingedrukt (om te debouncen)
bool startButtonState;
bool resetSwitchState;
bool laststartButtonState = 0;
bool lastresetSwitchState = 0;

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
int inwateren = 2;
int leegloop = 2;
int kraanPauze = 5000;  // 20 seconden voor openen en sluiten

unsigned long previousMillis = 0;  // gebruikt om niet blokkerend wachten


void setup() {
  Serial.begin(9600);  // instellen snelheid seriële communicatie

  /*
  Ter overweging, bij het minste geringste gaat het rek in alarmstand. Je zou ook hier een aantal pulsen kunnen tellen voor een alarm.
  */
  pinMode(overFlow, INPUT);
  pinMode(flowSensor, INPUT);

  attachInterrupt(overFlow, ISRalarm, CHANGE);      // interrupt aanzetten op de overflow
  attachInterrupt(resetButton, ISRreset, FALLING);  // interrupt aanzetten voor de reset

  pinMode(kraan0, OUTPUT);  // pins definiëren als uitgaande poorten
  pinMode(kraan1, OUTPUT);
  pinMode(kraan2, OUTPUT);
  pinMode(kraan3, OUTPUT);
  pinMode(kraan4, OUTPUT);
  pinMode(kraan5, OUTPUT);
  pinMode(kraan6, OUTPUT);

  pinMode(resetButton, INPUT_PULLUP);  // pins definiëren als ingaande poorten. Buttons zijn standaard HIGH
  pinMode(startButton, INPUT_PULLUP);

  pinMode(switch1, INPUT_PULLUP);  // schakelaars om de trays te selecteren die ingewaterd moeten worden
  pinMode(switch2, INPUT_PULLUP);
  pinMode(switch3, INPUT_PULLUP);
  pinMode(switch4, INPUT_PULLUP);
  pinMode(switch5, INPUT_PULLUP);
  pinMode(switch6, INPUT_PULLUP);

  // hoofdkraan dicht, ook veilig bij stroomuitvalt
  digitalWrite(kraan0, HIGH);  // relais lampje is uit, kraan is dicht

  // kogelkranen open zettem
  digitalWrite(kraan1, HIGH);  // nog onderzoeken of dit niet LOW moet zijn
  digitalWrite(kraan2, HIGH);  // is afhanklijk van het relaisblok
  digitalWrite(kraan3, HIGH);
  digitalWrite(kraan4, HIGH);
  digitalWrite(kraan5, HIGH);
  digitalWrite(kraan6, HIGH);
  delay(kraanPauze);  // voor de zekerheid wachten op de eindpositie van de kranen
  rekState = 0;       // startten in wachtstand van het rek
}

void loop() {
  // lezen van de buttons
  startButtonState = digitalRead(startButton);
  Serial.printf("Startknop: %u\n", startButtonState);
  Serial.printf("Rekstatus: %u\n", rekState);
  if (startButtonState == 0) {
    rekState = 1;
    Serial.println("Rekstatus = 1");
  }

  if (rekState == 1) {
    attachInterrupt(flowSensor, ISRpulsen, RISING);  // interrupt voor het tellen van de pulsen aanzetten, kan ook in de setup
    // als we begonnen zijn
    switch1State = digitalRead(switch1);  // lezen van de schakelaars
    switch2State = digitalRead(switch2);
    switch3State = digitalRead(switch3);
    switch4State = digitalRead(switch4);
    switch5State = digitalRead(switch5);
    switch6State = digitalRead(switch6);

    Serial.printf("switch1State %u\n", switch1State);
    Serial.printf("switch2State %u\n", switch2State);
    Serial.printf("switch3State %u\n", switch3State);
    Serial.printf("switch4State %u\n", switch4State);
    Serial.printf("switch5State %u\n", switch5State);
    Serial.printf("switch6State %u\n", switch6State);
    delay(5000);

    if (switch1State == 0) {
      tray1Status = 1;
    }
    if (switch2State == 0) {
      tray2Status = 1;
    }
    if (switch3State == 0) {
      tray3Status = 1;
    }
    if (switch4State == 0) {
      tray4Status = 1;
    }
    if (switch5State == 0) {
      tray5Status = 1;
    }
    if (switch5State == 0) {
      tray6Status = 1;
    }
  }

  //afhandelen standen van de schakelaars
  // Tray 1
  if (tray1Status == 1) {  // schakelaar 1 stond dus geselecteerd
    Serial.println("Tray 1 is aan de beurt");
    delay(2000);  // we gaan tray 1 vullen
                  // bij het opstarten / resetten stonden alle kranen open

    digitalWrite(kraan1, LOW);  // kogelkraan tray 1 dicht
    Serial.println("Traykraan 1 dicht");
    Serial.println("Wachten op sluiten");
    delay(kraanPauze);
    digitalWrite(kraan0, LOW);  // hoofdkraan open om te vullen
    Serial.println("Hoofdkraan open");
    delay(2000);
    /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, LOW);       // hoofdkraan open om te vullen
      Serial.println("Hoofdkraan open");
    }
    */
    for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
      Serial.printf("Pulsen tellen: %u\n", pulseCount);
      delay(20);
    }
    digitalWrite(kraan0, HIGH);  // als pulsen voldoende zijn dan hoofdkraan dicht
    Serial.println("Hoofdkraan dicht");
    Serial.println("Start inwateren tray 1");
    pulseCount = 0;           // pulsen terugzetten naar 0
    tray1Status = 2;          // traystatus 2 = inwateren
    delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
    tray1Status = 3;          // we gaan tray 1 laten leeglopen
    Serial.println("Inwateren klaar, tray kraan 1 weer open");
    digitalWrite(kraan1, HIGH);  // kraan 1 open (staan nu allemaal open)
    delay(leegloop * 6000);      // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    Serial.println("Leeglopen klaar. Naar volgende tray, anders klaar");
    tray1Status = 0;   // tray status 0
    switch1State = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
    delay(2000);
  }

  // Tray 2
  if (tray2Status == 1 && rekState == 1) {  //staat schakelaar 2 aan en is tray 1 klaar?
    tray2Status = 1;                        // we gaan tray 2 vullen
    Serial.println("Tray 2 is aan de beurt");
    delay(2000);                // bij het opstarten stonden alle kranen open
    digitalWrite(kraan2, LOW);  // kogelkraan tray 2 dicht
    Serial.println("Traykraan 2 dicht");
    Serial.println("Wachten op sluiten");
    delay(kraanPauze);
    digitalWrite(kraan0, LOW);  // hoofdkraan open om te vullen
    Serial.println("Hoofdkraan open");
    delay(2000);
    /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, LOW);       // hoofdkraan open om te vullen
      Serial.println("Hoofdkraan open");
    }
    */
    for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
      Serial.printf("Pulsen tellen: %u\n", pulseCount);
      delay(20);
    }
    digitalWrite(kraan0, HIGH);  // als pulsen voldoende zijn dan hoofdkraan dicht
    Serial.println("Hoofdkraan dicht");
    Serial.println("Start inwateren tray 2");
    pulseCount = 0;           // pulsen terugzetten naar 0
    tray2Status = 2;          // traystatus 2 = inwateren
    delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
    tray2Status = 3;          // we gaan tray 2 laten leeglopen
    Serial.println("Inwateren klaar, tray kraan 2 weer open");
    digitalWrite(kraan2, HIGH);  // kraan 1 open (staan nu allemaal open)
    delay(leegloop * 6000);      // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    Serial.println("Leeglopen klaar. Naar volgende tray, anders klaar");
    tray2Status = 0;   // tray status 0
    switch2State = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
  }

  //Tray 3
  if (tray3Status == 1 && rekState == 1) {  //staat schakelaar 2 aan?
    tray3Status = 1;                        // we gaan tray 3 vullen
    Serial.println("Tray 3 is aan de beurt");
    delay(2000);                // bij het opstarten stonden alle kranen open
    digitalWrite(kraan3, LOW);  // kogelkraan tray 3 dicht
    Serial.println("Traykraan 3 dicht");
    Serial.println("Wachten op sluiten");
    delay(kraanPauze);
    digitalWrite(kraan0, LOW);  // hoofdkraan open om te vullen
    Serial.println("Hoofdkraan open");
    delay(2000);
    /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, LOW);       // hoofdkraan open om te vullen
      Serial.println("Hoofdkraan open");
    }
    */
    for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
      Serial.printf("Pulsen tellen: %u\n", pulseCount);
      delay(20);
    }
    digitalWrite(kraan0, HIGH);  // als pulsen voldoende zijn dan hoofdkraan dicht
    Serial.println("Hoofdkraan dicht");
    Serial.println("Start inwateren tray 3");
    pulseCount = 0;           // pulsen terugzetten naar 0
    tray3Status = 2;          // traystatus 2 = inwateren
    delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
    tray3Status = 3;          // we gaan tray 2 laten leeglopen
    Serial.println("Inwateren klaar, tray kraan 3 weer open");
    digitalWrite(kraan3, HIGH);  // kraan 3 open (staan nu allemaal open)
    delay(leegloop * 6000);      // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    Serial.println("Leeglopen klaar. Naar volgende tray, anders klaar");
    tray3Status = 0;   // tray status 0
    switch3State = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
  }

  // Tray 4
  if (tray4Status == 1 && rekState == 1) {  //staat schakelaar 2
    tray4Status = 1;                        // we gaan tray 4 vullen
    Serial.println("Tray 4 is aan de beurt");
    delay(2000);                // bij het opstarten stonden alle kranen open
    digitalWrite(kraan4, LOW);  // kogelkraan tray 4 dicht
    Serial.println("Traykraan 4 dicht");
    Serial.println("Wachten op sluiten");
    delay(kraanPauze);
    digitalWrite(kraan0, LOW);  // hoofdkraan open om te vullen
    Serial.println("Hoofdkraan open");
    delay(2000);
    /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, LOW);       // hoofdkraan open om te vullen
      Serial.println("Hoofdkraan open");
    }
    */
    for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
      Serial.printf("Pulsen tellen: %u\n", pulseCount);
      delay(20);
    }
    digitalWrite(kraan0, HIGH);  // als pulsen voldoende zijn dan hoofdkraan dicht
    Serial.println("Hoofdkraan dicht");
    Serial.println("Start inwateren tray 4");
    pulseCount = 0;           // pulsen terugzetten naar 0
    tray4Status = 2;          // traystatus 2 = inwateren
    delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
    tray4Status = 3;          // we gaan tray 4 laten leeglopen
    Serial.println("Inwateren klaar, tray kraan 4 weer open");
    digitalWrite(kraan4, HIGH);  // kraan 4 open (staan nu allemaal open)
    delay(leegloop * 6000);      // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    Serial.println("Leeglopen klaar. Naar volgende tray, anders klaar");
    tray4Status = 0;   // tray status 0
    switch4State = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
  }

  // Tray 5
  if (tray5Status == 1 && rekState == 1) {  //staat schakelaar 2 aan en is tray 5 klaar?
    tray5Status = 1;                        // we gaan tray 5 vullen
    Serial.println("Tray 5 is aan de beurt");
    delay(2000);                // bij het opstarten stonden alle kranen open
    digitalWrite(kraan5, LOW);  // kogelkraan tray 5 dicht
    Serial.println("Traykraan 5 dicht");
    Serial.println("Wachten op sluiten");
    delay(kraanPauze);
    digitalWrite(kraan0, LOW);  // hoofdkraan open om te vullen
    Serial.println("Hoofdkraan open");
    delay(2000);
    /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, LOW);       // hoofdkraan open om te vullen
      Serial.println("Hoofdkraan open");
    }
    */
    for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
      Serial.printf("Pulsen tellen: %u\n", pulseCount);
      delay(20);
    }
    digitalWrite(kraan0, HIGH);  // als pulsen voldoende zijn dan hoofdkraan dicht
    Serial.println("Hoofdkraan dicht");
    Serial.println("Start inwateren tray 5");
    pulseCount = 0;           // pulsen terugzetten naar 0
    tray5Status = 2;          // traystatus 2 = inwateren
    delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
    tray5Status = 3;          // we gaan tray 5 laten leeglopen
    Serial.println("Inwateren klaar, tray kraan 5 weer open");
    digitalWrite(kraan5, HIGH);  // kraan 5 open (staan nu allemaal open)
    delay(leegloop * 6000);      // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    Serial.println("Leeglopen klaar. Naar volgende tray, anders klaar");
    tray5Status = 0;   // tray status 0
    switch5State = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
  }


  // Tray 6
  if (tray5Status == 1 && rekState == 1) {  //staat schakelaar 2 aan en is tray 5 klaar?
    tray5Status = 1;                        // we gaan tray 6 vullen
    Serial.println("Tray 6 is aan de beurt");
    delay(2000);                // bij het opstarten stonden alle kranen open
    digitalWrite(kraan6, LOW);  // kogelkraan tray 6 dicht
    Serial.println("Traykraan 6 dicht");
    Serial.println("Wachten op sluiten");
    delay(kraanPauze);
    digitalWrite(kraan0, LOW);  // hoofdkraan open om te vullen
    Serial.println("Hoofdkraan open");
    delay(2000);
    /*            // pauze om de kogelkraan de tijd te geven om te sluiten
    while (pulseCount < pulseWaarde) {  // pulsen tellen
      digitalWrite(kraan0, LOW);       // hoofdkraan open om te vullen
      Serial.println("Hoofdkraan open");
    }
    */
    for (pulseCount = 0; pulseCount <= pulseWaarde; pulseCount++) {
      Serial.printf("Pulsen tellen: %u\n", pulseCount);
      delay(20);
    }
    digitalWrite(kraan0, HIGH);  // als pulsen voldoende zijn dan hoofdkraan dicht
    Serial.println("Hoofdkraan dicht");
    Serial.println("Start inwateren tray 6");
    pulseCount = 0;           // pulsen terugzetten naar 0
    tray6Status = 2;          // traystatus 2 = inwateren
    delay(inwateren * 6000);  // timer starten voor het inwateren *60000 om aan minuten te komen
    tray6Status = 3;          // we gaan tray 6 laten leeglopen
    Serial.println("Inwateren klaar, tray kraan 6 weer open");
    digitalWrite(kraan6, HIGH);  // kraan 1 open (staan nu allemaal open)
    delay(leegloop * 6000);      // tijd gunnen voor het leeglopen *60000 om aan minuten te komen
    Serial.println("Leeglopen klaar. Einde van deze run");
    tray6Status = 0;   // tray status 0
    switch6State = 0;  // leesstatus van trayschakelaar van 1 terug naar 0
  }

  rekState = 0;  // rek in de ruststand

  while (rekState == 3) {
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

void ISRreset() {
  abort();
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
