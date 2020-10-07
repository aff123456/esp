#include <ESP8266WiFi.h>  // biblioteca wifi
#include <WiFiUdp.h>      // biblioteca udp
 
WiFiUDP udp;              // cria um objeto da classe UDP
 
String req;               // string que armazena os dados recebidos pela rede

const char* ssid = "NodeMCU";
const char* pass = "12345678";
int v1[4] = {16, 5, 4, 0};
int v2[4] = {14, 12, 13, 3};

bool m1 = false;
bool m2 = false;

bool ligar1 = false;
bool ligar2 = false;

int lim11 = 2500;
int lim12 = 0;
int lim21 = 2500;
int lim22 = 0;

int cont1 = 0;
int cont2 = 0;

int step1 = 0;
int step2 = 0;

int contador = 0;
int limite = 0;
int step_number = 0;

int PIN1 = 0;
int PIN2 = 0;
int PIN3 = 0;
int PIN4 = 0;

const int ledconfirma = 2;

IPAddress serverIP(192,168,4,11);

void setup() {

  pinMode(14,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  pinMode(15,OUTPUT);
  pinMode(16,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(0,OUTPUT);

  pinMode(ledconfirma, OUTPUT);
  digitalWrite(ledconfirma, LOW);
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);  
    
  udp.begin(555);
  
  Serial.print("Conectando.");
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print(".");
  }

  Serial.println("Conectado a rede!");
  Serial.print("IP station: ");
  Serial.println(WiFi.localIP());

  for(int i = 0; i < 50; i++){
    udp.beginPacket(serverIP,555);
    udp.write(1);
    udp.endPacket();
    delay(2);
  }
}
 
void loop() {
  listen();
  if(ligar1) {
    OneStep(m1,true);
    delay(2);
  }
  if(ligar2) {
    OneStep(m2,false);
    delay(2);
  }
}
 
void listen() {
  if(udp.parsePacket() > 0) {
    req = "";
    while (udp.available() > 0) {
      char z = udp.read();
      req += z;
    }
    Serial.println(req);
    digitalWrite(ledconfirma, HIGH);
    int a = req.toInt();
    Serial.print("a: ");
    Serial.println(a);
    funcao(a);
  } else {
    digitalWrite(ledconfirma, LOW);
  }
}


void funcao(int num) {
  if(num == 50) {
    ligar1 = true;
    m1 = true;
  } else if(num == 51) {
    ligar1 = true;
    m1 = false;
  } else if(num == 60) {
    ligar2 = true;
    m2 = true;
  } else if(num == 61) {
    ligar2 = true;
    m2 = false;
  }
}

void OneStep(bool dir, bool motor) {
  if(motor) {
    contador = cont1;
    step_number = step1;
    PIN1 = 16;
    PIN2 = 5;
    PIN3 = 4;
    PIN4 = 0;
    if(dir) {
      limite = lim11;
    } else {
      limite = lim12;
    }
  } else {
    contador = cont2;
    step_number = step2;
    PIN1 = 14;
    PIN2 = 12;
    PIN3 = 13;
    PIN4 = 15;
    if(dir) {
      limite = lim21;
    } else {
      limite = lim22;
    }
  }
  Serial.print("Contador: ");
  Serial.print(contador);
  Serial.print(", limite: ");
  Serial.print(limite);
  Serial.print(", step: ");
  Serial.println(step_number);
  if(dir) {
    if(contador < limite) {
      switch(step_number) {
        case 0:
        digitalWrite(PIN1, HIGH);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, LOW);
        break;
        case 1:
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, HIGH);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, LOW);
        break;
        case 2:
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, HIGH);
        digitalWrite(PIN4, LOW);
        break;
        case 3:
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, HIGH);
        break;
      }
      if(motor) {
        step1 = step1 + 1;
        cont1 += 1;
        if(step1 > 3) {
          step1 = 0;
        }
      } else {
        step2 = step2 + 1;
        cont2 += 1;
        if(step2 > 3) {
          step2 = 0;
        }
      }
    } else {
      if(motor) {
        ligar1 = false;
      } else {
        ligar2 = false;
      }
    }
  } else {
    if(contador > limite) {
      switch(step_number) {
        case 0:
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, HIGH);
        break;
        case 1:
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, HIGH);
        digitalWrite(PIN4, LOW);
        break;
        case 2:
        digitalWrite(PIN1, LOW);
        digitalWrite(PIN2, HIGH);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, LOW);
        break;
        case 3:
        digitalWrite(PIN1, HIGH);
        digitalWrite(PIN2, LOW);
        digitalWrite(PIN3, LOW);
        digitalWrite(PIN4, LOW);
        break;
      }
      if(motor) {
        step1 = step1 + 1;
        cont1 -= 1;
        if(step1 > 3) {
          step1 = 0;
        }
      } else {
        step2 = step2 + 1;
        cont2 -= 1;
        if(step2 > 3) {
          step2 = 0;
        }
      }  
    } else {
      if(motor) {
        ligar1 = false;
      } else {
        ligar2 = false;
      }
    }
  }
}
