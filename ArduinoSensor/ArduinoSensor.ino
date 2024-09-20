#include <DHT.h>
#include <Servo.h>

// Definición de pines para los sensores
#define DHTPIN 2      
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);

// Configuración de los motores con L298N
const int motor1A = 3; // Conecta esto al pin A del motor 1 en L298N
const int motor1B = 4; // Conecta esto al pin B del motor 1 en L298N
const int motor2A = 5; // Conecta esto al pin A del motor 2 en L298N
const int motor2B = 6; // Conecta esto al pin B del motor 2 en L298N

// Configuración del servo
Servo servo;
const int servoPin = 7; // Cambia este valor al pin que estás utilizando

void setup() {
  Serial.begin(9600); 
  
  // Configuración de los pines de entrada para los sensores
  pinMode(A0, INPUT); // MQ-4
  pinMode(A1, INPUT); // MG811
  pinMode(A2, INPUT); // FC-28
  
  // Inicialización del sensor DHT11
  dht.begin();

  // Configuración de los motores como salidas
  pinMode(motor1A, OUTPUT);
  pinMode(motor1B, OUTPUT);
  pinMode(motor2A, OUTPUT);
  pinMode(motor2B, OUTPUT);

  // Configuración del servo
  servo.attach(servoPin);
}

void loop() {
  // Leer valores de los sensores
  int mq4_value = analogRead(A0); // Leer MQ-4
  int mg811_value = analogRead(A1); // Leer MG811
  int fc28_value = analogRead(A2); // Leer FC-28
  
  // Leer temperatura y humedad del DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  // Enviar datos en formato CSV por el puerto serie
  Serial.print("CH4 ");
  Serial.print(mq4_value);
  Serial.println(" ppm");

  Serial.print("CO2 ");
  Serial.print(mg811_value);
  Serial.println(" ppm");

  Serial.print("FC-28 ");
  Serial.print(fc28_value);
  Serial.println(" %");

  Serial.print("Humedad DHT11 ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Temperatura DHT11 ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Gira el servo de 0 a 180 grados
  for (int posicion = 0; posicion <= 180; posicion++) {
    servo.write(posicion);  // Establece la posición del servo
    delay(1000);  // Espera 15 ms para que el servo alcance la posición
  }
  
  // Gira el servo de 180 a 0 grados
  for (int posicion = 180; posicion >= 0; posicion--) {
    servo.write(posicion);  // Establece la posición del servo
    delay(1000);  // Espera 15 ms para que el servo alcance la posición
  }
  
  // Avanzar el vehículo controlando los motores
  avanzar();

  delay(5000); // Esperar 5 segundos entre lecturas
}

void avanzar() {
  // Avanzar el vehículo encendiendo los motores
  digitalWrite(motor1A, HIGH);
  digitalWrite(motor1B, LOW);
  digitalWrite(motor2A, HIGH);
  digitalWrite(motor2B, LOW);
  delay(2000); // El vehículo avanza durante 2 segundos
  detener();
}

void detener() {
  // Detener el vehículo
  digitalWrite(motor1A, LOW);
  digitalWrite(motor1B, LOW);
  digitalWrite(motor2A, LOW);
  digitalWrite(motor2B, LOW);
}
