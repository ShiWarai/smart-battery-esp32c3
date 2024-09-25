#include <WiFi.h>
#include <INA226.h>

#define IDBAT 1  //id устр-ва
const char* ssid = "A-415";
const char* password = "LongRedAlert5";

const uint16_t port = 8090;
const char * host = "192.168.1.108";

INA226 INA(0x40);


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
}

void loop()
{
    WiFiClient client;
    if (!client.connect(host, port)) {
        Serial.println("Connection to host failed");
        return;
    }
    Serial.println("The Wi-Fi connection is fine");

    //сообщение\/
    client.print("{\"ID\":" + String(IDBAT)
    + ", \"V\":" + String(INA.getBusVoltage())
    + ", \"A\":" + String(INA.getCurrent_mA()) + "}");
    delay(1000);
}