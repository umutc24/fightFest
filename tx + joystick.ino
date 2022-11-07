// Bolton Dynamics

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>


#define CE_PIN   9
#define CSN_PIN 10

const byte address[6] = "00001"; 


RF24 radio(CE_PIN, CSN_PIN); // Create a Radio




// Define the digital inputs from joystick
#define jB1 2  // Joystick button 1



//Max size of this struct is 32 bytes - NRF24L01 buffer limit
struct Data_Package {
  byte j1PotX;
  byte j1PotY;
  byte j1Button;
};

Data_Package data; //Create a variable with the above structure


void setup() {

    Serial.begin(9600);

    // Activate the Arduino internal pull-up resistors
    pinMode(jB1, INPUT_PULLUP);

    radio.begin();
    radio.openWritingPipe(address);
    radio.setAutoAck(false);
    radio.setDataRate( RF24_250KBPS );
    radio.setPALevel(RF24_PA_LOW);
    
    

    data.j1PotX = 127; // Values from 0 to 255. When Joystick is in resting position, the value is in the middle, or 127. We actually map the pot value from 0 to 1023 to 0 to 255 because that's one BYTE value
    data.j1PotY = 127;
    data.j1Button = 1;
}

//====================

void loop() {
  // Read all analog inputs and map them to one Byte value
  data.j1PotX = map(analogRead(A0), 0, 1023, 0, 255); // Convert the analog read value from 0 to 1023 into a BYTE value from 0 to 255
  data.j1PotY = map(analogRead(A1), 0, 1023, 0, 255); 
  // Read all digital inputs
  data.j1Button = digitalRead(jB1);
  // Serial.print("j1PotX: ");
  // Serial.print(data.j1PotX);
  // Serial.print("; j1PotY: ");
  // Serial.print(data.j1PotY);
  // Serial.print(" button: ");
  // Serial.println(data.j1Button);
  // delay(2000);
  // currentMillis = millis();
  // if (currentMillis - prevMillis >= txIntervalMillis) {
  //     send();
  //       prevMillis = millis();
  //   }


  // Send the whole data from the structure to the receiver
  radio.write(&data, sizeof(Data_Package));
  
}

//====================

// void send() {

//     bool rslt;
//     rslt = radio.write( &dataToSend, sizeof(dataToSend) );
//         // Always use sizeof() as it gives the size as the number of bytes.
//         // For example if dataToSend was an int sizeof() would correctly return 2

//     Serial.print("Data Sent ");
//     Serial.print(dataToSend);
//     if (rslt) {
//         Serial.println("  Acknowledge received");
//         updateMessage();
//     }
//     else {
//         Serial.println("  Tx failed");
//     }
// }

//================

// void updateMessage() {
//         // so you can see that new data is being sent
//     txNum += 1;
//     if (txNum > '9') {
//         txNum = '0';
//     }
//     dataToSend[8] = txNum;
// }