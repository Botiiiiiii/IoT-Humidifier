#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>

#define Q_SIZE 20

// DEMO
// 센서 갯수
#define N_SENSOR 2

// 가습기의 상태코드
#define HUM_OFF 0
#define HUM_SLEEP 1
#define HUM_RUNNING 2
#define HUM_MOV 3


// 센서 상태코드
#define SEN_DEFAULT 0
#define SEN_WAIT 1

// 이동 command
#define VEH_STOP "Q0"
#define VEH_FORWARD "Q1"
#define VEH_LEFT_FRT "Q2"
#define VEH_RIGHT_FRT "Q3"
#define VEH_LEFT_BCK "Q4"
#define VEH_RIGHT_BCK "Q5"
#define VEH_BACKWARD "Q6"

int sensor_cnt = 0;      // 센서의 갯수
int state_humi; // 가습기 상태
int state_sen[Q_SIZE] = {0, };
String state_mov = VEH_STOP;

// setted room humidity
// 각 방의 설정 습도
float s_hum[Q_SIZE];

// 현재 센서의 습도
float c_hum[Q_SIZE];

// 현재 가습기의 위치
// location of Humidifier
// 센서1에 있으면 1이라고 함
int l_hum = 0;

// Destination_sensor queue
// 가야하는 목적지 센서 큐
// 0은 초기값 센서 1은 1, 센서2는 2
int destC_que[Q_SIZE] = {0, };
int destC_size = 0;
int destC_frt = 0;
int destC_rear = -1;

String serverName = "http://192.168.0.3:7579/Mobius";

// infrared sensor
int ir1 = D1;  //left
int ir2 = D3; //right
int rsw = D8;

// 현재 가습하고 있는 센서
// 큐에서 pop한 센서
int now_sensor = 0;

// Stop flag
int S_flag = 0;

// global time
unsigned long Time;

unsigned long Next_Setted_Time;
unsigned long Next_Current_Time;
unsigned long Next_Curloc_Time;
unsigned long Next_onoff_Time;

unsigned long Next_scon_Time;
unsigned long Next_stop_Time;
unsigned long Next_Debug_Time;

char* ssid = "SF_LOW";
char* password = "t1v0r20!^";

void con_veh(String state){
  if(state_mov != state){
    Serial.print("[Debug] : ");
    Serial.println(state_mov);
    Serial.println(state);
    state_mov = state;
  }
}

String jsonParse(String str, String findstr){
  int index = str.indexOf(findstr) + findstr.length() + 3;
  int index2 = str.indexOf('"', index);
  return str.substring(index, index2);
}

void queue_init(){
  int destC_size = 0;
  int destC_frt = 0;
  int destC_rear = -1;
}

void init_state(){
  for(int i = 0; i < Q_SIZE; i++){
    state_sen[i] = 0;
  }
  Serial.println(VEH_STOP);
  state_mov = VEH_STOP;
}

void queue_append(int dest){
  destC_rear++;
  destC_size++;
  destC_rear %= Q_SIZE;
  destC_que[destC_rear] = dest; 
}

int queue_pop(){
  int dest;
  
  destC_size--;
  dest = destC_que[destC_frt++];
  destC_frt %= Q_SIZE;
  
  return dest;
}

int sub_getNumofsensor(){
  return N_SENSOR;
}

void create_AE(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=2");
      http.addHeader("X-M2M-RI", "adnae/Humidifier");
      http.addHeader("X-M2M-Origin", "adn-ae/Humidifier");
      String IP = WiFi.localIP().toString();  
      String httpRequestData = "";
      httpRequestData += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      httpRequestData += "<m2m:ae xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"Humidifier\">\n";
      httpRequestData += "<api>Humidifier</api>\n";
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

void create_CNT(){
  if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      
      // DATA cnt 생성
      http.begin(serverName + "/Humidifier");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+xml;ty=3");
      http.addHeader("X-M2M-RI", "adnae/Humidifier");
      http.addHeader("X-M2M-Origin", "adn-ae/Humidifier");
      String httpRequestData = "";
      httpRequestData += "<m2m:cnt xmlns:m2m=\"http://www.onem2m.org/xml/protocols\" rn=\"power\">\n";
      httpRequestData += "<mni>100000</mni>";
      httpRequestData += "</m2m:cnt>";
      Serial.println(httpRequestData);
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
  }
}

