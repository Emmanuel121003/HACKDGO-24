#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


#define API_KEY "AIzaSyCF3UxJjh_jf_vQNlRPQxnKwCrH_lBZvjU"
#define DATABASE_URL "https://sensor-f318e-default-rtdb.firebaseio.com/"
#define USER_EMAIL "vargasemmanuel121003@gmail.com"
#define USER_PASSWORD "Emmanuel12@"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

ESP8266WebServer server(80);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000);

unsigned long sendDataPrevMillis = 0;
String ssid;
String password;

int mq4_value = 0;
int mg811_value = 0;

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Fallo en la asignación de SSD1306"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Bienvenido!");
  display.println("Inicie la configuracion");
  display.display();

  // Inicia el modo AP
  WiFi.softAP("ESP8266");

  server.on("/", handleRoot);
  server.on("/configuracion", handleConfigure);
  server.begin();
  Serial.println("Servidor iniciado");

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Conectese a AP:");
  display.println("ESP8266");
  display.println("y visite 192.168.4.1");
  display.display();
}

void handleRoot() {
  String page = "<html>\
<head>\
  <title>Configuracion WiFi</title>\
  <style>\
    body {\
      font-family: Arial, sans-serif;\
      background-color: #f4f4f4;\
      display: flex;\
      justify-content: center;\
      align-items: center;\
      height: 100vh;\
      margin: 0;\
    }\
    .container {\
      background: #fff;\
      padding: 20px;\
      border-radius: 8px;\
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\
      max-width: 400px;\
      width: 100%;\
    }\
    h1 {\
      text-align: center;\
      color: #333;\
    }\
    form {\
      display: flex;\
      flex-direction: column;\
    }\
    label {\
      margin-bottom: 5px;\
      color: #555;\
    }\
    input[type=\"text\"], input[type=\"password\"] {\
      padding: 10px;\
      margin-bottom: 20px;\
      border: 1px solid #ddd;\
      border-radius: 4px;\
      font-size: 16px;\
    }\
    input[type=\"submit\"] {\
      background-color: #28a745;\
      color: white;\
      padding: 10px;\
      border: none;\
      border-radius: 4px;\
      cursor: pointer;\
      font-size: 16px;\
    }\
    input[type=\"submit\"]:hover {\
      background-color: #218838;\
    }\
  </style>\
</head>\
<body>\
  <div class=\"container\">\
    <h1>Configuracion del WiFi</h1>\
    <form action=\"/configuracion\" method=\"POST\">\
      <label for=\"ssid\">SSID:</label>\
      <input type=\"text\" id=\"ssid\" name=\"ssid\"><br>\
      <label for=\"password\">Password:</label>\
      <input type=\"password\" id=\"password\" name=\"password\"><br>\
      <input type=\"submit\" value=\"Guardar\">\
    </form>\
  </div>\
</body> \
</html>";
  server.send(200, "text/html", page);
}

void handleConfigure() {
  ssid = server.arg("ssid");
  password = server.arg("password");

  server.send(200, "text/html", "<html>\
<head>\
  <title>Guardado</title>\
  <style>\
    body {\
      font-family: Arial, sans-serif;\
      background-color: #f4f4f4;\
      display: flex;\
      justify-content: center;\
      align-items: center;\
      height: 100vh;\
      margin: 0;\
    }\
    .container {\
      background: #fff;\
      padding: 20px;\
      border-radius: 8px;\
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);\
      max-width: 400px;\
      width: 100%;\
      text-align: center;\
    }\
    h1 {\
      color: #333;\
      margin: 0;\
    }\
  </style>\
</head>\
<body>\
  <div class=\"container\">\
    <h1>Guardado.</h1>\
  </div>\
</body>\
</html>");
  delay(1000);
  connectToWiFi(); // Conéctese a la red WiFi
}

void connectToWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Conectando a WiFi...");
  display.display();
  Serial.println("Conectando a WiFi...");

  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(1000);
    Serial.print(".");
  }

  display.clearDisplay();
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(0, 0);
    display.println("Conectado a:");
    display.println(ssid);
    display.print("IP: ");
    display.println(WiFi.localIP());
    Serial.println("Conectado a:");
    Serial.println(ssid);
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    display.setCursor(0, 0);
    display.println("No se pudo conectar");
    display.println("a la red WiFi.");
    Serial.println("No se pudo conectar a la red WiFi.");
  }
  display.display();
  delay(1000);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    config.timeout.serverResponse = 10 * 1000;
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024);

    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);
  }

  timeClient.begin(); // Inicia el cliente NTP

  display.clearDisplay();
}

void loop() {
  server.handleClient();

  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    int delimiterIndex = data.indexOf(' ');
    String type = data.substring(0, delimiterIndex);
    int value = data.substring(delimiterIndex + 1).toInt();

    if (type == "CH4") {
      mq4_value = value;
    } else if (type == "CO2") {
      mg811_value = value;
    }

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("CH4: ");
    display.print(mq4_value);
    display.println(" ppm");
    display.print("CO2: ");
    display.print(mg811_value);
    display.println(" ppm");
    display.display();

    Serial.print("CH4 ");
    Serial.print(mq4_value);
    Serial.println(" ppm");
    Serial.print("CO2 ");
    Serial.print(mg811_value);
    Serial.println(" ppm");

    if (WiFi.status() == WL_CONNECTED) {
      if (Firebase.ready() && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();

        timeClient.update();
        String timestamp = timeClient.getFormattedTime(); // Obtén la hora en formato HH:MM:SS

        bool success = true;

        if (!Firebase.setInt(fbdo, "/sensor_data/" + timestamp + "/CH4", mq4_value)) {
          Serial.println(fbdo.errorReason());
          success = false;
        }

        if (!Firebase.setInt(fbdo, "/sensor_data/" + timestamp + "/CO2", mg811_value)) {
          Serial.println(fbdo.errorReason());
          success = false;
        }

        if (success) {
          Serial.println("Datos enviados correctamente");
        } else {
          Serial.println("Error al enviar los datos");
        }
      }
    }

    delay(1000);
  }
}
