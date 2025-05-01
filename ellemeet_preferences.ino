#include <Preferences.h>

Preferences preferences;

uint tray1Status = 0;  // 0 = wachten, 1 = vullen, 2 = inwateren, 3 = leeglopen
uint tray2Status = 0;
uint tray3Status = 0;
uint tray4Status = 0;
uint tray5Status = 0;
uint tray6Status = 0;


void setup() {
  Serial.begin(115200);
  Serial.println();
  preferences.begin("ellemeet", false);

  tray1Status = preferences.getUInt("tray1Status", 0);
  tray2Status = preferences.getUInt("tray2Status", 0);
  tray3Status = preferences.getUInt("tray3Status", 0);
  tray4Status = preferences.getUInt("tray4Status", 0);
  tray5Status = preferences.getUInt("tray5Status", 0);
  tray6Status = preferences.getUInt("tray6Status", 0);

  Serial.printf("Retrived tray1Status: %u\n", tray1Status);
  Serial.printf("Retrived tray2Status: %u\n", tray2Status);
  // Random set tray status
  tray1Status = random(0, 3);
  tray2Status = random(0, 3);
  tray3Status = random(0, 3);
  tray4Status = random(0, 3);
  tray5Status = random(0, 3);
  tray6Status = random(0, 3);

  // Print the counter to Serial Monitor
  Serial.printf("Current tray1Status: %u\n", tray1Status);
  Serial.printf("Current tray2Status: %u\n", tray2Status);
  Serial.printf("Current tray3Status: %u\n", tray3Status);
  Serial.printf("Current tray4Status: %u\n", tray4Status);
  Serial.printf("Current tray5Status: %u\n", tray5Status);
  Serial.printf("Current tray6Status: %u\n", tray6Status);

  // Store the counter to the Preferences
  preferences.putUInt("tray1Status", tray1Status);
  preferences.putUInt("tray2Status", tray2Status);
  preferences.putUInt("tray3Status", tray3Status);
  preferences.putUInt("tray4Status", tray4Status);
  preferences.putUInt("tray5Status", tray5Status);
  preferences.putUInt("tray6Status", tray6Status);

  // Close the Preferences
  preferences.end();

  // Wait 10 seconds
  Serial.println("Restarting in 10 seconds...");
  delay(10000);

  // Restart ESP
  ESP.restart();
}

void loop() {}
