# Thermal-Vac Sensor Suite
Code for the sensor suite that is used with the thermal-vac chamber. 
The sensor suite is used to monitor the inner-conditions of the chamber, including pressure and temperature.
The sensor suite has been upgraded across many iterations to the point where the new sensor suite is on a Teensy 3.5
and includes Honeywell pressure sensors, Adafruit thermocouples, XBee radio communication, relays controlling/powering
fans and heating mesh, the ability to input both commands and external pressure readings, and a predicted alititude
based upon pressure.

Used preferably with the CoolTerm software, which is found [here](http://freeware.the-meiers.org/).

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
  
