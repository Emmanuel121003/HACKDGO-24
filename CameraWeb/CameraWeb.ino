#include "esp_camera.h"
#include <WiFi.h>
#include <FirebaseESP32.h>
#include "mbedtls/base64.h"  // Librería para base64
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// Reemplaza con las credenciales de tu proyecto Firebase
#define FIREBASE_HOST "https://hackaton-709f6-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "AIzaSyDeImTDEyzwNYN4ZTkjdagA1rVI5mJza50"
#define FIREBASE_DB_URL "gs://hackaton-709f6.appspot.com"

// Credenciales WiFi
const char* ssid = "XIXIM STARLINK";
const char* password = "XIXIM-STARLINK-24";

// Inicialización de variables de Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
String imageData;

void startCameraServer();  // Función declarada para iniciar el servidor

// Configuración de la cámara
camera_config_t configCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;  // Resolución UXGA
  config.pixel_format = PIXFORMAT_JPEG;  // Para transmitir en JPEG
  config.fb_count = 1;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;  // Calidad de imagen
  return config;
}

// Configurar e inicializar la cámara
void initCamera() {
  camera_config_t config = configCamera();

  // Si PSRAM está presente, ajustamos la calidad y el búfer
  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limitar el tamaño de la imagen si no hay PSRAM
    config.frame_size = FRAMESIZE_SVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // Inicialización de la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error de inicialización de la cámara: 0x%x", err);
    return;
  }

  sensor_t* s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);  // Volteo vertical
    s->set_brightness(s, 1);  // Incremento leve de brillo
    s->set_saturation(s, -2);  // Saturación menor
  }

  // Ajustar el tamaño de la imagen para mejorar la tasa de cuadros
  s->set_framesize(s, FRAMESIZE_QVGA);  // Tamaño inicial de la imagen
}

// Función para capturar y enviar imagen a Firebase
void captureAndSendImage() {
  camera_fb_t* fb = esp_camera_fb_get();  // Captura de la imagen
  if (!fb) {
    Serial.println("Error al capturar imagen");
    return;
  }

  // Calcular el tamaño del buffer para base64 (4 * ((tamano_original + 2) / 3))
  size_t encoded_length = 4 * ((fb->len + 2) / 3);
  unsigned char encoded_data[encoded_length];  // Arreglo para almacenar los datos codificados

  // Convertir la imagen a base64
  size_t output_length;
  mbedtls_base64_encode(encoded_data, sizeof(encoded_data), &output_length, fb->buf, fb->len);
  String encodedImage = String((char*)encoded_data);

  // Enviar imagen a Firebase
  if (Firebase.pushString(fbdo, "/images", encodedImage)) {
    Serial.println("Imagen enviada a Firebase");
  } else {
    Serial.println("Error al enviar la imagen a Firebase");
  }

  esp_camera_fb_return(fb);  // Liberar el búfer de la cámara
}

void setup() {
  Serial.begin(115200);

  // Conectar al WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Conectado a WiFi");

  // Configurar Firebase
  config.database_url = FIREBASE_HOST;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Inicializar la cámara
  initCamera();

  // Iniciar el servidor de la cámara
  startCameraServer();

  Serial.print("Cámara lista! Accede a: http://");
  Serial.print(WiFi.localIP());
  Serial.println(" para visualizar el stream");
}

void loop() {
  // Captura y envío de imagen cada 10 segundos
  captureAndSendImage();
  delay(10000);
}
