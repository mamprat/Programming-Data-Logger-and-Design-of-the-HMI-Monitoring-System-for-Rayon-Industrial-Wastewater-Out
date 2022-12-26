#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#define USE_SERIAL Serial
SoftwareSerial NodeMcuRecv(D5, D6);

ESP8266WiFiMulti WiFiMulti;
HTTPClient http;

float celsius;
float ph;
float tss;

int year;
int txt_month;
int txt_day;
int txt_hour;
int txt_minute;
int txt_second;

String bulan;
String hari;
String jam;
String menit;
String detik;

String postData;
String data;

unsigned long millis_kirimdata = 0;

String url = "http://monitoringoutlet.com/inputsensor.php";
String var1 = "waktu=";
String var2 = "&celsius=";
String var3 = "&ph=";
String var4 = "&tss=";

void setup() {
  USE_SERIAL.begin(9600);
  NodeMcuRecv.begin(9600);
  USE_SERIAL.setDebugOutput(false);

  for (uint8_t t = 4; t > 0; t--) {
    //    USE_SERIAL.printf("[SETUP] Tunggu %d...\n", t);
    USE_SERIAL.flush();
    delay(500);
  }

  WiFi.mode(WIFI_STA);
  //WiFiMulti.addAP("AndroidAP1210", "bayarajadulu"); // Sesuaikan SSID dan password ini
  //  WiFiMulti.addAP("KoriqSquad", "GalonPecah11"); // Sesuaikan SSID dan password ini
  WiFiMulti.addAP("ECC", "Setandame");
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    if (NodeMcuRecv.available()) {
      year = NodeMcuRecv.parseInt();
      txt_month = NodeMcuRecv.parseInt();
      bulan = String (txt_month);
      if (txt_month < 10) {
        bulan = String("0") + bulan;
      }

      txt_day = NodeMcuRecv.parseInt();
      hari = String (txt_day);
      if (txt_day < 10) {
        hari = String("0") + hari;
      }

      txt_hour = NodeMcuRecv.parseInt();
      jam = String (txt_hour);
      if (txt_hour < 10) {
        jam = String("0") + jam;
      }

      txt_minute = NodeMcuRecv.parseInt();
      menit = String (txt_minute);
      if (txt_minute < 10) {
        menit = String("0") + menit;
      }

      txt_second = NodeMcuRecv.parseInt();
      detik = String (txt_second);
      if (txt_second < 10) {
        detik = String("0") + detik;
      }

      celsius = NodeMcuRecv.parseFloat();
      ph = NodeMcuRecv.parseFloat();
      tss = NodeMcuRecv.parseFloat();

      if ((millis() - millis_kirimdata) >= 1000) {
        millis_kirimdata = millis();
        if (NodeMcuRecv.read() == '\n') {
          postData =
            var1 + (String) year + "-" +
            bulan + "-" +
            hari + " " +
            jam + ":" +
            menit + ":" +
            detik + " " +
            var2 + (String) celsius +
            var3 + (String) ph +
            var4 + (String) tss ;

          http.begin( url );
          http.addHeader("Content-Type", "application/x-www-form-urlencoded");

          int httpCode = http.POST(postData);
          USE_SERIAL.println(postData);
          http.end();
        }
      }
    }
  }
}
