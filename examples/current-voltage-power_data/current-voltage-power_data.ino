// Current sensor readings from the Adafruit INA219

#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 current_1(0x40); // Solar Pannel 1
Adafruit_INA219 current_2(0x41); // Solar Pannel 2
Adafruit_INA219 current_3(0x42); // Solar Pannel 3
Adafruit_INA219 current_4(0x43); // Solar Pannel 4
Adafruit_INA219 current_5(0x44); // Battery Board

Adafruit_INA219 *p[ARTEMIS_CURRENT_SENSOR_COUNT] = {&current_1, &current_2, &current_3, &current_4, &current_5};
const char *current_sen_names[ARTEMIS_CURRENT_SENSOR_COUNT] = {"solar_panel_1", "solar_panel_2", "solar_panel_3", "solar_panel_4", "battery_board"};

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
  for (int i = 0; i < ARTEMIS_CURRENT_SENSOR_COUNT; i++)
  {
    beacon.busvoltage[i] = (p[i]->getBusVoltage_V());
    beacon.current[i] = (p[i]->getCurrent_mA());
    beacon.power[i] = (p[i]->getPower_mW());

    Serial.println();
  }
  delay(2000);
}
