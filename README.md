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
- [Uso com Arduino IDE](#-uso-com-arduino-ide)
- [Personalização](#-personalização)
- [Brokers MQTT](#-brokers-mqtt)

## 🚀 Funcionalidades

### Configuração WiFi via Portal Web
- Modo AP automático quando não há configuração salva
- Interface web responsiva para configurar credenciais WiFi
- Armazenamento na EEPROM para persistência
- Reset via botão FLASH (pressione por 3 segundos)

### Comunicação MQTT
- Conexão com brokers públicos (test.mosquitto.org ou broker.emqx.io) com keep-alive de 2 minutos
- Publicação de leituras do sensor no tópico `meuESP8266/saida`
- Publicação automática do IP no tópico `meuESP8266/IPnaRede` a cada minuto
- Publicação periódica do estado do relé a cada 5 segundos no tópico `meuESP8266/relay`
- Controle do relé via comandos MQTT com atualização em tempo real do estado
- Reconexão automática em caso de falha de conexão WiFi ou MQTT
- ID de cliente único baseado no chip ID para evitar desconexões

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
- **Monitoramento em Tempo Real**: O estado atual do relé é publicado no tópico `meuESP8266/relay` sempre que há mudança
- **Publicação Periódica**: O estado do relé é publicado periodicamente a cada 5 segundos para garantir sincronização
- **Mensagens Retidas**: As mensagens de estado do relé são marcadas como "retidas" no broker para persistência
- **Controle Prioritário via App**: Comandos enviados pelo app têm prioridade sobre o controle automático
- **Modo Automático/Manual**: Possibilidade de alternar entre controle automático e manual
- **Verificação de Consistência**: O sistema verifica e corrige automaticamente inconsistências entre o estado físico e lógico do relé

## 🔌 Hardware

### Componentes Necessários
- NodeMCU ESP8266 ou compatível
- Sensor ultrassônico HC-SR04
- Relé para controle baseado em distância
- Cabos de conexão

### Mapeamento de Pinos

| Componente | Pino ESP8266 | Pino NodeMCU | Função |
|------------|--------------|--------------|--------|
| Trigger (Sensor) | GPIO4 | D2 | Saída para trigger do sensor |
| Echo (Sensor) | GPIO14 | D5 | Entrada para echo do sensor |
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
  - **Status do Sistema**: Informações sobre WiFi, MQTT e estado atual
  - **Broker MQTT**: Mostra qual broker está sendo usado e seu status
  - **Tópicos MQTT**: Lista todos os tópicos usados pelo dispositivo
  - **Comandos Disponíveis**: Lista todos os comandos que podem ser enviados
- Para resetar as configurações, mantenha o botão FLASH pressionado por 3 segundos
- Observe o LED integrado para entender o status:
  - **Sempre aceso**: Modo AP ativo (aguardando configuração)
  - **Apagado**: Conectado a uma rede WiFi
  - **Piscando**: Enviando ou recebendo dados via MQTT

## 📱 Uso com MQTT Panel

### Configuração do Broker
1. Instale "IoT MQTT Panel" da Google Play Store
2. Adicione um novo Broker (escolha uma das opções):

   **Opção 1: test.mosquitto.org**
   - Nome: ESP8266 Monitor
   - Endereço: `test.mosquitto.org`
   - Porta: `1883`
   - Cliente ID: (opcional)
   - Sem autenticação
   
   **Opção 2: broker.emqx.io (alternativa mais estável)**
   - Nome: ESP8266 Monitor EMQX
   - Endereço: `broker.emqx.io`
   - Porta: `1883`
   - Cliente ID: (opcional)
   - Sem autenticação

### Painéis de Controle
1. **Leitura do Sensor**:
   - Tipo: Text
   - Nome: Distância
   - Tópico: `meuESP8266/saida`
   - QoS: 0

2. **Controle do Relé**:
   - Tipo: Switch
   - Nome: Controle Relé
   - Tópico: `meuESP8266/entrada`
   - Valor ON: `LIGAR_RELE`
   - Valor OFF: `DESLIGAR_RELE`
   - QoS: 0

3. **Modo de Controle do Relé**:
   - Tipo: Button
   - Nome: Modo Automático
   - Tópico: `meuESP8266/entrada`
   - Valor: `AUTO_RELE`
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
   - Atualização: A cada minuto

### Configuração de Limites de Distância
- Para alterar o limite mínimo, envie `MIN:valor` para o tópico `meuESP8266/entrada`
- Para alterar o limite máximo, envie `MAX:valor` para o tópico `meuESP8266/entrada`
- Exemplo: `MIN:5.0` define o limite mínimo para 5 cm

### Uso
1. Conecte ao broker (botão no canto superior direito)
2. Os painéis mostrarão os dados recebidos do ESP8266
3. Use o switch para controlar o relé manualmente (prioridade sobre o modo automático)
4. Pressione o botão "Modo Automático" para retornar ao controle baseado na distância
5. Envie comandos MIN: e MAX: para configurar os limites de distância
6. Monitore o estado atual do relé e o IP do dispositivo nos painéis de texto

## 🔌 Uso com Arduino IDE

Se preferir usar a Arduino IDE em vez do PlatformIO, siga estas instruções:

### Configuração da Arduino IDE

1. **Instale a Arduino IDE**:
   - Download: [Arduino IDE](https://www.arduino.cc/en/software)

2. **Configure o suporte ao ESP8266**:
   - Abra a Arduino IDE
   - Vá para Arquivo > Preferências
   - Em "URLs Adicionais para Gerenciadores de Placas", adicione:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Clique em OK
   - Vá para Ferramentas > Placa > Gerenciador de Placas
   - Procure por "ESP8266" e instale "ESP8266 Community"

3. **Selecione a placa correta**:
   - Vá para Ferramentas > Placa > ESP8266 Boards
   - **IMPORTANTE: Selecione "NodeMCU 1.0 (ESP-12E Module)"** - Esta é a placa recomendada para este projeto
   - Configure as opções:
     - Upload Speed: 115200
     - CPU Frequency: 80MHz
     - Flash Size: 4MB (FS:2MB OTA:~1019KB)
     - Debug Port: Disabled
     - Debug Level: None
     - IwIP Variant: v2 Lower Memory
     - VTables: Flash
     - Exceptions: Enabled
     - Erase Flash: Only Sketch
   
   ![Seleção da placa NodeMCU](https://down-br.img.susercontent.com/file/8da4f352b2486d88cf666f7e86e912bc.webp)

### Instalação das Bibliotecas

1. Vá para Sketch > Incluir Biblioteca > Gerenciar Bibliotecas
2. Instale as seguintes bibliotecas:
   - **PubSubClient** (por Nick O'Leary)
   - **ESP8266WebServer** (já incluída no pacote ESP8266)

### Preparação do Código

1. Crie um novo sketch na Arduino IDE
2. Copie todo o conteúdo do arquivo `main.cpp` para o sketch
3. Salve o sketch com um nome descritivo (ex: ESP8266_MQTT_Sensor)

### Compilação e Upload

1. Conecte o ESP8266 ao computador via USB
2. Selecione a porta correta em Ferramentas > Porta
3. Clique no botão "Verificar" para compilar
4. Clique no botão "Carregar" para fazer o upload para o ESP8266
5. Abra o Monitor Serial (Ferramentas > Monitor Serial) e configure para 115200 baud

### Solução de Problemas

- **Erro de compilação**: Verifique se todas as bibliotecas estão instaladas
- **Erro de upload**: Verifique se a placa está conectada e a porta correta está selecionada
- **Modo de boot**: Se o upload falhar, tente pressionar o botão FLASH durante o início do upload
- **Problemas de conexão MQTT**: 
  - Verifique se o broker está acessível (test.mosquitto.org pode ficar sobrecarregado)
  - Experimente usar o broker alternativo `broker.emqx.io` que geralmente é mais estável
  - Considere usar um broker MQTT local como o Mosquitto ou HiveMQ
  - Aumente o valor de `mqttKeepAlive` se a conexão cair frequentemente
- **Estado do relé não atualiza**:
  - Verifique os logs no Monitor Serial para confirmar que as mensagens estão sendo publicadas
  - Certifique-se de que o aplicativo MQTT está inscrito no tópico correto (`meuESP8266/relay`)
  - Verifique se os tópicos MQTT estão escritos corretamente (meuESP8266, não meuESP8866)
  - Reinicie o aplicativo MQTT se ele não mostrar as atualizações
- **Distância não é publicada**:
  - Verifique se o sensor ultrassônico está conectado corretamente
  - Confirme que o tópico de publicação está correto (`meuESP8266/saida`)
  - Observe os logs no Monitor Serial para ver se a distância está sendo medida e publicada
  - Verifique se o cliente MQTT está conectado ao broker

## 🛠️ Personalização

Para adaptar o projeto às suas necessidades:

- **Broker MQTT**: Altere `mqttServer` e `mqttPort` para seu próprio broker
  ```cpp
  // Opção 1: test.mosquitto.org (padrão)
  const char* mqttServer = "test.mosquitto.org";
  const int mqttPort = 1883;
  
  // Opção 2: broker.emqx.io (alternativa mais estável)
  const char* mqttServer = "broker.emqx.io";
  const int mqttPort = 1883;
  ```
- **Tópicos MQTT**: Modifique `topicBase`, `topicSubscribe` e `topicPublish`
- **Intervalo de Leitura**: Ajuste `interval` para mudar a frequência de leitura
- **Limites de Distância**: Modifique `minDistance` e `maxDistance` para ajustar os limites de acionamento do relé
- **Pinos**: Modifique as definições de pinos conforme seu hardware
- **Intervalos de Publicação**: Ajuste `ipPublishInterval` e `relayPublishInterval` para controlar a frequência de publicação
- **Keep-Alive MQTT**: Modifique `mqttKeepAlive` para ajustar o tempo de keep-alive da conexão MQTT

## 🌐 Brokers MQTT

### Sobre Brokers MQTT

Um broker MQTT é um servidor que recebe mensagens dos clientes e as encaminha para os clientes inscritos nos tópicos correspondentes. Este projeto pode usar diferentes brokers MQTT:

### Opções de Brokers Públicos

1. **test.mosquitto.org**
   - Broker público mantido pelo projeto Eclipse Mosquitto
   - Não requer autenticação
   - Pode ficar sobrecarregado em horários de pico
   - Porta padrão: 1883 (não criptografada)

2. **broker.emqx.io**
   - Broker público mantido pela EMQ
   - Geralmente mais estável que o test.mosquitto.org
   - Não requer autenticação
   - Porta padrão: 1883 (não criptografada)
   - Suporta WebSocket na porta 8083

### Como Mudar o Broker

Para mudar o broker no código, altere as seguintes linhas:

```cpp
// No início do arquivo main.cpp
const char* mqttServer = "broker.emqx.io";  // Altere para o broker desejado
const int mqttPort = 1883;                  // Porta do broker
```

### Testando a Conexão com o Broker

Você pode testar se o broker está acessível usando ferramentas como:

1. **MQTT Explorer**: Uma interface gráfica para testar conexões MQTT
   - Download: [MQTT Explorer](http://mqtt-explorer.com/)

2. **Mosquitto CLI**: Ferramentas de linha de comando
   ```bash
   # Inscrever-se em um tópico
   mosquitto_sub -h broker.emqx.io -t meuESP8266/# -v
   
   # Publicar em um tópico
   mosquitto_pub -h broker.emqx.io -t meuESP8266/entrada -m "LIGAR_RELE"
   ```

3. **Aplicativos móveis**:
   - IoT MQTT Panel (Android)
   - MQTT Client (iOS)

### Broker Local

Para maior confiabilidade, você pode configurar seu próprio broker MQTT local:

1. **Mosquitto**: Broker leve e de código aberto
   - [Instruções de instalação](https://mosquitto.org/download/)

2. **HiveMQ**: Broker com interface web
   - [HiveMQ Community Edition](https://www.hivemq.com/downloads/)

Após configurar um broker local, altere o código para usar o endereço IP do seu servidor:

```cpp
const char* mqttServer = "192.168.1.100";  // Substitua pelo IP do seu broker local
const int mqttPort = 1883;
```