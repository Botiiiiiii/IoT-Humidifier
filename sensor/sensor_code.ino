#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <Arduino.h>

char* ssid = "SF_LOW";
char* password = "t1v0r20!^";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.0.3:7579/Mobius";
String Name = "Sensor2";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

unsigned long Next_Setted_Time = 0;

int infrared = 0;
int temp = 1;

#define PIN_DHT D7
#define PIN_INF D6

DHT DHTsensor(PIN_DHT, DHT11);

void createAE(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=2");
      http.addHeader("X-M2M-RI", "adnae/" + Name);
      http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
      String IP = WiFi.localIP().toString();  
      String httpRequestData = "";
      httpRequestData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      httpRequestData += "<m2m:ae xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"" + Name + "\">\n";
      httpRequestData += "<api>" + Name + "</api>\n";
      httpRequestData += "<rr>false</rr>\n";
      httpRequestData += "</m2m:ae>\n";
      Serial.println(httpRequestData);
          
      int httpResponseCode = http.POST(httpRequestData);
      
      Serial.print("HTTP Response code: ");

      Serial.println(httpResponseCode);
      // Free resources
      http.end();
    }
}

void createCnt(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // DATA cnt 생성
      http.begin(serverName + "/" + Name);
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=3");
      http.addHeader("X-M2M-RI", "adnae/" + Name);
      http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
      String httpRequestData = "";
      httpRequestData += "<m2m:cnt xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"DATA\">\n";
      httpRequestData += "<mni>100000</mni>";
      httpRequestData += "</m2m:cnt>";
      Serial.println(httpRequestData);
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      // DATA/humidity 생성
      http.begin(serverName + "/" + Name + "/DATA");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=3");
      http.addHeader("X-M2M-RI", "adnae/" + Name);
      http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
      httpRequestData = "";
      httpRequestData += "<m2m:cnt xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"humidity\">\n";
      httpRequestData += "<mni>100000</mni>";
      httpRequestData += "</m2m:cnt>";
      Serial.println(httpRequestData);
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      // DATA/loaction 생성
      http.begin(serverName + "/" + Name + "/DATA");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=3");
      http.addHeader("X-M2M-RI", "adnae/" + Name);
      http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
      httpRequestData = "";
      httpRequestData += "<m2m:cnt xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"local\">\n";
      httpRequestData += "<mni>100000</mni>";
      httpRequestData += "</m2m:cnt>";
      Serial.println(httpRequestData);
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      // DATA/humidity/current 생성
      http.begin(serverName + "/" + Name + "/DATA/humidity");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=3");
      http.addHeader("X-M2M-RI", "adnae/" + Name);
      http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
      httpRequestData = "";
      httpRequestData += "<m2m:cnt xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"current\">\n";
      httpRequestData += "<mni>100000</mni>";
      httpRequestData += "</m2m:cnt>";
      Serial.println(httpRequestData);
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
      // DATA/humidity/target 생성
      http.begin(serverName + "/" + Name + "/DATA/humidity");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=3");
      http.addHeader("X-M2M-RI", "adnae/" + Name);
      http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
      httpRequestData = "";
      httpRequestData += "<m2m:cnt xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"target\">\n";
      httpRequestData += "<mni>100000</mni>";
      httpRequestData += "</m2m:cnt>";
      Serial.println(httpRequestData);
      httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();   
    }
}

void sendHumidity(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      float Humidity = DHTsensor.readTemperature();
      Serial.println(Humidity);
 
      if ( !isnan(Humidity)){
        http.begin(serverName + "/" + Name + "/DATA/humidity/current");
        http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=4");
        http.addHeader("X-M2M-RI", "adnae/1234");
        http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
        String httpRequestData = "";
        httpRequestData += "<m2m:cin xmlns:m2m=\"http://www.onem2m.org/xml/protocols\">\n";
        httpRequestData += "<con>" + String(Humidity) + "</con>";
        httpRequestData += "</m2m:cin>";
        Serial.println(httpRequestData);
          
        int httpResponseCode = http.POST(httpRequestData);
      
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      
      // Free resources
      http.end();
    }
}

void sendInfrared(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
        http.begin(serverName + "/" + Name + "/DATA/local");
        http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=4");
        http.addHeader("X-M2M-RI", "adnae/" + Name);
        http.addHeader("X-M2M-Origin", "adn-ae/" + Name);
        String httpRequestData = "";
        httpRequestData += "<m2m:cin xmlns:m2m=\"http://www.onem2m.org/xml/protocols\">\n";
        httpRequestData += "<con>" + String(infrared) + "</con>";
        httpRequestData += "</m2m:cin>";
        Serial.println(httpRequestData);
          
        int httpResponseCode = http.POST(httpRequestData);
      
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      
      // Free resources
      http.end();
    }
}

void setup() {
  Serial.begin(9600); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  createAE();
  createCnt();
}

void loop() {
      unsigned long Now = millis();
      infrared = digitalRead(PIN_INF);
      if(Now >= Next_Setted_Time){
        sendHumidity();
        Next_Setted_Time = Now + 10000;
      }
      
      if(infrared != temp){
          //Serial.println(infrared);
          temp = infrared;
          sendInfrared();
      }
}
