#include <ESP8266WiFi.h>    // biblioteca wifi

const char* ssid = "WIFI_UNIFIQUE_624";         // ID da rede wi-fi existente
const char* pass = "62451897";      // senha da rede wi-fi existente

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  
  
  Serial.print("Conectando.");
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConectado!");
  }
}

void loop() {
  
}
