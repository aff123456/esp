#include <ESP8266WiFi.h>          // Replace with WiFi.h for ESP32
#include <ESP8266WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>
#include <WiFiUdp.h>      // biblioteca udp
#include <ArduinoJson.h>
 
WiFiUDP udp;              // cria um objeto da classe UDP

IPAddress serverIP(255,255,255,255);
IPAddress broadcastIP(255,255,255,255);

int port = 5001;

bool flag_err = false;
int err_cod = 0;

String confirmation = "OK";
String errmsg = "error";
String notFound = "404";

const int luz_1 = 1;
const int luz_2 = 2;
const int luz_3 = 3;


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

  udp.begin(port);

  StaticJsonDocument<200> doc;
  doc["id"] = 45;
  doc["name"] = "Esp_Luz_45";
  doc["connected"] = 0;
  doc["function"] = "Luz";
  doc["value"] = 1;
  String response;
  serializeJson(doc, response);
  Serial.println(response);
  
  Server.on("/", rootPage);
  // codigo fica travado nesse if até o cliente conectar na rede do esp e configurar a rede do cliente no portal
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
  // serverIP == broadcastIP quer dizer que o cliente não sabe o IP do servidor
  while(serverIP == broadcastIP) {
    broadcast(response);    // manda um pacote e vê se tem resposta
    delay(2500);            // espera pra uma nova tentativa
  }
  Serial.println("Servidor conectado, ip: " + serverIP.toString());
}

/*
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  udp.begin(port);

  const char* ssid = "WIFI_UNIFIQUE_624";
  const char* pass = "62451897";
  
  Serial.print("Conectando.");
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected: " + WiFi.localIP().toString());
  // serverIP == broadcastIP quer dizer que o cliente não sabe o IP do servidor
  while(serverIP == broadcastIP) {
    broadcast();    // manda um pacote e vê se tem resposta
    delay(1000);     // espera 1000ms pra uma nova tentativa
  }
  Serial.println("Servidor conectado, ip: " + serverIP.toString());
}
*/

void loop() {
  Portal.handleClient();
  listen(false);
}

// função que envia pacote e verifica se tem resposta do servidor
void broadcast(String mensagem) {
  //Serial.println("começa a enviar");
  //String mensagem = "pfg_ip_broadcast_cl";
  udp.beginPacket(broadcastIP, port);     // manda um pacote broadcast (mandar pacote para o ip 255.255.255.255
  udp.print(mensagem);       // conteúdo do pacote
  int test = udp.endPacket();             // pacote terminou de enviar
  //Serial.println(test);
  delay(2500);                            // espera pra ver se tem resposta
  listen(true);                           // checa pra ver se tem resposta
}

// checa a porta udp pra ver se tem pacotes disponíveis
void listen(bool config) {
  //Serial.println("escutando a porta udp");
  String req;                             // string onde vai ficar o conteúdo do que for lido
  if (udp.parsePacket() > 0) {            // parsePacket() =  tamanho do pacote a ser lido, 0 = não há pacotes a serem lidos (https://www.arduino.cc/en/Reference/WiFiUDPParsePacket)
    req = "";                             // zera a string em preparação pra mensagem
    while (udp.available() > 0) {         // available() =  número de bytes/caracteres já recebidos a serem lidos (https://www.arduino.cc/en/Reference/WiFiUDPAvailable)
      char z = udp.read();                // 
      req += z;
    }
    Serial.print("Mensagem recebida: ");
    Serial.println(req);
    if(config) {
      serverIP = udp.remoteIP();
      Serial.print("Server ip: ");
      Serial.println(serverIP);
    } else {
      int a = req.toInt();
      Serial.print("Comando: ");
      Serial.println(a);
      filtrar(a);
    }
    udp.flush();
  } else if(config) {
    Serial.println("Esperando busca");
  }
}

void filtrar(int comando) {
  switch(comando) {
    case 0:
      // erro na mensagem recebida
      Serial.println("Erro na mensagem recebida, verificar");
      send(errmsg);
    break;
    case 1:
      // mandar confirmação pra verificar se a conexão está ok
      send(confirmation);
    break;
    case 101:
      // ligar a luz 01
      atuar(luz_1,true);
      send(confirmation);
    break;
    case 102:
      // ligar a luz 02
      atuar(luz_2,true);
      send(confirmation);
    break;
    case 103:
      // ligar a luz 03
      atuar(luz_3,true);
      send(confirmation);
    break;
    case 111:
      // desligar a luz 01
      atuar(luz_1,false);
      send(confirmation);
    break;
    case 112:
      // desligar a luz 02
      atuar(luz_2,false);
      send(confirmation);
    break;
    case 113:
      // desligar a luz 03
      atuar(luz_3,false);
      send(confirmation);
    break;
    default:
      // código não encontrado, verificar
      Serial.println("Código não encontrado, verificar");
      send(notFound);
    break;
  }
}

void atuar(int porta, int sinal) {
  /*
  if(sinal) {
    digitalWrite(porta, HIGH);
  } else {
    digitalWrite(porta, LOW);
  }
  */
  if(sinal) {
    Serial.print("luz ");
    Serial.print(porta);
    Serial.println(" ligada");
  } else {
    Serial.print("luz ");
    Serial.print(porta);
    Serial.println(" desligada");
  }
}

void send(String mensagem) {
  Serial.print("Retorno enviado: ");
  Serial.println(mensagem);
  udp.beginPacket(serverIP, port);
  udp.print(mensagem);
  if(udp.endPacket() != 1) {
    Serial.println("Erro ao enviar a mensagem");
  }
}

// serial print variable type
void types(String a) { Serial.println("it's a String"); }
void types(int a) { Serial.println("it's an int"); }
void types(char *a) { Serial.println("it's a char*"); }
void types(float a) { Serial.println("it's a float"); }
void types(bool a) { Serial.println("it's a bool"); }
