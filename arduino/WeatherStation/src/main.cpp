#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/*
  Array to store the Auduino pins used for controlling which digits of the 4 digit 7 segment display is ON at a time, using BJTs.

  The unit digit is connect to pin 4, tens digit is conneted to pin 5 and so on...
 */
const int digits[] = {4, 5, 6, 7};

//pin declaration for the Shift Register("SN74HC595")
const int dataPin = 8; // also called SER or Ds. It's pin 14 on SN74HC595
const int clockPin = 9; // also called SRCLK(Shift Register Clock) or SHcp (SHift register clock pin). It's pin 11 on SN74HC595
const int latchPin = 10; //also called RCLK or STcp(STorage clock pin). It's pin 12 on SN74HC595
// const int masterReset = 11; //also called SRCLR(Shift Register clear or MR. It's pin 10 on SN74HC595

//pin declaration for DHT11 temperatue and humidity sensor
#define DHTPIN  2
#define DHTTYPE           DHT11

DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t dataDisplayDuration;

/*
  Data structure to store the mappign of english charaters(e.g. '0') and their binary repesentation(e.g. '00000011') that represents
  which LEDs should light up on a 7 segment display to output the charater.
*/
struct SevenSegmentCharacter {
        char id;
        byte binaryRepresentation;
} sevenSegmentCharacters[15];

/* Numbers to be displayed on the 7 segment display in binary format where each digit represents the state of a LED segment on the display
   i.e. 1= "off" and zero = "on" (It's flipped as I am using a common annode 7 segment display)
   e.g. to display number 1 on the display LEDs a=1 i.e off, b=0 i.e. on, c=0, d=1, e=1, f=1, g=1 and dp=1 Combined it forms binary number 10011111
 */
const byte numbers[] = {
        0b00000011, //0
        0b10011111, //1
        0b00100101, //2
        0b00001101, //3
        0b10011001, //4
        0b01001001, //5
        0b01000001, //6
        0b00011111, //7
        0b00000001, //8
        0b00001001  //9
};

const byte degreeSymbol = 0b00111001;
const byte charC = 0b01100011;
const byte charP = 0b00110001;
const byte charE = 0b01100001;
const byte charR = 0b11110101;

int numberOfDisplayableCharacters;

void initializeDispalyCharacters(){
        for(int i=0; i<=9; i++) {
                sevenSegmentCharacters[i].id = i + '0';
                sevenSegmentCharacters[i].binaryRepresentation= numbers[i];
        }

        sevenSegmentCharacters[10].id = 'C';
        sevenSegmentCharacters[10].binaryRepresentation = charC;

        sevenSegmentCharacters[11].id = 'P';
        sevenSegmentCharacters[11].binaryRepresentation = charP;

        sevenSegmentCharacters[12].id = 'E';
        sevenSegmentCharacters[12].binaryRepresentation = charE;

        //degrees symbol
        sevenSegmentCharacters[13].id = 'd';
        sevenSegmentCharacters[13].binaryRepresentation = degreeSymbol;

        sevenSegmentCharacters[14].id = 'r';
        sevenSegmentCharacters[14].binaryRepresentation = charR;

        numberOfDisplayableCharacters = sizeof(sevenSegmentCharacters) / sizeof(sevenSegmentCharacters[0]);
}

void setup() {
//display
        for(int i=0; i<= 3; i++) {
                pinMode(digits[i], OUTPUT);
                digitalWrite(digits[i], LOW);
        }
        initializeDispalyCharacters();

//Shift Register
        pinMode(dataPin, OUTPUT);
        pinMode(clockPin, OUTPUT);
        pinMode(latchPin, OUTPUT);

//DHT sensor
        dht.begin();
        sensor_t sensor;
        dht.temperature().getSensor(&sensor);
        dht.humidity().getSensor(&sensor);

        dataDisplayDuration = (sensor.min_delay / 1000) + 1000; // adding 1 second more to the make it easier for users to read
}

byte getBinaryRepresentationForCharacter(char characterToDisplay){

        for(int i=0; i < numberOfDisplayableCharacters; i++) {
                if(sevenSegmentCharacters[i].id == characterToDisplay) {
                        return sevenSegmentCharacters[i].binaryRepresentation;
                }
        }
}

void printDataOnDisplay(char* charData){

        uint32_t displayUntilMillis = millis() + dataDisplayDuration;
        for(uint32_t j = millis(); j < displayUntilMillis; j=millis()) {
                for(int i = 0; i<= 3; i++) {
                        digitalWrite(latchPin, LOW);
                        shiftOut(dataPin, clockPin, LSBFIRST, getBinaryRepresentationForCharacter(charData[i]));
                        //We need to first shift the data and then open up the display digit for output to avoid ghosting
                        digitalWrite(digits[i], HIGH);
                        digitalWrite(latchPin, HIGH);
                        delay(5);
                        digitalWrite(digits[i], LOW);
                }
        }
}

void displayErrorMessage(){
        char error[4] = {'E','r','r','0'};
        printDataOnDisplay(error);
}

void displayTemprature(){
        sensors_event_t event;
        char temperatureReading[4];
        // Get temperature data
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature)) {
                displayErrorMessage();
        }
        else {
                itoa(event.temperature, temperatureReading, 10);
                strcat(temperatureReading, "dC");
                printDataOnDisplay(temperatureReading);
        }
}

void displayHumidity(){
        sensors_event_t event;
        char humidityReading[4];
        // Get humidity data
        dht.humidity().getEvent(&event);
        if(isnan(event.relative_humidity)) {
                displayErrorMessage();
        }
        else {
                itoa(event.relative_humidity, humidityReading, 10);
                strcat(humidityReading, "PC");
                printDataOnDisplay(humidityReading);
        }
}

void loop() {
        displayTemprature();
        displayHumidity();
}
