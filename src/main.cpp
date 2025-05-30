#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>

/**
 * ESP8266 IoT com MQTT e Configuração Web
 * 
 * Este projeto implementa um sistema IoT baseado em ESP8266 com as seguintes funcionalidades:
 * - Interface web de configuração acessível em http://192.168.0.1
 * - Página de diagnóstico em http://192.168.0.1/debug
 * - Modo AP automático com SSID "ESP8266_Config" e senha "12345678"
 * - Comunicação MQTT com broker público (broker.emqx.io)
 * - Sensor ultrassônico para medição de distância
 * - Controle de relé baseado na distância ou comandos MQTT
 * - Modos de operação automático e manual para o relé
 * - Publicação periódica do IP e estado do relé
 * 
 * Configuração Inicial:
 * 1. Na primeira execução, conecte-se à rede "ESP8266_Config" (senha: 12345678)
 * 2. Acesse http://192.168.0.1 no navegador
 * 3. Selecione sua rede WiFi e insira a senha
 * 4. O dispositivo reiniciará e se conectará à sua rede
 * 
 * Para resetar as configurações:
 * - Pressione o botão FLASH por 3 segundos
 * - O dispositivo entrará em modo AP novamente
 * 
 * Autor: Tyago Alves
 * Data: Maio/2025
 */

// --- Definições de pinos ---
#define TRIGGER D2  // GPIO4 - Pino para o trigger do sensor ultrassônico
#define READ D5     // GPIO14 - Pino para o echo do sensor ultrassônico
#define SWITCH LED_BUILTIN // LED integrado para indicação visual
#define RELAY_PIN D1  // GPIO5 - Pino para o relé
#define FLASH_BUTTON 0 // GPIO0 normalmente é o botão FLASH no ESP8266

// --- Protótipos de funções auxiliares ---
void startAPMode();        // Inicia o modo Access Point para configuração
void stopAPMode();         // Para o modo Access Point
void handleFlashButton();  // Trata o pressionamento do botão FLASH
void blinkLED();           // Pisca o LED para indicação visual
void setupConfigServerRoutes(); // Configura as rotas do servidor web
void publishIPAddress();   // Publica o endereço IP via MQTT
void publishRelayState();  // Publica o estado do relé via MQTT
void checkWiFiConnection(); // Verifica e reconecta o WiFi se necessário
void callback(char* topic, byte* payload, unsigned int length); // Callback para mensagens MQTT recebidas

// --- Variáveis para controle de modo AP e LED ---
ESP8266WebServer configServer(80);
bool apMode = false;
unsigned long buttonPressTime = 0;
bool buttonPressed = false;
unsigned long lastBlink = 0;
const unsigned long blinkDuration = 80; // ms

// --- Configurações do Broker MQTT ---
const char* mqttServer = "broker.emqx.io";
const int mqttPort = 1883;

// --- Tópicos MQTT ---
const char* topicBase = "meuESP8266/";
const char* topicSubscribe = "meuESP8266/entrada";  // Corrigido de meuESP8866
const char* topicPublish = "meuESP8266/saida";      // Corrigido de meuESP8866
const char* topicIP = "meuESP8266/IPnaRede";        // Corrigido de meuESP8866
const char* topicRelay = "meuESP8266/relay";        // Corrigido de meuESP8866

// --- Variáveis Globais ---
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
unsigned long lastIPPublish = 0;
unsigned long lastWifiCheck = 0;
unsigned long lastRelayPublish = 0;
const long interval = 1000; // Envio a cada 1 segundo
const long ipPublishInterval = 60000; // Envio do IP a cada 60 segundos (1 minuto)
const long wifiCheckInterval = 30000; // Verificar WiFi a cada 30 segundos
const long relayPublishInterval = 5000; // Publicar estado do relé a cada 5 segundos
const int mqttKeepAlive = 120; // Keep-alive de 120 segundos (2 minutos)

// --- Variáveis para controle do relé baseado na distância ---
float minDistance = 10.0;  // Distância mínima em cm (liga o relé)
float maxDistance = 50.0;  // Distância máxima em cm (desliga o relé)
bool relayState = false;   // Estado atual do relé
bool relayControlledByApp = false; // Indica se o relé está sendo controlado pelo app (modo manual)

