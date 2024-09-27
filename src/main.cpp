#include <WiFi.h>
#include <INA226.h>
#include <ESPmDNS.h>
//--------------НАСТРОЙКИ---------------
#define IDBAT 1                         //id устр-ва
const char* ssid = "A-415";             //WI-FI
const char* password = "LongRedAlert5"; //Пароль от WI-FI

const uint16_t port = 8090;             
const char * hostname = "WORK-SHIWARAI";
//---------------------------------------

IPAddress HOSTIP;

INA226 INA(0x40);
WiFiClient client;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  Wire.begin();
  if (!INA.begin() )
  {Serial.println("it was not possible to connect to the voltampermeter. Fix the error");}
  INA.setMaxCurrentShunt(1, 0.002);
  
  //подключение к WIFI
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());


  while(mdns_init()!= ESP_OK){delay(1000);}

//поиск ip адреса hostname
  while (HOSTIP.toString() == "0.0.0.0") {
  delay(250);
  HOSTIP = MDNS.queryHost(hostname);
  }
}

void loop()
{
    //если потеряли связь с клиентом, то устонавливаем её заново
    while(!client.connected()){client.connect(HOSTIP, port);delay(1000);}

    //   \/ сообщение по COM порту \/
    Serial.println("{\"ID\":" + String(IDBAT)
    + ", \"V\":" + String(INA.getBusVoltage())
    + ", \"A\":" + String(INA.getCurrent_mA()) 
    + ", \"Ch\":" + map(INA.getBusVoltage(),3.3,4.2,0,100) + "}");

    //     \/ сообщение по WIFI \/
    client.print("{\"ID\":" + String(IDBAT)
    + ", \"V\":" + String(INA.getBusVoltage())
    + ", \"A\":" + String(INA.getCurrent_mA()) 
    + ", \"Ch\":" + map(INA.getBusVoltage(),3.3,4.2,0,100) + "}");
    delay(1000);
}