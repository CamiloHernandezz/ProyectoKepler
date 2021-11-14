/* 
 *               UNIVERSIDAD SERGIO ARBOLEDA
 *               ARQUITECTURA DE COMPUTADORES
 *                    PROYECTO KEPLER
 *           
 *                   Juan Esteban Arias
 *                  Juan Camilo Hernández
 *                 Christian David Jimenez
 *         ...................................      
 *    El siguiente codigo, representa el funcionamineto
 * de un cohete al recopilar, guardar y mostrar informacion 
 * como temperatura, presion, altura y velocidad alcanzada
 *                    durante el vuelo.
 */

 
#include <SPI.h>                // incluye libreria interfaz SPI
#include <SD.h>                 // incluye libreria para tarjetas SD
#include <SoftwareSerial.h>     // libreria que permite establecer pines digitales
#include <Wire.h>
#include <Adafruit_BMP085.h>    //Libreria para usar BMP180

File archivo;     // objeto archivo del tipo File

SoftwareSerial miBT(4, 5);  // pin 11 como RX, pin 10 como TX

Adafruit_BMP085 bmp;  //Instanca del sensor Barometrico


int RELEM = 2; //ini2
int RELEP = 3; //ini1

bool encendido = false;
bool motoresOn = false;
bool paracaidasOn = false;
bool caida = false;

int tiempo;
int tiempoBase=0;
int tiempoCaida=0;
int tiempoAterrizaje=0;

char datoBT = 4;

float correccionAl = 0;
float maximaAltura;
float altura;
float alturaReal;
float temperatura;
float presion;
float velocidad;

int datoMenor;
int segundo;
int aux = 300;

void setup() {
  Serial.begin(9600);
     
  miBT.begin(38400);        // comunicacion serie entre Arduino y el modulo a 38400 bps
  
  pinMode(9, OUTPUT);

  inicializarRele();
  
  inicializarMicroSD();
    
  inicializarBarometro();

  digitalWrite(9, HIGH);
}

void inicializarBarometro(){
  digitalWrite(9, LOW);
  if (!bmp.begin()) {
    Serial.println("HHAHA");
  while (1) {}
  }else{
    alturaReal = bmp.readAltitude(0);
    while(alturaReal<=2){
      if(alturaReal>-100){
        aux = 5;
      }
      correccionAl+=aux;
      alturaReal = bmp.readAltitude(correccionAl); 
    } 
  }
}

void inicializarMicroSD(){
  if (!SD.begin(10)) {     // inicializacion de tarjeta SD
    Serial.println("HOHOHO");
    while (1) {}                   
  }
  archivo = SD.open("Datos.txt", FILE_WRITE);  // apertura para lectura/escritura de archivo prueba.txt
  if (archivo) {
    archivo.println("SEGUNDO,ALTURA,ALTURACORREGIDA,VELOCIDAD,PRESION,TEMPERATURA");  // escritura de una linea de texto en archivo
  }
}


 void inicializarRele(){
  pinMode(RELEM, OUTPUT);
  pinMode(RELEP, OUTPUT);
  digitalWrite(RELEM, HIGH);
  digitalWrite(RELEP, HIGH);
}

void loop() {
  if(!encendido){
    if (miBT.available()){
      datoBT = miBT.read();    // almacena en DATO el caracter recibido desde modulo
      if(datoBT=='1'){
        encendido = true;
        tiempoBase = millis()/1000; //5
        digitalWrite(9, LOW);  
      }
    }
  }else{
    if(!motoresOn){                 //Si los propulsores estan apagados
      prenderMotores();
    }else{
      if(!caida){
        estamosCayendo();
        tiempoCaida=millis()/1000;
      }else{
        if(!paracaidasOn){
          abrirParacaidas();  
        }else{
          if(aterrizajeExitoso()>10){
            archivo.close();
            while(1){}
          }
        }
      }
    }
    obtenerInformacion();
  }
}

void estamosCayendo(){
  if(alturaReal > maximaAltura){
    maximaAltura = alturaReal;
    datoMenor = 0;
  }else{
    if(maximaAltura-alturaReal>2.50){
      caida = true;
    }
  }
}

void guardarInformacion(){
  if(archivo){
    archivo.print(segundo);
    archivo.print(",");
    archivo.print(altura);
    archivo.print(",");
    archivo.print(alturaReal); 
    archivo.print(",");
    archivo.print(velocidad);  
    archivo.print(",");
    archivo.print(presion);
    archivo.print(",");
    archivo.println(temperatura); 
   }
}

void obtenerInformacion(){
  tiempo=millis()/1000;
  segundo = tiempo-tiempoBase;
  temperatura = bmp.readTemperature();
  presion = bmp.readPressure();
  altura = bmp.readAltitude();
  alturaReal = bmp.readAltitude(correccionAl);
  velocidad = alturaReal/segundo;
  guardarInformacion();
}

bool prenderMotores(){ //Led rojo encendido: RELE Apagado
  digitalWrite(RELEM, LOW); 
  if((millis()/1000)-tiempoBase>=2){
    digitalWrite(RELEM, HIGH); 
    motoresOn=true;
  }
}

void abrirParacaidas(){
  digitalWrite(RELEP, LOW);
  if((millis()/1000)-tiempoCaida>=1.8){
    digitalWrite(RELEP, HIGH);
    paracaidasOn = true; 
  }
}

int aterrizajeExitoso(){
  if(alturaReal>=-10 && alturaReal<=10){
    tiempoAterrizaje++;
  }
  return tiempoAterrizaje;
}
