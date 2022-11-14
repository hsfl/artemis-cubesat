#include <Wire.h>
#include "Adafruit_SI1145.h"

Adafruit_SI1145 uv = Adafruit_SI1145();

void setup()
{
  Serial.begin(9600);

  if (!uv.begin())
  {
    Serial.println("Didn't find Si1145");
    while (1)
      ;
  }
}

void loop()
{
  Serial.print("Vis: ");
  Serial.println(uv.readVisible());
  Serial.print("IR: ");
  Serial.println(uv.readIR());

  float UVindex = uv.readUV();
  UVindex /= 100.0;
  Serial.print("UV: ");
  Serial.println(UVindex);

  delay(1000);
}
