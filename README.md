# Projeto ESP8266 com MQTT e Configuração Web

Este projeto implementa um sistema IoT baseado em ESP8266 que combina comunicação MQTT, servidor web para configuração e leitura de sensor ultrassônico.

## Sobre o main.cpp

O arquivo `main.cpp` é o núcleo do projeto, responsável por gerenciar todas as funcionalidades do dispositivo ESP8266. Abaixo estão as principais características e componentes:

### Bibliotecas Utilizadas

```cpp
#include <ESP8266WiFi.h>    // Conectividade WiFi para ESP8266
#include <PubSubClient.h>   // Cliente MQTT
#include <EEPROM.h>         // Armazenamento persistente
#include <ESP8266WebServer.h> // Servidor web para configuração
```

### Definições de Hardware

- **TRIGGER (GPIO5)**: Pino para acionar o sensor ultrassônico
- **READ (GPIO18)**: Pino para leitura do sensor ultrassônico
- **SWITCH (GPIO16)**: Pino para controle de saída (LED/relé)
- **FLASH_BUTTON (GPIO0)**: Botão flash do ESP8266 para reset de configurações

### Principais Funcionalidades

1. **Modo de Configuração AP**
   - Cria um ponto de acesso WiFi quando não há configuração salva
   - Interface web para configurar credenciais WiFi
   - Página de depuração em `/debug` para monitoramento do dispositivo

2. **Conectividade MQTT**
   - Conexão com broker MQTT (test.mosquitto.org)
   - Publicação de dados do sensor em tópicos configuráveis
   - Recebimento de comandos para controle do pino SWITCH

3. **Gerenciamento de WiFi**
   - Armazenamento de credenciais na EEPROM
   - Reconexão automática
   - Reset de configurações através do botão FLASH

4. **Sensor Ultrassônico**
   - Leitura periódica de distância
   - Publicação dos valores via MQTT
   - Controle automático baseado na leitura do sensor

### Fluxo de Execução

1. **Inicialização (setup)**
   - Configura pinos e comunicação serial
   - Tenta conectar ao WiFi usando credenciais salvas
   - Se falhar, inicia modo AP para configuração
   - Configura servidor MQTT e callbacks

2. **Loop Principal (loop)**
   - Monitora o botão FLASH para reset
   - Processa requisições web
   - Mantém conexão MQTT ativa
   - Realiza leituras periódicas do sensor
   - Publica dados e controla saídas

## Sistema de Versões

O projeto suporta diferentes versões através da importação seletiva de arquivos `.ino`. Abaixo estão as versões disponíveis:

### Versão 1.0 - Básica
- Implementação padrão com funcionalidades essenciais
- Arquivo: `main.cpp` (versão atual)

### Versão 2.0 - Debug Avançado
- Adiciona página de depuração melhorada
- Arquivo: `pagina debug boa 15-54 20-05-2025.ino`

### Versão 3.0 - Configuração Completa
- Sistema completo com múltiplas páginas de configuração
- Arquivo: `vaidarcerto.ino`

## Como Alterar Versões

Para alterar entre as diferentes versões do projeto, você pode modificar o arquivo `main.cpp` para importar o código desejado. Existem duas abordagens:

### Abordagem 1: Substituição Direta

Substitua o conteúdo do `main.cpp` pelo conteúdo do arquivo `.ino` desejado.

### Abordagem 2: Sistema de Versões por Compilação Condicional

Adicione ao início do `main.cpp`:

```cpp
// Selecione a versão desejada descomentando apenas uma linha
#define VERSION_BASIC
//#define VERSION_DEBUG
//#define VERSION_FULL

#ifdef VERSION_BASIC
// Código da versão básica
#endif

#ifdef VERSION_DEBUG
// Código da versão com debug avançado
#endif

#ifdef VERSION_FULL
// Código da versão completa
#endif
```

## Instruções de Uso

1. Compile e carregue o código para o ESP8266
2. Se não houver configuração WiFi salva, conecte-se à rede "ESP8266_Config" (senha: 12345678)
3. Acesse 192.168.4.1 no navegador para configurar o WiFi
4. Após configurado, o dispositivo se conectará à sua rede WiFi
5. Para acessar a página de debug, use http://[IP-DO-ESP]/debug
6. Para resetar as configurações, mantenha o botão FLASH pressionado por 3 segundos

## Notas de Desenvolvimento

- O servidor web utiliza HTML e CSS inline para uma interface amigável
- As credenciais WiFi são armazenadas de forma segura na EEPROM
- O sistema é projetado para ser resiliente a falhas de conexão
- A página de debug fornece informações úteis para diagnóstico