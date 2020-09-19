/*
 A CO2 monitor displaying CO2 concentration on an OLED screen
 and to the serial monitor using SparkFun libraries.
 If the CO2 concentration is too high, a red LED is fired-up.

 In addition, the relative humidity in % and temperature in C is 
 also measured and displayed.
 
 Author:  Alf Köhn-Seemann
 Email:   alf.koehn@gmail.com
 License: MIT

 Hardware Connections:
 Attach NodeMCU to computer using a USB cable.
 Connect OLED display, SCD30, red LED to NodeMCU.
 Optional: Open Serial Monitor at 115200 baud.
*/

#include <Wire.h>                 // for I2C communication
#include <Adafruit_GFX.h>         // for writing to display
#include <Adafruit_SSD1306.h>     // for writing to display

#define CO2_THRESHOLD 1500        // threshold for warning
#define WARNING_DIODE_PIN D8      // NodeMCU pin for red LED

#define SCREEN_WIDTH 128          // OLED display width in pixels
#define SCREEN_HEIGHT 32          // OLED display height in pixels

// OLDE reset pin, 4 is default (-1 if sharing Arduino reset pin)
// using NodeMCU, we have to use LED_BUILTIN
#define OLED_RESET LED_BUILTIN
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// install library via library manager
// or click here: http://librarymanager/All#SparkFun_SCD30
#include "SparkFun_SCD30_Arduino_Library.h"
SCD30 airSensor;


void setup() {
  // initialize serial monitor at baud rate of 115200
  Serial.begin(115200);
  
  Serial.println("Using the SCD30 to get CO2 concentration (among other things)");
 
  Wire.begin();

  // initialize LED pin as an output
  pinMode(WARNING_DIODE_PIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC: generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();

  // Pause for 2 seconds
  delay(2000); 

  // Clear the buffer
  display.clearDisplay();

  // text size and color have to be set
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(2,5);
  display.println("SCD30 test program");
  display.println("twitter.com/formbar");

  // apply changes to display
  display.display();  

  // turn warning LED on and off to test it
  digitalWrite(WARNING_DIODE_PIN, HIGH);
  delay(2000); 
  digitalWrite(WARNING_DIODE_PIN, LOW);

  // initialize SCD30
  // SCD30 has data ready every two seconds
  if (airSensor.begin() == false) {
   Serial.println("Air sensor not detected. Please check wiring. Freezing...");
   while (1)
     ;
  }
}

void loop() {

  float 
    co2,
    temperature,
    humidity;
  
  if (airSensor.dataAvailable()) {
    // get data from SCD30 sensor
    co2 = airSensor.getCO2();
    temperature = airSensor.getTemperature();
    humidity = airSensor.getHumidity();

    // print data to serial console
    printToSerial(co2, temperature, humidity);

    // print data to OLED display
    printToOLED(co2, temperature, humidity);
  }
  else
    Serial.println("Waiting for new data");

  // if CO2-value is too high, issue a warning  
  if (co2 >= CO2_THRESHOLD) {
    digitalWrite(WARNING_DIODE_PIN, HIGH);
  } else if (co2 < CO2_THRESHOLD) {
    digitalWrite(WARNING_DIODE_PIN, LOW);
  }

  // wait 2 seconds: SCD30 has new data every 2 seconds
  delay(2000);
}

void printToSerial( float co2, float temperature, float humidity) {
  Serial.print("co2(ppm):");
  Serial.print(co2, 1);
  Serial.print(" temp(C):");
  Serial.print(temperature, 1);
  Serial.print(" humidity(%):");
  Serial.print(humidity, 1);
  Serial.println();
}

void printToOLED( float co2, float temperature, float humidity) {
  int
    x0, x1;           // to align output on OLED display vertically

  x0  = 2;
  x1  = 86;

  display.clearDisplay();
  display.setCursor(x0,5);
  display.print("CO2 (ppm):");
  display.setCursor(x1,5);
  // for floats, 2nd parameter in display.print sets number of decimals
  display.print(co2, 0);
    
  display.setCursor(x0,15);
  display.print("temp. ( C)");
  display.setCursor(x0+7*6,15);
  display.write(167);
  display.setCursor(x1,15);
  display.print(temperature, 1);
    
  display.setCursor(x0,25);
  display.print("humidity (%):");
  display.setCursor(x1,25);
  display.print(humidity, 1);
    
  display.display();
}