/**
 * Função para publicar o IP no MQTT
 * Publica o endereço IP local no tópico definido em topicIP
 * com a flag retained=true para que o broker mantenha a mensagem
 */
void publishIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    String ip = WiFi.localIP().toString();
    Serial.print("Publicando IP: ");
    Serial.println(ip);
    
    // Tenta publicar o IP
    if (client.connected()) {
      bool published = client.publish(topicIP, ip.c_str(), true); // Adiciona retained=true
      Serial.print("Publicação do IP: ");
      Serial.println(published ? "Sucesso" : "Falha");
    } else {
      // Se não estiver conectado ao MQTT, tenta conectar
      Serial.println("Cliente MQTT desconectado. Tentando reconectar...");
      String clientId = "ESP8266Client-" + String(random(0xffff), HEX);  // Corrigido de ESP8866
      if (client.connect(clientId.c_str())) {
        Serial.println("Reconectado ao MQTT");
        bool published = client.publish(topicIP, ip.c_str(), true); // Adiciona retained=true
        Serial.print("Publicação do IP após reconexão: ");
        Serial.println(published ? "Sucesso" : "Falha");
        client.subscribe(topicSubscribe);
      } else {
        Serial.print("Falha na reconexão, rc=");
        Serial.println(client.state());
      }
    }
  } else {
    Serial.println("WiFi não conectado. Não é possível publicar o IP.");
  }
}

/**
 * Funções EEPROM para armazenamento de credenciais WiFi
 * Estas funções permitem salvar, carregar e limpar as credenciais
 * WiFi armazenadas na memória EEPROM do ESP8266
 */
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

/**
 * Conecta ao Wi-Fi usando credenciais armazenadas na EEPROM
 * Retorna true se a conexão foi bem-sucedida, false caso contrário
 */
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

/**
 * Conecta ao Wi-Fi usando credenciais fixas
 * Função de fallback caso não haja credenciais na EEPROM
 */
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

/**
 * Callback para mensagens MQTT recebidas
 * Processa comandos recebidos no tópico de entrada
 */
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

  // Processa comandos recebidos
  if (strcmp(topic, topicSubscribe) == 0) {
    if (message == "LIGAR_RELE") {
      digitalWrite(RELAY_PIN, HIGH);
      relayState = true;
      relayControlledByApp = true; // Marca que o relé está sendo controlado pelo app (modo manual)
      bool published = client.publish(topicRelay, "ON", true); // Adiciona retained=true
      Serial.print("Relé LIGADO - Comando MQTT. Publicação: ");
      Serial.println(published ? "Sucesso" : "Falha");
    } else if (message == "DESLIGAR_RELE") {
      digitalWrite(RELAY_PIN, LOW);
      relayState = false;
      relayControlledByApp = true; // Marca que o relé está sendo controlado pelo app (modo manual)
      bool published = client.publish(topicRelay, "OFF", true); // Adiciona retained=true
      Serial.print("Relé DESLIGADO - Comando MQTT. Publicação: ");
      Serial.println(published ? "Sucesso" : "Falha");
    } else if (message == "AUTO_RELE") {
      relayControlledByApp = false; // Volta para o modo automático
      Serial.println("Relé voltou para o modo automático");
      bool published = client.publish(topicRelay, "AUTO", true);
      Serial.print("Modo automático publicado: ");
      Serial.println(published ? "Sucesso" : "Falha");
    } else if (message.startsWith("MIN:")) {
      minDistance = message.substring(4).toFloat();
      client.publish(topicRelay, ("MIN:" + String(minDistance)).c_str());
    } else if (message.startsWith("MAX:")) {
      maxDistance = message.substring(4).toFloat();
      client.publish(topicRelay, ("MAX:" + String(maxDistance)).c_str());
    }
  }
}

/**
 * Função para publicar o estado do relé
 * Tenta publicar até 3 vezes em caso de falha
 * Verifica se o estado físico do relé corresponde ao estado lógico
 */
void publishRelayState() {
  if (!client.connected()) return;
  
  // Verifica se o estado físico do relé corresponde ao estado lógico
  bool physicalRelayState = digitalRead(RELAY_PIN);
  if (physicalRelayState != relayState) {
    Serial.println("Corrigindo inconsistência no estado do relé");
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
  }
  
  const char* state = relayState ? "ON" : "OFF";
  bool success = false;
  
  // Tenta publicar até 3 vezes
  for (int i = 0; i < 3 && !success; i++) {
    success = client.publish(topicRelay, state, true);
    if (!success) {
      Serial.print("Falha ao publicar estado do relé, tentativa ");
      Serial.println(i+1);
      delay(100);
    }
  }
  
  Serial.print("Estado do relé publicado: ");
  Serial.print(state);
  Serial.print(" - ");
  Serial.println(success ? "Sucesso" : "Falha após 3 tentativas");
}

