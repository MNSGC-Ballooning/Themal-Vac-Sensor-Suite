/*
  /-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\
  \               Thermal-Vac Computer on Teensy 3.5                            /
  /               To be Used with the PuTTY software                            \
  \               Written by Billy Straub (strau257) Summer 2019                /
  /               Based on Code by Jacob Meyer (meye2497)                       \
  \-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/


   Teensy 3.5 pin connections:
   --------------------------------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used               | Notes
     -                              -                         -
     SD card reader               |                         |
     xBee serial                  | D33 D34                 |  First pin is D_in and second is D_out
     MS5607 Parallax Altimeter    | D19 D18                 |  I2C communication for MS5607 where D19 is SCL0 and D18 is SDA0
     Adafruit Thermocouple #1     | D9  D8  D7  D5          |  First SCK, then SDO, then SDI, then CS pins in that order
     Adafruit Thermocouple #2     | D9  D8  D7  D6          |  Pins in same order as above, expect different chip select (CS) for second thermocouple
   --------------------------------------------------------------------------------------------------------------------------------------------------
*/

//Libraries
#include <Adafruit_MAX31856.h>  //thermocouple library
#include <XBee.h>  //XBee library
#include <SD.h>  //SD card library
#include <i2c_t3.h>  //Required for usage of MS5607 with Teensy 3.5/3.6
#include <Salus_Baro.h>  //Library for MS5607 Altimeter



//SD ardunio shield pin definition
#define SDchipSelect BUILTIN_SDCARD
//const int SDchipSelect = BUILTIN_SDCARD;

//SD file logging
File datalog;                     //File object for datalogging
char filename[] = "TVac00.csv";   //Template for file name to save data
bool SDactive = false;            //Used to check for SD card before attempting to log data


//XBee hardware serial connection
#define XBee_Serial Serial5

//XBee network ID
String networkID = "CAFE";       //Choose a unique 2 to 4 digit, A-F code

//XBee object
XBee xBee = XBee(&XBee_Serial);


//Thermocouple object
//Template: Adafruit_MAX31856 maxthermo# = Adafruit_MAX31856(cs)
Adafruit_MAX31856 maxthermo1 = Adafruit_MAX31856(5, 7, 8, 9);
Adafruit_MAX31856 maxthermo2 = Adafruit_MAX31856(6, 7, 8, 9);


//Parallax MS5607 Altimeter Sensor object
#define Baro_Rate (TIMER_RATE / 200)                        //Process MS5607 data at 100Hz
Salus_Baro myBaro;
float pressure = 0;
float temperatureALT = 0;
float alt_pressure_library = 0;  //Altitiude calculated by the pressure sensor library
float altitude0 = 0;


unsigned long time;     //Used to keep time running



void setup() {

  Serial.begin(9600);

  //Begin XBee communications
    XBee_Serial.begin(9600); //-XBEE_BAUD

  //Set XBee send/recieve channels
 /* Serial.println(xBee.enterATmode());
    Serial.println(xBee.atCommand("ATMY0"));
    Serial.println(xBee.atCommand("ATDL1"));
    Serial.println(xBee.atCommand("ATID" + networkID));
    Serial.println(xBee.exitATmode()); */
    xBee.enterATmode();
    xBee.atCommand("ATMY0");
    xBee.atCommand("ATDL1");
    xBee.atCommand("ATID" + networkID);
    xBee.exitATmode();


  //Setup Adafruit MAX_31856 Thermocouples #1 and #2
  maxthermo1.begin();
  maxthermo2.begin();
  maxthermo1.setThermocoupleType(MAX31856_TCTYPE_K);
  maxthermo2.setThermocoupleType(MAX31856_TCTYPE_K);

  //MS5607 Parallax Altimeter Setup
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, I2C_RATE_400);
  myBaro.begin();
  Wire.setRate(I2C_RATE_400);
  
    //SD card setup
    pinMode(10, OUTPUT);                                      //Needed for SD library, regardless of shield used
    pinMode(SDchipSelect, OUTPUT);
    Serial.print("Initializing SD card...");
    if (!SD.begin(SDchipSelect))                                //Attempt to start SD communication
      Serial.println("Card failed, not present, or voltage supply is too low.");          //Print out error if failed; remind user to check card
    else {                                                    //If successful, attempt to create file
      Serial.println("Card initialized successfully.\nCreating File...");
      for (byte i = 0; i < 100; i++) {                        //Can create up to 100 files with similar names, but numbered differently
        filename[4] = '0' + i / 10;
        filename[5] = '0' + i % 10;
        if (!SD.exists(filename)) {                           //If a given filename doesn't exist, it's available
          datalog = SD.open(filename, FILE_WRITE);            //Create file with that name
          SDactive = true;                                    //Activate SD logging since file creation was successful
          Serial.println("Logging to: " + String(filename));  //Tell user which file contains the data for this run of the program
          break;                                              //Exit the for loop now that we have a file
        }
      }
      if (!SDactive) Serial.println("No available file names; clear SD card to enable logging");
    } 
  String header = "Pressure (atm)   Temp 1 (°C)   Temp 2 (°C)   Temp 3 (°C)   Time (s)   Altitude (m)   Input-Pressure";
  Serial.println(header);
  xBee.println(header);
    if (SDactive) {
      datalog.println(header);
      datalog.close();
    }
  

  delay(1000);
}



