#define BLINKER_WIFI
#define BLINKER_ESP_SMARTCONFIG
#include <Blinker.h>
#include <DHT.h>
#include "OneButton.h"
#include <stdlib.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>


boolean oledstate = false;//oled显示状态

#define Oled_font u8g2_font_open_iconic_weather_4x_t //u8g2天气图像font库

//定义给水开关舵机位置
#define servo_on 30
#define servo_off 90

//oled屏幕构造器
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ D2, /* data=*/ D1, /* reset=*/ U8X8_PIN_NONE);

#define BLINKER_BUTTON_PIN D8 //定义 D8引脚 为WIFI重置接口
// button trigged when pin input level is LOW 即D8引脚为低电平时触发
OneButton wifi_reset(BLINKER_BUTTON_PIN, true);

//定义单片机接口
#define DHTTYPE DHT11   //定义DHT11类
#define DHTPIN D7       //定义 D7引脚 为DHT11数据接口
#define soil_pin 0     //定义 A0引脚 为土壤湿度传感器数据接口
#define SW D5          //定义 D5引脚 为震动传感器接口

//WiFi与硬件密匙
char auth[] = "b56e22dc440a";
char ssid[] = "MI8";
char pswd[] = "chaikaixin88";

//定义软件数字组件
BlinkerNumber HUMI("humi");  //空气湿度数据
BlinkerNumber TEMP("temp");  //空气温度数据
BlinkerNumber Soil("soil");  //土壤湿度数据

//定义按键组件
BlinkerButton BUTTON_WATER("water");//给水按键
BlinkerButton BUTTON_MODE("mode");//模式切换按键
BlinkerButton BUTTON_STATE("state");//状态切换按键


//定义文本组件
BlinkerText Time("time");//时间文本
BlinkerText Date("date");//日期文本
BlinkerText Weather("weather");//天气文本
BlinkerText Mode("mode_change");//模式显示文本
BlinkerText State("state_change");//状态显示文本


///DHT11传感器类
DHT dht(DHTPIN, DHTTYPE);
//创建一个舵机控制对象 
Servo myservo;  
 

//数据初始化
int oled_state = 0;//oled 显示状态
float humi_read = 0;//室内湿度
float temp_read = 0;//室内温度
int soil_read = 0;//土壤湿度
String o_date, o_time;//oled显示日期，时间
unsigned long SW_time = 0;// 震动传感器预留显示时间
int mode_change = 0;//模式切换
int state_change = 0;//状态切换

//土壤湿度上下限
int soil_upper, soil_lower;

//时间
String time_all;
String date_all;
int8_t mysec, mymin, myhour;//秒，分，时
int16_t mywday, mymday, mymonth, myyear;//星期，日，月，年
String week[7] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

/**************************天气*******************************/
unsigned long lastConnectionTime = 3600000;      //设定查询时间
const unsigned long postingInterval =  3600000;  // 一小时查询一次

const char* host = "api.seniverse.com";
const int httpPort = 80;
String reqUserKey1 = "SVuZggh2HZSrGVO1d";
String reqUserKey2 = "SMZ1nQF9iPLzO8o5T";
String reqLocation = "Xian";
String reqUnit = "c";

//现在
String results_0_now_text = "    ";  
String results_0_now_code = " "; 
String results_0_now_temperature = "  "; 
String results_0_last_update = "           " ;
int results_0_now_code_int;  
int results_0_now_temperature_int;
//明天
String results_1_ming_text = "    ";  
String results_1_ming_code = " "; 
String results_1_ming_high = "  ";
String results_1_ming_low = "  "; 
int results_1_ming_code_int;  
//后天
String results_2_hou_text = "    ";  
String results_2_hou_code = " "; 
String results_2_hou_high = "  ";
String results_2_hou_low = "  "; 
int results_2_hou_code_int;  

