Docs
sketch.ino
diagram.json
libraries.txt
waterpump.chip.json
waterpump.chip.c
Library Manager

#define BLYNK_TEMPLATE_ID "TMPL6XDXcoIur"
#define BLYNK_TEMPLATE_NAME "proyekk akhir"
#define BLYNK_AUTH_TOKEN "M_fuuTdSMgTO0ltSIFPPZaIXhcFT8yBm"

#include <DHT.h>
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>

char auth[] = BLYNK_AUTH_TOKEN;
#define ssid "DeniseNesta"
#define pass "Desta1234"

#define DO_PIN 13   // ESP32's pin GPIO13 connected to DO pin of the LDR module
#define LED_PIN 14  // ESP32's pin GPIO14 connected to an LED
#define SOIL_SENSOR_PIN 4 // Pin for soil moisture sensor
#define TEMP_LED_PIN 15  // ESP32's pin GPIO15 connected to the temperature LED
#define SOIL_LED_PIN_1 18  // ESP32's pin GPIO18 connected to the first soil moisture LED
#define SOIL_LED_PIN_2 19  // ESP32's pin GPIO19 connected to the second soil moisture LED

// Set up the DHT sensor 
DHT dht(4, DHT22);
float temperature;

int soilMoistureValue; // Value read from soil moisture sensor
int soilMoisturePercent;

void readLDRState(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    // Read the digital state from the LDR module
    int lightState = digitalRead(DO_PIN);

    // Print the corresponding message based on the light state
    if (lightState == HIGH) {
      Serial.println("It is light");
      // Turn off the LED when it's light
      digitalWrite(LED_PIN, LOW);
      Blynk.virtualWrite(V1, "Current LED Condition: Off");
    } else {
      Serial.println("It is dark");
      // Turn on the LED when it's dark
      digitalWrite(LED_PIN, HIGH);
      Blynk.virtualWrite(V1, "Current LED Condition: On");
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay for 1000 milliseconds
  }
}

void readDHTSensor(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    temperature = dht.readTemperature();
    // Print the values of temperature in Celsius
    Serial.print("Temperature:\t");
    Serial.print(temperature);
    Serial.println("C");

    // Send temperature data to Blynk
    Blynk.virtualWrite(V0, temperature);

    // Control temperature LED
    if (temperature < 20) {
      digitalWrite(TEMP_LED_PIN, HIGH);
    } else {
      digitalWrite(TEMP_LED_PIN, LOW);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 2000 milliseconds
  }
}

void checkSoilMoisture(void *pvParameters) {
  (void)pvParameters;

  for (;;) {
    soilMoistureValue = analogRead(SOIL_SENSOR_PIN);
    Serial.print("Soil Moisture Analog Value = ");
    Serial.println(soilMoistureValue);
    soilMoisturePercent = map(soilMoistureValue, 4095, 0, 0, 100);

    Serial.print("Soil Moisture Percentage = ");
    Serial.print(soilMoisturePercent);
    Serial.println("% ");

    // Check soil moisture and control LEDs accordingly
    if (soilMoisturePercent > 60 && soilMoisturePercent <= 100) {
      Serial.println("Soil is wet");
      digitalWrite(SOIL_LED_PIN_1, LOW);  // Soil is wet, turn off the first LED
      digitalWrite(SOIL_LED_PIN_2, LOW);  // Soil is wet, turn off the second LED
      Blynk.virtualWrite(V2, "Soil is wet, stop watering");
    } else if (soilMoisturePercent > 30 && soilMoisturePercent <= 60) {
      Serial.println("Soil is in normal condition");
      digitalWrite(SOIL_LED_PIN_1, LOW);  // Soil is in normal condition, turn off the first LED
      digitalWrite(SOIL_LED_PIN_2, LOW);  // Soil is in normal condition, turn off the second LED
      Blynk.virtualWrite(V2, "Soil is in normal condition, stop watering");
    } else if (soilMoisturePercent >= 0 && soilMoisturePercent <= 30) {
      Serial.println("Soil is dry");
      digitalWrite(SOIL_LED_PIN_1, HIGH);  // Soil is dry, turn on the first LED
      digitalWrite(SOIL_LED_PIN_2, HIGH);  // Soil is dry, turn on the second LED
      Blynk.virtualWrite(V2, "Soil is dry, start watering");
    }

    vTaskDelay(10000 / portTICK_PERIOD_MS); // Delay for 10000 milliseconds
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.begin(ssid, pass);
  Blynk.begin(auth, ssid, pass);

  // Set the LED pins as outputs
  pinMode(LED_PIN, OUTPUT);
  pinMode(TEMP_LED_PIN, OUTPUT);
  pinMode(SOIL_LED_PIN_1, OUTPUT);
  pinMode(SOIL_LED_PIN_2, OUTPUT);

  // Create FreeRTOS tasks
  xTaskCreate(readLDRState, "LDRStateTask", 4096, NULL, 1, NULL);
  xTaskCreate(readDHTSensor, "DHTSensorTask", 4096, NULL, 1, NULL);
  xTaskCreate(checkSoilMoisture, "SoilMoistureTask", 4096, NULL, 1, NULL);

  Serial.println("Connected to WiFi");
}

void loop() {
  // Nothing in the loop since tasks are running in parallel
  delay(1000);
}