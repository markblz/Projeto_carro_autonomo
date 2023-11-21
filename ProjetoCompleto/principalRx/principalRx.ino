//Rx Principal
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>
RF24 radio(49, 48); // CE, CSN
const byte address[6] = "00001";

//Transmission Servos Definitions --------------------------------------------------------------------------
//Servos L and R Objects
Servo servoL;           //Pin will be set in Setup
Servo servoR;           //Pin will be set in Setup
//Servos L and R Pins
#define servoLpin 9
#define servoRpin 10
//Transmission Variables
//Definir angulo de engate dos servos
int neutro  = 90;       //Angulo neutro dos servos
int frenteR = 40;       //Angulo de engate do servo R pra frente
int trasR   = 140;      //Angulo de engate do servo R pra tras
int frenteL = 120;      //Angulo de engate do servo L pra frente
int trasL   = 50;       //Angulo de engate do servo L pra tras
int recuo   = 20;       //Recuo para voltar marcha ao neutro
// ---------------------------------------------------------------------------------------------------------

//Gaspedal Servo Definitions -------------------------------------------------------------------------------
//Gaspedal servo Object
Servo servoGaspedal;
//Gaspedal servo Pins
#define servogasPin 7
//Gaspedal variables

//Clutch Actuator Definitions ------------------------------------------------------------------------------
//Clutch pins
#define relayclutchOne 31 // Set pin for first clutch Relay (Controls first pole of linear actuator)
#define relayclutchTwo 32 // Set pin for second clutch Relay (Controls second pole of linear actuator)
//Clutch potentiometer pins ----------------------------------
// - - - pin 1 = Vcc       - - -
// - - - pin 2 = Signal A3 - - -
// - - - pin 3 = Ground    - - -
int potmeterclutchPin = A3;              // Slide potentiometer on pin A3
int potmeterclutchVal = 0;               // Slide potentiometer value
//Clutch Variables
int clutchMin = 585;
int clutchMax = 990;
int varminmapClutch  = 2 * clutchMin - clutchMax;
int varmaxmapClutch  = clutchMax;
int toleranciaClutch = 10;
// ----------------------------------------------------------------------------------------------------------

//Brake Actuator Definitions --------------------------------------------------------------------------------
//Brake pins
#define relaybrakeOne 33 // Set pin for first Brake Relay (Controls first pole of linear actuator)
#define relaybrakeTwo 34 // Set pin for second Brake Relay (Controls second pole of linear actuator)
// potenciometro do freio pins
// pin 1 = Vcc
// pin 2 = Signal A2
// pin 3 = Ground
int potmeterbrakePin = A2;              // Slide potentiometer on pin A2
int potmeterbrakeVal = 0;               // Slide potentiometer value
//Brake Variables
int brakeMin = 125;
int brakeMax = 660;
int varminmapBrake  = 2 * brakeMin - brakeMax;
int varmaxmapBrake  = brakeMax;
int toleranciaBrake = 50;
// ----------------------------------------------------------------------------------------------------------

//Time and other Variables --------------------------------------------------------------------------------------------
//Definir intervaloMarcha (milissegundos) entre linhas do codigo da marcha
int intervaloMarcha = 2000;
int intervaloAcelerador = 2000;
int contadorMarcha = 0;
int lastReceiveTime;
int currentTime;
//-----------------------------------------------------------------------------------------------------------

// Max size of this struct is 32 bytes - NRF24L01 buffer limit-----------------------------------------------
struct dataPackage {
  byte upButton;
  byte downButton;
  byte leftButton;
  byte rightButton;
  byte startButton;
  byte selectButton;
  byte analogButton;
  byte xAxis;
  byte yAxis;
};
dataPackage data; //Create a variable with the above structure
//-----------------------------------------------------------------------------------------------------------

void setup() {

  //relayclutchOne and relayclutchTwo control the clutch actuator
  pinMode(relayclutchOne, OUTPUT);
  pinMode(relayclutchTwo, OUTPUT);

  //relaybrakeOne and relaybrakeTwo control the brake actuator
  pinMode(relaybrakeOne, OUTPUT);
  pinMode(relaybrakeTwo, OUTPUT);

  // Attach Transmission servos
  servoL.attach(servoLpin); // Pino do servo L
  servoR.attach(servoRpin); // Pino do servo R

  // Attach Gas Pedal servo
  servoGaspedal.attach(servogasPin);

  //RF receiver part
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
}


