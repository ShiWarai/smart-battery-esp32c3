
// Подключаем библиотеки
#include <stdarg.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsClient.h>

#define IDNUMB 1//идентификационный номер ус-ва
#define DIV_R1 1
#define DIV_R2 2
#define VREF 5
char ssid[] = "A-415";
char password[] = "LongRedAlert5";

const char* ADDR = "n.n.n.n"; // Определяем адрес сервера (например "192.168.1.3")
const char* URL = "/"; // Определяем url подключения
const uint16_t PORT = 81; // Определяем порт

//               id, V, A
float sdata[3] {IDNUMB, 3.4, 10}; // массив для отправки

// Создаём экземпляр класса клиента
WebSocketsClient webSocket;

/* Callback функция события WebSocket.
   Парамтетры:
   type - тип событыя
   payload - указатель на данные
   length - размер данных
*/
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  
  if (type == WStype_BIN && length > 0) { // Если тип данных двоичный и размер больше нуля

    // Преобразуем данные из байтов в десятичные числа без знака
    const unsigned long* tmp = (unsigned long*) payload;
  }
}

void setup() {
  Serial.begin(115200);
  
  // Подключаемся к WiFi
  WiFi.begin(ssid, password);
  // Ждём подключения WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());

  // Подключаемся к серверу
  webSocket.begin(ADDR, PORT, URL);

  // Метод событий WebSocket
  webSocket.onEvent(webSocketEvent);

  // Если соединение прервано, повторить попытку через 5 сек.
  webSocket.setReconnectInterval(5000);
}


void loop() {
  webSocket.loop(); // Цикл WebSocket  
  
  if (millis() % 1000 == 0) { // Если прошла одна секунда и сервер хотя бы раз прислал данные
    // Отправляем данные в двоичном формате серверу
    webSocket.sendBIN((uint8_t*)sdata, sizeof(sdata));
    Serial.println("Данные отправлены.");
  }
  // Если соединение WiFi прервано
  if (WiFi.status() != WL_CONNECTED)

    // Вызываем функцию setup(), для повторного подключения
    setup();
}