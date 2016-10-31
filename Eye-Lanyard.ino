#include <RTCZero.h>
#include <Arduino.h>
#include <PrintEx.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define DEBUGMODE_ENABLE            1


Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* Create an rtc object */
RTCZero rtc;



struct MedAlarm{
    int id;
    int h;
    int m;
    int s;
    int enable;    
};

int enableMessage = 1;
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
int  alarm_run_time;

void alarmMatch(void);
int find_text(String needle, String haystack);
void BLEcommand(void);
void setAlarmInput(String cmd);
void setTimeInput(String cmd);
MedAlarm getMedAlarm(int id);
void setCurrentAlarm(void);


MedAlarm alarm1{1,0,0,0,0};
MedAlarm alarm2{2,0,0,0,0};
MedAlarm alarm3{3,0,0,0,0};
MedAlarm alarm4{4,0,0,0,0};
MedAlarm alarm5{5,0,0,0,0};
MedAlarm alarm6{6,0,0,0,0};

int currentAlarm = 1;
int speed = 200; // speed should be within 0 - 255

/*Definations for pins */
int motor1 = 12;
int motor2 = 11;
int button = 8 ;

StreamEx mySerial = Serial; //serial with printf

void debuger(String message){
    if(DEBUGMODE_ENABLE && enableMessage){
          mySerial.println(message);    
      }
  }

/**********************************************************/
void setup(void)
{
  while(!Serial);
  delay(500);

  Serial.begin(115200);
  debuger("****Eye Lanyard project Debug Mode****");
  mySerial.printf("123%d",4);
  
  pinMode(13, OUTPUT);
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(button,INPUT_PULLUP); // to be changed
  attachInterrupt(digitalPinToInterrupt(button),buttonClicked,CHANGE);

  digitalWrite(13, LOW);
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(days, months, years);

  debuger("RTC start");

  if(DEBUGMODE_ENABLE){
  rtc.setAlarmTime(14, 00, 5);
  rtc.enableAlarm(rtc.MATCH_HHMMSS);
  rtc.attachInterrupt(alarmMatch);
  setMedAlarm(1,14,00,5,1);  
  setMedAlarm(2,14,00,10,1); 
  //setMedAlarm(3,14,00,20,1); 
  }

  

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

  //rtc.standbyMode();

  debuger("BLE setup");


}

int floop = 1;
int fcommand = 1;

void loop(void)
{
  
  if(floop){
  fcommand = 1;
  debuger("Enter the loop function, waitting for ble connection");
  floop=0;
  }
  while ( ble.isConnected()) {
    if(fcommand){
        debuger("BLE connected, waitting for commmad");
        fcommand = 0;
        floop =1; 
    }
    BLEcommand();
  }
   


  delay(500);
}

void BLEcommand(void)
{
 //debuger("BLE connected watting for command");
    
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
    debuger("Find a command: ");
    debuger(cmd);

    
    if (cmd == "On") {
      digitalWrite(13, HIGH);
      debuger("LED turned ON");
      if(DEBUGMODE_ENABLE){
      updateAlarmTime();
      setMedAlarm(1,alarm_hours,alarm_minutes,alarm_seconds+5,1);
      setCurrentAlarm();
      }
      
    }
    else if (cmd == "Off") {
      digitalWrite(13, LOW);
      debuger("LED turned OFF");
    }
    else if (find_text("alarm",cmd)==0) {
      //digitalWrite(13, HIGH);
//    
        //updateAlarmTime();
        //int i = cmd.substring(5,7).toInt();
      
      //rtc.setAlarmTime(alarm_hours, alarm_minutes, alarm_seconds+i);
      //rtc.enableAlarm(rtc.MATCH_HHMMSS);

      //rtc.attachInterrupt(alarmMatch);

       debuger("Start setting alarm:");
        delay(50);
        setAlarmInput(cmd.substring(5));
    
    }else if (find_text("setTime",cmd)==0)//set time input
//format:setTime:y16:o09:d28:h12:m39:s22
    {
        setTimeInput(cmd.substring(6));
    }else if (find_text("speed",cmd)==0)
    {
        speed = cmd.substring(4).toInt();
        debuger((String)speed);

    }

    ble.waitForOK();
    //delay(500);      



}

void setAlarmInput(String cmd)    //string cmd input format:
//alarm:h12:m30:s22 --> 12:30:22
{
   
    debuger("In setAlarmInput function: ");
    int index;
    index = cmd.indexOf('i');
    int input_id       = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('h');
    int input_hours    = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('m');
    int input_minutes  = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('s');
    int input_seconds  = cmd.substring(index+1,index+3).toInt();
    delay(500);
    ble.print("AT+BLEUARTTX=");
    ble.println(input_hours);

    setMedAlarm(input_id,input_hours,input_minutes,input_seconds,1);
    MedAlarm tmp = getMedAlarm(input_id);
    
    debuger("The inputed id is: ");
    debuger((String)input_id);
    debuger((String)tmp.h);
    debuger((String)tmp.m );
    debuger((String)tmp.s);
    
    delay(500);
    ble.print("AT+BLEUARTTX=");
    ble.println("Alarm Added");
    setCurrentAlarm();
    

}

