# ESP8266 IoT com MQTT e Configuração Web

Este projeto implementa um sistema IoT baseado em ESP8266 que combina comunicação MQTT, servidor web para configuração e leitura de sensor ultrassônico.

## Funcionalidades do muitoBom.h

O arquivo `muitoBom.h` é o componente principal do projeto e oferece as seguintes funcionalidades:

### 1. Configuração WiFi via Portal Web
- **Modo AP Automático**: Cria um ponto de acesso WiFi quando não há configuração salva
- **Interface Web Amigável**: Página web responsiva para configurar credenciais WiFi
- **Armazenamento na EEPROM**: Salva as credenciais WiFi na memória não-volátil
- **Reset via Botão**: Pressione o botão FLASH por 3 segundos para limpar configurações

### 2. Comunicação MQTT
- **Conexão com Broker**: Conecta-se ao broker MQTT público (test.mosquitto.org)
- **Publicação de Dados**: Envia leituras do sensor ultrassônico periodicamente
- **Publicação de IP**: Publica automaticamente o IP no tópico "meuESP8266/IPnaRede" ao conectar
- **Recebimento de Comandos**: Processa comandos "LIGAR" e "DESLIGAR" para controlar saídas
- **Reconexão Automática**: Gerencia reconexões em caso de falha

### 3. Sensor Ultrassônico
- **Leitura de Distância**: Mede distâncias usando sensor ultrassônico HC-SR04
- **Controle Automático**: Aciona saídas com base nas leituras do sensor
- **Publicação MQTT**: Envia os valores medidos para o broker MQTT

### 4. Página de Depuração
- **Informações em Tempo Real**: Acesse `/debug` para ver status do sistema
- **Dados Exibidos**: WiFi, MQTT, memória, tempo de execução e mais
- **Interface Responsiva**: Design moderno e fácil de usar

## Requisitos de Hardware

- **Placa**: NodeMCU ESP8266 ou compatível
- **Sensor**: HC-SR04 (sensor ultrassônico)
- **Componentes**:
  - LED ou relé conectado ao pino GPIO16 (D0)
  - Botão FLASH do ESP8266 (GPIO0)
  - Conexões para o sensor ultrassônico:
    - TRIGGER: GPIO5 (D1)
    - ECHO: GPIO18 (pode precisar ser ajustado para GPIO4/D2 em alguns modelos)

## Requisitos de Software e Dependências

- **PlatformIO** (recomendado) ou Arduino IDE
- **Bibliotecas**:
  - ESP8266WiFi
  - PubSubClient (para MQTT)
  - EEPROM
  - ESP8266WebServer

## Como Usar o PlatformIO

PlatformIO é um ecossistema de desenvolvimento para IoT que facilita o trabalho com ESP8266 e outras placas.

### Instalação do PlatformIO

1. **Instale o Visual Studio Code**: Baixe e instale o [VS Code](https://code.visualstudio.com/)
2. **Instale a Extensão PlatformIO**: Abra o VS Code, vá para Extensions (Ctrl+Shift+X) e busque por "PlatformIO"
3. **Reinicie o VS Code** após a instalação

### Configuração do Projeto

1. **Abra o Projeto**: File > Open Folder e selecione a pasta do projeto
2. **Verifique o platformio.ini**: Este arquivo deve conter:

```ini
[env:nodemcuv2]
platform = espressif8266
board = nodemcuv2
framework = arduino
lib_deps = 
    PubSubClient
monitor_speed = 115200
```

### Compilação e Upload

1. **Compilar**: Clique no ícone ✓ (Check) na barra inferior do VS Code
2. **Upload**: Clique no ícone → (Right Arrow) na barra inferior
3. **Monitor Serial**: Clique no ícone 🔌 (Plug) para abrir o monitor serial

## Instruções de Uso

1. **Primeira Inicialização**: O ESP8266 criará uma rede WiFi chamada "ESP8266_Config" (senha: 12345678)
2. **Configuração WiFi**: Conecte-se a esta rede e acesse 192.168.4.1 no navegador
3. **Selecione sua Rede**: Escolha sua rede WiFi e insira a senha
4. **Operação Normal**: Após configurado, o dispositivo se conectará à sua rede WiFi
5. **Página de Debug**: Acesse http://[IP-DO-ESP]/debug para monitorar o sistema
6. **Reset**: Para resetar as configurações, mantenha o botão FLASH pressionado por 3 segundos

## Personalização

Para personalizar o projeto:

1. **Broker MQTT**: Altere as variáveis `mqttServer` e `mqttPort` para usar seu próprio broker
2. **Tópicos MQTT**: Modifique as variáveis `topicBase`, `topicSubscribe` e `topicPublish`
3. **Intervalo de Leitura**: Ajuste a variável `interval` para mudar a frequência de leitura do sensor
4. **Pinos**: Modifique as definições de pinos conforme necessário para seu hardware