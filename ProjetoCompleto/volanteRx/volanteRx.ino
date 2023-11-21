//Volante
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(3, 4); // CE, CSN
const byte address[6] = "00001";

//Stepper motor definitions --------------------------------------------------------------------------------
//Stepper motor pins
#define Step 8
#define Dir 10
// ---------------------------------------------------------------------------------------------------------

// Time Variables
int intervalo_stepper = 2000;
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;
// **********************************************************************************************************

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
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

void setup() {
  //Stepper motor pinmodes
  pinMode(Step, OUTPUT);
  pinMode(Dir, OUTPUT);

  //RF receiver part
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

//Steppermotor actions
void leftTurn() {
  digitalWrite(Dir, HIGH);
  delayMicroseconds(intervalo_stepper);
  digitalWrite(Step, HIGH);
  delayMicroseconds(intervalo_stepper);
  digitalWrite(Step, LOW);
}

void rightTurn() {
  digitalWrite(Dir, LOW);
  delayMicroseconds(intervalo_stepper);
  digitalWrite(Step, HIGH);
  delayMicroseconds(intervalo_stepper);
  digitalWrite(Step, LOW);
}

void noTurn() {
  digitalWrite(Step, LOW);
  digitalWrite(Dir, LOW);
}

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(dataPackage)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
    if (data.xAxis < 57) {
      leftTurn();
    }
    if (data.xAxis > 197) {
      rightTurn();
    }
    if (data.xAxis > 60 && data.xAxis < 187) {
      noTurn();
    }
    /* Print the data in the Serial Monitor
      Serial.print("xAxis: ");
      Serial.print(data.xAxis);
      Serial.print("; yAxis ");
      Serial.print(data.yAxis);
      Serial.print("; upButton: ");
      Serial.print(data.upButton);
      Serial.print("; downButton: ");
      Serial.println(data.downButton);*/
  }
}
