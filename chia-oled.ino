#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <Arduino_JSON.h>

// дефайн перед подключением либы - использовать microWire (лёгкая либа для I2C)
//#define USE_MICRO_WIRE

// дефайн перед подключением либы - скорость SPI
//#define OLED_SPI_SPEED 4000000ul

#include <GyverOLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

const char* ssid = "YOUR_NETWORK_NAME";                          //from credentials.h file
const char* password = "YOUR_NETWORK_PASSWORD"; 
const char* serverName1 = "http://YOUR-IP:8927/farms/";
const char* serverName2 = "http://YOUR-IP:8927/wallets/";
const char* accessToken = "";

WiFiClient wifiClient;

String sensorReadings1;
String sensorReadings2;
String d1;
String d2;
// Change this to fit the size of your oled
//GyverOLED<SSD1306_128x32, OLED_BUFFER> oled;
//GyverOLED<SSD1306_128x32, OLED_NO_BUFFER> oled;
// GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;
GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;
//GyverOLED<SSD1306_128x64, OLED_BUFFER, OLED_SPI, 8, 7, 6> oled;
//GyverOLED<SSH1106_128x64> oled;


// битмап создан в ImageProcessor https://github.com/AlexGyver/imageProcessor
// с параметрами вывода vertical byte (OLED)
const uint8_t bitmap_32x32[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xC0, 0xE0, 0xF0, 0x70, 0x70, 0x30, 0x30, 0x30, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF0, 0x70, 0x30, 0x30, 0x20, 0x00, 0x00,
  0x00, 0x30, 0x78, 0xFC, 0x7F, 0x3F, 0x0F, 0x0F, 0x1F, 0x3C, 0x78, 0xF0, 0xE0, 0xC0, 0x80, 0x80, 0x80, 0x40, 0xE0, 0xF0, 0xF8, 0xFC, 0xFF, 0x7F, 0x33, 0x13, 0x1E, 0x1C, 0x1C, 0x0E, 0x07, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF9, 0xF7, 0xEF, 0x5F, 0x3F, 0x7F, 0xFE, 0xFD, 0xFB, 0xF1, 0xE0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x1E, 0x33, 0x33, 0x1F, 0x0F, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x1F, 0x0E, 0x04, 0x00, 0x00, 0x00, 0x00,
};


void setup() {
  Serial.begin(115200);
  oled.setContrast(15);
  oled.clear();
  WiFi.begin(ssid, password);
}


void loop() {
  oled.init();
  if (WiFi.status() == WL_CONNECTED) {
    sensorReadings1 = httpGETRequest(serverName1);
    sensorReadings2 = httpGETRequest(serverName2);

    // JSONVar myObject = JSON.parse(sensorReadings);
    // Stream& input;

    StaticJsonDocument<512> doc1;
    StaticJsonDocument<768> doc2;

    deserializeJson(doc1, sensorReadings1);
    deserializeJson(doc2, sensorReadings2);

    // These can be modified to get the results you want from your API.
    // You can explore your Machinaris API at http://YOUR_IP:8927
    // The Assistant-tool on the ArduinoJson website can be helptful 
    // figuring out how to get the correct variables
    JsonObject root_0 = doc1[0];
    int plot_count = root_0["plot_count"]; // 50
    int total_chia = root_0["total_chia"]; // 0
    const char* updated_at = root_0["updated_at"]; // "2021-08-22T22:07:50.139984"

    JsonObject root_1 = doc2[0];
    String details = root_1["details"]; // "Wallet height: 756249\nSync status: ..
    d1 = getStringPartByNr(details, '\n', 4);
    d2 = getStringPartByNr(d1, ':', 1);
    d2.trim();

    // --------------------------
    
    oled.clear();   // очистить дисплей (или буфер)
    oled.update();  // обновить. Только для режима с буфером! OLED_BUFFER
    

    // --------------------------
    oled.setCursorXY(47, 5);           // курсор в 0,0
    oled.setScale(3);
    oled.print(String(plot_count));   // печатай что угодно: числа, строки, float, как Serial!
    oled.update();
    delay(200);
    oled.setCursorXY(1, 45); // курсор в (пиксель X, пиксель Y)
    oled.setScale(2);
    oled.print(d2.substring(0, 4) + " xch");
    oled.update();


    delay(600000);

  }
  else {
    oled.clear();
    oled.home();
    oled.print("WiFi Disconnected");
    oled.update();
    delay(2000);
  }
}

String httpGETRequest(const char* serverName) {
  HTTPClient http;
  http.begin(wifiClient, serverName);
  http.addHeader("x-access-token", accessToken);
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  return payload;
}

// splitting a string and return the part nr index split by separator
String getStringPartByNr(String data, char separator, int index) {
    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text

    for(int i = 0; i<data.length()-1; i++) {    //Walk through the text one letter at a time
        if(data[i]==separator) {
            //Count the number of times separator character appears in the text
            stringData++;
        } else if(stringData==index) {
            //get the text when separator is the rignt one
            dataPart.concat(data[i]);
        } else if(stringData>index) {
            //return text and stop if the next separator appears - to save CPU-time
            return dataPart;
            break;
        }
    }
    //return text if this is the last part
    return dataPart;
}