// 서버로부터 센서별로 설정된 습도 읽어오기
float sub_getsettedHumidity(int n_sen){
  float hum;

  char buf[5];

  sprintf(buf, "%d", n_sen);
  
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName + "/Sensor" +  String(buf) + "/DATA/humidity/target/la"); 
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-M2M-RI","/Mobius");
    http.addHeader("X-M2M-Origin","SOrigin");
    int httpResponseCode = http.GET();
    if(httpResponseCode == 200){
      // Serial.println(payload); debug용 
      String payload = http.getString();
      hum = jsonParse(payload, "con").toFloat(); // 문자열을 float 형으로 전환
    }
    else{
      Serial.println("Response Code is not 200 !!");
    }
    http.end();
  }   
  
  // n_sen은 센서 번호 1 아니면 2

  //Serial.print("[Debug] Setted Humidity : ");
  //Serial.println(hum);
  
  return hum;
}

// 서버로부터 현재 센서의 습도 읽어오기
float sub_getcurHumidity(int n_sen){
  float hum;
  char buf[5];

  sprintf(buf, "%d", n_sen);
  
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName + "/Sensor" +  String(buf) + "/DATA/humidity/current/la");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-M2M-RI","/Mobius");
    http.addHeader("X-M2M-Origin","SOrigin");
    int httpResponseCode = http.GET();
    if(httpResponseCode == 200){
      // Serial.println(payload); debug용 
      String payload = http.getString();
      hum = jsonParse(payload, "con").toFloat(); // 문자열을 float 형으로 전환
    }
    else{
      Serial.println("Response Code is not 200 !!");
    }
    http.end();
  }   
  // n_sen은 센서 번호 1 아니면 2

  //Serial.print("[Debug] Current Humidity : ");
  //Serial.println(hum);
  
  return hum;
}

int sub_curLocation(){
  int fl;
  int loc = 0;
  int len = N_SENSOR;
  
  // for i = 0 to sensor_cnt:
  // 읽어오면서 센서에서 가습기 위치하는지 판단
  // i번째 센서에 가습기가 있으면 loc = i하고 break;
  if(WiFi.status() == WL_CONNECTED){
        HTTPClient http;
        for(int i = 1 ; i <= len ; i++){
          char buf[5];
          sprintf(buf, "%d", i);
          http.begin(serverName + "/Sensor" + String(buf) + "/DATA/local/la");
          http.addHeader("Content-Type", "application/json");
          http.addHeader("X-M2M-RI","/Mobius");
          http.addHeader("X-M2M-Origin","SOrigin");

          int httpResponseCode = http.GET();
    
          if(httpResponseCode == 200){
            // Serial.println(payload); debug용 
            String payload = http.getString();
            fl = jsonParse(payload, "con").toInt(); // 문자열을 float 형으로 전환
            if(fl == 0){
              loc = i;
              break;
            }
          }
          else{
            Serial.println("Response Code is not 200 !!");
          }
          http.end();
        }
   }

  // Serial.print("[Debug] current location : ");
  // Serial.println(loc);
  
  return loc;
}

String location(String dest){
    String fl;
    if(WiFi.status() == WL_CONNECTED){
        HTTPClient http;
        http.begin(serverName + "/Sensor" + dest + "/DATA/local/la");
        http.addHeader("Content-Type", "application/json");
        http.addHeader("X-M2M-RI","/Mobius");
        http.addHeader("X-M2M-Origin","SOrigin");

        int httpResponseCode = http.GET();
    
        if(httpResponseCode == 200){
          String payload = http.getString();
          fl = jsonParse(payload, "con");
          //Serial.println(fl);
          return fl;
        }
        else{
          Serial.println("Response Code is not 200 !!");
        }
          http.end();
    }
    //Serial.println("location return 0");
    return "0";
}

