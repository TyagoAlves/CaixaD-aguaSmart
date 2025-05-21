# ESP8266 IoT com MQTT e Configuração Web

![ESP8266 IoT](https://img.shields.io/badge/ESP8266-IoT-blue)
![MQTT](https://img.shields.io/badge/MQTT-Enabled-green)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Ready-orange)

Sistema IoT baseado em ESP8266 com comunicação MQTT, servidor web para configuração e sensor ultrassônico.

## 📋 Índice

- [Funcionalidades](#-funcionalidades)
- [Hardware](#-hardware)
- [Software](#-software)
- [Instalação](#-instalação)
- [Configuração](#-configuração)
- [Uso com MQTT Panel](#-uso-com-mqtt-panel)
- [Personalização](#-personalização)

## 🚀 Funcionalidades

### Configuração WiFi via Portal Web
- Modo AP automático quando não há configuração salva
- Interface web responsiva para configurar credenciais WiFi
- Armazenamento na EEPROM para persistência
- Reset via botão FLASH (pressione por 3 segundos)

### Comunicação MQTT
- Conexão com broker público (test.mosquitto.org)
- Publicação de leituras do sensor no tópico `meuESP8266/saida`
- Publicação automática do IP no tópico `meuESP8266/IPnaRede`
- Recebimento de comandos `LIGAR`/`DESLIGAR` para controle
- Reconexão automática em caso de falha

### Sensor Ultrassônico e Depuração
- Medição de distâncias com sensor HC-SR04
- Controle automático baseado nas leituras
- Página de depuração em `/debug` com informações em tempo real

### Indicação Visual com LED
- **LED_BUILTIN** usado como indicador de status
- **Modo AP**: LED permanece aceso continuamente
- **Modo Normal**: LED normalmente apagado
- **Comunicação MQTT**: LED pisca brevemente ao enviar/receber dados
- **Lógica Invertida**: No ESP8266, o LED_BUILTIN acende com LOW e apaga com HIGH

### Controle de Relé Baseado em Distância
- **Acionamento Automático**: O relé é acionado quando a distância medida é menor que o limite mínimo
- **Desligamento Automático**: O relé é desligado quando a distância medida é maior que o limite máximo
- **Limites Configuráveis**: Os limites mínimo e máximo podem ser alterados via MQTT
- **Monitoramento**: O estado atual do relé é publicado no tópico `meuESP8266/relay`

## 🔌 Hardware

### Componentes Necessários
- NodeMCU ESP8266 ou compatível
- Sensor ultrassônico HC-SR04
- Relé para controle baseado em distância
- Cabos de conexão

### Mapeamento de Pinos

| Componente | Pino ESP8266 | Pino NodeMCU | Função |
|------------|--------------|--------------|--------|
| Trigger (Sensor) | GPIO5 | D1 | Saída para trigger do sensor |
| Echo (Sensor) | GPIO18/GPIO4 | D2 | Entrada para echo do sensor |
| LED Integrado | GPIO2 | LED_BUILTIN | Indicador de status do sistema |
| Relé | GPIO5 | D1 | Controle baseado na distância |
| Botão FLASH | GPIO0 | FLASH | Reset de configurações |

> **Nota sobre o LED_BUILTIN**: O LED integrado no ESP8266 tem lógica invertida - ele acende quando o pino está em LOW (0) e apaga quando está em HIGH (1). O código foi adaptado para considerar esta característica.

## 💻 Software

### Dependências
- **PlatformIO** (recomendado) ou Arduino IDE
- **Bibliotecas**:
  - ESP8266WiFi
  - PubSubClient
  - EEPROM
  - ESP8266WebServer

### Arquivo platformio.ini
```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps = 
    PubSubClient
monitor_speed = 115200
```

## 🔧 Instalação

### Instalação do PlatformIO
1. **Instale o Visual Studio Code**:
   - Download: [VS Code](https://code.visualstudio.com/)
   - Ou via Winget: `winget install --id Microsoft.VisualStudioCode -e`

2. **Instale a Extensão PlatformIO**:
   - Abra o VS Code
   - Vá para Extensions (Ctrl+Shift+X)
   - Busque por "PlatformIO"
   - Instale e reinicie o VS Code

### Compilação e Upload
1. **Abra o Projeto**: File > Open Folder > selecione a pasta do projeto
2. **Compilar**: Clique no ícone ✓ (Check) na barra inferior
3. **Upload**: Clique no ícone → (Right Arrow) na barra inferior
4. **Monitor Serial**: Clique no ícone 🔌 (Plug) para monitoramento

## ⚙️ Configuração

### Primeira Configuração
1. O ESP8266 cria uma rede WiFi "ESP8266_Config" (senha: 12345678)
2. Conecte-se a esta rede e acesse 192.168.4.1 no navegador
3. Selecione sua rede WiFi e insira a senha
4. O dispositivo reiniciará e se conectará à sua rede

### Monitoramento
- Acesse http://[IP-DO-ESP]/debug para ver o status do sistema
- Para resetar as configurações, mantenha o botão FLASH pressionado por 3 segundos
- Observe o LED integrado para entender o status:
  - **Sempre aceso**: Modo AP ativo (aguardando configuração)
  - **Apagado**: Conectado a uma rede WiFi
  - **Piscando**: Enviando ou recebendo dados via MQTT

## 📱 Uso com MQTT Panel

### Configuração do Broker
1. Instale "IoT MQTT Panel" da Google Play Store
2. Adicione um novo Broker:
   - Nome: ESP8266 Monitor
   - Endereço: `test.mosquitto.org`
   - Porta: `1883`
   - Cliente ID: (opcional)
   - Sem autenticação

### Painéis de Controle
1. **Leitura do Sensor**:
   - Tipo: Text
   - Nome: Distância
   - Tópico: `meuESP8266/saida`
   - QoS: 0

2. **Controle do LED**:
   - Tipo: Switch
   - Nome: Controle LED
   - Tópico: `meuESP8266/entrada`
   - Valor ON: `LIGAR`
   - Valor OFF: `DESLIGAR`
   - QoS: 0

3. **Controle do Relé**:
   - Tipo: Switch
   - Nome: Controle Relé
   - Tópico: `meuESP8266/entrada`
   - Valor ON: `LIGAR_RELE`
   - Valor OFF: `DESLIGAR_RELE`
   - QoS: 0

4. **Estado do Relé**:
   - Tipo: Text
   - Nome: Estado do Relé
   - Tópico: `meuESP8266/relay`
   - QoS: 0

5. **Monitoramento do IP**:
   - Tipo: Text
   - Nome: IP do ESP8266
   - Tópico: `meuESP8266/IPnaRede`
   - QoS: 0

### Configuração de Limites de Distância
- Para alterar o limite mínimo, envie `MIN:valor` para o tópico `meuESP8266/entrada`
- Para alterar o limite máximo, envie `MAX:valor` para o tópico `meuESP8266/entrada`
- Exemplo: `MIN:5.0` define o limite mínimo para 5 cm

### Uso
1. Conecte ao broker (botão no canto superior direito)
2. Os painéis mostrarão os dados recebidos do ESP8266
3. Use os switches para enviar comandos LIGAR/DESLIGAR
4. Envie comandos MIN: e MAX: para configurar os limites de distância

## 🛠️ Personalização

Para adaptar o projeto às suas necessidades:

- **Broker MQTT**: Altere `mqttServer` e `mqttPort` para seu próprio broker
- **Tópicos MQTT**: Modifique `topicBase`, `topicSubscribe` e `topicPublish`
- **Intervalo de Leitura**: Ajuste `interval` para mudar a frequência de leitura
- **Limites de Distância**: Modifique `minDistance` e `maxDistance` para ajustar os limites de acionamento do relé
- **Pinos**: Modifique as definições de pinos conforme seu hardware