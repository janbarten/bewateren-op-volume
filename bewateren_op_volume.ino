// Code door J. Barten
// maart 2025

// bij het opstarten starten we in de wachtstand
byte tray1Status = 0;  // 0 = wachten, 1 = vullen, 2 = inwateren, 3 = leeglopen
byte tray2Status = 0;
byte tray3Status = 0;
byte tray4Status = 0;
byte tray5Status = 0;
byte tray6Status = 0;

// bewateringskraan toekennen aan een pin
byte kraan0 = 10;

// 6 kogelkranen toekennen aan een pin
byte kraan1 = 11;
byte kraan2 = 12;
byte kraan3 = 13;
byte kraan4 = 14;
byte kraan5 = 15;
byte kraan6 = 16;

// 6 tray schakelaars toekennen aan een pin
byte switch1 = 17;
byte switch2 = 18;
byte switch3 = 19;
byte switch4 = 20;
byte switch5 = 21;
byte switch6 = 22;

// 6 tray schakelaarstanden kunnen opslaan in een boolse waarde
bool switch1State = 0;
bool switch2State = 0;
bool switch3State = 0;
bool switch4State = 0;
bool switch5State = 0;
bool switch6State = 0;

// start schakelaar
byte switch0 = 23;

// puls teller voor de flowmeter
unsigned int pulseCount = 0;

void setup() {
  Serial.begin(9600);  // instellen snelheid seriële communicatietrayStatus = 0;

  pinMode(kraan0, OUTPUT);  // pins definiëren als uitgaande poorten
  pinMode(kraan1, OUTPUT);
  pinMode(kraan2, OUTPUT);
  pinMode(kraan3, OUTPUT);
  pinMode(kraan4, OUTPUT);
  pinMode(kraan5, OUTPUT);
  pinMode(kraan6, OUTPUT);

  pinMode(switch0, INPUT);  // pins definiëren als ingaande poorten
  pinMode(switch1, INPUT);
  pinMode(switch2, INPUT);
  pinMode(switch3, INPUT);
  pinMode(switch4, INPUT);
  pinMode(switch5, INPUT);
  pinMode(switch6, INPUT);

  // kogelkranen open zettem
  digitalWrite(kraan1, HIGH);  // nog onderzoeken of dit niet LOW moet zijn
  digitalWrite(kraan2, HIGH);  // is afhanklijk van het relaisblok
  digitalWrite(kraan3, HIGH);
  digitalWrite(kraan4, HIGH);
  digitalWrite(kraan5, HIGH);
  digitalWrite(kraan6, HIGH);
}

void loop() {
  if (digitalRead(switch0 == HIGH)) {    // lees de startschakelaar. Dit moet een drukchakelaar zijn
    if (digitalRead(switch1 == HIGH)) {  // lees trayschakelaar 1
      switch1State = 1;                  // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch2 == HIGH)) {  // lees trayschakelaar 2
      switch2State = 1;                  // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch3 == HIGH)) {  // lees trayschakelaar 3
      switch3State = 1;                  // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch4 == HIGH)) {  // lees trayschakelaar 4
      switch4State = 1;                  // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch5 == HIGH)) {  // lees trayschakelaar 5
      switch5State = 1;                  // sla de stand op in een boolse waarde
    }
    if (digitalRead(switch6 == HIGH)) {  // lees trayschakelaar 6
      bool switch6State = 1;             // sla de stand op in een boolse waarde
    }
  }
  //afhandelen standen van de schakelaars

  if (switch1State == 1) {  // staat schakelaar 1 aan?
    // traystatus 1
    // bij het opstarten stonden alle kranen open
    // traykraan 1 dicht
    // kraan 0 open
    // pulsen tellen
    // als pulsen voldoende kraan 0 dicht
    // tray status 2
    // timer starten voor het inwateren
    // timer klaar?
    // tray status 3
    // kraan 1 open (staan nu allemaal open)
    // tray status 0
    // switchState 1 terug naar 0
  }

  if (switch2State == 1 && tray2Status == 0) {  //staat schakelaar 2 aan en is tray 1 klaar?
    // tray2Status = 1
    // bij het opstarten stonden alle kranen open
    // traykraan 2 dicht
    // kraan 0 open
    // pulsen tellen
    // als pulsen voldoende kraan 0 dicht
    // tray status 2
    // timer starten voor het inwateren
    // timer klaar?
    // tray status 3
    // kraan 1 open (staan nu allemaal open)
    // tray2status = 0
    // switch2State = 0
  }
  //enz
}
