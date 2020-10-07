#include <ESP8266WiFi.h>    // biblioteca wifi
#include <WiFiUdp.h>        // biblioteca udp
#include <ESP8266mDNS.h>
 
WiFiUDP udp;                // cria um objeto da classe UDP

const char* ssid = "Redmi";         // ID da rede wi-fi existente
const char* pass = "12345678";      // senha da rede wi-fi existente
const char* ap_ssid = "NodeMCU";    // ID da rede wi-fi criada
const char* ap_pass = "12345678";   // senha da rede wi-fi criada

const int port = 555;                     // porta para comunicacao UDP
const int serverport = 80;                // porta para conexao com o servidor

IPAddress stationIP(1,1,1,1);             // IP do esp estacao, placeholder ate a estacao conectar e fornecer seu ip (ver funcao listen)

// variaveis para geracao e formatacao da pagina html
String header;
String tab = "&nbsp;&nbsp;&nbsp;&nbsp;";
String singletab = "&nbsp;";
String paragrafo = "<p></p>";

// portas nas quais estarao ligados os leds acionados pelo esp servidor
const int led1 = 5;
const int led2 = 4;
const int led3 = 14;
const int led4 = 12;
const int ledconfirma = 2;

// variaveis para configuracao do ip local do esp servidor na rede existente
IPAddress local_ip(192,168,43,128);    // ip do esp servidor na rede existente
IPAddress gateway(192,168,43,1);
IPAddress subnet(255,255,255,0);

// variaveis que guardam os estados dos leds e motores
String ledstate_1 = "desligado";
String ledstate_2 = "desligado";
String ledstate_3 = "desligado";
String ledstate_4 = "desligado";
String motorstate_1 = "fechado";
String motorstate_2 = "fechado";

WiFiServer server(serverport);        // cria um objeto da classe WiFiServer

int contador = 0;
int limitecontagem = 500;
bool confirmacao = false;
bool erro = false;

void setup() { 
  Serial.begin(115200);               // inicia a comunicacao serial]

  // inicializa as portas dos leds
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  
  led(led1,false);
  led(led2,false);
  led(led3,false);
  led(led4,false);

  pinMode(ledconfirma, OUTPUT);
  digitalWrite(ledconfirma, LOW);

  udp.begin(port);                        // inicializa protocolo udp na porta escolhida

  WiFi.mode(WIFI_AP_STA);                 // configura o esp servidor pro modo access point + station (AP para o esp cliente, station na rede existente)
  WiFi.begin(ssid,pass);                  // conecta na rede existente
  WiFi.config(local_ip,subnet,gateway);   // configura o ip do esp servidor na rede existente
  Serial.print("Conectando a rede: ");
  Serial.print(ssid);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("");
  Serial.print("Conectado! IP local: ");
  Serial.println(WiFi.localIP());         // mostra o ip do servidor na rede existente (configurado acima)

  // inicia o servido de multicast dns
  if(MDNS.begin("myesp")) {
    Serial.println("MDNS iniciado!");
  }
  MDNS.addService("http","tcp",serverport); // inicia o servico de mDNS para os protocolos http e tcp na porta escolhida
  
  // configuracao do ip local na rede wi-fi a ser criada
  IPAddress local_ip(192,168,4,11);
  IPAddress gateway(192,168,1,11);
  IPAddress subnet(255,255,255,0);
  
  WiFi.softAP(ap_ssid,ap_pass);             // configura nome e senha da rede criada
  WiFi.begin();                             // inicia a rede wi-fi na qual o esp cliente vai se conectar
  WiFi.softAPConfig(local_ip, subnet, gateway); // configura o ip do esp servidor na rede criada
  
  Serial.print("IP access point: ");
  Serial.println(WiFi.softAPIP());          // mostra o ip do esp servidor na rede criada (configurado acima)
  
  server.begin();                           // inicia o webserver
  
}
 
void loop() {
  MDNS.update();                            // checa por conexoes no webserver utilizando o servidor dns local
  WiFiClient client = server.available();   // cria um objeto da classe WiFiClient e mostra o webserver como disponivel
  listen();                                 // checa se ha pacotes udp na porta 555
  if(client){                               // caso um cliente conecte a funcao clientconnected trata da conexao
    clientconnected(client);
  }
}

