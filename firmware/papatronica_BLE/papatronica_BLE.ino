/*********************************************************************
  Firmware Papatronica BLE 
  Para la siguiente configuración de hardware/software:
  - Placa Adafruit Feather Bluefruit LE -> Bluefruit LE Connect app
  - Sensor Adafruit ADXL375

  Proyecto Papatronica
  https://github.com/ecoagrotec/papatronica
  
  Licencia CERN-OHL-S
  https://ohwr.org/cern_ohl_s_v2.txt
*********************************************************************/


// Código BLE - Adafruit Feather Bluefruit LE
// Adaptado de: https://github.com/adafruit/Bluefruit_LE_Connect_Plotter
// MIT License
/*********************************************************************
  Bluefruit LE Connect Plotter 
  for Feather Bluefruit -> Bluefruit LE Connect app
  
  outputs dummy values for demo use with BLuefruit LE Connect app
  change SEND_SECOND_PLOT define to 1 to output second plot using sine wave table

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  MIT license, check LICENSE for more information
  All text above, and the splash screen below must be included in
  any redistribution
*********************************************************************/

// Código sensor ADXL375
// Adaptado de: https://learn.adafruit.com/adafruit-adxl375/arduino
// BSD License - https://github.com/adafruit/Adafruit_ADXL375/blob/master/license.txt

// Código para medir voltaje de batería
// Adaptado de: https://learn.adafruit.com/adafruit-feather-m0-bluefruit-le/power-management



// BLE - Adafruit Feather Bluefruit LE
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"

// ADXL375
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL375.h>
#define ADXL375_SCK 13
#define ADXL375_MISO 12
#define ADXL375_MOSI 11
#define ADXL375_CS 10
/* Assign a unique ID to this sensor at the same time */
/* Hardware SPI */
Adafruit_ADXL375 accel = Adafruit_ADXL375(ADXL375_CS, &SPI, 12345);

// batería
#define VBATPIN A7 


void displayDataRate(void)
{
  if (accel.getDataRate() == ADXL343_DATARATE_3200_HZ)
  {
      Serial.println("Tasa de muestreo correcta (3200 Hz)");
  } else {
      Serial.println("Tasa de muestreo incorrecta");
  }
}


/*=========================================================================
    APPLICATION SETTINGS

      FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
     
                                Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                                running this at least once is a good idea.
     
                                When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                                Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
         
                                Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "BLEUART" // el LED rojo titila cuando transmite datos por BLE - el led azul se enciende cuando hay una conexión establecida
#define SEND_SECOND_PLOT            0
/*=========================================================================*/

// Create the bluefruit object, hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  // initialize digital pin 13 as an output (LED)
  pinMode(13, OUTPUT);

  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Command <-> Data Mode Example"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
    delay(500);
  }

  Serial.println(F("* * * * *"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("* * * * *"));

  // ADXL375
  Serial.println("ADXL375 Accelerometer Test"); Serial.println("");
  /* Initialise the sensor */
  if(!accel.begin())
  {
    /* There was a problem detecting the ADXL375 ... check your connections */
    Serial.println("Ooops, no ADXL375 detected ... Check your wiring!");
    while(1);
  }
  // Range is fixed at +-200g
  // Set data rate at 3200 Hz
  accel.setDataRate(ADXL343_DATARATE_3200_HZ);
  /* Display some basic information on this sensor */
  accel.printSensorDetails();
  displayDataRate();
  Serial.println("");

}


// ********************************************************************** //

float medicion = 0;
float max_medicion = 0;
float medicion_anterior = 0;
float umbral = 5;
unsigned long timeout = 200;
unsigned long last_timeout = millis ();
int mediciones_bajo_umbral = 0;

void loop(void) {

    /* Get a new sensor event */
    sensors_event_t event;
    last_timeout = millis ();
    max_medicion = 0;
    
    while (timeout > (millis () - last_timeout) ){
      accel.getEvent(&event); 
      medicion = sqrt( sq(event.acceleration.x) + sq(event.acceleration.y) + sq(event.acceleration.z) ) / 9.8;     // convertir a g
      if (medicion > max_medicion ){
        max_medicion = medicion;
      }
    }
   
    if (max_medicion < umbral) {
      mediciones_bajo_umbral += 1;
      float measuredvbat = analogRead(VBATPIN);
      measuredvbat = measuredvbat * 2 * 3.3 / 1024;   // convertir a volts
      measuredvbat = (measuredvbat - 3.45) / 0.7;  // convertir a un rango entre 0 y 1 aprox (entre 3.45V y 4.15V)
      if (mediciones_bajo_umbral < 300) {
        ble.println(measuredvbat);              // graficar voltaje de la batería
      } else if ((mediciones_bajo_umbral % 300) == 0) {
        ble.println(measuredvbat);              // graficar voltaje de la batería
      } 
    } else {
      ble.println(max_medicion);              // graficar valor máximo medido
      mediciones_bajo_umbral = 0;
    }
  
}
