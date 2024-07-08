#include "heltec.h" 
#include "images.h"
#include "config.h"

#define BAND    915E6  //definindo a banda de comunicacao do lora
#define TIME    5
#define QUANT   2

//declarando variaveis
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String distance;

int distancia;
int cont = 0;

String id;
char *envia;

void feedDivider(); //declarando a funcao que permite a conexao de diversos loras

AdafruitIO_Feed *ultrassom1 = io.feed("ultrassom1");
AdafruitIO_Feed *ultrassom2 = io.feed("ultrassom2"); //conexao com o adafruit

void dados(); //declarando a funcao que divide os dados recebidos no pacote

void logo(){  //impressao da logo do fabricante (mantido pra saber se houve inicializacao)
  Heltec.display->clear();
  Heltec.display->drawXbm(0,5,logo_width,logo_height,logo_bits);
  Heltec.display->display();
}

//funcao que imprime no display as informacoes recebidas no pacote
void LoRaData(){
  Heltec.display->clear();
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  Heltec.display->setFont(ArialMT_Plain_16);
  Heltec.display->drawString(0 , 15 , "Receb.: "+ packSize + " bytes");
  Heltec.display->drawString(0, 0, rssi);
  Heltec.display->drawStringMaxWidth(0 , 38, 128, distance+" cm, Id: "+id);
  Heltec.display->display();
}

void SendACK(){
  LoRa.beginPacket();
  LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
  LoRa.print(id);
  LoRa.endPacket();
}

//funcao que recebe e interpreta o pacote
void cbk(int packetSize) {
  packet ="";
  packSize = String(packetSize,DEC);
  for (int i = 0; i < packetSize; i++) { packet += (char) LoRa.read(); }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) ;
  dados();
  LoRaData();
  feedDivider();
}

//funcao que separa e organiza as informacoes do pacote
void dados(){
  int i = 0;
  int j = 0;
  distance = "";
  id="";
  while(packet[i] != '\n'){
    if(packet[i] >= '0' && packet[i] <='9')
      distance+=packet[i];
    i++;
  }
  while(packet[i] != '\0'){
      if(packet[i] >= '0' && packet[i] <='9')
        id+=packet[i];
         
    i++;
  }
  distancia = distance.toInt();
  distancia = 100-distancia;
}

//SWITCH CASE PARA O ADAFRUIT
void feedDivider(){
  int intID = id.toInt();
  switch(intID){
    case 1:
      ultrassom1->save(distancia);
      break;
    case 2:
      ultrassom2->save(distancia);
      break;
    default:
      break;
  }
}

void LowEnergy(){
  esp_sleep_enable_timer_wakeup(TIME * 1000000);
}

void TriggerLowEnergy(){
  LoRa.end();
  LoRa.sleep();
  esp_deep_sleep_start();
}

void setup() { 

  io.connect();
  while(io.status() < AIO_CONNECTED);
  // wait for a connection
   //WIFI Kit series V1 not support Vext control
  Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.Heltec.Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
 
  Heltec.display->init();
  Heltec.display->flipScreenVertically();  
  Heltec.display->setFont(ArialMT_Plain_10);
  logo();
  delay(1500);
  Heltec.display->clear();
  
  Heltec.display->drawString(0, 0, "Heltec.LoRa OK!");
  Heltec.display->drawString(0, 10, "Aguardando dados...");
  Heltec.display->display();
  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();
  LowEnergy();
}

void loop() {
  int packetSize = 0;
  io.run();
  LoRa.receive();
  delay(1000);
  while(!packetSize){
    packetSize = LoRa.parsePacket();
  }
  if (packetSize) { 
    cbk(packetSize);
    delay(2100);
    LoRa.beginPacket();
    LoRa.setTxPower(14,RF_PACONFIG_PASELECT_PABOOST);
    LoRa.print(id);
    LoRa.endPacket();
  }
  cont++;
  if (cont==QUANT){
    TriggerLowEnergy();
  }
}