// 向心知天气服务器服务器请求信息并对信息进行解析
void httpRequest(String reqRes){
  WiFiClient client;

  // 建立http请求信息
  String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" + 
                              "Host: " + host + "\r\n" + 
                              "Connection: close\r\n\r\n";
  // 尝试连接服务器
  if (client.connect(host, 80)){
    Serial.println(" Success!");
    // 向服务器发送http请求信息
    client.print(httpRequest);  
    // 使用find跳过响应状态行、HTTP响应头
    client.find("\r\n\r\n");   
    // 利用ArduinoJson库解析心知天气响应信息
    parseInfo(client); 
  } else {
    Serial.println(" connection failed!");
  }   
  //断开客户端与服务器连接工作
  client.stop(); 
}
// 利用ArduinoJson库解析响应信息
void parseInfo(WiFiClient client){
  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
  DynamicJsonDocument doc(capacity);
  
  deserializeJson(doc, client);
  
  JsonObject results_0 = doc["results"][0];
  
  JsonObject results_0_now = results_0["now"];
  results_0_now_text.replace(results_0_now_text,results_0_now["text"]); 
  results_0_now_code.replace(results_0_now_code,results_0_now["code"]);
  results_0_now_temperature.replace(results_0_now_temperature,results_0_now["temperature"]);
  results_0_last_update.replace(results_0_last_update,results_0["last_update"]); 
 
  // 通过串口监视器显示以上信息
  results_0_now_code_int = results_0_now["code"].as<int>(); 
  results_0_now_temperature_int = results_0_now["temperature"].as<int>(); 
    
  
  BLINKER_LOG("======Weahter Now=======");
  BLINKER_LOG("Weather Now: ", results_0_now_text, " ",results_0_now_code_int );
  BLINKER_LOG("Temperature: ", results_0_now_temperature_int);
  BLINKER_LOG("Last Update: ", results_0_last_update);
  BLINKER_LOG("========================");
  results_0_now_temperature += "°C";
  Weather.print(results_0_now_text,results_0_now_temperature);
}

/*******************预测天气函数***********************/
// 向心知天气服务器服务器请求信息并对信息进行解析
void httpRequest_pre(String reqRes){
  WiFiClient client;

  // 建立http请求信息
  String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" + 
                              "Host: " + host + "\r\n" + 
                              "Connection: close\r\n\r\n";
  // 尝试连接服务器
  if (client.connect(host, 80)){
    Serial.println(" Success!");
    BLINKER_LOG(" Success!");
    // 向服务器发送http请求信息
    client.print(httpRequest);  
    // 使用find跳过响应状态行、HTTP响应头
    client.find("\r\n\r\n");   
    // 利用ArduinoJson库解析心知天气响应信息
    parseInfo_pre(client); 
  } else {
    Serial.println(" connection failed!");
  }   
  //断开客户端与服务器连接工作
  client.stop(); 
}

// 利用ArduinoJson库解析响应信息
void parseInfo_pre(WiFiClient client){
  //const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 230;
  DynamicJsonDocument doc(1600);
  
  deserializeJson(doc, client);
  
  JsonObject results_0 = doc["results"][0];
  
  JsonObject results_1_daily = results_0["daily"][1];
  JsonObject results_2_daily = results_0["daily"][2];
  results_1_ming_text.replace(results_1_ming_text,results_1_daily["text_day"]);
  results_1_ming_code.replace(results_1_ming_code,results_1_daily["code_day"]); 
  results_1_ming_high.replace(results_1_ming_high,results_1_daily["high"]);
  results_1_ming_low.replace(results_1_ming_low,results_1_daily["low"]);
  
  results_2_hou_text.replace(results_2_hou_text,results_2_daily["text_day"]); 
  results_2_hou_code.replace(results_2_hou_code,results_2_daily["code_day"]);
  results_2_hou_high.replace(results_2_hou_high,results_2_daily["high"]);
  results_2_hou_low.replace(results_2_hou_low,results_2_daily["low"]);
 
  // 通过串口监视器显示以上信息
  results_1_ming_code_int = results_1_daily["code_day"].as<int>(); 
  results_2_hou_code_int = results_2_daily["code_day"].as<int>();

  BLINKER_LOG("======Weahter T=======");
  BLINKER_LOG("Weather T: ", results_1_ming_text, " ",results_1_ming_code );
  BLINKER_LOG("Temperature: ", results_1_ming_low, "~", results_1_ming_high);
  BLINKER_LOG("========================");
}
/***********************************************************/