int sub_onoff(){
  int power;
  
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(serverName + "/Humidifier/power/la");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-M2M-RI","/Mobius");
    http.addHeader("X-M2M-Origin","SOrigin");
    int httpResponseCode = http.GET();
    if(httpResponseCode == 200){
      // Serial.println(payload); debug용 
      String payload = http.getString();
      power = jsonParse(payload, "con").toInt(); // 문자열을 float 형으로 전환
    }
    else{
      Serial.println("Response Code is not 200 !!");
    }
    http.end();
  }

  // Serial.print("[Debug] power : ");
  // Serial.println(power);
  
  return power;
}

int veh_movto(int src, int dest){
  /*
  Serial.println("[Debug] Moving");
  Serial.print(src);
  Serial.println(dest);
  */
  if(dest == 2){
    return moveforward();
  }
  else if (dest == 1){
    return movebackward();
  }
  return 0;
}

int moveforward(){
    int left;
    int right;
    unsigned long Now = 0;
    unsigned long Next = 0;
    //Serial.println("Start move");
    while(1){
      Now = millis(); 
      left = digitalRead(ir1);
      right = digitalRead(ir2);
      Serial.print("[Sensor] : ");
      Serial.print(left);
      Serial.println(right);
      if(Now >= Next){
        Next += 100;
        if(location("2") == "0"){
          con_veh(VEH_STOP);
          return 1;
        }
      }
      if(left == 1 && right == 1){
        con_veh(VEH_FORWARD);
      }
      else if(left == 0 && right == 0){
        //Serial.println("Line out!");
        con_veh(VEH_STOP);
        return 0;
      }
      else if(left == 0){
        // 왼쪽이 감지되었으므로 지금 가습기가 오른쪽으로 치우쳐져 있음
        con_veh(VEH_RIGHT_FRT);
      }
      else if(right == 0){
        // 오른쪽이 감지 되었으므로 지금 가습기가 왼쪽으로 치우쳐져 있음
        con_veh(VEH_LEFT_FRT);
      }
    }
    //Serial.println(VEH_STOP);
    con_veh(VEH_STOP);
    return 0;
}

int movebackward(){
    int left = digitalRead(ir1);
    int right = digitalRead(ir2);

    while(sub_curLocation() == 0){
      left = digitalRead(ir1);
      right = digitalRead(ir2);
      if(left == 0 && right == 0){
        con_veh(VEH_FORWARD);
      }
      else if(left == 1 && right == 1){
        //Serial.println("Line out!");
        con_veh(VEH_STOP);
        return 0;
        
      }
      else if(right == 1){
        con_veh(VEH_LEFT_BCK);
        
      }
      else if(left == 1){
        con_veh(VEH_RIGHT_BCK);
      
      }  
    }
    return 1;
}

void setup(){
  state_humi = HUM_OFF;
  Serial.begin(9600);

  pinMode(rsw, OUTPUT);

  pinMode(ir1, INPUT);
  pinMode(ir2, INPUT);

  // Connect WIFI
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
   
  create_AE();
  create_CNT();
  
  Time = 0;
  Next_Setted_Time = 0;
  Next_Current_Time = 0;
  Next_Curloc_Time = 0;
  Next_onoff_Time = 0;
  Next_stop_Time = 0;
  Next_scon_Time = 0;
  Next_Debug_Time = 0;

  l_hum = sub_curLocation();
}

