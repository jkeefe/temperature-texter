//// Temperature-Humidity Sensor Code Comments
//    FILE: dht22_test.ino
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.03
// PURPOSE: DHT library test sketch for DHT22 && Arduino
//     URL: https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib
// HISTORY:
// 0.1.03 extended stats for all errors
// 0.1.02 added counters for error-regression testing.
// 0.1.01
// 0.1.00 initial version
//
// Released to the public domain
//
// Edited by John Keefe for DHT22 only
// Get the full library, including the dht.h file at the URL above
// More info at http://johnkeefe.net/make-every-week-temperature-texter

//// Fona Code Comments
/*************************************************** 
  This is an example for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA 
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963
  ----> http://www.adafruit.com/products/2468
  ----> http://www.adafruit.com/products/2542

  These cellular modules use TTL Serial to communicate, 2 pins are 
  required to interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// turnOnFONA and turnOffFONA functions by Kina Smith
// http://www.kinasmith.com/spring2015/towersofpower/fona-code/

// low-power sleep code from Jean-Claude Wippler
// http://jeelabs.net/pub/docs/jeelib/classSleepy.html and
// http://jeelabs.net/projects/jeelib/wiki
// Which is well-described here: http://jeelabs.org/2011/12/13/developing-a-low-power-sketch/
//
// Note that I'm using loseSomeTime() in place of delay(), and that going
// into low-power mode screws up the serial monitor feed! To debug, replace
// loseSomeTime(XXX) with delay(XXX).

// All of the above combined, mashed-up and edited by 
// John Keefe
// http://johnkeefe.net/
// August 2015

// library for the temperature humidity sensor
// get at https://github.com/RobTillaart/Arduino/tree/master/libraries/DHTlib
#include <dht.h>
dht DHT;

#include <SoftwareSerial.h>

// library for fona
// get at https://learn.adafruit.com/adafruit-fona-mini-gsm-gprs-cellular-phone-module/arduino-test
#include "Adafruit_FONA.h"

// library for low-power sleep move
// get at http://jeelabs.net/projects/jeelib/wiki
#include <JeeLib.h> 

// temp sensor definitino
#define DHT22_PIN 5

// fona definitions
#define FONA_RX 2 // communications
#define FONA_TX 3 // communications
#define FONA_RST 4 // the reset pin
#define FONA_KEY 6 // pulse to power up or down
#define FONA_PS 7 //status pin. Is the board on or not?
int keyTime = 2000; // Time needed to turn on/off the Fona
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

// low-power sleep definitions
int mins_between_texts = 20; // minutes between texts

// must be defined in case we're using the watchdog for low-power waiting
// WDT = watchdog timer
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

float temp_farenheit;
String phone_number = "40404"; // Set for Twitter; put receiving number here
String base_message;

// this is for the sensor
struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} stat = { 0,0,0,0,0,0,0,0};

void setup()
{
  // Set FONA pins (actually pretty important to do FIRST, 
  // otherwise the board can be inconsistently powered during startup. 
  pinMode(FONA_PS, INPUT); 
  pinMode(FONA_KEY,OUTPUT);   
  digitalWrite(FONA_KEY, HIGH);
  
  // make communications with fona slow so it is easy to read
  fonaSS.begin(4800);

  // establish a speed for the serial monitor
  Serial.begin(115200);

  // Power up FONA
  turnOnFONA();
}

void loop()
{
  // Read Sensor Data

  // this code is from the original noted above. it is probably
  // more involved than necessary, but broke when I tried to 
  // pare it down.
  Serial.print("DHT22, \t");

  uint32_t start = micros();
  int chk = DHT.read22(DHT22_PIN);
  uint32_t stop = micros();

  stat.total++;
  switch (chk)
  {
  case DHTLIB_OK:
      stat.ok++;
      Serial.print("OK,\t");
      break;
  case DHTLIB_ERROR_CHECKSUM:
      stat.crc_error++;
      Serial.print("Checksum error,\t");
      break;
  case DHTLIB_ERROR_TIMEOUT:
      stat.time_out++;
      Serial.print("Time out error,\t");
      break;
  case DHTLIB_ERROR_CONNECT:
      stat.connect++;
      Serial.print("Connect error,\t");
      break;
  case DHTLIB_ERROR_ACK_L:
      stat.ack_l++;
      Serial.print("Ack Low error,\t");
      break;
  case DHTLIB_ERROR_ACK_H:
      stat.ack_h++;
      Serial.print("Ack High error,\t");
      break;
  default:
      stat.unknown++;
      Serial.print("Unknown error,\t");
      break;
  }
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.print(DHT.temperature, 1);
  Serial.print(",\t");
  Serial.print(stop - start);
  Serial.println();

  if (stat.total % 20 == 0)
  {
      Serial.println("\nTOT\tOK\tCRC\tTO\tUNK");
      Serial.print(stat.total);
      Serial.print("\t");
      Serial.print(stat.ok);
      Serial.print("\t");
      Serial.print(stat.crc_error);
      Serial.print("\t");
      Serial.print(stat.time_out);
      Serial.print("\t");
      Serial.print(stat.connect);
      Serial.print("\t");
      Serial.print(stat.ack_l);
      Serial.print("\t");
      Serial.print(stat.ack_h);
      Serial.print("\t");
      Serial.print(stat.unknown);
      Serial.println("\n");
  }

  // convert celcius to farenheit because U.S.
  temp_farenheit = (DHT.temperature * 1.8) + 32;

  Serial.print("Humidity: ");
  Serial.println(DHT.humidity, 1);
  Serial.print("Temperature (F): ");
  Serial.println(temp_farenheit, 1);

  // Power up FONA
  turnOnFONA();

  // Read the FONA battery level
  uint16_t vbat;
  if (! fona.getBattPercent(&vbat)) {
    Serial.println(F("Failed to read Batt"));
  } else {
    Serial.print(F("Battery: ")); 
    Serial.print(vbat); 
    Serial.println(F("%"));
  }

  // Compose the text message
  char sendto[21], message[141];
  base_message = "John's Arduino detects Temp:";

  // Converting a float variable (temp_farenheit) to a String (tempTemp)
  // using dtostrf as defined here: http://forum.arduino.cc/index.php?topic=126618.0
  //   dtostrf(YourNumber, TotalStringLength, NumberOfDecimals, TheTargetArray)
  //   TotalStringLength    > Must include all characters the '.' and the null terminator
  //   NumberOfDecimals  > The output is rounded .456 become .46 at 2 decimals
  //   TheTargetArray must be declared
  char tempTemp[6];
  dtostrf(temp_farenheit, 5, 1, tempTemp);

  // Ditto float -> String conversion for the humidity
  char humidityTemp[6];
  dtostrf(DHT.humidity, 5, 1, humidityTemp);

  // Convert battery_percent int to String
  String battery_percent = String(vbat);  

  // building the message to text
  base_message = base_message + tempTemp + " F | Humidity" + humidityTemp + "% | Battery " + battery_percent + "%";

  // turn the phone number String into a character array called sendto
  phone_number.toCharArray(sendto,20);

  // convert base_message String to a character array called message
  // for the fona software
  base_message.toCharArray(message,140);

  // Send text via Fona
  Serial.print("SMS message: ");
  Serial.println(message); 

  Serial.print("Sending to: ");
  Serial.println(sendto);   

  if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("Sent!"));
  }

  // wait for the sending process to conmplete
  Serial.println(F("Waiting for sending to complete..."));
  Sleepy::loseSomeTime(10000); // instead of delay(10000);

  // power down fona
  turnOffFONA();
  
  // Enter arduino low-power mode
  // loseSomeTime has a max of 60secs ... so looping for multiple minutes
  for (byte i = 0; i < mins_between_texts; ++i) {
    Sleepy::loseSomeTime(60000);
  }

}

// This turns FONA ON
void turnOnFONA() { 
    if(! digitalRead(FONA_PS)) { //Check if it's On already. LOW is off, HIGH is ON.
        Serial.print("FONA was OFF, Powering ON: ");
        digitalWrite(FONA_KEY,LOW); //pull down power set pin
        unsigned long KeyPress = millis(); 
        while(KeyPress + keyTime >= millis()) {} //wait two seconds
        digitalWrite(FONA_KEY,HIGH); //pull it back up again
        Serial.println("FONA Powered Up");
        Serial.println("Initializing please wait 20sec...");
        // delay for 20sec to establish cell network handshake.
        Sleepy::loseSomeTime(20000); 
    } else {
        Serial.println("FONA Already On, Did Nothing");
    }

    // test to make sure all is well & FONA is responding
    // this also appears ke to reestablishing the software serial
    if (! fona.begin(fonaSS)) {
      Serial.println(F("Couldn't find FONA"));
    } else {
      Serial.println(F("FONA is OK"));
    }
}

// This does the opposite of turning the FONA ON (ie. OFF)
void turnOffFONA() { 
    if(digitalRead(FONA_PS)) { //check if FONA is OFF
        Serial.print("FONA was ON, Powering OFF: "); 
        digitalWrite(FONA_KEY,LOW);
        unsigned long KeyPress = millis();
        while(KeyPress + keyTime >= millis()) {}
        digitalWrite(FONA_KEY,HIGH);
        Serial.println("FONA is Powered Down");
    } else {
        Serial.println("FONA is already off, did nothing.");
    }
}


//
// END OF FILE
//