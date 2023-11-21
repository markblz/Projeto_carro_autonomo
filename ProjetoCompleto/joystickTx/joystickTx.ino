//Arduino Joystick_Tx Pronto
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

int upButton     = 2; // Botao Azul  - A
int downButton   = 4; // Botao Verde - C
int leftButton   = 5; // Boton Azul  - D
int rightButton  = 3; // Boton Verde - B
int startButton  = 6; // Botao       - F
int selectButton = 7; // Botao       - E
int analogButton = 8; //
int xAxis = A0;
int yAxis = A1;
int buttons[] = {upButton, downButton, leftButton,
                 rightButton, startButton, selectButton, analogButton
                };

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
  for (int i; i < 7 ; i++)
  {
    pinMode(buttons[i], INPUT);
    digitalWrite(buttons[i], HIGH);
  }
  Serial.begin(9600);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.stopListening();

  // Set initial default values
  data.xAxis = 127; // Values from 0 to 255. When Joystick is in resting position, the value is in the middle, or 127. We actually map the pot value from 0 to 1023 to 0 to 255 because that's one BYTE value
  data.yAxis = 127;
  data.upButton = 1;
  data.downButton = 1;
  data.leftButton = 1;
  data.rightButton = 1;
  data.startButton = 1;
  data.selectButton = 1;
  data.analogButton = 1;
}

void loop() {
  //Read all analog inputs and map them to one Byte value
  data.xAxis = map(analogRead(A0), 0, 1023, 0, 255); // Convert the analog read value from 0 to 1023 into a BYTE value from 0 to 255
  data.yAxis = map(analogRead(A1), 0, 1023, 0, 255);
  //Read all digital inputs
  data.upButton = digitalRead(upButton);
  data.downButton = digitalRead(downButton);
  data.leftButton = digitalRead(leftButton);
  data.rightButton = digitalRead(rightButton);
  data.startButton = digitalRead(startButton);
  data.selectButton = digitalRead(selectButton);
  data.analogButton = digitalRead(analogButton);

  //Send the whole data from the structure to the receiver
  radio.write(&data, sizeof(dataPackage));

}