/**
 * Verifica e reconecta o WiFi se necessário
 */
void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    WiFi.reconnect();
    
    // Aguarda reconexão por até 10 segundos
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi reconectado!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nFalha ao reconectar WiFi");
    }
  }
}

/**
 * Reconecta ao broker MQTT
 * Limita a 3 tentativas por vez para evitar bloqueios
 */
void reconnect() {
  int attempts = 0;
  while (!client.connected() && attempts < 3) { // Limita a 3 tentativas por vez
    attempts++;
    Serial.print("Tentando conectar ao MQTT (tentativa ");
    Serial.print(attempts);
    Serial.print(")...");
    
    // Usar ID de cliente baseado no chip ID para evitar duplicação
    String clientId = "ESP8266-" + String(ESP.getChipId(), HEX);  // Corrigido de ESP8866
    
    // Conectar com keep-alive e last will testament
    if (client.connect(clientId.c_str(), NULL, NULL, topicPublish, 0, true, "ESP8266 Online", true)) {  // Corrigido de ESP8866
      Serial.println("Conectado!");
      
      // Publica mensagem de status
      bool statusPublished = client.publish(topicPublish, "ESP8266 Online", true);  // Corrigido de ESP8866
      Serial.print("Status publicado: ");
      Serial.println(statusPublished ? "Sim" : "Não");
      
      // Inscreve-se no tópico de entrada
      bool subscribed = client.subscribe(topicSubscribe);
      Serial.print("Inscrito no tópico: ");
      Serial.println(subscribed ? "Sim" : "Não");
      
      // Publica o estado atual do relé
      publishRelayState();
      
      // Publica o IP atual
      publishIPAddress();
      
      return; // Sai da função se conectou com sucesso
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 2 segundos");
      delay(2000);
    }
  }
}

/**
 * Leitura do sensor ultrassônico
 * Retorna a distância em centímetros
 */