void setTimeInput(String cmd)
{
    int index;
    index = cmd.indexOf('h');
    int input_hours    = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('m');
    int input_minutes  = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('s');
    int input_seconds  = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('y');
    int input_years    = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('o');
    int input_months   = cmd.substring(index+1,index+3).toInt();
    index = cmd.indexOf('d');
    int input_days     = cmd.substring(index+1,index+3).toInt();
    debuger("Setting time!");

    debuger((String)input_hours);
    debuger((String)input_minutes);
    debuger((String)input_seconds);
    rtc.setTime(input_hours, input_minutes, input_seconds);
    rtc.setDate(input_days, input_months, input_years);
 }

MedAlarm getMedAlarm(int id)
{
    switch (id)
    {
        case 1:
            debuger("get alarm 1");
            //alarm1.enable = 1;
            return alarm1;
            break;
        case 2:
            return alarm2;
            break;
        case 3:
            return alarm3;
            break;
        case 4:
            return alarm4;
            break;
        case 5:
            return alarm5;
            break;
        case 6:
            return alarm6;
            break;
        default:
            return alarm1;
            break;
    }
}


void setMedAlarm(int id,int h, int m, int s,int en)
{
    switch (id)
    {
        case 1:
            debuger("setting alarm 1");
            alarm1.h = h;
            alarm1.m = m;
            alarm1.s = s;
            alarm1.enable = en;
            break;
        case 2:
            debuger("setting alarm 2");
            alarm2.h = h;
            alarm2.m = m;
            alarm2.s = s;
            alarm2.enable = en;
            break;
        case 3:
            debuger("setting alarm 3");
            alarm3.h = h;
            alarm3.m = m;
            alarm3.s = s;
            alarm3.enable = en;
            
            break;
        case 4:
            debuger("setting alarm 4");
            alarm4.h = h;
            alarm4.m = m;
            alarm4.s = s;
            alarm4.enable = en;
            break;
        case 5:
            debuger("setting alarm 5");
            alarm5.h = h;
            alarm5.m = m;
            alarm5.s = s;
            alarm5.enable = en;
            break;
        case 6:
            debuger("setting alarm 6");
            alarm6.h = h;
            alarm6.m = m;
            alarm6.s = s;
            alarm6.enable = en;
            break;
        default:
            debuger("The alarm id is 1-6");
            break;
    }
}

void setCurrentAlarm(void)
{
    MedAlarm tmp = getMedAlarm(currentAlarm);
    debuger("Setting Current Alarm");
    int i = 0;
    while(tmp.enable==0 && i < 7)
    {
        currentAlarm++;
        if(currentAlarm==7) currentAlarm = 1;
        tmp = getMedAlarm(currentAlarm);
        i++;
        debuger("FINDING ALARM");
        debuger((String)tmp.id);
        if(tmp.enable==0){
        debuger("is not enabled");
        }else
        {
          debuger("is enabled");
          }
        
    }

    if(tmp.enable){
    debuger("The avialable alarms is : ");
    debuger(String(currentAlarm));
    
    rtc.setAlarmTime(tmp.h,tmp.m,tmp.s);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(alarmMatch);

    debuger("The alarm h/m/s is");
    debuger((String)tmp.h);
    debuger((String)tmp.m);
    debuger((String)tmp.s);

    debuger("SUCCESSFULLY SETTED THIS ALARM");
    }else
    {
      debuger("No alarm is avilable");
    }
}


void alarmMatch()
{
  digitalWrite(13, HIGH);
  MedAlarm tmp = getMedAlarm(currentAlarm);
  if(tmp.id<=3){
    analogWrite(motor1,speed);
    debuger("Motor1 ON");
  }else
  {
    analogWrite(motor2,speed);
    debuger("Moter2 ON");
  }

  debuger("Alarm Match!!!****************");
  debuger("The Matched alarm is");
  debuger((String)currentAlarm);
  currentAlarm++;
  if(currentAlarm==6) currentAlarm = 1;
  debuger("go to next alarm");
  debuger((String)currentAlarm);
  setCurrentAlarm();
}

void updateAlarmTime()
{     alarm_seconds = rtc.getSeconds();
      alarm_minutes = rtc.getMinutes();
      alarm_hours   = rtc.getHours();
}

int find_text(String needle, String haystack) {
  int foundpos = -1;
  for (int i = 0; i <= haystack.length() - needle.length(); i++) {
    if (haystack.substring(i,needle.length()+i) == needle) {
      foundpos = i;
    }
  }
  return foundpos;
}
void buttonClicked()
{
  digitalWrite(13, LOW);
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
  
}
