#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

// --- Definições de pinos ---
#define TRIGGER 5
#define READ 18
#define SWITCH LED_BUILTIN
#define FLASH_BUTTON 0 // GPIO0 normalmente é o botão FLASH no ESP8266

// --- Protótipos de funções auxiliares ---
void startAPMode();
void stopAPMode();
void handleFlashButton();
void blinkLED();
void setupConfigServerRoutes();
void publishIPAddress(); // Nova função para publicar IP

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
const char* topicBase = "meuESP8266/";
const char* topicSubscribe = "meuESP8266/entrada";
const char* topicPublish = "meuESP8266/saida";
const char* topicIP = "meuESP8266/IPnaRede"; // Tópico específico para IP

// --- Variáveis Globais ---
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
const long interval = 1000; // Envio a cada 1 segundos

// --- Função para publicar o IP no MQTT ---
void publishIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.print("Publicando IP: ");
    Serial.println(ip);
    
    // Tenta publicar o IP
    if (client.connected()) {
      client.publish(topicIP, ip.c_str());
    } else {
      // Se não estiver conectado ao MQTT, tenta conectar
      String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
      if (client.connect(clientId.c_str())) {
        client.publish(topicIP, ip.c_str());
        client.subscribe(topicSubscribe);
      }
    }
  }
}

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
    Serial.print("Conectado ao WiFi. IP: ");
    Serial.println(WiFi.localIP().toString());
    
    // Configura o servidor MQTT e publica o IP
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);
    publishIPAddress();
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
  
  // Publica o IP após conectar
  publishIPAddress();
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
      publishIPAddress();
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

  setupConfigServerRoutes(); // Sempre define as rotas

  if (!connectWiFiFromEEPROM()) {
    startAPMode();
  } else {
    configServer.begin(); // Inicia o servidor web também no modo STA
  }

  // MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // Semente para ID aleatório
  randomSeed(micros());
  
  // Publica o IP se estiver conectado
  if (WiFi.status() == WL_CONNECTED) {
    publishIPAddress();
  }
}

// --- LOOP ---
void loop() {
  handleFlashButton();
  configServer.handleClient(); // Sempre processa requisições HTTP
  
  // Se estiver em modo AP, mantenha o LED sempre aceso
  if (apMode) {
    digitalWrite(SWITCH, HIGH); // LED sempre aceso no AP
    return; // Sai da função para não executar o resto do código
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Mantém o LED desligado quando conectado (só pisca ao publicar/receber)
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
    
    // Pisca LED ao publicar no MQTT
    blinkLED();
    
    // Não controla o LED baseado na distância para manter o comportamento desejado
    // Removido o controle baseado na distância para que o LED só pisque ao publicar
  }

  // Exemplo de uso do blinkLED() em publish/receive:
  // blinkLED(); // Chame após publicar ou receber MQTT
}

// --- Funções para Wi-Fi config portal ---
void setupConfigServerRoutes() {
  // CSS compartilhado para todas as páginas
  String style = "<style>"
    "body{background:#0a2540;font-family:sans-serif;margin:0;padding:0;}"
    ".centerbox{background:#fff3;border-radius:12px;box-shadow:0 2px 8px #0002;width:340px;margin:60px auto;padding:32px 24px;}"
    "h2{color:#fff;text-align:center;margin-bottom:24px;}"
    "form{display:flex;flex-direction:column;gap:16px;}"
    "label{color:#fff;font-size:1.1em;}"
    "select,input[type=password]{padding:8px 10px;border-radius:6px;border:1px solid #ccc;font-size:1em;}"
    "button{background:#1976d2;color:#fff;border:none;padding:10px 0;border-radius:6px;font-size:1.1em;cursor:pointer;transition:.2s;}"
    "button:hover{background:#125ea2;}"
    ".debug-list{background:#fff3;border-radius:8px;padding:18px 18px 18px 32px;margin-top:18px;}"
    ".debug-list li{color:#222;font-size:1.05em;margin-bottom:8px;}"
    "</style>";

  configServer.on("/", HTTP_GET, [style]() {
    String html = "<!DOCTYPE html><html lang='pt-br'><head><meta charset='UTF-8'><title>Configuração Wi-Fi</title>" + style + "</head><body>"
      "<div class='centerbox'>"
      "<h2>Configuração Wi-Fi</h2>"
      "<form method='POST'>"
      "<label for='ssid'>Rede Wi-Fi:</label>"
      "<select name='ssid' id='ssid'>";
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + "</option>";
    }
    html += "</select>"
      "<label for='pass'>Senha:</label>"
      "<input name='pass' id='pass' placeholder='Senha' type='password' autocomplete='off'>"
      "<button type='submit'>Salvar</button>"
      "</form>"
      "</div></body></html>";
    configServer.send(200, "text/html; charset=utf-8", html);
  });
  configServer.on("/", HTTP_POST, [style]() {
    String ssid = configServer.arg("ssid");
    String pass = configServer.arg("pass");
    saveWiFiToEEPROM(ssid.c_str(), pass.c_str());
    String html = "<!DOCTYPE html><html lang='pt-br'><head><meta charset='UTF-8'><title>Configuração</title>" + style + "</head><body>"
      "<div class='centerbox'><h2>Configuração recebida! O ESP irá reiniciar para conectar à rede.</h2></div></body></html>";
    configServer.send(200, "text/html; charset=utf-8", html);
    delay(2000);
    ESP.restart();
  });
  configServer.on("/debug", HTTP_GET, [style]() {
    String html = "<!DOCTYPE html><html lang='pt-br'><head><meta charset='UTF-8'><title>Depuração ESP8266</title>" + style + "</head><body>"
      "<div class='centerbox'>"
      "<h2>Depuração ESP8266</h2>"
      "<ul class='debug-list'>";
    html += "<li>Status Wi-Fi: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado") + "</li>";
    html += "<li>IP Local: " + WiFi.localIP().toString() + "</li>";
    html += "<li>SSID: " + WiFi.SSID() + "</li>";
    html += "<li>RSSI: " + String(WiFi.RSSI()) + " dBm</li>";
    html += "<li>MQTT conectado: " + String(client.connected() ? "Sim" : "Não") + "</li>";
    html += "<li>Memória livre: " + String(ESP.getFreeHeap()) + " bytes</li>";
    html += "<li>Tempo desde boot: " + String(millis() / 1000) + " s</li>";
    html += "</ul></div></body></html>";
    configServer.send(200, "text/html; charset=utf-8", html);
  });
}

void startAPMode() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP8266_Config", "12345678");
  setupConfigServerRoutes();
  configServer.begin();
  digitalWrite(SWITCH, HIGH); // LED sempre aceso no AP
  Serial.println("Modo AP ativado - LED permanecerá aceso");
}
void stopAPMode() {
  apMode = false;
  configServer.stop();
  WiFi.softAPdisconnect(true);
  digitalWrite(SWITCH, LOW); // Desliga o LED ao sair do modo AP
  Serial.println("Saindo do modo AP - LED será controlado pelo modo normal");
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
  // Não pisca o LED se estiver em modo AP (já está sempre aceso)
  if (!apMode) {
    digitalWrite(SWITCH, HIGH);
    delay(80);
    digitalWrite(SWITCH, LOW);
    delay(80);
  }
}