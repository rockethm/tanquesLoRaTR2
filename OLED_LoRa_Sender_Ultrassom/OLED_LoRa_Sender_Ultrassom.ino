#include <stdlib.h>
#include <NewPing.h>
#include "heltec.h"
#include "images.h"

#define BAND    915E6  //definindo banda de comunicacao lora
#define TRIGGER_PIN 12
#define ECHO_PIN 13
#define MAX_DISTANCE 200

//Declarando o ultrassom
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //trigger, echo
int dist;

//Receber ACK
String packSizeACK = "--";
String packetACK ;
char received = 0;

//Declarando ID
String id = "2";

//parametros rssi
String rssi = "RSSI --";
String packSize = "--";
String packet ;

void logo() { //impressao da logo do fabricante (mantido pra saber se houve inicializacao)
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}


//funcao de lowenergy do lora
void LowEnergy(){
  esp_sleep_enable_timer_wakeup(12000000);
}

void TriggerLowEnergy(){
  LoRa.end();
  LoRa.sleep();
  esp_deep_sleep_start();
}

void cbk(int packetSize) {
  packetACK ="";
  packSizeACK = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packetACK += (char) LoRa.read(); }
  //rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
}

char ReceiveACK(){
  int packetSize = LoRa.parsePacket();
  if (packetSize){
   cbk(packetSize);
   if (packetACK == id)
    received = 1;
  }
  else{received = 0;}
  delay(10);
  return received;
}

void setup()
{
  Serial.begin(115200);
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
 
  //inicializando o display da esp32
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  
  //se inicializado corretamente printa isso na tela
  Heltec.display->drawString(0, 0, "Heltec.LoRa OK!");
  Heltec.display->display();
  delay(1000);
  LowEnergy();
}

void loop()
{
  //lendo distancia
  dist = sonar.ping_cm();
  //convertendo para texto
  char diststring[20];
  dtostrf(dist, 3, 0, diststring);
  String distancia(diststring);

  //limpando e configurando o texto
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_10);
  
  //imprimindo o texto
  Heltec.display->drawString(0, 0, "Enviando distancia:");
  Heltec.display->setFont(ArialMT_Plain_24);
  Heltec.display->drawString(0, 26, distancia+" cm ");
  Heltec.display->display();

  // envia o pacote
  delay(100);
  LoRa.beginPacket();
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print("Dist: ");
  LoRa.print(distancia);
  LoRa.print('\n');
  LoRa.print(id);
  LoRa.endPacket();


  //delay de 2 sec 
  digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                     // wait for a second

  int cont = 0;
  LoRa.receive();
  while (!ReceiveACK() && cont < 100){
    cont++;
    Serial.println(cont);
  }
  //if (dist<75)
  //  TriggerLowEnergy();
  if (cont != 100)
    TriggerLowEnergy();
}
