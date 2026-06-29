#include <Arduino.h>
#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

#define ZIGBEE_LIGHT_ENDPOINT  10
#define ZIGBEE_SOIL_ENDPOINT   11
#define SOIL_PIN               2

uint8_t relayPin = 8;
uint8_t button = BOOT_PIN;

int dryValue = 3473;
int wetValue = 1500;

ZigbeeLight zbLight = ZigbeeLight(ZIGBEE_LIGHT_ENDPOINT);
ZigbeeAnalog zbSoil = ZigbeeAnalog(ZIGBEE_SOIL_ENDPOINT);

unsigned long lastSoilRead = 0;
unsigned long buttonPressStart = 0;

void setRelay(bool value) {
  digitalWrite(relayPin, !value);
  Serial.printf("Relay: %s\n", value ? "ON" : "OFF");
}

float readSoilMoisture() {
  int sum = 0;
  for (int i = 0; i < 5; i++) {
    sum += analogRead(SOIL_PIN);
    delayMicroseconds(100);
  }
  int raw = sum / 5;
  float percent = (float)(dryValue - raw) / (dryValue - wetValue) * 100.0;
  percent = constrain(percent, 0, 100);
  Serial.printf("Soil raw: %d, moisture: %.1f%%\n", raw, percent);
  return percent;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Setup started...");

  // Relay
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  Serial.println("✓ Relay pins configured");

  // Soil sensor
  pinMode(SOIL_PIN, INPUT);
  Serial.println("✓ Soil pin configured");

  // Button
  pinMode(button, INPUT_PULLUP);
  Serial.println("✓ Button configured");

  // Relay endpoint
  Serial.println("About to add light endpoint...");
  zbLight.setManufacturerAndModel("Espressif", "ZBRelaySwitch");
  zbLight.onLightChange(setRelay);
  Zigbee.addEndpoint(&zbLight);
  Serial.println("✓ Light endpoint added");

  // Soil sensor endpoint
  Serial.println("About to configure soil endpoint...");
  zbSoil.setManufacturerAndModel("Espressif", "ZBSoilSensor");
  Serial.println("✓ Soil manufacturer/model set");

  zbSoil.addAnalogInput();
  Serial.println("✓ Analog input added");

  zbSoil.setAnalogInputMinMax(0.0, 100.0);
  Serial.println("✓ Min/Max set");

  zbSoil.setAnalogInputDescription("Soil Moisture");
  Serial.println("✓ Description set");

  Zigbee.addEndpoint(&zbSoil);
  Serial.println("✓ Soil endpoint added to Zigbee");

  // Start Zigbee
  Serial.println("About to start Zigbee...");
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start! Rebooting...");
    ESP.restart();
  }
  Serial.println("✓ Zigbee started");

  Serial.println("Connecting to Zigbee network...");
  int timeout = 0;
  while (!Zigbee.connected() && timeout < 200) {
    Serial.print(".");
    delay(100);
    timeout++;
  }
  Serial.println("\nConnected!");

  // Configure reporting AFTER Zigbee is fully connected
  Serial.println("Configuring analog reporting...");
  zbSoil.setAnalogInputReporting(10, 60, 1.0);  // Report every 10s if change > 1%, max every 60s
  Serial.println("✓ Reporting configured");

  // Send first reading immediately
  float moisture = readSoilMoisture();
  zbSoil.setAnalogInput(moisture);
  zbSoil.reportAnalogInput();
  lastSoilRead = millis();
  Serial.println("Initial reading sent");
}

void loop() {
  // REMOVED: Manual reporting every 60 seconds
  // Let setAnalogInputReporting() handle it automatically
  
  // But we still need to READ the sensor and UPDATE the value
  if (millis() - lastSoilRead >= 10000) {  // Read every 10 seconds
    float moisture = readSoilMoisture();
    zbSoil.setAnalogInput(moisture);
    // DO NOT call zbSoil.reportAnalogInput() - let Zigbee handle it
    lastSoilRead = millis();
    Serial.println("Soil moisture updated locally");
  }

  // Non-blocking BOOT button handling
  if (digitalRead(button) == LOW && buttonPressStart == 0) {
    buttonPressStart = millis();
  } else if (digitalRead(button) == LOW && (millis() - buttonPressStart > 3000)) {
    Serial.println("Factory reset...");
    delay(1000);
    Zigbee.factoryReset();
    buttonPressStart = 0;
  } else if (digitalRead(button) == HIGH && buttonPressStart > 0) {
    if ((millis() - buttonPressStart) < 1000) {
      bool newState = !zbLight.getLightState();
      zbLight.setLight(newState);
      setRelay(newState);
      Serial.println("Relay toggled manually");
    }
    buttonPressStart = 0;
  }

  delay(50);
}