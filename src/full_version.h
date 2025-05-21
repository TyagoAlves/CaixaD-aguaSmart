#ifndef FULL_VERSION_H
#define FULL_VERSION_H

// Este arquivo contém o código da versão completa
// Baseado no arquivo "vaidarcerto.ino"

#include <FS.h>

// --- Definições de pinos ---
#define TRIGGER 5
#define READ 4 // GPIO4 (D2) é seguro para ESP8266
#define SWITCH 16 // GPIO16 (D0)
#define FLASH_BUTTON 0 // GPIO0 normalmente é o botão FLASH em ESP8266

// --- Configurações do Broker MQTT ---
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;

// --- Tópicos MQTT ---
const char* topicBase = "meuESP32/testeBasico/";
const char* topicSubscribe = "meuESP32/testeBasico/entrada";
const char* topicPublish = "meuESP32/testeBasico/saida";

// --- Variáveis Globais ---
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
const long interval = 5000; // Envio a cada 5 segundos

// --- Variáveis para configuração web ---
const char *apSSID = "ESP8266_Config";
const char *apPassword = "12345678";
ESP8266WebServer configServer(80);
ESP8266WebServer dataServer(8080);
String lastIP = "";

// Contador de cliques do botão FLASH
int flashPressCount = 0;
unsigned long lastFlashPress = 0;

// --- Função para salvar IP na EEPROM ---
void saveIP(String ip) {
  EEPROM.begin(64);
  for (int i = 0; i < ip.length(); i++) {
    EEPROM.write(i, ip[i]);
  }
  EEPROM.write(ip.length(), '\0');
  EEPROM.commit();
}

// --- Função para carregar IP da EEPROM ---
String loadIP() {
  EEPROM.begin(64);
  String ip = "";
  for (int i = 0; i < 64; i++) {
    char c = EEPROM.read(i);
    if (c == '\0') break;
    ip += c;
  }
  return ip;
}

// --- Função para escanear redes disponíveis ---
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

// --- Página de configuração Wi-Fi ---
void ICACHE_FLASH_ATTR handleRoot() {
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

// --- Salvar configuração e conectar ao Wi-Fi ---
void ICACHE_FLASH_ATTR handleSave() {
  String ssid = configServer.arg("ssid");
  String password = configServer.arg("password");

  configServer.send(200, "text/html", "<html><body><h2>Conectando a " + ssid + "...</h2></body></html>");
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP_STA); // Mantém AP ativo ao conectar na rede
  WiFi.softAP(apSSID, apPassword); // Garante que o AP continue ativo
  WiFi.begin(ssid.c_str(), password.c_str());

  int timeout = 30;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    lastIP = WiFi.localIP().toString();
    saveIP(lastIP);
    if (SPIFFS.begin()) {
      File log = SPIFFS.open("/wifi_last.log", "w");
      if (log) {
        log.print("SSID conectado: "); log.println(ssid);
        log.print("IP DHCP: "); log.println(WiFi.localIP());
        log.close();
      }
      SPIFFS.end();
    }
    configServer.send(200, "text/html", "<html><body><h2>Conectado! IP: " + lastIP + "</h2></body></html>");
    dataServer.begin();
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID, apPassword);
    configServer.send(200, "text/html", "<html><body><h2>Erro ao conectar ao Wi-Fi! Senha incorreta ou rede indisponível.</h2><a href='/'>Voltar</a></body></html>");
  }
}

void handleDataPage() {
  String page = "<html><body>";
  page += "<h2>Monitoramento ESP8266</h2>";
  page += "<p>IP Atual: " + lastIP + "</p>";
  page += "<p>Distância do sensor: " + String(readDistanceCM()) + " cm</p>";
  page += "<p>Estado da porta SWITCH: " + String(digitalRead(SWITCH)) + "</p>";
  page += "<p>Memória livre: " + String(ESP.getFreeHeap()) + " bytes</p>";
  page += "<p>Versão: " + String(VERSION_NAME) + "</p>";
  page += "</body></html>";
  dataServer.send(200, "text/html", page);
}

void IRAM_ATTR handleFlashButton() {
  unsigned long now = millis();
  if (now - lastFlashPress > 500) {
    flashPressCount++;
    lastFlashPress = now;
    if (flashPressCount >= 3) {
      flashPressCount = 0;
      Serial.println("Reset Wi-Fi solicitado pelo botão FLASH!");
      WiFi.softAPdisconnect(true);
      WiFi.disconnect(true);
      delay(500);
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(apSSID, apPassword);
      Serial.println("Hotspot reativado para configuração!");
    }
  }
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado!");
      client.publish(topicPublish, "ESP8266 Online");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// --- Leitura do sensor ultrassônico ---
float readDistanceCM() {
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  int duration = pulseIn(READ, HIGH);
  return duration * 0.034 / 2;
}

void checkRAM() {
  int freeHeap = ESP.getFreeHeap();
  if (freeHeap < 5000) {
    Serial.println("AVISO: Memória baixa! " + String(freeHeap) + " bytes");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Iniciando setup...");
  
  // Exibe informações da versão
  printVersionInfo();

  EEPROM.begin(64);
  lastIP = loadIP();

  WiFi.mode(WIFI_AP_STA);
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
  pinMode(FLASH_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLASH_BUTTON), handleFlashButton, FALLING);

  client.setServer(mqttServer, mqttPort);
  randomSeed(micros());
}

void loop() {
  configServer.handleClient();
  dataServer.handleClient();
  checkRAM();

  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  static unsigned long lastMsgMQTT = 0;
  unsigned long now = millis();
  if (now - lastMsgMQTT > interval) {
    lastMsgMQTT = now;
    float distancia = readDistanceCM();
    Serial.print("Distância: ");
    Serial.println(distancia);
    String mensagem = String(distancia, 2);
    client.publish(topicPublish, mensagem.c_str());
  }

  if (flashPressCount > 0 && millis() - lastFlashPress > 3000) {
    flashPressCount = 0;
  }

  delay(100);
}

#endif // FULL_VERSION_H