// ------------ Entrada e saida das marchas --------------------------------------------------------
void servomarchaentra01() {
  potmeterclutchVal = analogRead(potmeterclutchPin);
  if (potmeterclutchVal > clutchMax - 15) {
    delay(intervaloMarcha);
    servoL.write(frenteL);
    delay(intervaloMarcha);
    servoR.write(trasR);
    delay(intervaloMarcha);
  }
}

void servomarchasai01() {
  potmeterclutchVal = analogRead(potmeterclutchPin);
  if (potmeterclutchVal > clutchMax - 15) {
    delay(intervaloMarcha);
    servoR.write(neutro - recuo);
    delay(intervaloMarcha);
    servoR.write(neutro);
    delay(intervaloMarcha);
    servoL.write(neutro - recuo);
    delay(intervaloMarcha);
    servoL.write(neutro);
    delay(intervaloMarcha);
  }
}

void servomarchaentrare() {
  potmeterclutchVal = analogRead(potmeterclutchPin);
  if (potmeterclutchVal > clutchMax - 15) {
    delay(intervaloMarcha);
    servoL.write(trasL);
    delay(intervaloMarcha);
    servoR.write(frenteR);
    delay(intervaloMarcha);
  }
}

void servomarchasaire() {
  potmeterclutchVal = analogRead(potmeterclutchPin);
  if (potmeterclutchVal > clutchMax - 15) {
    delay(intervaloMarcha);
    servoR.write(neutro + recuo);
    delay(intervaloMarcha);
    servoR.write(neutro);
    delay(intervaloMarcha);
    servoL.write(neutro + recuo);
    delay(intervaloMarcha);
    servoL.write(neutro);
    delay(intervaloMarcha);
  }
}
void entra01() { //Engata primeira marcha
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal < clutchMax - 10) {
    potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
    extendClutch();
  }
  holdClutch();
  servomarchaentra01();
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal > clutchMin + 10) {
    potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
    retractClutch();
  }
  holdClutch();
}
void sai01() { //Volta pro neutro
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal < clutchMax - 10) {
    potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
    extendClutch();
  }
  holdClutch();
  servomarchasai01();

  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal > clutchMin + 10) {
    potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
    retractClutch();
  }
  holdClutch();
}
void entraRe() {
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal < clutchMax - 5) {
    potmeterclutchVal = analogRead(potmeterclutchPin);  // Read the analog value of the slide potentiometer
    extendClutch();
  }
  holdClutch();
  delay(intervaloMarcha);
  servoL.write(trasL);
  delay(intervaloMarcha);
  servoR.write(frenteR);
  delay(intervaloMarcha);
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal > clutchMin + 5) {
    potmeterclutchVal = analogRead(potmeterclutchPin);  // Read the analog value of the slide potentiometer
    retractClutch();
  }
  holdClutch();
}
void saiRe() {
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal < clutchMax - 5) {
    potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
    extendClutch();
  }
  holdClutch();
  servomarchasaire();
  potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
  while (potmeterclutchVal > clutchMin + 5) {
    potmeterclutchVal = analogRead(potmeterclutchPin);    // Read the analog value of the slide potentiometer
    retractClutch();
  }
  holdClutch();
}

void checkGear() {
  if (data.startButton == 0 && servoL.read() == neutro && servoR.read() == neutro) {
    servomarchaentra01();
    //ERRO APARENTE É NÃO MEXER COM A EMBREAGEM NO MAXIMO
  }
  if (data.startButton == 0 && servoL.read() == trasL && servoR.read() == frenteR) {
    servomarchasaire();
  }
  if (data.selectButton == 0 && servoL.read() == neutro && servoR.read() == neutro) {
    servomarchaentrare();
  }
  if (data.selectButton == 0 && servoL.read() == frenteL && servoR.read() == trasR) {
    servomarchasai01();
  }
}

// --------------------------------------------------------------------------------------------------

