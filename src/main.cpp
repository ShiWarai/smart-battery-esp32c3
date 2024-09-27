#include <WiFi.h>
#include <INA226.h>
#include <ESPmDNS.h>

#define SERIALIZE_DATA(ID, V, A, P, C) "{\"ID\":" + ID + ", \"V\":" + V + ", \"A\":" + A + ", \"P\":" + P + ", \"C\":" + C + "}"

IPAddress HOSTIP;

INA226 INA(0x40);
WiFiClient client;
String data;

void setup()
{
	Serial.begin(115200);
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	
	Wire.begin();
	if (!INA.begin())
		Serial.println("it was not possible to connect to the voltampermeter. Fix the error");
	INA.setMaxCurrentShunt(60, 0.00125, true);
	
	// подключение к WIFI
	while (WiFi.status() != WL_CONNECTED) {delay(500);}
	Serial.print("WiFi connected with IP: ");
	Serial.println(WiFi.localIP());


	while(mdns_init()!= ESP_OK){delay(1000);}

	// поиск ip адреса hostname
	while (HOSTIP.toString() == "0.0.0.0") {
		delay(250);
		HOSTIP = MDNS.queryHost(HOSTNAME);
	}
}

void loop()
{
	delay(1000);
	// если потеряли связь с клиентом, то устонавливаем её заново
	if(!client.connected())
	{
		client.connect(HOSTIP, PORT);
		return;
	}

	data = SERIALIZE_DATA(String(ID), INA.getBusVoltage(), INA.getCurrent(), INA.getPower(), map(INA.getBusVoltage(),3.3,4.2,0,100));

	//   \/ сообщение по COM порту \/
	Serial.println(data);

	//     \/ сообщение по WIFI \/
	client.print(data);
}