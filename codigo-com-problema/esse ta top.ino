#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

// --- Definições de pinos ---
#define TRIGGER 5
#define READ 18
#define SWITCH 16
#define FLASH_BUTTON 0 // GPIO0 normalmente é o botão FLASH no ESP8266

// --- Protótipos de funções auxiliares ---
void startAPMode();
void stopAPMode();
void handleFlashButton();
void blinkLED();

// --- Variáveis para controle de modo AP e LED ---
ESP8266WebServer configServer(80);
bool apMode = false;
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
unsigned long lastBlink = 0;
const unsigned long blinkDuration = 80; // ms

// --- Configurações do Broker MQTT ---
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;

// --- Tópicos MQTT ---
const char* topicBase = "meuESP32/testeBasico/";
const char* topicSubscribe = "meuESP32/testeBasico/entrada";
const char* topicPublish = "meuESP8266/testeBasico/saida";

// --- Variáveis Globais ---
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
const long interval = 1000; // Envio a cada 1 segundos

// --- Funções EEPROM para Wi-Fi ---
void saveWiFiToEEPROM(const char* ssid, const char* pass) {
  EEPROM.begin(96);
  for (int i = 0; i < 32; ++i) EEPROM.write(i, ssid[i]);
  for (int i = 0; i < 64; ++i) EEPROM.write(32 + i, pass[i]);
  EEPROM.commit();
}
void loadWiFiFromEEPROM(char* ssid, char* pass) {
  EEPROM.begin(96);
  for (int i = 0; i < 32; ++i) ssid[i] = EEPROM.read(i);
  for (int i = 0; i < 64; ++i) pass[i] = EEPROM.read(32 + i);
  ssid[31] = '\0'; pass[63] = '\0';
}
void clearWiFiEEPROM() {
  EEPROM.begin(96);
  for (int i = 0; i < 96; ++i) EEPROM.write(i, 0);
  EEPROM.commit();
}

// --- Conecta ao Wi-Fi usando EEPROM ---
bool connectWiFiFromEEPROM() {
  char ssid[32], pass[64];
  loadWiFiFromEEPROM(ssid, pass);
  if (ssid[0] == 0 || pass[0] == 0) return false;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 10000) {
    delay(200);
  }
  if (WiFi.status() == WL_CONNECTED) {
    // Publica o IP atual no tópico meuESP8266/IPnaRede
    if (client.connected()) {
      String ip = WiFi.localIP().toString();
      client.publish("meuESP8266/IPnaRede", ip.c_str());
    }
  }
  return WiFi.status() == WL_CONNECTED;
}

