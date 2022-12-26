#include <TimeLib.h>
#include <DS1307RTC.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Nextion.h"
#include <EEPROM.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 7
#define PH A0
#define samplingInterval 20
#define samplingInterval_1 20
#define samplingInterval_2 20
#define printInterval 800
#define ArrayLenth  40
#define interval 1000
#define interval1 1000

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

const int chipSelect = 4; 

float Celsius;
float Offset_suhu;

int pHArray[ArrayLenth];
int pHArrayIndex = 0;
float Offset_ph;
static float pH;
static float teganganph;

int TSS = A1;
float tss;
float Offset_tss;
float tegangantss;

String data_string;
String txt_month;
String txt_day;
String txt_hour;
String txt_minute;
String txt_second;

static unsigned long time;
static unsigned long time_for_action;
static unsigned long time_for_action_1;
static unsigned long samplingTime = millis();
static unsigned long samplingTime_1 = millis();
static unsigned long samplingTime_2 = millis();
static unsigned long printTime_1 = millis();
unsigned long millis_string;
unsigned long millis_kirimdata;

File myFile;

NexButton s1 = NexButton(2, 3, "s1");
NexButton s2 = NexButton(2, 4, "s2");
NexButton p1 = NexButton(2, 6, "p1");
NexButton p2 = NexButton(2, 7, "p2");
NexButton t1 = NexButton(2, 9, "t1");
NexButton t2 = NexButton(2, 10, "t2");

char buffer[100] = {0};
NexTouch *nex_listen_list[] = {
  &s1,    &s2,  &p1,  &p2,  &t1,  &t2,
  NULL
};

void s2PushCallback(void *ptr) {
  Offset_suhu += 0.1;
  memset(buffer, 0, sizeof(buffer));
  itoa(Offset_suhu, buffer, 10);
  EEPROM.write(Offset_suhu, Celsius);
}

void s1PushCallback(void *ptr) {
  Offset_suhu -= 0.1;
  memset(buffer, 0, sizeof(buffer));
  itoa(Offset_suhu, buffer, 10);
  EEPROM.write(Offset_suhu, Celsius);
}

void p2PushCallback(void *ptr) {
  Offset_ph += 0.1;
  memset(buffer, 0, sizeof(buffer));
  itoa(Offset_ph, buffer, 10);
  EEPROM.write(Offset_ph, pH);
}

void p1PushCallback(void *ptr) {
  Offset_ph -= 0.1;
  memset(buffer, 0, sizeof(buffer));
  itoa(Offset_ph, buffer, 10);
  EEPROM.write(Offset_ph, pH);
}

void t2PushCallback(void *ptr) {
  Offset_tss += 0.1;
  memset(buffer, 0, sizeof(buffer));
  itoa(Offset_tss, buffer, 10);
  EEPROM.write(Offset_tss, tss);
}

void t1PushCallback(void *ptr) {
  Offset_tss -= 0.1;
  memset(buffer, 0, sizeof(buffer));
  itoa(Offset_tss, buffer, 10);
  EEPROM.write(Offset_tss, tss);
}

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);

  while (!Serial) ;
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");
    
  if (!SD.begin(chipSelect)) {
  //if (!SD.begin(10, 11, 12, 13)) {              
    Serial.println("Gagal Membuka Micro SD!");
    return;
  }
  Serial.println("Berhasil Membuka Micro SD");

  DS18B20.begin();
  delay(2000);

  nexInit();
  s1.attachPush(s1PushCallback);
  s2.attachPush(s2PushCallback);
  p1.attachPush(p1PushCallback);
  p2.attachPush(p2PushCallback);
  t1.attachPush(t1PushCallback);
  t2.attachPush(t2PushCallback);

  time_for_action   = 0;
  time_for_action_1 = 0;

  millis_kirimdata  = 0;
}

void loop() {
  time = millis();
  if (millis () > time_for_action) {
    time_for_action = millis() + (unsigned long)interval;

    void loggingDS18B20();
    if (millis() - samplingTime > samplingInterval) {
      DS18B20.requestTemperatures();
      Celsius = (DS18B20.getTempCByIndex(0));
      String command = "displaysuhu.txt=\"" + String(Celsius + Offset_suhu) + "\"";
      Serial.print(command);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      samplingTime = millis();
    }

    void loggingPH();
    if (millis() - samplingTime_1 > samplingInterval_1) {
      pHArray[pHArrayIndex++] = analogRead(PH);
      if (pHArrayIndex == ArrayLenth)pHArrayIndex = 0;
      teganganph = (averageData() / 1023) * 5.0;
      pH = (3 * teganganph)- 0.10;
      String command = "displayph.txt=\"" + String(pH + Offset_ph) + "\"";
      Serial.print(command);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      samplingTime_1 = millis();
    }

    void loggingKEKERUHAN();
    if (millis() - samplingTime_2 > samplingInterval_2) {
      TSS = analogRead(A1);
      tegangantss = TSS * (5.0 / 1023.0);
      //tss = (-16.97 * tegangantss) + 69.29;//ntu
      //tss = (-5.839* tegangantss) + 45.43;//rayon
      tss = (-250.8* tegangantss) + 871.5;//tempe
      String command = "displaytss.txt=\"" + String(tss + Offset_tss) + "\"";
      Serial.print(command);
      Serial.write(0xff);
      Serial.write(0xff);
      Serial.write(0xff);
      samplingTime_2 = millis();
    }
  }

  if (millis () > time_for_action_1) {
    time_for_action_1 = millis() + (unsigned long)interval1;
    (timeStatus() == timeSet);
    myFile = SD.open ("logger.txt", FILE_WRITE);
    myFile.print(year());
    myFile.print("-");
    if (month() < 10) {
      txt_month = String("0") + month();
    }
    else {
      txt_month = String(month());
    }
    myFile.print(txt_month);
    myFile.print("-");
    if (day() < 10) {
      txt_day = String("0") + day();
    }
    else {
      txt_day = String(day());
    }
    myFile.print(txt_day );
    myFile.print(" ");
    if (hour() < 10) {
      txt_hour = String("0") + hour();
    }
    else {
      txt_hour = String(hour());
    }
    myFile.print(txt_hour);
    myFile.print(":");
    if (minute() < 10) {
      txt_minute = String("0") + minute();
    }
    else {
      txt_minute = String(minute());
    }
    myFile.print(txt_minute);
    myFile.print(':');
    if (second() < 10) {
      txt_second = String("0") + second();
    }
    else {
      txt_second = String(second());
    }
    myFile.print(txt_second);

    myFile.print(",");
    myFile.print(Celsius + Offset_suhu);
    myFile.print(",");
    myFile.print(pH + Offset_ph);
    myFile.print(",");
    myFile.print(tss + Offset_tss);
    myFile.println();
    myFile.close();
  }
  nexLoop(nex_listen_list);

  if (millis () - millis_string >= 250) {
    millis_string = millis();
    data_string =
      String (year())+ "," +
      String (txt_month)+ "," + 
      String (txt_day)+ "," + 
      String (txt_hour)+ "," + 
      String (txt_minute)+ "," + 
      String (txt_second)+ "," +
      String (Celsius + Offset_suhu) + "," +
      String (pH + Offset_ph) + "," +
      String (tss + Offset_tss);
  }

  if (millis() - millis_kirimdata >= 500) {
    millis_kirimdata = millis();
    Serial3.print(data_string);
    Serial3.print('\n');
  }
}

double averageData() {
  double temp = 0 ;
  for ( int i = 0 ; i < 500 ; i++ ) {
    temp += analogRead(A0);
  }
  return temp / 500 ;
}
