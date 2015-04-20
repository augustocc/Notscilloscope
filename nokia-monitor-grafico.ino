// nokia-monitor-grafico
// um nao-osciloscopio em um Arduino com display Nokia 5110
// (c) Augusto Campos 2015 - BR-Arduino.org
// licenca ao final deste arquivo.
// Usa 2 potenciometros (um para definir o centro do eixo Y
// e o outro para definir a amplitude) e um botao para 
// selecionar automaticamente os melhores parametros de
// exibicao.



#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


// configuracoes de hardware
const byte pinoBotao=12;
const byte pot1Analogico=A2;
const int pot1max=1007; // valor lido no pino do potenciometro quando girado ate o maximo
const byte pot2Analogico=A4;
const int pot2max=1007;

// SPI via software:
// pino 13 - Serial clock out (SCLK;MOSI)
// pino 11 - Serial data out (DN;DIN)
// pino 8 - Data/Command select (D/C)
// pino 10 - LCD chip select (CS;SCE)
// pino 9 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(13, 11, 8, 10, 9);


void setup()   {
  display.begin();
  display.clearDisplay();
  display.setContrast(50);
  display.display();
  pinMode(pinoBotao,INPUT_PULLUP);
}

int X=0;
int Y=0;
int cMin;
int cMax;

int oldX=-1;
int oldY=-1;
int cY;
int cCentro;
int amplitude;
int old_cMin;
int old_cMax;
int linha[40];
int extremoAlto=0;
int extremoBaixo=1024;
int old_extremoAlto=1024;
int old_extremoBaixo=0;
unsigned long botaoUltimoM=0;
int pot1val;
int pot2val;
int old_pot1val;
int old_pot2val;

void plotaPonto(int YY, boolean limpar=true) {
  const byte xMax=80;
  if (limpar) {
    if (X<xMax-10) display.drawLine(X+10,5,X+10,40,WHITE);
    if (X==0) display.fillRect(X,5,X+10,40,WHITE);
    linha[X]=YY;
    if (YY>extremoAlto) extremoAlto=YY;
    if (YY<extremoBaixo) extremoBaixo=YY;
    if (X==40) {
      old_extremoAlto=extremoAlto;
      extremoAlto=0;
      old_extremoBaixo=extremoBaixo;
      extremoBaixo=1024;
    }  
  }  
  if (cMax-cMin!=1023) {
    cY=constrain(YY,cMin,cMax)-cMin;
    cY=map(cY,0,cMax-cMin,0,30);
  }  
  else cY=map(YY,0,1024,0,35);
  if (X>0) display.drawLine(oldX,oldY,X,39-cY,BLACK);
  else display.drawPixel(X,39-cY,BLACK);
  X++;
  if (X==80) X=0;
  oldX=X;
  oldY=39-cY;
}  


void replota() {
  int Xsave=X;
  X=0;
  display.fillRect(0,8,84,40,WHITE);
  for (byte i=0;i<Xsave;i++) {
    plotaPonto(linha[i],false);
  }  
  X=Xsave;
}  

void autoEscala() {
  display.setCursor(75,40);
  display.print("x");
  int _dAmp;
  if (X<10) {
    _dAmp=(old_extremoAlto-old_extremoBaixo)/20;  
    if (_dAmp<30) _dAmp=30;  
    cMin=old_extremoBaixo-_dAmp;
    cMax=old_extremoAlto+_dAmp;
  } else {
    _dAmp=(extremoAlto-extremoBaixo)/20;
    if (_dAmp<30) _dAmp=30;  
    cMin=extremoBaixo-_dAmp;
    cMax=extremoAlto+_dAmp;  
  }  
  cCentro=cMin+((cMax-cMin)/2);
  old_cMin=cMin;
  old_cMax=cMax;
  replota();
}  


void atualizaEscala() {
  cCentro=1024-int(pot1val*1.0/pot1max*1024);
  int folga=cCentro;
  if ((1024-cCentro)<folga) folga=1024-cCentro;
  amplitude=1024-int(pot2val*1.0/pot2max*1024);
  if (amplitude > (2*folga)) amplitude=2*folga;
  cMin=cCentro-amplitude/2;
  cMax=cCentro+amplitude/2;
  if (abs((old_cMin-cMin)>2) | (abs(old_cMax-cMax)>2)) {
    if (X>0) replota();
  }  
  old_cMin=cMin;
  old_cMax=cMax;
}  

void atualizaPlacares() {  // informa na tela o numero de vidas e os pontos
  display.fillRect(0,40,84,8,WHITE);
  display.setCursor(5,40);
  display.print(cY);
  display.setCursor(40,40);
  display.print(Y);
  display.fillRect(0,0,84,8,WHITE);
  display.setCursor(0,0);
  display.print(cMin);
  display.setCursor(32,0);
  display.print(cCentro);
  display.setCursor(58,0);
  display.print(cMax);
}  

boolean botaoMudou=true;
void loop() {
  delay(100);
  Y=analogRead(A0);
  pot1val=analogRead(pot1Analogico);
  pot2val=analogRead(pot2Analogico);
  plotaPonto(Y);
  if ((digitalRead(pinoBotao)==LOW) & ((abs(millis()-botaoUltimoM))>200)) {
    if (botaoMudou==true) {
      autoEscala();
      botaoUltimoM=millis();
    }  
    botaoMudou=false;
  }  
  if (!botaoMudou & ((digitalRead(pinoBotao)==HIGH))) botaoMudou=true;
  if ( (abs(old_pot1val-pot1val)>10) | (abs(old_pot2val-pot2val)>10) ) {
    atualizaEscala();   
    old_pot1val=pot1val;
    old_pot2val=pot2val;
  }  
  atualizaPlacares();
  display.display(); 

}

/*
© 2015 Augusto Campos http://augustocampos.net/ (13.04.2015)
Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at 
http://www.apache.org/licenses/LICENSE-2.0 

Unless required by applicable law or agreed to in writing, software 
distributed under the License is distributed on an "AS IS" BASIS, 
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and 
limitations under the License. 
*/