float readDistanceCM() {
  digitalWrite(TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  int duration = pulseIn(READ, HIGH);
  return duration * 0.034 / 2;
}

/**
 * Pisca o LED para indicação visual
 */
void blinkLED() {
  // Não pisca o LED se estiver em modo AP (já está sempre aceso)
  if (!apMode) {
    digitalWrite(SWITCH, LOW);  // Liga o LED (lógica invertida)
    delay(80);
    digitalWrite(SWITCH, HIGH); // Desliga o LED (lógica invertida)
    delay(80);
  }
}

/**
 * Configura as rotas do servidor web
 * Inclui páginas para configuração WiFi e depuração
 */
void setupConfigServerRoutes() {
  // CSS compartilhado para todas as páginas
  String style = "<style>"
    "body{background:#0a2540;font-family:sans-serif;margin:0;padding:0;}"
    ".centerbox{background:#fff3;border-radius:12px;box-shadow:0 2px 8px #0002;width:340px;margin:60px auto;padding:32px 24px;}"
    "h2{color:#fff;text-align:center;margin-bottom:24px;}"
    "h3{color:#fff;margin-top:20px;margin-bottom:10px;border-top:1px solid #fff3;padding-top:15px;}"
    "form{display:flex;flex-direction:column;gap:16px;}"
    "label{color:#fff;font-size:1.1em;}"
    "select,input[type=password]{padding:8px 10px;border-radius:6px;border:1px solid #ccc;font-size:1em;}"
    "button{background:#1976d2;color:#fff;border:none;padding:10px 0;border-radius:6px;font-size:1.1em;cursor:pointer;transition:.2s;}"
    "button:hover{background:#125ea2;}"
    ".debug-list{background:#fff3;border-radius:8px;padding:18px 18px 18px 32px;margin-top:18px;}"
    ".debug-list li{color:#222;font-size:1.05em;margin-bottom:8px;}"
    "strong{color:#1976d2;}"
    ".topic{font-family:monospace;background:#fff1;padding:2px 6px;border-radius:3px;}"
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
      "<ul class='debug-list'>"
      "<h3>Status do Sistema</h3>"
      "<li><strong>Status Wi-Fi:</strong> " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado") + "</li>"
      "<li><strong>IP Local:</strong> " + WiFi.localIP().toString() + "</li>"
      "<li><strong>SSID:</strong> " + WiFi.SSID() + "</li>"
      "<li><strong>RSSI:</strong> " + String(WiFi.RSSI()) + " dBm</li>"
      "<li><strong>Broker MQTT:</strong> " + String(mqttServer) + ":" + String(mqttPort) + "</li>"
      "<li><strong>MQTT conectado:</strong> " + String(client.connected() ? "Sim" : "Não") + "</li>"
      "<li><strong>Estado do Relé:</strong> " + String(relayState ? "LIGADO" : "DESLIGADO") + "</li>"
      "<li><strong>Controle do Relé:</strong> " + String(relayControlledByApp ? "APP (Manual)" : "Automático") + "</li>"
      "<li><strong>Distância Min:</strong> " + String(minDistance) + " cm</li>"
      "<li><strong>Distância Max:</strong> " + String(maxDistance) + " cm</li>"
      "<li><strong>Memória livre:</strong> " + String(ESP.getFreeHeap()) + " bytes</li>"
      "<li><strong>Tempo desde boot:</strong> " + String(millis() / 1000) + " s</li>"
      "<h3>Tópicos MQTT</h3>"
      "<li><strong>Entrada de comandos:</strong> <span class='topic'>" + String(topicSubscribe) + "</span></li>"
      "<li><strong>Saída de distância:</strong> <span class='topic'>" + String(topicPublish) + "</span></li>"
      "<li><strong>Estado do relé:</strong> <span class='topic'>" + String(topicRelay) + "</span></li>"
      "<li><strong>IP na rede:</strong> <span class='topic'>" + String(topicIP) + "</span></li>"
      "<h3>Comandos Disponíveis</h3>"
      "<li><strong>LIGAR_RELE</strong> - Liga o relé manualmente</li>"
      "<li><strong>DESLIGAR_RELE</strong> - Desliga o relé manualmente</li>"
      "<li><strong>AUTO_RELE</strong> - Volta ao modo automático</li>"
      "<li><strong>MIN:valor</strong> - Define distância mínima (ex: MIN:10.0)</li>"
      "<li><strong>MAX:valor</strong> - Define distância máxima (ex: MAX:50.0)</li>"
      "</ul></div></body></html>";
    configServer.send(200, "text/html; charset=utf-8", html);
  });
}

/**
 * Inicia o modo Access Point para configuração
 */
void startAPMode() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  
  // Configura o IP fixo para o modo AP como 192.168.0.1
  IPAddress apIP(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  
  // Garante que o IP seja configurado corretamente
  bool success = WiFi.softAPConfig(apIP, apIP, subnet);
  
  // Inicia o AP com o nome e senha
  WiFi.softAP("ESP8266_Config", "12345678");
  
  setupConfigServerRoutes();
  configServer.begin();
  digitalWrite(SWITCH, LOW); // LED sempre aceso no AP (lógica invertida)
  
  Serial.println("Modo AP ativado - LED permanecerá aceso");
  Serial.print("IP do AP configurado com sucesso: ");
  Serial.println(success ? "Sim" : "Não");
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());
}

/**
 * Para o modo Access Point
 */
void stopAPMode() {
  apMode = false;
  configServer.stop();
  WiFi.softAPdisconnect(true);
  digitalWrite(SWITCH, HIGH); // Desliga o LED ao sair do modo AP (lógica invertida)
  Serial.println("Saindo do modo AP - LED será controlado pelo modo normal");
}

/**
 * Trata o pressionamento do botão FLASH
 * Reseta as configurações WiFi se pressionado por 3 segundos
 */
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