void deviceReset()
{
    Blinker.reset(); //重置设备，清除wifi配置
}

void dataRead(const String & data)
{
    BLINKER_LOG("Blinker readString: ", data);

    Blinker.vibrate();
    
    uint32_t BlinkerTime = millis();
    
    Blinker.print("millis", BlinkerTime);
}


//给水按键反馈函数
void button_water_callback(const String & state)
{
    //BLINKER_LOG("get button_led state: ", state);
    if (state == "on")
    {
      myservo.write(servo_on);
      BUTTON_WATER.print("on");//反馈开关状态
      }
    else if(state == "off")
    {
      myservo.write(servo_off);
      BUTTON_WATER.print("off");
      }
}

//模式切换按键反馈函数
void button_mode_callback(const String & state)
{
    //BLINKER_LOG("get button_led state: ", state);
    if (state == "tap")
    {
      mode_change =(mode_change+1)%2;
      BUTTON_MODE.print("tap");//反馈开关状态
      if(mode_change == 0){Mode.print("托管模式");}
      else{Mode.print("辅助模式");}
      }
    //Blinker.vibrate(250);
}

//状态切换按键反馈函数
void button_state_callback(const String & state)
{
    //BLINKER_LOG("get button_led state: ", state);
    if (state == "tap")
    {
      state_change =(state_change+1)%3;
      BUTTON_STATE.print("tap");//反馈开关状态
      switch(state_change)
      {
          case 0:
          State.print("偏干状态");
          break;
          case 1:
          State.print("适中状态");
          break;
          case 2:
          State.print("偏湿状态");
          break;   
      }
      }
    //Blinker.vibrate(250);
}



//获取时间函数
void mytime()
{
    mysec = Blinker.second();
    mymin = Blinker.minute();
    myhour = Blinker.hour();
    mywday = Blinker.wday();
    mymday = Blinker.mday();
    myyear = Blinker.year();
    mymonth = Blinker.month();
}

/************************天气显示***************************/
void drawWeatherSymbol(int x, int y, uint8_t symbol)
{
  switch(symbol)
  {
    //case后面为心知天气气象代码
    case 0:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 69); //晴天
      break;
    case 1:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 68); //夜晚晴
      break;  
    case 4:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 65); //晴间多云
      break;
    case 5:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 64); //多云
      break;
    case 6:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 64); //多云
      break;
    case 7:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 64); //多云
      break;
    case 8:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 64); //多云
      break;
    case 9:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 64); //多云
    case 30:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 64); //多云  
      break;   
    case 10:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 67); //雨
      break;
    case 11:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 67); //雨
      break;
    case 12:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 67); //雨
      break;
    case 13:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 67); //雨
      break;
    case 14:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 67); //雨
      break;
    case 15:
      u8g2.setFont(Oled_font);
      u8g2.drawGlyph(x, y, 67); //雨
      break;  
  }
}

void drawWeather(uint8_t symbol, int degree)
{
  //绘制天气符号
  drawWeatherSymbol(20, 33, symbol);
  //绘制温度
  u8g2.setFont(u8g2_font_logisoso24_tf);
  u8g2.setCursor(64, 30);
  u8g2.print(degree);
  u8g2.print("°C");   // requires enableUTF8Print()
}
  

void draw(uint8_t symbol, int degree)
{ 
  u8g2.clearBuffer();         // clear the internal memory
  drawWeather(symbol, degree);    // draw the icon and degree only once
  u8g2.sendBuffer();        // transfer internal memory to the display
}
/************************天气显示***************************/


