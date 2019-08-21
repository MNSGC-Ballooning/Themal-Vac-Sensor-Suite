//Libraries
#include <Adafruit_MAX31856.h>  //thermocouple library
#include <XBee.h>  //xbee library
#include <SD.h>  //SD card library


/*
  /-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\
  \               Thermal-Vac Computer on Arduino Mega 2560                                         /
  /               Updated to be used with the PuTTY software                                        \
  \               Written originally by Jacob Meyer (meye2497) for XCTU software                    /
  /               Removal, additions, and conversion to PuTTY by Billy Straub (strau257)            \
  \-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/-\-/


  Arduino Mega 2560 pin connections:
   -------------------------------------------------------------------------------------------------------------------------
     Component                    | Pins used               | Notes
     -                              -                         -
     xBee serial on shield        | D18 D19 (Default)       | Defined as Serial 1
     SD card reader on shield     | D4  D11 D12 D13         | CS, MOSI, MISO, CLK pins in that order
     Honeywell Pressure           | A12                     |
     Adafruit Thermocouple #1     | D36 D37 D38 D30         | SCK, SDO, SDI, CS pins in that order
     Adafruit Thermocouple #2     | D36 D37 D38 D31         |
     Adafruit Thermocouple #3     | D36 D37 D38 D28         |
     Adafruit Thermocouple #4     | D36 D37 D38 D32         |
   -------------------------------------------------------------------------------------------------------------------------

Note: Pin 53 must be left empty due to it being used as a D_out for the SD shield
*/



//SD ardunio shield pin definition
#define chipSelect 4

//SD file logging
File datalog;                     //File object for datalogging
char filename[] = "TVac00.csv";   //Template for file name to save data
bool SDactive = false;            //Used to check for SD card before attempting to log data


//XBee serial connection
#define XBeeSerial Serial1

//XBee network ID
const String ID = "MEYER1";       //Choose a unique 4-digit hexadecimal network ID

//XBee object
XBee xBee = XBee(&XBeeSerial);


//Thermocouple object
//Template: Adafruit_MAX31856 maxthermo# = Adafruit_MAX31856(cs)
Adafruit_MAX31856 maxthermo1 = Adafruit_MAX31856(30, 38, 37, 36);
Adafruit_MAX31856 maxthermo2 = Adafruit_MAX31856(31, 38, 37, 36);
Adafruit_MAX31856 maxthermo3 = Adafruit_MAX31856(28, 38, 37, 36);
Adafruit_MAX31856 maxthermo4 = Adafruit_MAX31856(32, 38, 37, 36);


//Honeywell Analog Pressure Sensor Values:
int pressureSensor = 0;
float pressureSensorV = 0;
float psi = 0;
float atm = 0;


unsigned long time;     //Used to keep time running



void setup() {

  Serial.begin(9600);

  //Begin XBee communications
  XBeeSerial.begin(9600);         
  Serial.println(xBee.enterATmode());
  Serial.println(xBee.atCommand("ATMY0"));
  Serial.println(xBee.atCommand("ATDL1"));
  Serial.println(xBee.atCommand("ATID" + ID));
  Serial.println(xBee.exitATmode());

  //Setup Adafruit MAX_31856 Thermocouples #1-4
  maxthermo1.begin();
  maxthermo2.begin();
  maxthermo3.begin();
  maxthermo4.begin();
  maxthermo1.setThermocoupleType(MAX31856_TCTYPE_K);
  maxthermo2.setThermocoupleType(MAX31856_TCTYPE_K);
  maxthermo3.setThermocoupleType(MAX31856_TCTYPE_K);
  maxthermo4.setThermocoupleType(MAX31856_TCTYPE_K);

  //SD card setup
  pinMode(10, OUTPUT);                                      //Needed for SD library, regardless of shield used
  pinMode(chipSelect, OUTPUT);
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect))                                //Attempt to start SD communication
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
  String header = "Pressure   Temp 1   Temp 2   Temp 3   Temp 4   Time     Input-Pressure";
  Serial.println(header);
  if (SDactive) {
    datalog.println(header);
    datalog.close();
  }
  
}



void loop() {

  //Pressure Sensor
  pressureSensor = analogRead(A12);                       //Read the analog pin
  pressureSensorV = pressureSensor * (5.0 / 1024);        //Convert the digital number to voltage
  psi = (pressureSensorV - (0.1 * 5.0)) / (4.0 / 15.0);   //Convert the voltage to proper unitime
  atm = psi / 14.696;                                     //Convert psi to atm
  
  //Thermocouples 1-4 temperatures
  float T1int = maxthermo1.readThermocoupleTemperature();
  float T2int = maxthermo2.readThermocoupleTemperature();
  float T3int = maxthermo3.readThermocoupleTemperature();
  float T4int = maxthermo4.readThermocoupleTemperature();


  int PR;               //Creates integers that can be changed, allowing the sig figs to be changed in the later strings depending on temp/pressure.
  int TT;
  int TU;
  int TV;
  int TW;

  if (0<atm){           //Conditional function to detemine number of sig figs based on pressure (and temperature in lower statements) to keep columns organized.
    PR = 3;
  }
    else if (0>=atm){
      PR = 2;
    }

  if (T1int<=-10){
    TT = 2;
  }
    else if (-10<T1int<=0 || 10<=T1int){
      TT = 3;
    }
    else if (0<T1int<10){
      TT = 4;
    }

  if (T2int<=-10){
    TU = 2;
  }
    else if (-10<T2int<=0 || 10<=T2int){
      TU = 3;
    }
    else if (0<T2int<10){
      TU = 4;
    }

  if (T3int<=-10){
    TV = 2;
  }
    else if (-10<T3int<=0 || 10<=T3int){
      TV = 3;
    }
    else if (0<T3int<10){
      TV = 4;
    }

  if (T4int<=-10){
    TW = 2;
  }
    else if (-10<T4int<=0 || 10<=T4int){
      TW = 3;
    }
    else if (0<T4int<10){
      TW = 4;
    }

  String atmSTR = String(atm, PR);                        //Converts Pressure to a string and uses sig figs based on conditional function above.
  String T1intSTR = String(T1int, TT);
  String T2intSTR = String(T2int, TU);
  String T3intSTR = String(T3int, TV);
  String T4intSTR = String(T4int, TW);
  
  time = millis() / 1000 - 26;                            //Converts time to seconds and starts the time at zero by subtracting the intial 26 seconds.

  String data = String(atmSTR + "      " + T1intSTR + "   " + T2intSTR + "   " + T3intSTR + "   " + T4intSTR + "   " + time);
  Serial.println(data);
  if (SDactive) {
    datalog = SD.open(filename, FILE_WRITE);
    datalog.println(data);                                //Takes serial monitor data and adds to SD card
    datalog.close();                                      //Close file afterward to ensure data is saved properly
  }
  
  delay(1000);

}
