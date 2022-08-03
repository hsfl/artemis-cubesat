#include <Wire.h>
#include <Adafruit_INA219.h>
//----------------------------------------------------------------------------------------------------------------------
Adafruit_INA219 ina219_1(0x40); // Solar 1
Adafruit_INA219 ina219_2(0x41); // Solar 2
Adafruit_INA219 ina219_3(0x42); // Solar 3
Adafruit_INA219 ina219_4(0x43); // Solar 4
Adafruit_INA219 ina219_5(0x44); // Battery
//----------------------------------------------------------------------------------------------------------------------
void setup(void)
{
  Serial.begin(115200);

  while (!Serial)
  {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }

  ina219_1.begin(&Wire2);
  ina219_2.begin(&Wire2);
  ina219_3.begin(&Wire2);
  ina219_4.begin(&Wire2);
  ina219_5.begin(&Wire2);
}
//----------------------------------------------------------------------------------------------------------------------
void loop(void)
{
  Adafruit_INA219 * p[5] = { &ina219_1, &ina219_2, &ina219_3, &ina219_4, &ina219_5 };

  for (int i = 0; i < 5; i++)
  {
    const char *labels[5] = { "Solar Panel 1", "Solar Panel 2", "Solar Panel 3", "Solar Panel 4", "Battery Board" };

    Serial.printf("%s: Bus Voltage %.3f V Current %.1f mA Power %.0f mW\n", labels[i], p[i]->getBusVoltage_V(), p[i]->getCurrent_mA(), p[i]->getPower_mW());
  }
  {
    Serial.println();
  }
  delay(2000);
}
//----------------------------------------------------------------------------------------------------------------------