void oled_start()
{
    u8g2.clearBuffer();//清除缓冲区
    u8g2.setFont(u8g2_font_unifont_t_chinese2);//设置字体集，非常重要
    u8g2.setCursor(0,15);
    u8g2.print("system booting ");
    u8g2.setCursor(0,30);
    u8g2.print("auto-config");
    u8g2.sendBuffer();//发送缓冲区的内容到显示器
    Blinker.delay(1000);
    if(Blinker.connect())
    {
         u8g2.clearBuffer();//清除缓冲区
         u8g2.setCursor(0,15);
         u8g2.print("Wifi Connected!");
         u8g2.setCursor(0,30);
         u8g2.print("Start Working!"); 
         u8g2.sendBuffer();//发送缓冲区的内容到显示器
    }
    else
    {
         u8g2.clearBuffer();//清除缓冲区
         u8g2.setCursor(0,15);
         u8g2.print("Wifi Disconnect!");
         u8g2.setCursor(0,30);
         u8g2.print("Please check!"); 
         u8g2.sendBuffer();//发送缓冲区的内容到显示器
         //while(!Blinker.connect());
    }
      
}

void oled_date(const String & t, const String & d, const String & w)
{
  u8g2.clearBuffer();//清除缓冲区
  u8g2.setFont(u8g2_font_logisoso24_tf);
  u8g2.setCursor(10,28);
  u8g2.print(t);
  u8g2.setFont(u8g2_font_7x14_tf);
  u8g2.setCursor(88,15);
  u8g2.print(w);
  u8g2.setCursor(88,31);
  u8g2.print(d);
  u8g2.sendBuffer();//发送缓冲区的内容到显示器
}

void oled_weather(uint8_t myweather,int degree)
{
  u8g2.clearBuffer();//清除缓冲区
  draw(myweather, degree);
  u8g2.sendBuffer();//发送缓冲区的内容到显示器
}

void oled_weather_pre(const String & Ming_temp, int code_Ming, const String & Hou_temp, int code_Hou)
{
    u8g2.clearBuffer();//清除缓冲区
    u8g2.setFont(u8g2_font_wqy13_t_chinese2);
    u8g2.setCursor(18,15);
    u8g2.print("明天");
    u8g2.setCursor(76,15);
    u8g2.print("后天");
    u8g2.drawLine(64,0,64,32);//中间划线隔开
    draw_weather(48,15,code_Ming);
    u8g2.setFont(u8g2_font_ncenB08_tf);
    u8g2.setCursor(18,30);
    u8g2.print(Ming_temp);//明天  
    draw_weather(108,15,code_Hou);
    u8g2.setFont(u8g2_font_ncenB08_tf);
    u8g2.setCursor(76,30);//后天
    u8g2.print(Hou_temp);
    u8g2.sendBuffer();//发送缓冲区的内容到显示器
}

void draw_weather(int x, int y, int symbol)
{
    u8g2.setFont(u8g2_font_unifont_t_symbols);
    if(symbol == 0)
    {
        u8g2.drawGlyph(x, y, 9728);//晴天
    }
    else if(symbol == 4 or symbol ==5 or symbol ==7 or symbol ==9)
    {
        u8g2.drawGlyph(x, y, 9729);//阴天
    }
    else if(symbol == 10 or symbol ==11 or symbol == 13 or symbol ==14 or symbol ==15 or symbol ==16 or symbol ==17 or symbol ==18 )
    {
        u8g2.drawGlyph(x, y, 9730);//下雨
    }
}

void oled_soil(int soil)
{
  u8g2.clearBuffer();//清除缓冲区
  u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
  u8g2.drawGlyph(10, 32, 152);//水滴
  u8g2.setFont( u8g2_font_logisoso16_tf);
  u8g2.setCursor(45, 28);
  u8g2.print(soil);
  u8g2.print("%");
  
  //根据湿度显示状态，缺水，适宜，水过多
  u8g2.setFont(u8g2_font_helvR10_tf);
  if (soil < soil_lower)
  {
        u8g2.setCursor(86,14);
        u8g2.print("Too");
        u8g2.setCursor(86,28);
        u8g2.print("Dry");
        u8g2.sendBuffer();//发送缓冲区的内容到显示器
  }
  else if(soil > soil_upper)
  {
        u8g2.setCursor(86,14);
        u8g2.print("Too");
        u8g2.setCursor(86,30);
        u8g2.print("Wet");
        u8g2.sendBuffer();//发送缓冲区的内容到显示器
  }
  else
  {
        u8g2.setCursor(84,14);
        u8g2.print("Adapt");
        u8g2.setCursor(84,30);
        u8g2.print("Humi");
        u8g2.sendBuffer();//发送缓冲区的内容到显示器
  }
}

