#include <SPI.h>
#include <SD.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "DS3231.h"
#define WIZNET_W5100 1

unsigned int param_value_int[7];
#include <Ethernet.h>
#include <ArduinoRS485.h> // ArduinoModbus depends on the ArduinoRS485 library
#include <ArduinoModbus.h>

byte mac[] = {0x90, 0xA2, 0xDA, 0x0E, 0x94, 0xB5 };
IPAddress ip(192, 168, 1, 94);
IPAddress server(192, 168, 1, 239); 
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
EthernetClient client;
ModbusTCPClient modbusTCPClient(client);

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

RTClib RTC; //real time clock

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac, ip, gateway, subnet);  
  
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("  PHILLIPS 66");

  lcd.setCursor(1,1);
  lcd.print("JJ8 Megalogger");

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);                      // To disable slave select for SD card; depricated.

  delay(2000);
}

void loop() {
  lcd.display();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Working...Please");
  lcd.setCursor(0,1);
  lcd.print("Stand By...");

  ethernetLoop();
  
  lcd.clear();
  lcd.setCursor(0,0);
  writeToFile(getTime());
  lcd.clear();
  lcd.print(getTime());
  delay(3000);
}

String getTime() {
  DateTime now = RTC.now();
  return (String)(now.year()) + '-' + (String)(now.month()) + '-' + (String)(now.day()) + " " + (String(now.hour())) + ":" + (String)(now.minute()) +":" + (String)(now.second());
}

void writeToFile(String the_string) {
  // Open serial communications and wait for port to open:
  File myFile;

  if (!SD.begin(4)) {
    lcd.clear();
    lcd.print("FAILED CARD!");
    while (1);
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("JJ8.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {

    myFile.println(the_string);
    // close the file:
    myFile.close();
    lcd.clear();
    lcd.print("Write success!");
    lcd.setCursor(0,1);
    lcd.print("You may power down.");
    delay(5000);
  } else {
    // if the file didn't open, print an error:
    lcd.clear();
    lcd.print("error opening test.txt");
    while (1);
  }
}

void ethernetLoop()
{ long response;
  if (!modbusTCPClient.connected()) {
    // client not connected, start the Modbus TCP client
    Serial.println("Attempting to connect to Modbus TCP server");
    
    if (!modbusTCPClient.begin(server)) {
      
      Serial.println("Modbus TCP Client failed to connect!");
      Serial.println(modbusTCPClient.lastError());
      return;
    } else {
      Serial.println("Modbus TCP Client connected");
    }
  } else   Serial.println("Modbus TCP Client connected");
  
  Serial.println("About to read holding registers...");
  response=modbusTCPClient.holdingRegisterRead(1,7001); //https://www.arduino.cc/en/ArduinoModbus/ModbusClientholdingRegisterRead
  Serial.println("complete ");
  
  if (!response) {
    Serial.println("Could not read register! ");
    Serial.println(modbusTCPClient.lastError());
  } else {
    Serial.println("Register Value=" + String(response) );
    lcd.clear();
    lcd.print("Did it:" + String(response));
    delay(3000);
  }
  // wait for 1 second
  modbusTCPClient.stop();
  delay(100);  
}