//clutch actions -----------------------------------------------------------------------------------
void extendClutch() {
  digitalWrite(relayclutchOne, LOW);
  digitalWrite(relayclutchTwo, HIGH);
}
void retractClutch() {
  digitalWrite(relayclutchOne, HIGH);
  digitalWrite(relayclutchTwo, LOW);
}
void holdClutch() {
  digitalWrite(relayclutchOne, HIGH);
  digitalWrite(relayclutchTwo, HIGH);
}

void moveClutch() {
  potmeterclutchVal = analogRead(potmeterclutchPin);
  if (data.yAxis > 207 && potmeterclutchVal < clutchMax - toleranciaClutch  ) {
    extendClutch();
  }
  else if (data.yAxis < 57 && potmeterclutchVal > clutchMin + toleranciaClutch) {
    retractClutch();
  }
  else {
    holdClutch();
  }
}
// --------------------------------------------------------------------------------------------------

//Brake actions ------------------------------------------------------------------------------------
void extendBrake() {
  digitalWrite(relaybrakeOne, LOW);
  digitalWrite(relaybrakeTwo, HIGH);
}
void retractBrake() {
  digitalWrite(relaybrakeOne, HIGH);
  digitalWrite(relaybrakeTwo, LOW);
}
void holdBrake() {
  digitalWrite(relaybrakeOne, LOW);
  digitalWrite(relaybrakeTwo, LOW);
}

void brakeModes() { //modos de freio

  if (data.leftButton == 0 && data.upButton == 0) { //freio padrão(recua freio quando nada OS DOIS ESTAO APERTADOS)
    while (potmeterbrakeVal > brakeMin + 10) {
      retractBrake();
    }
    holdBrake();
  }
  if (data.leftButton == 0) { //freia de leve
    potmeterbrakeVal = analogRead(potmeterbrakePin);    // Read the analog value of the slide potentiometer
    while (potmeterbrakeVal < 350) {
      potmeterbrakeVal = analogRead(potmeterbrakePin);    // Read the analog value of the slide potentiometer
      extendBrake();
    }
    holdBrake();
  }
  if (data.upButton == 0) { //freia pesado
    potmeterbrakeVal = analogRead(potmeterbrakePin);    // Read the analog value of the slide potentiometer
    while (potmeterbrakeVal < 550) {
      potmeterbrakeVal = analogRead(potmeterbrakePin);    // Read the analog value of the slide potentiometer
      extendBrake();
    }
    holdBrake();
  }
}
/*
  void freioPesado() { //freia forte
  if (data.upButton == 0) {
    potmeterbrakeVal = analogRead(potmeterbrakePin);
    while (potmeterbrakeVal < 550) {
      potmeterbrakeVal = analogRead(potmeterbrakePin);
      extendBrake();
    }
    holdBrake();
  }
  }*/
// --------------------------------------------------------------------------------------------------

//Gaspedal actions ----------------------------------------------------------------------------------
void run() {
  delay(intervaloAcelerador);
  servoGaspedal.write(40);
  delay(intervaloAcelerador);
  servoGaspedal.write(30);
  delay(intervaloAcelerador);
}

void gaspedalModes() {
  if (data.rightButton==0) {//acelera muito e mantem
    servoGaspedal.write(40);
    delay(intervaloAcelerador);
    //para testar inserir retorno para posição inicial aqui
  }
  if (data.downButton==0) {//acelera pouco e mantem
    servoGaspedal.write(80);
    delay(intervaloAcelerador);
    //para testar inserir retorno para posição inicial aqui
  }
}
//adicionar outra função que, enquanto o joystick estiver acionado, ele acelera

// --------------------------------------------------------------------------------------------------

//Reset Data ----------------------------------------------------------------------------------------
void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.yAxis = 127;
  data.xAxis = 127;
  data.upButton = 1;
  data.downButton = 1;
  data.analogButton = 1;
  data.leftButton = 1;
  data.rightButton = 1;
  data.startButton = 1;
  data.selectButton = 1;
}
//---------------------------------------------------------------------------------------------------

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(dataPackage)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
    //Funções controladoras do carro
    checkGear();
    moveClutch();
    brakeModes();
    gaspedalModes();
    //Serial.print("\n X = "), Serial.print(data.xAxis), Serial.print(" \n Y = "), Serial.print(data.yAxis);
    //Serial.print("  ");
    //Serial.println(potmeterclutchVal);
  }
}
