# Thermal-Vac Sensor Suite
Code for the sensor suite that is used with the thermal-vac chamber. 
The sensor suite is used to monitor the inner-conditions of the chamber, including pressure and temperature.
The sensor suite has been upgraded across many iterations to the point where the new sensor suite is on a Teensy 3.5
and includes Honeywell pressure sensors, Adafruit thermocouples, XBee radio communication, relays controlling/powering
fans and heating mesh, the ability to input both commands and external pressure readings, and a predicted alititude
based upon pressure.

**CoolTerm - a Serial Monitor for Radio Usage**

- Used preferably with the CoolTerm software, which is found [here](http://freeware.the-meiers.org/). 
- CoolTerm can be used by plugging a USB-XBee adapter into the computer running CoolTerm, which communicates with another XBee connected to a microcontroller.
- Removing the buffer on CoolTerm:
  - To remove the buffer on CoolTerm, open the software and click on "Options". 
  - Then click on "Receive".
  - In the "Receive" page, under "Receive Options", there is an input box next to "Receive Buffer Size".
  - Change the "Receive Buffer Size" from 10000 to 2147483647. 
  - (This doesn't techincally remove the buffer, but instead changes its size to 2 gigabytes.) 
  - Then go to "File" and then to "Save As", and save the settings as a shortcut on your desktop. 
  - When opening CoolTerm from now on, use this shortcut. 
- Sending strings (commands/input-pressure) over CoolTerm:
  - Open CoolTerm and click the "Connection" tab.
  - Under "Connection", click "Send String...".
  - A new window will open, where strings can be sent from CoolTerm to the microcontroller.
  
**Commands and Input-Pressure**

  - The newest thermal-vac sensor suite allows for the Teensy 3.5 to accept 6 characters from CoolTerm's "Send String" window.
  - Commands:
    - To turn the fans on, "Send" the command "R1..ON"
    - To turn the fans off, "Send" the command "R1.OFF"
    - To turn active heating on, "Send" the command "R2..ON"
    - To turn active heating off, "Send" the command "R2..OFF"
  - Input-Pressure:
    - To log the pressure reading from Omega DVG-64A pressure sensor, which is connected to the thermal-vac chamber, type the pressure reading and click "Send".
    - Since the sensor suite requires six characters, type zeros before the reading so that there are six characters being sent.
    - Example: If the pressure is 26.52 mBar, "Send" 026.52 so that there are six characters being sent.
    - Example: If the pressure is 5.48 mBar, "Send" 005.48 so that there are six characters being sent.

**Libraries:**

- **Required Libraries for most recent version:**
  - <Adafruit_MAX31856.h> 
    - Adafruit thermocouple library
    - Found [here](https://github.com/adafruit/Adafruit_MAX31856).

- **Required Libraries for older versions:**
  - <i2c_t3.h>
    - Enhanced I2C library for Teensy 3.x (used with Parallax Altimeter)
    - Found [here](https://github.com/nox771/i2c_t3).
  - <Salus_Baro.h>
    - Parallax Altimeter library
    - Found [here](https://github.com/MNSGC-Ballooning/baro).
    - Note: Parallax Altimeter requires a logic change below 20 degrees Celcius, hence why I switched back to Honeywell pressure sensors.
  
