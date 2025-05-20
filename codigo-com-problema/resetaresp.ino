#include <EEPROM.h>
#include <ESP8266WiFi.h>

void setup() {
    Serial.begin(115200);
    
    // Apagar a EEPROM (memória não volátil)
    EEPROM.begin(512);
    for (int i = 0; i < 512; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    
    // Apagar WiFi settings
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.persistent(false);
    WiFi.mode(WIFI_OFF);
    
    Serial.println("Todas as configurações foram apagadas!");
    
    // Resetar o ESP8266
    ESP.restart();
}

void loop() {
    // O código não chega aqui pois o ESP será reiniciado no setup()
}