void oled_indoor(float t, float h)
{
  u8g2.clearBuffer();//清除缓冲区
  u8g2.setFont(u8g2_font_open_iconic_all_4x_t);
  u8g2.drawGlyph(12, 33, 184);//home
  u8g2.setFont(u8g2_font_helvR12_tf);
  u8g2.setCursor(60, 16);
  u8g2.print(t);
  u8g2.print("°C");
  u8g2.setCursor(60,32);
  u8g2.print(h);
  u8g2.print("%");
  u8g2.sendBuffer();//发送缓冲区的内容到显示器
}
void oled_close()
{
   u8g2.clearBuffer();//清除缓冲区
   u8g2.sendBuffer();//发送缓冲区的内容到显示器    
}

/************************OLED显示***************************/


//心跳函数
void heartbeat()
{
    //数据更新
    HUMI.print(humi_read);
    TEMP.print(temp_read);
    Soil.print(soil_read);
    if(mode_change == 0){Mode.print("托管模式");}
    else{Mode.print("辅助模式");}
    switch(state_change)
    {
          case 0:
          State.print("偏干状态");
          break;
          case 1:
          State.print("适中状态");
          break;
          case 2:
          State.print("偏湿状态");
          break;   
    }      
}


void setup()
{
    //初始化串口，开启调试信息
    Serial.begin(9600);
    //BLINKER_DEBUG.stream(Serial);
    //BLINKER_DEBUG.debugAll();

    //oled屏幕初始化
    u8g2.begin();
    u8g2.enableUTF8Print();
    u8g2.setFontDirection(0);
    
    //初始化IO接口
    pinMode(SW, INPUT);
    
    //绑定 D6引脚 为舵机控制引脚
    myservo.attach(D6);
    myservo.write(servo_off);//初始状态为关闭
    
    //设置时区
    Blinker.setTimezone(8.0);//北京时区
       
    //初始化Blinker
    Blinker.begin(auth);
    
    //注册心跳包
    Blinker.attachHeartbeat(heartbeat);
    
    //注册按键
    BUTTON_WATER.attach(button_water_callback);
    BUTTON_MODE.attach(button_mode_callback);
    BUTTON_STATE.attach(button_state_callback);
    
    //注册wifi重置相关按键
    Blinker.attachData(dataRead);
    wifi_reset.attachLongPressStop(deviceReset);
    dht.begin();

    oled_start();//oled屏幕显示数据
    
    Blinker.delay(1000);
}

