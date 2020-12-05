#include <ESP8266WiFi.h>          // biblioteca para conexão wifi
#include <ESP8266WebServer.h>     // biblioteca necessária para autoconnect
#include <AutoConnect.h>          // biblioteca autoconnect
#include <WiFiUdp.h>              // biblioteca udp
#include <ArduinoJson.h>          // biblioteca JSON para sistemas embarcados

// bibliotecas de ar condicionado gree
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>

WiFiUDP udp;              // cria um objeto da classe UDP

IPAddress serverIP(255,255,255,255);    // ip placeholder para o servidor
IPAddress broadcastIP(255,255,255,255); // ip especial de broadcast

int port = 5001;        // porta udp
int port_broad = 5050;  // porta udp broadcast

bool flag_err = false;  // flag de erro
int err_cod = 0;        // código de erro

String confirmation = "OK"; // string de confirmação de pacote recebido
String errmsg = "error";    // string de erro no pacote recebido
String notFound = "404";    // string de comando não encontrado
String tempErr = "tempErr"; // string de erro na temperatura informada

int temperatura = 21;
const int temp_min = 16;
const int temp_max = 30;
String string_comando = "";

const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRGreeAC ac(kIrLed);  // Set the GPIO to be used for sending messages.

const int IRled = 14;     // pino do led que simboliza o status do ar
const int sensor = A0;    // pino do sensor de temperatura

ESP8266WebServer Server;          // Replace with WebServer for ESP32
AutoConnect      Portal(Server);  // autoconnect magic

void rootPage() {
  char content[] = "Hello, world";
  Server.send(200, "text/plain", content);
}

void printState() {
  // Display the settings.
  Serial.println("GREE A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
  // Display the encoded IR sequence.
  unsigned char* ir_code = ac.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < kGreeStateLength; i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();
}

void setup() {
  delay(1000);            // delay de 1 segundo
  Serial.begin(115200);   // inicializa comunicação serial
  Serial.println();       // print de uma linha em branco

  udp.begin(port);        // iniciliza comunicação udp na porta
  udp.begin(port_broad);

  pinMode(IRled,OUTPUT);
  pinMode(sensor,INPUT);
  
  ac.begin();
  Serial.println("Default state of the remote.");
  printState();
  
  StaticJsonDocument<200> doc;  // cria um documento JSON para enviar ao servidor na conexão com dados do cliente
  doc["id"] = 76;
  doc["name"] = "Esp_Ar_76";
  doc["ip"] = 0;
  doc["ip2"] = 1;
  doc["function"] = "Ar";
  String response;
  serializeJson(doc, response); // armazena o pacote JSON em uma variável do tipo String
  Serial.println(response);     // print no monitor serial da variável
  
  Server.on("/", rootPage);
  // codigo fica travado nesse if até o cliente conectar na rede do esp e configurar a rede do cliente no portal
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }
  // serverIP == broadcastIP quer dizer que o cliente ainda não sabe o IP do servidor
  while(serverIP == broadcastIP) {
    broadcast(response);    // manda um pacote e vê se tem resposta
    delay(2500);            // espera pra uma nova tentativa
  }
  Serial.println("Servidor conectado, ip: " + serverIP.toString()); // print do ip do servidor no monitor serial
}

// setup sem autoconnect
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
  udp.beginPacket(broadcastIP, port_broad);     // manda um pacote broadcast (mandar pacote para o ip 255.255.255.255)
  udp.print(mensagem);                          // conteúdo do pacote
  int test = udp.endPacket();                   // pacote terminou de enviar
  //Serial.println(test);
  delay(250);                                   // espera pra ver se tem resposta
  listen(true);                                 // checa pra ver se tem resposta
}