void loop() {

  //MS5607 Parallax Altimeter Sensor
  myBaro.baroTask();
  pressure = myBaro.getPressure() / 10 * 0.009869233;       //Converts pressure to atm
  altitude0 = myBaro.getAltitude();
  temperatureALT = myBaro.getTemperature();

  //Thermocouples #1 and #2 temperatures
  float T1int = maxthermo1.readThermocoupleTemperature();
  float T2int = maxthermo2.readThermocoupleTemperature();


  //Creates integers that can be changed, allowing the sig figs to be changed in the later strings depending on leading values, allowing for straight columns.
  int PR;                  //Alitmeter pressure sig fig integer
  int TT;                  //Thermocouple #1 temp sig fig integer
  int TU;                  //Thermocouple #2 temp sig fig integer
  int TA;                  //Altimeter temp sig fig integer
  int TS;                  //Time in seconds sig fig integer
  int AL;                  //Calculated altitude sig fig integer

  if (0 < pressure) {      //Conditional function to detemine number of sig figs based on pressure (and temperature in lower statements) to keep columns organized.
    PR = 3;
  }
  else if (0 >= pressure) {
    PR = 2;
  }

  if (T1int <= -10) {
    TT = 2;
  }
  else if (-10 < T1int <= 0 || 10 <= T1int) {
    TT = 3;
  }
  else if (0 < T1int < 10) {
    TT = 4;
  }

  if (T2int <= -10) {
    TU = 2;
  }
  else if (-10 < T2int <= 0 || 10 <= T2int) {
    TU = 3;
  }
  else if (0 < T2int < 10) {
    TU = 4;
  }

  if (temperatureALT <= -10) {
    TA = 2;
  }
  else if (-10 < temperatureALT <= 0 || 10 <= temperatureALT) {
    TA = 3;
  }
  else if (0 < temperatureALT < 10) {
    TA = 4;
  }


  String AtmSTR = String(pressure, PR);                     //Converts Pressure to a string and uses sig figs based on conditional function above.
  String T1intSTR = String(T1int, TT);
  String T2intSTR = String(T2int, TU);
  String T3intSTR = String(temperatureALT, TA);
  String TimeSTR = String(time);
  String PredictAltSTR = String(altitude0);


  time = millis() / 1000 - 2;                               //Converts time to seconds and starts the time at zero by subtracting the intial 26 seconds.

  String data = String(AtmSTR + "            " + T1intSTR + "        " + T2intSTR + "        " + T3intSTR + "        " + TimeSTR + "     " + PredictAltSTR);

  Serial.println(data);
  xBee.println(data);
    if (SDactive) {
      datalog = SD.open(filename, FILE_WRITE);
      datalog.println(data);                                //Takes serial monitor data and adds to SD card
      datalog.close();                                      //Close file afterward to ensure data is saved properly
    }
  
  delay(466);  //CHANGE IF NEEDED TO ATTAIN ONE SECOND INTERVAL OF READINGS

}
