#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define TRIGGER 5
#define READ 4 // GPIO4 (D2) é seguro para ESP8266
#define SWITCH 16 // GPIO16 (D0)

const char *apSSID = "ESP8266_Config";
const char *apPassword = "12345678";

ESP8266WebServer configServer(80);
ESP8266WebServer dataServer(8080);
String lastIP = "";

void handleRoot();
void handleSave();
void handleDataPage();
void saveIP(String ip);
String loadIP();
void checkRAM();
float readDistanceCM();
String scanNetworks();

void saveIP(String ip) {
  for (int i = 0; i < ip.length(); i++) {
    EEPROM.write(i, ip[i]);
  }
  EEPROM.write(ip.length(), '\0');
  EEPROM.commit();
}

String loadIP() {
  String ip = "";
  for (int i = 0; i < 64; i++) {
    char c = EEPROM.read(i);
    if (c == '\0') break;
    ip += c;
  }
  return ip;
}

void checkRAM() {
  Serial.print("Memória livre: ");
  Serial.println(ESP.getFreeHeap());
}

String scanNetworks() {
  String networks = "<select name='ssid'>";
  int n = WiFi.scanNetworks();
  Serial.println("Redes Wi-Fi encontradas:");
  for (int i = 0; i < n; i++) {
    Serial.println(WiFi.SSID(i));
    networks += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
  }
  networks += "</select>";
  return networks;
}

float readDistanceCM() {
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  int duration = pulseIn(READ, HIGH);
  return duration * 0.034 / 2;
}

void handleRoot() {
  Serial.println("handleRoot chamado");
  String page = "<html><body>";
  page += "<h2>Configuração Wi-Fi</h2>";
  page += "<form action='/save' method='POST'>";
  page += "Selecione a rede Wi-Fi: " + scanNetworks() + "<br>";
  page += "Senha: <input type='password' name='password'><br>";
  page += "<input type='submit' value='Salvar'>";
  page += "</form>";
  page += "<h3>Último IP conectado: " + lastIP + "</h3>";
  page += "</body></html>";
  configServer.send(200, "text/html", page);
}

void handleSave() {
  Serial.println("handleSave chamado");
  String ssid = configServer.arg("ssid");
  String password = configServer.arg("password");

  Serial.print("Tentando conectar a: ");
  Serial.println(ssid);

  configServer.send(200, "text/html", "<html><body><h2>Conectando a " + ssid + "...</h2></body></html>");

  WiFi.softAPdisconnect(true);
  WiFi.begin(ssid.c_str(), password.c_str());

  int timeout = 30;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    lastIP = WiFi.localIP().toString();
    saveIP(lastIP);
    Serial.println("\nConectado ao Wi-Fi!");
    Serial.print("IP recebido: ");
    Serial.println(lastIP);
    configServer.send(200, "text/html", "<html><body><h2>Conectado! IP: " + lastIP + "</h2></body></html>");
    dataServer.begin();
  } else {
    Serial.println("\nFalha na conexão ao Wi-Fi!");
    configServer.send(200, "text/html", "<html><body><h2>Erro ao conectar ao Wi-Fi!</h2></body></html>");
  }
}

void handleDataPage() {
  String page = "<html><body>";
  page += "<h2>Monitoramento ESP8266</h2>";
  page += "<p>IP Atual: " + lastIP + "</p>";
  page += "<p>Distância do sensor: " + String(readDistanceCM()) + " cm</p>";
  page += "<p>Estado da porta SWITCH: " + String(digitalRead(SWITCH)) + "</p>";
  page += "<p>Memória livre: " + String(ESP.getFreeHeap()) + " bytes</p>";
  page += "</body></html>";
  dataServer.send(200, "text/html", page);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Iniciando setup...");

  EEPROM.begin(64);
  lastIP = loadIP();

  WiFi.mode(WIFI_AP);
  bool apOk = WiFi.softAP(apSSID, apPassword);
  if (apOk) {
    Serial.println("Hotspot criado!");
    Serial.print("IP do AP: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("Falha ao criar Hotspot!");
  }

  configServer.on("/", handleRoot);
  configServer.on("/save", handleSave);
  configServer.begin();
  Serial.println("Servidor de configuração iniciado na porta 80");

  dataServer.on("/", handleDataPage);
  dataServer.begin();
  Serial.println("Servidor de dados iniciado na porta 8080");

  pinMode(TRIGGER, OUTPUT);
  pinMode(READ, INPUT);
  pinMode(SWITCH, INPUT);

  // Mensagem de instrução para acessar a página de configuração
  Serial.println("Acesse http://" + WiFi.softAPIP().toString() + " no seu navegador para configurar o Wi-Fi.");
}

void loop() {
  configServer.handleClient();
  dataServer.handleClient();
  checkRAM();
  delay(5000);
}
