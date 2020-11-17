#include <ESP8266WiFi.h>          // Replace with WiFi.h for ESP32
#include <ESP8266WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>
#include <WiFiUdp.h>      // biblioteca udp
 
WiFiUDP udp;              // cria um objeto da classe UDP

IPAddress serverIP(255,255,255,255);
IPAddress broadcastIP(255,255,255,255);
int port = 5001;

ESP8266WebServer Server;          // Replace with WebServer for ESP32
AutoConnect      Portal(Server);

void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  Server.on("/", rootPage);
  // codigo fica travado nesse if até o cliente conectar na rede do esp e configurar a rede do cliente no portal
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
  // serverIP == broadcastIP quer dizer que o cliente não sabe o IP do servidor
  while(serverIP == broadcastIP) {
    broadcast();    // manda um pacote e vê se tem resposta
    delay(500);     // espera 500ms
  }
  Serial.println("Servidor conectado, ip: " + serverIP.toString());
}

void loop() {
  Portal.handleClient();
  // codigo loop
  
}

// função que envia pacote e verifica se tem resposta do servidor
void broadcast() {
  udp.beginPacket(broadcastIP, port);     // manda um pacote broadcast (mandar pacote para o ip 255.255.255.255
  udp.write("pfg_ip_broadcast_cl");       // conteúdo do pacote
  udp.endPacket();                        // pacote terminou de enviar
  delay(25);                              // espera 25ms pra ver se tem resposta
  listen();                               // checa pra ver se tem resposta
}

// checa a porta udp pra ver se tem pacotes disponíveis
void listen() {
  String req;                             // string onde vai ficar o conteúdo do que for lido
  if (udp.parsePacket() > 0) {            // parsePacket() =  tamanho do pacote a ser lido, 0 = não há pacotes a serem lidos (https://www.arduino.cc/en/Reference/WiFiUDPParsePacket)
    req = "";                             // zera a string em preparação pra mensagem
    while (udp.available() > 0) {         // available() =  número de bytes/caracteres já recebidos a serem lidos (https://www.arduino.cc/en/Reference/WiFiUDPAvailable)
      char z = udp.read();                // 
      req += z;
    }
    serverIP = udp.remoteIP();
    Serial.print("IP server: ");
    Serial.println(serverIP);
    }
  udp.flush();
}