// checa a porta udp pra ver se tem pacotes disponíveis
void listen(bool config) {
  //Serial.println("escutando a porta udp");
  String req;                             // string onde vai ficar o conteúdo do que for lido
  if (udp.parsePacket() > 0) {            // parsePacket() =  tamanho do pacote a ser lido, 0 = não há pacotes a serem lidos (https://www.arduino.cc/en/Reference/WiFiUDPParsePacket)
    req = "";                             // zera a string em preparação pra mensagem
    while (udp.available() > 0) {         // available() =  número de bytes/caracteres já recebidos a serem lidos (https://www.arduino.cc/en/Reference/WiFiUDPAvailable)
      char z = udp.read();                // lê o próximo caractere e salva na variável
      req += z;                           // adiciona o caractere à string recebida
    }
    Serial.print("Mensagem recebida: ");
    Serial.println(req);
    if(req == "pfg_ip_response_serv") {
      config = true;
    }
    if(config) {
      serverIP = udp.remoteIP();
      Serial.print("Server ip: ");
      Serial.println(serverIP);
    } else if (udp.remoteIP() == serverIP) {
      int a = req.toInt();
      Serial.print("Comando: ");
      Serial.println(a);
      filtrar(a);
    } else {
      Serial.println("Essa mensagem veio de outro dispositivo, nenhum outra ação é necessária");
    }
    udp.flush();
  } else if(config) {
    Serial.println("Esperando busca");
  }
}

void filtrar(int comando) {
  int temp = temperatura;
  if(comando >= 3000 && comando <= 3099) {
    temp = comando - 3000;
    comando = 30;
  } else if(comando >= 3300 && comando <= 3399) {
    temp = comando - 3300;
    comando = 33;
  }
  if(temp >= temp_min && temp <= temp_max) {
    temperatura = temp;
  } else {
    comando = 99;
  }
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
    case 30:
      // ligar o ar
      atuar(true);
      send(confirmation);
    break;
    case 31:
      // desligar o ar
      atuar(false);
      send(confirmation);
    break;
    case 32:
      // ler entrada do sensor de temperatura
      // atuar(luz_1,false);
      temp = random(15,40);
      Serial.print("Temperatura do ambiente: ");
      Serial.println(temp);
      char buff[16];
      itoa(temp,buff,10);
      send(buff);
    break;
    case 33:
      // modificar a varivel temperatura
      // atuar(true);
      Serial.print("Nova temperatura: ");
      Serial.print(temperatura);
      Serial.println(". Caso o ar esteja ligado essa temperatura será enviada");
      send(confirmation);
    break;
    case 99:
      // temperatura inválida
      Serial.println("Temperatura inválida, verificar");
      send(tempErr);
    break;
    default:
      // código não encontrado, verificar
      Serial.println("Código não encontrado, verificar");
      send(notFound);
    break;
  }
}

void atuar(bool sinal) {
  if(sinal) {
    ac.on();
    ac.setFan(1);
    ac.setTemp(temperatura);
    Serial.println("Comando enviado:");
    printState();

    digitalWrite(IRled, HIGH);
    Serial.print("Ar ligado, temperatura: ");
    Serial.println(temperatura);
    
  } else {
    ac.off();
    printState();
    
    digitalWrite(IRled, LOW);
    Serial.println("Ar desligado");
  }
}
/*
  ac.on();
  ac.setFan(1);
  // kGreeAuto, kGreeDry, kGreeCool, kGreeFan, kGreeHeat
  ac.setMode(kGreeCool);
  ac.setTemp(20);  // 16-30C
  ac.setSwingVertical(true, kGreeSwingAuto);
  ac.setXFan(false);
  ac.setLight(false);
  ac.setSleep(false);
  ac.setTurbo(false);
*/
void send(String mensagem) {
  Serial.print("Retorno enviado: ");
  Serial.println(mensagem);

  udp.beginPacket(broadcastIP, port);     // manda um pacote broadcast (mandar pacote para o ip 255.255.255.255)
  udp.print(mensagem);                    // conteúdo do pacote
  int test = udp.endPacket();             // pacote terminou de enviar
  
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
