//Libraries
#include <Adafruit_MAX31856.h>  //thermocouple library
#include <XBee.h>  //xbee library

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

     xBee serial                  | D18 D19 (Default)       | Defined as Serial 1
     Honeywell Pressure           | A12                     |
     Adafruit Thermocouple #1     | D52 D50 D51 D30         | SCK, SDO, SDI, CS pins in that order
     Adafruit Thermocouple #2     | D52 D50 D51 D31         |
     Adafruit Thermocouple #3     | D52 D50 D51 D28         |
     Adafruit Thermocouple #4     | D52 D50 D51 D53         |
   -------------------------------------------------------------------------------------------------------------------------
*/


//XBee serial connection
#define XBeeSerial Serial1

//XBee network ID
const String ID = "MEYER1";       //Choose a unique 4-digit hexadecimal network ID

//XBee object
XBee xBee = XBee(&XBeeSerial);

//Thermocouple object
//Template: Adafruit_MAX31856 maxthermo# = Adafruit_MAX31856(cs)
Adafruit_MAX31856 maxthermo1 = Adafruit_MAX31856(30);
Adafruit_MAX31856 maxthermo2 = Adafruit_MAX31856(31);
Adafruit_MAX31856 maxthermo3 = Adafruit_MAX31856(28);
Adafruit_MAX31856 maxthermo4 = Adafruit_MAX31856(53);



// Global Variables
// For Honeywell Analog Pressure Sensor:
int pressureSensor = 0;
float pressureSensorV = 0;
float psi = 0;
float atm = 0;


// For data collection:
bool TakeData = false;            //Boolean to start or stop data collection
bool SecS = false;                //Boolean to start or stop data collection

unsigned long time;               //Used to keep time running



void setup() {

  Serial.begin(9600);

  XBeeSerial.begin(9600);         //Start XBee communication
  //Configure radio with ID; designate as "flight" (thermal-vac test) unit
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

  //Project Title and leading column titles (and maybe Author name)
  Serial.println("Thermal-Vac Chamber");
  Serial.println("Pressure Temp1 Temp2 Temp3 Temp4 Time  Input-Pressure");

}



void loop() {

  //Always check for incoming commands
  String command = Serial.readStringUntil('\n');

  //Pressure Sensor
  pressureSensor = analogRead(A12);                       //Read the analog pin
  pressureSensorV = pressureSensor * (5.0 / 1024);        //Convert the digital number to voltage
  psi = (pressureSensorV - (0.1 * 5.0)) / (4.0 / 15.0);   //Convert the voltage to proper unitime
  atm = psi / 14.696;                                     //Convert psi to atm
  String atmSTR = String(atm, 3);                         //Convertime Pressure to a string, Fixes to 3 decimal places so it takes up less space on the screen and gives us the desired sig figs

  //Thermocouples 1-4 temperatures
  float T1 = maxthermo1.readThermocoupleTemperature();
  float T2 = maxthermo2.readThermocoupleTemperature();
  float T3 = maxthermo3.readThermocoupleTemperature();
  float T4 = maxthermo4.readThermocoupleTemperature();

  //Converting the temperature floats to intime takes up less space on the screen
  int T1int = T1;
  int T2int = T2;
  int T3int = T3;
  int T4int = T4;
  
  time = millis() / 1000 - 27;

//  Serial.println(P + "    " + T1 + " " + T2 + " " + T3 + " " + T4 + " " + time);
  
  serial(atmSTR, T1int, T2int, T3int, T4int, time, TakeData, command);

  delay(1000);


}



void serial(String P, float T1, float T2, float T3, float T4, float , bool &TakeData, String command) {

  Serial.println(P + "    " + T1 + " " + T2 + " " + T3 + " " + T4 + " " + time);

}