// --- Conecta ao Wi-Fi ---
void conectawifi() {
  WiFi.begin("Ferruagem_2G", "natanael439"); // Substitua pelo seu SSID e senha
  Serial.print("Conectando ao WiFi ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// --- Callback para mensagens recebidas ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Pisca LED ao receber mensagem
  blinkLED();

  // Exemplo de comando recebido
  if (strcmp(topic, topicSubscribe) == 0) {
    if (message == "LIGAR") {
      digitalWrite(SWITCH, HIGH);
    } else if (message == "DESLIGAR") {
      digitalWrite(SWITCH, LOW);
    }
  }
}

// --- Reconectar MQTT ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectado!");
      client.publish(topicPublish, "ESP32 Online");
      client.subscribe(topicSubscribe);
      // Publica o IP atual no tópico meuESP8266/IPnaRede
      String ip = WiFi.localIP().toString();
      client.publish("meuESP8266/IPnaRede", ip.c_str());
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

// --- SETUP ---
void setup() {
  Serial.begin(115200);
  pinMode(TRIGGER, OUTPUT);
  pinMode(READ, INPUT);
  pinMode(FLASH_BUTTON, INPUT_PULLUP);
  pinMode(SWITCH, OUTPUT);
  digitalWrite(SWITCH, LOW);

  if (!connectWiFiFromEEPROM()) {
    startAPMode();
  }

  // MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // Semente para ID aleatório
  randomSeed(micros());
  // Se já estiver conectado ao Wi-Fi e MQTT, publica o IP
  if (WiFi.status() == WL_CONNECTED && client.connected()) {
    String ip = WiFi.localIP().toString();
    client.publish("meuESP8266/IPnaRede", ip.c_str());
  }
}

// --- LOOP ---
void loop() {
  handleFlashButton();
  if (apMode) {
    configServer.handleClient();
    digitalWrite(SWITCH, HIGH); // LED sempre aceso no AP
    return;
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // LED OFF quando conectado (fica só piscando em publish/receive)
  digitalWrite(SWITCH, LOW);

  // A cada intervalo, envia leitura
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;
    float distancia = readDistanceCM();
    Serial.print("Distância: ");
    Serial.println(distancia);
    // Publica distância
    String mensagem = String(distancia, 2); // 2 casas decimais
    client.publish(topicPublish, mensagem.c_str());
    blinkLED(); // Pisca LED ao publicar MQTT
    // Controle do SWITCH baseado na distância
    if (distancia > 300) {
      digitalWrite(SWITCH, HIGH);
    } else {
      digitalWrite(SWITCH, LOW);
    }
  }

  // Exemplo de uso do blinkLED() em publish/receive:
  // blinkLED(); // Chame após publicar ou receber MQTT
}

// --- Funções para Wi-Fi config portal ---
void startAPMode() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP8266_Config", "12345678");
  configServer.on("/", HTTP_GET, []() {
    String html = "<h2>Configuração Wi-Fi</h2><form method='POST'>";
    // Busca redes Wi-Fi e monta o select
    int n = WiFi.scanNetworks();
    html += "<label>Rede Wi-Fi:</label> <select name='ssid'>";
    for (int i = 0; i < n; i++) {
      html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
    html += "</select><br>";
    html += "<input name='pass' placeholder='Senha' type='password'><br><button type='submit'>Salvar</button></form>";
    configServer.send(200, "text/html", html);
  });
  configServer.on("/", HTTP_POST, []() {
    String ssid = configServer.arg("ssid");
    String pass = configServer.arg("pass");
    saveWiFiToEEPROM(ssid.c_str(), pass.c_str());
    configServer.send(200, "text/html", "<h2>Configuração recebida! O ESP irá reiniciar para conectar à rede.</h2>");
    delay(2000);
    ESP.restart();
  });
  configServer.on("/debug", HTTP_GET, []() {
    String html = "<h2>Depuração ESP8266</h2>";
    html += "<ul>";
    html += "<li>Status Wi-Fi: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado") + "</li>";
    html += "<li>IP Local: " + WiFi.localIP().toString() + "</li>";
    html += "<li>SSID: " + WiFi.SSID() + "</li>";
    html += "<li>RSSI: " + String(WiFi.RSSI()) + " dBm</li>";
    html += "<li>MQTT conectado: " + String(client.connected() ? "Sim" : "Não") + "</li>";
    html += "<li>Memória livre: " + String(ESP.getFreeHeap()) + " bytes</li>";
    html += "<li>Tempo desde boot: " + String(millis() / 1000) + " s</li>";
    html += "</ul>";
    configServer.send(200, "text/html", html);
  });
  configServer.begin();
  digitalWrite(SWITCH, HIGH); // LED sempre aceso no AP
}
void stopAPMode() {
  apMode = false;
  configServer.stop();
  WiFi.softAPdisconnect(true);
}

void handleFlashButton() {
  if (digitalRead(FLASH_BUTTON) == LOW) { // Pressionado (ativo em LOW)
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressTime = millis();
    } else if (millis() - buttonPressTime > 3000) { // 3 segundos
      clearWiFiEEPROM();
      startAPMode();
      buttonPressed = false;
    }
  } else {
    buttonPressed = false;
  }
}

void blinkLED() {
  digitalWrite(SWITCH, HIGH);
  delay(80);
  digitalWrite(SWITCH, LOW);
  delay(80);
}