void clientconnected(WiFiClient client){
  Serial.println("Novo cliente!");
  String currentLine = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      header += c;
      if (c == '\n') {
        if (currentLine.length() == 0) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
        
          // liga e desliga os LEDs, manda pacotes para o cliente acionar os motores
          if (header.indexOf("GET /1/on") >= 0) {
            Serial.println("LED 1 ligado");
            ledstate_1 = "ligado";
            led(led1,true);
          } else if (header.indexOf("GET /1/off") >= 0) {
            Serial.println("LED 1 desligado");
            ledstate_1 = "desligado";
            led(led1,false);
          } else if (header.indexOf("GET /2/on") >= 0) {
            Serial.println("LED 2 ligado");
            ledstate_2 = "ligado";
            led(led2,true);
          } else if (header.indexOf("GET /2/off") >= 0) {
            Serial.println("LED 2 desligado");
            ledstate_2 = "desligado";
            led(led2,false);
          } else if (header.indexOf("GET /3/on") >= 0) {
            Serial.println("LED 3 ligado");
            ledstate_3 = "ligado";
            led(led3,true);
          } else if (header.indexOf("GET /3/off") >= 0) {
            Serial.println("LED 3 desligado");
            ledstate_3 = "desligado";
            led(led3,false);
          } else if (header.indexOf("GET /4/on") >= 0) {
            Serial.println("LED 4 ligado");
            ledstate_4 = "ligado";
            led(led4,true);
          } else if (header.indexOf("GET /4/off") >= 0) {
            Serial.println("LED 4 desligado");
            ledstate_4 = "desligado";
            led(led4,false);
          } else if (header.indexOf("GET /m1/l1") >=0) {
            send(50);
            Serial.println("Motor 1 ligado, sentido horario");
            motorstate_1 = "aberto";
          } else if (header.indexOf("GET /m1/l2") >=0) {
            send(51);
            Serial.println("Motor 1 ligado, sentido anti-horario");
            motorstate_1 = "fechado";
          } else if (header.indexOf("GET /m2/l1") >=0) {
            send(60);
            Serial.println("Motor 2 ligado, sentido horario");
            motorstate_2 = "aberto";
          } else if (header.indexOf("GET /m2/l2") >=0) {
            send(61);
            Serial.println("Motor 2 ligado, sentido anti-horario");
            motorstate_2 = "fechado";
          }
          
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          
          client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
          client.println(".button {background-color: #41DD83; border: none; color: white; padding: 8px 12px;");
          client.println("text-decoration: none; font-size: 22px; margin: 2px; cursor: pointer;}");
          client.println(".button2 {background-color: #FF5733;}");
          client.println(".button3 {background-color: #71BEEA;}");
          client.println(".button4 {background-color: #BA71EA;}</style></head>");
          
          client.println("<body><h1>Sistema de automacao residencial</h1>");
          
          // mostra o estado do led 1
          client.println("Led 1: " + ledstate_1);
          if(ledstate_1=="ligado") {
            client.println(tab);
          }
          client.println(tab);
          
          // mostra o estado do led 2
          client.println("Led 2: " + ledstate_2);
          if(ledstate_2=="ligado") {
            client.println(tab);
          }
          
          client.println(paragrafo);
          
          // mostra os botoes correspondentes aos estados dos leds
          if (ledstate_1=="desligado") {
            client.println(tab + tab + singletab);
            client.println("<a href=\"/1/on\"><button class=\"button\">LIGAR</button></a>");
          } else {
            client.println("<a href=\"/1/off\"><button class=\"button button2\">DESLIGAR</button></a>");
          }

          client.println(tab);
          
          if (ledstate_2=="desligado") {
            client.println("<a href=\"/2/on\"><button class=\"button\">LIGAR</button></a>");
            client.println(tab + tab + singletab);
          } else {
            client.println("<a href=\"/2/off\"><button class=\"button button2\">DESLIGAR</button></a>");
          }
          
          client.println(paragrafo);
          
          // mostra o estado do led 3
          client.println("Led 3: " + ledstate_3);
          if(ledstate_3=="ligado") {
            client.println(tab);
          }
          client.println(tab);
          
          // mostra o estado do led 4
          client.println("Led 4: " + ledstate_4);
          if(ledstate_4=="ligado") {
            client.println(tab);
          }
          client.println(paragrafo);
          
          // mostra os botoes correspondentes aos estados dos leds
          if (ledstate_3=="desligado") {
            client.println(tab + tab + singletab);
            client.println("<a href=\"/3/on\"><button class=\"button\">LIGAR</button></a>");
          } else {
            client.println("<a href=\"/3/off\"><button class=\"button button2\">DESLIGAR</button></a>");
          }
          
          client.println(tab);
          
          if (ledstate_4=="desligado") {
            client.println("<a href=\"/4/on\"><button class=\"button\">LIGAR</button></a>");
            client.println(tab + tab + singletab);
          } else {
            client.println("<a href=\"/4/off\"><button class=\"button button2\">DESLIGAR</button></a>");
          }
          
          client.println(paragrafo);
          
          // mostra o estado do motor 1
          client.println("Motor 1: " + motorstate_1);
          if(motorstate_1=="ligado") {
            client.println(singletab);
          }
          client.println(tab);
          
          // mostra o estado do motor 2
          client.println("Motor 2: " + motorstate_2);
          if(motorstate_2=="ligado") {
            client.println(singletab);
          }
          client.println(paragrafo);
          
          // mostra os botoes correspondentes aos estados dos motores
          if (motorstate_1=="fechado") {
            client.println(tab + singletab);
            client.println("<a href=\"/m1/l1\"><button class=\"button button3\">ABRIR</button></a>");
          } else {
            client.println("<a href=\"/m1/l2\"><button class=\"button button4\">FECHAR</button></a>");
          }
          
          client.println(tab);
          
          if (motorstate_2=="fechado") {
            client.println("<a href=\"/m2/l1\"><button class=\"button button3\">ABRIR</button></a>");
            client.println(tab + singletab);
          } else {
            client.println("<a href=\"/m2/l2\"><button class=\"button button4\">FECHAR</button></a>");
          }
          
          client.println("</body></html>");
          
          client.println();

          break;
        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }
  header = "";
  client.stop();
}

void send(int num) {
  udp.beginPacket(stationIP, port);
  udp.println(num);
  udp.endPacket();
}

void listen() {
  String req;
  if (udp.parsePacket() > 0) {
    req = "";
    while (udp.available() > 0) {
      char z = udp.read();
      req += z;
    }
    stationIP = udp.remoteIP();
    Serial.print("IP station: ");
    Serial.println(stationIP);
    digitalWrite(ledconfirma, HIGH);
    }
  udp.flush();
}

void led(int porta, bool sinal) {
  if(sinal) {
    digitalWrite(porta,HIGH);
  } else {
    digitalWrite(porta,LOW);
  }
}
