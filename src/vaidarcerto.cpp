// Arquivo convertido automaticamente do vaidarcerto.ino para build PlatformIO
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <PubSubClient.h>

#define TRIGGER 5
#define READ 4 // GPIO4 (D2) é seguro para ESP8266
#define SWITCH 16 // GPIO16 (D0)
#define FLASH_BUTTON 0 // GPIO0 normalmente é o botão FLASH em ESP8266

// ...restante do código do vaidarcerto.ino aqui, sem o include do .ino...
