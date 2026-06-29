#include <Arduino.h>
#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"
#include "MHZ19.h"
#include <HardwareSerial.h>

#define CARBON_DIOXIDE_SENSOR_ENDPOINT_NUMBER 10
#define RX_PIN 22
#define TX_PIN 23

uint8_t button = BOOT_PIN;

MHZ19 myMHZ19;
HardwareSerial mySerial(1);

ZigbeeCarbonDioxideSensor zbCarbonDioxideSensor = ZigbeeCarbonDioxideSensor(CARBON_DIOXIDE_SENSOR_ENDPOINT_NUMBER);

void setup() {
  Serial.begin(115200);

  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration(false);

  pinMode(button, INPUT_PULLUP);

  zbCarbonDioxideSensor.setManufacturerAndModel("Espressif", "ZigbeeCarbonDioxideSensor");
  zbCarbonDioxideSensor.setMinMaxValue(0, 5000);

  Zigbee.addEndpoint(&zbCarbonDioxideSensor);

  Serial.println("Starting Zigbee...");
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  } else {
    Serial.println("Zigbee started successfully!");
  }

  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  zbCarbonDioxideSensor.setReporting(0, 30, 0);
}

void loop() {
  static uint32_t timeCounter = 0;

  if (!(timeCounter++ % 20)) {
    int co2 = myMHZ19.getCO2();
    Serial.printf("CO2: %d ppm\n", co2);
    if (co2 > 0) {
      zbCarbonDioxideSensor.setCarbonDioxide(co2);
    }
  }

  if (digitalRead(button) == LOW) {
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
    zbCarbonDioxideSensor.report();
  }
  delay(100);
}