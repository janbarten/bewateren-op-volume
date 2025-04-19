#include <WiFi.h>
#include <WebServer.h>

WebServer server(80);

int pulsenPerLiter = 450;  // Standaardwaarde

// Access Point naam
const char* ssid = "ellemeet1";
const char* password = "esp32config";  // Wachtwoord van het WiFi-netwerk

void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleSetValue() {
  if (server.hasArg("ppl")) {
    pulsenPerLiter = server.arg("ppl").toInt();
    Serial.println("Nieuwe waarde: " + String(pulsenPerLiter));
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

String getHTML() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Daphne's moestuin</title>
  <style>
    body { font-family: sans-serif; margin: 20px; }
    .menu { margin-bottom: 1em; }
    .submenu { margin-left: 1em; }
    .item { cursor: pointer; color: blue; }
  </style>
</head>
<body>
  <h1>Instellingenmenu</h1>
  <div class="menu">
    <div class="item" onclick="toggle('settings')">Instellingen openen</div>
    <div id="settings" class="submenu" style="display:none">
      <form action="/set" method="GET">
        <label for="ppl">Pulsen per liter:</label>
        <input type="number" id="ppl" name="ppl" value=")rawliteral";

  html += pulsenPerLiter;

  html += R"rawliteral(">
        <button type="submit">Opslaan</button>
      </form>
    </div>
  </div>

  <script>
    function toggle(id) {
      var el = document.getElementById(id);
      el.style.display = el.style.display === 'none' ? 'block' : 'none';
    }
  </script>
</body>
</html>
)rawliteral";

  return html;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Opstarten...");

  WiFi.mode(WIFI_AP);
  Serial.println("WiFi modus gezet op AP");

  bool apSuccess = WiFi.softAPConfig(
    IPAddress(192, 168, 4, 1),
    IPAddress(192, 168, 4, 1),
    IPAddress(255, 255, 255, 0));

  Serial.println(apSuccess ? "AP-config ingesteld" : "AP-config FAILED");

  bool wifiSuccess = WiFi.softAP(ssid, password);
  Serial.println(wifiSuccess ? "Access Point gestart" : "AP start FAILED");

  Serial.print("IP adres: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleSetValue);
  server.begin();
  Serial.println("Webserver gestart");
}

void loop() {
  server.handleClient();
}