void loop()
{
    Blinker.run();
    wifi_reset.tick();
    
    if (digitalRead(SW) == 1 and oledstate == false){oledstate = true;}
    
    //时间显示
    mytime();
    //BLINKER_LOG("Time ",myhour,":",mymin);
    //BLINKER_LOG("Date ",myyear,"/",mymonth,"/",mymday," ",week[mywday]);
    time_all = "Time ";time_all.concat(myhour);time_all +=":";time_all.concat(mymin);
    date_all = "Date ";date_all.concat(myyear);date_all +="/";date_all.concat(mymonth);date_all +="/";date_all.concat(mymday);
    date_all +=" ";date_all+=week[mywday];
    //Time.print(time_all);
    //Date.print(date_all);
    
    
    
    //天气
    // 建立心知天气API<当前天气>请求资源地址
    String reqRes = "/v3/weather/now.json?key=" + reqUserKey1 +
                  + "&location=" + reqLocation + 
                  "&language=zh-Hans&unit=" +reqUnit;
                  
    // 建立心知天气API<预测天气>请求资源地址
    String reqRes_pre = "/v3/weather/daily.json?key=" + reqUserKey2 + 
                  + "&location=" + reqLocation + 
                  "&language=zh-Hans&unit=" +reqUnit;
                  
    // 向心知天气服务器服务器请求信息并对信息进行解析------一小时查询一次
    if (millis()-lastConnectionTime >  postingInterval or results_0_now_temperature_int == NULL or results_1_ming_text ==NULL )
    {
        lastConnectionTime = (signed long)millis();
        httpRequest(reqRes);
        httpRequest_pre(reqRes_pre);
        Weather.print(results_0_now_text, results_0_now_temperature);
    }
    
   
    //数据读取
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float s = analogRead(soil_pin);
    s = (1-(s-310)/(750-310))*100; //读取土壤湿度数据转换为湿度百分数
    //空气温湿度
    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
    }
    else
    {
        //Serial.println("Humidity: ", h, " %");
        //Serial.println("Temperature: ", t, " *C");
        humi_read = h;
        temp_read = t;
    }
    //土壤湿度
    if (isnan(s))
    {
        Serial.println("Failed to read from Soil sensor!");
    }
    else
    {        
        //Serial.println("Soil Humi: ", s, " %");
        soil_read = s;      
    }
    
    /**************************模式切换和状态切换************************************/
    //状态
    switch(state_change)
    {
        case 0:   //偏干状态
        soil_lower = 20;
        soil_upper = 50;
        break;
        case 1:   //适中状态
        soil_lower = 30;
        soil_upper = 60;        
        break;
        case 2:   //偏湿状态
        soil_lower = 40;
        soil_upper = 70;       
        break;  
    }
       
    //模式
    if(mode_change == 0)//托管模式      
    {
        if(soil_read <soil_lower)
        {
            myservo.write(servo_on);
        }
        if(soil_read >soil_upper)
        {
            myservo.write(servo_off);
        }
        
    }
    else //辅助模式
    {
        if(soil_read <soil_lower - 10)
          {
              //myservo.write(servo_on);
              oledstate = true;
              oled_state = 3;
          }
         else if(soil_read >soil_upper + 5)
          {
              //myservo.write(servo_off);
              oledstate = true;
              oled_state = 3;
          }
    }      
    /**************************模式切换和状态切换************************************/

    if (digitalRead(SW) == 1 and oledstate == false){oledstate = true;}
    if ( oledstate == true) //震动触发，切换显示状态
    {
        oled_state =(oled_state+1)%6;
        SW_time = millis();     
        Serial.println("state change");
        oledstate = false;
        Blinker.delay(20);
    }
    if (millis() - SW_time > 10000) //超出显示时间，屏幕熄灭
    {
        oled_state = 0;
        SW_time = millis();
    }
    
    //oled显示数据处理
    if( myhour<10 ){o_time="0";}else{o_time="";}o_time.concat(myhour);
    if( mymin<10 ){o_time+=":0";}else{o_time+=":";}o_time.concat(mymin);//时间
    o_date="";o_date.concat(mymonth);o_date+="/";o_date.concat(mymday);//日期
    String ming_temp = results_1_ming_low + "~" + results_1_ming_high + "°C";
    String hou_temp = results_2_hou_low + "~" + results_2_hou_high + "°C";
    switch (oled_state)
    {
        case 0:
        oled_close();//屏幕熄灭
        break;
        case 1:
        oled_date(o_time, o_date, week[mywday]);//时间，日期，周 
        break;
        case 2: 
        oled_weather(results_0_now_code_int, results_0_now_temperature_int);//天气
        break;
        case 3:
        oled_weather_pre(ming_temp, results_1_ming_code_int, hou_temp, results_2_hou_code_int);//天气预测
        break;
        case 4: 
        oled_soil(soil_read);//土壤湿度 
        break;
        case 5: 
        oled_indoor(t, h);//室内温湿度
        break;

    }
   
    
    Blinker.delay(200);//延时0.5s， 每0.5秒读取一次
}