void loop(){
  int i = 0;
  unsigned long Now = millis();

  if(Now >= Next_onoff_Time){
    Next_onoff_Time += 1000;
    if(sub_onoff()){
      // Switch On Humi
      //digitalWrite(rsw, HIGH);
      if(state_humi == HUM_OFF){
        queue_init();
        init_state();
        state_humi = HUM_SLEEP;
      }
    }
    else{
      // Switch Off Humi
      digitalWrite(rsw, LOW);
      state_humi = HUM_OFF;
    }
  }
  
  // ================== 구독 기능 ======================
  // 서버로부터 데이터를 받아오는 기능
  
  
  // 센서의 갯수 갱신하기
  sensor_cnt = sub_getNumofsensor();

  //Serial.println(Now);
  
  // 원하는 센서마다 설정 된 습도값 읽어오기
  if(Now >= Next_Setted_Time){
    Next_Setted_Time += 10000;
    for(i = 0; i < sensor_cnt; i++){
      s_hum[i] = sub_getsettedHumidity(i+1);
    }
  }

  // 현재 센서의 습도 계속 읽어와서 갱신하기
  if(Now >= Next_Current_Time){
    Next_Current_Time += 10000;
    for(i = 0; i < sensor_cnt; i++){
      c_hum[i] = sub_getcurHumidity(i+1);
    }
  }
  
  // ========== Debug Code ============
  /*
  if(Now >= Next_Debug_Time){
    Serial.print("[Deubg] Queue : [");
    for(int i = 0; i < destC_size; i++){
    Serial.print(destC_que[(destC_frt+i)%Q_SIZE]);
    Serial.print(' ');
    }
    Serial.println(']');
    
    Serial.print("[Debug] l_hum : ");
    Serial.println(l_hum);
    
    Serial.print("[Debug] State : ");  
    switch(state_humi){
      case HUM_OFF:{
        Serial.println("HUM_OFF");
        break;
      }
      case HUM_SLEEP:{
        Serial.println("HUM_SLEEP");
        break;
      }
      case HUM_MOV:{
        Serial.println("HUM_MOV");
        Serial.print("[dest] : ");
        Serial.println(destC_que[destC_frt]);
        break;
      }
      case HUM_RUNNING:{
        Serial.println("HUM_RUNNING");
        Serial.print("[n_sensor] : ");
        Serial.println(now_sensor);
        break;
      }
    }
    Next_Debug_Time += 3000;
  }
  */


  // ================== 작동부 ======================
  /* --- work flow
  default : HUM_SLEEP - 대기상태
    현재 센서들의 습도와 설정 습도를 비교하여
    destC_que에 센서 넣음
    
    현재 위치가 목적지 위치랑 다를 때,
    -> 상태 : HUM_MOV - 이동중
    가습이 필요한 방이 없으면
    -> 상태 : HUM_SLEEP - 그대로
  
  상태 : HUM_MOV
    센서에 도착할때 까지 계속 이동함
    도착하면 
    -> 상태 : HUM_RUNNING
    -> dsetC_que에서 현재 센서 삭제
  
  상태 : HUM_RUNNING - 가습기가 작동중일 때,
    현재 센서의 습도와 설정 습도를 비교하여
    현재 습도가 설정 습도에 도달하면,
    -> 상태 : HUM_SLEEP
  */
  switch(state_humi){
    case HUM_OFF:{
    if(Now >= Next_stop_Time){
      con_veh(VEH_STOP);
      Next_stop_Time += 1000;
    }
    break;
  }
    case HUM_SLEEP:{
      //Serial.println(VEH_STOP);
      if(Now >= Next_stop_Time){
      Next_stop_Time += 1000;
        con_veh(VEH_STOP);
      }
      for(i = 0; i < sensor_cnt; i++){
        if(state_sen[i] == SEN_DEFAULT && s_hum[i] > c_hum[i]){
          queue_append(i+1);
          state_sen[i] = SEN_WAIT;
        }
      }
      if(destC_size != 0){
        if(l_hum != destC_que[destC_frt]){
          state_humi = HUM_MOV;
          //Serial.print("[Debug] l_hum : ");
          //Serial.println(l_hum);
          //Serial.print("[Debug] dest : ");
          //Serial.println(destC_que[destC_frt]);
        }
        if(l_hum == destC_que[destC_frt]){
          state_humi = HUM_RUNNING;
          now_sensor = queue_pop();
        }
      }
      break;
    }
    
    case HUM_MOV:{
      if(veh_movto(l_hum, destC_que[destC_frt])){
        state_humi = HUM_RUNNING;
        l_hum = sub_curLocation();
        now_sensor = queue_pop();
      }
      else{
        //Serial.println("error detect!");
      }
      break;
    }
    case HUM_RUNNING:{
      //Serial.println(VEH_STOP);
      digitalWrite(rsw, HIGH);
      if(s_hum[now_sensor-1] <= c_hum[now_sensor-1]){
        // 가습기 off하는 code
        digitalWrite(rsw, LOW);
        state_humi = HUM_SLEEP;
        state_sen[now_sensor-1] = SEN_DEFAULT;
      }
      break;
    }
    default:{
      state_humi = HUM_SLEEP;
      break;   
    } 
  }
}
