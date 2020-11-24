#include <ESP8266WiFi.h>          // Replace with WiFi.h for ESP32
//#include <ESP8266WebServer.h>     // Replace with WebServer.h for ESP32
//#include <AutoConnect.h>
#include <WiFiUdp.h>      // biblioteca udp
 
WiFiUDP udp;              // cria um objeto da classe UDP

IPAddress serverIP(255,255,255,255);
IPAddress broadcastIP(255,255,255,255);

int port = 5001;

const char* ssid = "WIFI_UNIFIQUE_624";
const char* pass = "62451897";

bool flag_err = false;
int err_cod = 0;

/*
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
  
  Server.on("/", rootPage);
  // codigo fica travado nesse if até o cliente conectar na rede do esp e configurar a rede do cliente no portal
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
  // serverIP == broadcastIP quer dizer que o cliente não sabe o IP do servidor
  while(serverIP == broadcastIP) {
    broadcast();    // manda um pacote e vê se tem resposta
    delay(500);     // espera 500ms pra uma nova tentativa
  }
  Serial.println("Servidor conectado, ip: " + serverIP.toString());
}
*/

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();

  udp.begin(port);

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

void loop() {
  // Portal.handleClient();
  listen(false);
  if(flag_err) {
    Serial.println();
    Serial.print("Ocorreu um erro na execução do código. Erro:");
    Serial.println(err_cod);
  }
}

// função que envia pacote e verifica se tem resposta do servidor
void broadcast() {
  //Serial.println("começa a enviar");
  udp.beginPacket(broadcastIP, port);     // manda um pacote broadcast (mandar pacote para o ip 255.255.255.255
  udp.write("pfg_ip_broadcast_cl");       // conteúdo do pacote
  int test = udp.endPacket();             // pacote terminou de enviar
  //Serial.println(test);
  delay(500);                             // espera 500ms pra ver se tem resposta
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
      act(a);
    }
  } else {
    Serial.println("Mensagem de retorno não recebida");
  }
  udp.flush();
}

void act(int comando) {
  switch(comando) {
    case 0:
      flag_err = true;
      err_cod = 1;
    break;
    case 1:
      send("OK");
    break;
    case 101:
      // ligar a luz 01
    break;
    case 102:
      // ligar a luz 02
    break;
    case 103:
      // ligar a luz 03
    break;
    case 111:
      // desligar a luz 01
    break;
    case 112:
      // desligar a luz 02
    break;
    case 113:
      // desligar a luz 03
    break;
    default:
      // código não encontrado, verificar
    break;
  }
}

void send(String mensagem) {
  Serial.print("Retorno enviado: ");
  Serial.println(mensagem);
  udp.beginPacket(serverIP, port);
  udp.write("OK");
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