/**
 * Configuração inicial
 */
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== ESP8266 IoT com MQTT e Sensor Ultrassônico ===");  // Corrigido de ESP8866
  Serial.println("Iniciando...");
  
  pinMode(TRIGGER, OUTPUT);
  pinMode(READ, INPUT);
  pinMode(FLASH_BUTTON, INPUT_PULLUP);
  pinMode(SWITCH, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(SWITCH, HIGH); // LED desligado inicialmente (lógica invertida)
  digitalWrite(RELAY_PIN, LOW); // Relé desligado inicialmente

  setupConfigServerRoutes(); // Sempre define as rotas

  if (!connectWiFiFromEEPROM()) {
    Serial.println("Não foi possível conectar usando credenciais da EEPROM");
    startAPMode();
  } else {
    Serial.println("Conectado com credenciais da EEPROM");
    configServer.begin(); // Inicia o servidor web também no modo STA
  }

  // MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  // Aumenta o tamanho do buffer para mensagens MQTT (padrão é 256 bytes)
  client.setBufferSize(512);
  
  // Configura o keep-alive para 120 segundos (2 minutos)
  client.setKeepAlive(mqttKeepAlive);

  // Semente para ID aleatório
  randomSeed(micros());
  
  // Publica o IP se estiver conectado
  if (WiFi.status() == WL_CONNECTED) {
    publishIPAddress();
    lastIPPublish = millis(); // Inicializa o contador de tempo para publicação do IP
  }
  
  Serial.println("Setup concluído!");
}

/**
 * Loop principal
 */
void loop() {
  handleFlashButton();
  configServer.handleClient(); // Sempre processa requisições HTTP
  
  // Se estiver em modo AP, mantenha o LED sempre aceso
  if (apMode) {
    digitalWrite(SWITCH, LOW); // LED sempre aceso no AP (lógica invertida)
    return; // Sai da função para não executar o resto do código
  }

  unsigned long now = millis();
  
  // Verificar conexão WiFi periodicamente
  if (now - lastWifiCheck > wifiCheckInterval) {
    lastWifiCheck = now;
    checkWiFiConnection();
  }

  // Verifica conexão MQTT e tenta reconectar se necessário
  if (!client.connected()) {
    Serial.println("Cliente MQTT desconectado no loop principal");
    reconnect();
  }
  
  // Processa mensagens MQTT recebidas
  client.loop();

  // Publicação periódica do estado do relé
  if (now - lastRelayPublish > relayPublishInterval) {
    lastRelayPublish = now;
    publishRelayState();
  }

  // Publicação do IP a cada minuto
  if (now - lastIPPublish > ipPublishInterval) {
    lastIPPublish = now;
    Serial.println("Intervalo de publicação do IP atingido");
    publishIPAddress();
  }
  
  // Forçar publicação do IP a cada 10 segundos durante os primeiros 2 minutos após inicialização
  static bool initialPhase = true;
  static unsigned long startTime = millis();
  if (initialPhase && (now - startTime < 120000)) { // 2 minutos
    if (now % 10000 < interval) { // A cada 10 segundos
      Serial.println("Fase inicial: publicação frequente do IP");
      publishIPAddress();
    }
  } else {
    initialPhase = false;
  }
  
  // Mantém o LED desligado quando conectado (só pisca ao publicar/receber)
  digitalWrite(SWITCH, HIGH); // LED desligado (lógica invertida)

  // A cada intervalo, envia leitura
  if (now - lastMsg > interval) {
    lastMsg = now;
    float distancia = readDistanceCM();
    Serial.print("Distância: ");
    Serial.println(distancia);
    
    // Controle do relé baseado na distância SOMENTE se não estiver sendo controlado pelo app
    if (!relayControlledByApp) {
      bool oldRelayState = relayState;
      if (distancia <= minDistance && !relayState) {
        digitalWrite(RELAY_PIN, HIGH);
        relayState = true;
        Serial.println("Relé LIGADO - Distância mínima atingida");
      } else if (distancia >= maxDistance && relayState) {
        digitalWrite(RELAY_PIN, LOW);
        relayState = false;
        Serial.println("Relé DESLIGADO - Distância máxima atingida");
      }
      
      // Publica o estado do relé se houve mudança
      if (oldRelayState != relayState) {
        publishRelayState();
      }
    }
    
    // Publica distância
    String mensagem = String(distancia, 2); // 2 casas decimais
    bool distPublished = client.publish(topicPublish, mensagem.c_str(), true); // Adiciona retained=true
    Serial.print("Publicação da distância: ");
    Serial.print(mensagem);
    Serial.print(" cm - ");
    Serial.println(distPublished ? "Sucesso" : "Falha");
    
    // Pisca LED ao publicar no MQTT
    blinkLED();
  }
}
