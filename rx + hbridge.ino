// SimpleRx - the slave or the receiver

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>

#define CE_PIN   7
#define CSN_PIN 8

const byte address[6] = "00001";

RF24 radio(CE_PIN, CSN_PIN);

unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

char dataReceived[10]; // this must match dataToSend in the TX
bool newData = false;

// H-bridge pins
const int A1A=5; //A1=right side motors
const int A1B=6; 
const int B1A=9; //A2=left side motors
const int B1B=10;

// Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j1Button; 

};

Data_Package data; //Create a variable with the above structure

int steering, throttle, stop;
int motorSpeedA = 0; // virtual mapping of the left side
int motorSpeedB = 0; // virtual mapping of the left side



void setup() {
  // put your setup code here, to run once:
  pinMode(A1A, OUTPUT);
  pinMode(A1B, OUTPUT);
  pinMode(B1A, OUTPUT);
  pinMode(B1B, OUTPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening(); //  Set the module as receiver
  resetData();

}

void loop() {

  // Check whether there is data to be received
  if (radio.available()) {
    radio.read(&data, sizeof(Data_Package)); // Read the whole data and store it into the 'data' structure
    lastReceiveTime = millis(); // At this moment we have received the data
    
  } 
  // Check whether we keep receving data, or we have a connection between the two modules
  currentTime = millis();
  if ( currentTime - lastReceiveTime > 1000 ) { // If current time is more then 1 second since we have recived the last data, that means we have lost connection
    resetData(); // If connection is lost, reset the data. It prevents unwanted behavior, for example if a drone has a throttle up and we lose connection, it can keep flying unless we reset the values
  }
  // Print the data in the Serial Monitor

  throttle = data.j1PotY;
  steering = data.j1PotX;
  stop = data.j1Button;
// Throttle used for forward and backward control
  // Joystick values: 0 to 255; down = 0; middle = 127; up = 255
  if (throttle > 140) {
    // Set Motors A backward
    digitalWrite(A1A, HIGH);
    digitalWrite(A1B, LOW);
    // Set Motors B backward
    digitalWrite(B1A, HIGH);
    digitalWrite(B1B, LOW);
    // Convert the declining throttle readings for going forward from 140 to 255 into 0 to 255 value for the PWM signal for increasing the motor speed
    motorSpeedA = map(throttle, 140, 255, 70, 255); //raise the 0 to 70 to speed up the motors?
    motorSpeedB = map(throttle, 140, 255, 70, 255);
  
  }
  else if (throttle < 110) {
    // Set Motor A forward
    digitalWrite(A1A, LOW);
    digitalWrite(A1B, HIGH);
    // Set Motor B forward
    digitalWrite(B1A, LOW);
    digitalWrite(B1B, HIGH);
    // Convert the increasing throttle readings for going backward from 110 to 0 into 0 to 255 value for the PWM signal for increasing the motor speed
    motorSpeedA = map(throttle, 110, 0, 70, 255);
    motorSpeedB = map(throttle, 110, 0, 70, 255);

  }
  // If joystick stays in middle the motors are not moving. 
  else {
    motorSpeedA = 0; // These values need tuning.
    motorSpeedB = 0;

  }
  // Steering used for left and right control
  if (steering < 110) {
    // Convert the declining steering readings from 140 to 255 into increasing 0 to 255 value
    int xMapped = map(steering, 110, 0, 0, 255);
    // Move to left - decrease left motors (A) speed, increase right motors (B) speed
    motorSpeedA = motorSpeedA - xMapped;
    motorSpeedB = motorSpeedB + xMapped;
    
    // Confine the range from 0 to 255
    if (motorSpeedA< 0) {
      motorSpeedA = 0;
    }
    if (motorSpeedB > 255) {
      motorSpeedB = 255;
    }

  }
  if (steering > 140) {
    // Convert the increasing steering readings from 110 to 0 into 0 to 255 value
    int xMapped = map(steering, 140, 255, 0, 255);
    // Move right - decrease right motors speed (A), increase left motors (B) speed
    motorSpeedA = motorSpeedA + xMapped;

    motorSpeedB = motorSpeedB - xMapped; 

    // Confine the range from 0 to 255
    if (motorSpeedA > 255) {
      motorSpeedA = 255;
    }
    if (motorSpeedB < 0) {
      motorSpeedB = 0;
    }

  }
  //Prevent buzzing at low speeds (Adjust according to your motors. My motors couldn't start moving if PWM value was below value of 70)
  if (motorSpeedA < 70) {
    motorSpeedA = 0;
  }
  if (motorSpeedB < 70) {
    motorSpeedB = 0;
  }


  analogWrite(A1A, motorSpeedA); // Send PWM signal to motor A
  analogWrite(B1A, motorSpeedB); // Send PWM signal to motor A

  // Serial.print("j1PotX: ");
  // Serial.println(data.j1PotX);
  // Serial.print("; j1PotY: ");
  // Serial.println(data.j1PotY);
}


void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
  data.j1PotX = 127;
  data.j1PotY = 127;
  data.j1Button = 1;
}
