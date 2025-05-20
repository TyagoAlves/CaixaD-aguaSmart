#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// --- Definições de pinos ---
#define TRIGGER 5
#define READ 18
#define SWITCH 16

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
const long interval = 1000; // Envio a cada 1 segundos

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
  conectawifi();

  // Pinos
  pinMode(TRIGGER, OUTPUT);
  pinMode(READ, INPUT);
  pinMode(SWITCH, OUTPUT);

  // MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // Semente para ID aleatório
  randomSeed(micros());
}

// --- LOOP ---
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

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

    // Controle do SWITCH baseado na distância
    if (distancia > 300) {
      digitalWrite(SWITCH, HIGH);
    } else {
      digitalWrite(SWITCH, LOW);
    }
  }
}
