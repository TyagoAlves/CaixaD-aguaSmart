# ESP8266 IoT com MQTT e Configuração Web

Este projeto implementa um sistema IoT baseado em ESP8266 que combina comunicação MQTT, servidor web para configuração e leitura de sensor ultrassônico.

## Funcionalidades Principais

### 1. Configuração WiFi via Portal Web
- **Modo AP Automático**: Cria um ponto de acesso WiFi quando não há configuração salva
- **Interface Web Amigável**: Página web responsiva para configurar credenciais WiFi
- **Armazenamento na EEPROM**: Salva as credenciais WiFi na memória não-volátil
- **Reset via Botão**: Pressione o botão FLASH por 3 segundos para limpar configurações

### 2. Comunicação MQTT
- **Conexão com Broker**: Conecta-se ao broker MQTT público (test.mosquitto.org)
- **Publicação de Dados**: Envia leituras do sensor ultrassônico periodicamente no tópico "meuESP8266/saida"
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

## Mapeamento de Pinos no Arduino/ESP8266

Para conectar corretamente os componentes ao ESP8266, use este mapeamento de pinos:

| Componente | Pino ESP8266 | Pino NodeMCU | Função |
|------------|--------------|--------------|--------|
| Trigger (Sensor) | GPIO5 | D1 | Saída para trigger do sensor ultrassônico |
| Echo (Sensor) | GPIO18 (ou GPIO4) | D2 | Entrada para echo do sensor ultrassônico |
| LED/Relé | GPIO16 | D0 | Saída para controle do LED ou relé |
| Botão FLASH | GPIO0 | FLASH | Botão para reset de configurações |

**Nota**: O ESP8266 usa numeração GPIO diferente da numeração física dos pinos. A tabela acima mostra ambas as referências para facilitar a conexão.

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
   **Ou instale usando Winget
   ```winget install --id Microsoft.VisualStudioCode -e```
3. **Instale a Extensão PlatformIO**: Abra o VS Code, vá para Extensions (Ctrl+Shift+X) e busque por "PlatformIO"
4. **Reinicie o VS Code** após a instalação



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

## Conexão com IoT MQTT Panel (Android)

O IoT MQTT Panel é um aplicativo Android que permite monitorar e controlar dispositivos MQTT de forma visual. Siga estas etapas para conectar ao seu ESP8266:

### Configuração do Broker

1. **Instale o aplicativo**: Baixe "IoT MQTT Panel" da Google Play Store
2. **Adicione um novo Broker**:
   - Clique no botão "+" e selecione "Broker"
   - Nome: ESP8266 Monitor (ou qualquer nome de sua preferência)
   - Endereço: `test.mosquitto.org` (mesmo usado no código)
   - Porta: `1883`
   - Cliente ID: Deixe em branco ou use um ID único
   - Usuário/Senha: Deixe em branco (o broker público não requer autenticação)

### Configuração dos Painéis

1. **Painel para Leitura do Sensor**:
   - Clique no botão "+" e selecione "Text"
   - Nome: Distância
   - Tópico: `meuESP8266/testeBasico/saida` (tópico onde o ESP publica as leituras)
   - QoS: 0
   - Mantenha as outras configurações padrão

2. **Painel para Controle do LED/Relé**:
   - Clique no botão "+" e selecione "Switch"
   - Nome: Controle LED
   - Tópico: `meuESP32/testeBasico/entrada` (tópico que o ESP assina)
   - Valor ON: `LIGAR`
   - Valor OFF: `DESLIGAR`
   - QoS: 0

3. **Painel para Monitorar o IP**:
   - Clique no botão "+" e selecione "Text"
   - Nome: IP do ESP8266
   - Tópico: `meuESP8266/IPnaRede`
   - QoS: 0

### Conexão e Uso

1. **Conecte ao Broker**: Clique no botão "Conectar" no canto superior direito
2. **Visualize os Dados**: Os painéis mostrarão os dados recebidos do ESP8266
3. **Controle o Dispositivo**: Use o switch para enviar comandos LIGAR/DESLIGAR

## Personalização

Para personalizar o projeto:

1. **Broker MQTT**: Altere as variáveis `mqttServer` e `mqttPort` para usar seu próprio broker
2. **Tópicos MQTT**: Modifique as variáveis `topicBase`, `topicSubscribe` e `topicPublish`
3. **Intervalo de Leitura**: Ajuste a variável `interval` para mudar a frequência de leitura do sensor
4. **Pinos**: Modifique as definições de pinos conforme necessário para seu hardware
