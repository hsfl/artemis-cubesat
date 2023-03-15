// Current sensor readings from the Adafruit INA219

#include <Wire.h>
#include <Adafruit_INA219.h>

#define current_sensor_count 5

Adafruit_INA219 current_1(0x40); // Solar Pannel 1
Adafruit_INA219 current_2(0x41); // Solar Pannel 2
Adafruit_INA219 current_3(0x42); // Solar Pannel 3
Adafruit_INA219 current_4(0x43); // Solar Pannel 4
Adafruit_INA219 current_5(0x44); // Battery Board

const char *labels[current_sensor_count] = {"Solar Panel 1", "Solar Panel 2", "Solar Panel 3", "Solar Panel 4", "Battery Board"};
Adafruit_INA219 *p[current_sensor_count] = {&current_1, &current_2, &current_3, &current_4, &current_5};

void setup()
{
  delay(5000); // Give time for the Teensy to boot
  Serial.begin(115200);

  current_1.begin(&Wire2);
  current_2.begin(&Wire2);
  current_3.begin(&Wire2);
  current_4.begin(&Wire2);
  current_5.begin(&Wire2);
}

void loop()
{

  for (int i = 0; i < current_sensor_count; i++)
  {

    Serial.printf("%s: Bus Voltage %.3f V Current %.1f mA Power %.0f mW\n", labels[i], p[i]->getBusVoltage_V(), p[i]->getCurrent_mA(), p[i]->getPower_mW());
    Serial.println();
  }

  delay(10000);
}