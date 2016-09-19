#include <RTCZero.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* Create an rtc object */
RTCZero rtc;

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 00;
const byte hours = 14;

/* Change these values to set the current initial date */
const byte days = 19;
const byte months = 9;
const byte years = 16;

byte alarm_seconds;
byte alarm_minutes;
byte alarm_hours;

void alarmMatch(void);

/**********************************************************/
void setup(void)
{
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(13, LOW);
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(days, months, years);

  if ( !ble.begin(VERBOSE_MODE) )
  {
  }

  if ( FACTORYRESET_ENABLE )
  {
    if ( ! ble.factoryReset() ) {
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);
  ble.info();
  ble.verbose(false);  // debug info is a little annoying after this point!
  /* Wait for connection */

  rtc.standbyMode();


}

void loop(void)
{
  while ( ble.isConnected()) {


    // Check for incoming characters from Bluefruit
    ble.println("AT+BLEUARTRX");
    ble.readline();
    if (strcmp(ble.buffer, "OK") == 0) {
      // no data
      return;
    }
    // Some data was found, its in the buffer
    String cmd;
    cmd = ble.buffer;
    digitalWrite(12, HIGH);
    digitalWrite(11, HIGH);
    if (cmd == "On") {
      digitalWrite(13, HIGH);
    }
    if (cmd == "Off") {
      digitalWrite(13, LOW);
    }
    if (find_text("alarm",cmd) {

      rtc.setAlarmTime(14, 00, 10);
      rtc.enableAlarm(rtc.MATCH_HHMMSS);

      rtc.attachInterrupt(alarmMatch);
    }

    ble.waitForOK();
  }


  delay(500);
}


void alarmMatch()
{
  digitalWrite(13, HIGH);
}
