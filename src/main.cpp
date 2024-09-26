#include <WiFi.h>
#include <INA226.h>
#include <ESPmDNS.h>

#define IDBAT 1  //id устр-ва
const char* ssid = "A-415";
const char* password = "LongRedAlert5";

const uint16_t port = 8090;
const char * host = "192.168.1.108";

IPAddress HOSTIP;

INA226 INA(0x40);
WiFiClient client;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  Wire.begin();
  if (!INA.begin() )
  {
    Serial.println("it was not possible to connect to the voltammeter. Fix the error");
  }
  INA.setMaxCurrentShunt(1, 0.002);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("...");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());


  while(mdns_init()!= ESP_OK){
    delay(1000);
    Serial.println("Starting MDNS...");
}

}

void loop()
{
     Serial.print("Hostname: ");
     Serial.println(WiFi.getHostname());
    while(!client.connected()){Serial.println("Connecting...");client.connect(host, port);delay(1000);}
    
    while (HOSTIP.toString() == "0.0.0.0") {
    Serial.println("Resolving host...");
    delay(250);
    HOSTIP = MDNS.queryHost("WORK-SHIWARAI");
    }
    Serial.println(HOSTIP.toString());

    //сообщение\/
    client.print("{\"ID\":" + String(IDBAT)
    + ", \"V\":" + String(INA.getBusVoltage())
    + ", \"A\":" + String(INA.getCurrent_mA()) + "}");
    delay(1000);
}