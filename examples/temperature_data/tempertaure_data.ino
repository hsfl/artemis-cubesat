// Temperature read from seven TMP36 sensors

#define aref_voltage 3.3
#define temps_count 7 // total number of temperarture sensors

const int temps[temps_count] = {A0, A1, A6, A7, A8, A9, A17}; // Analog Pin temperature sensors are connected to
const char *labels[temps_count] = {"obc", "pdu", "battery board", "solar pannel 1", "solar panel 2", "solar panel 3", "solar panel 4"};

int tempReading;

void setup()
{
  Serial.begin(9600);
}

void loop()
{

  float voltage[temps_count] = {0}, temperatureC[temps_count] = {0};

  for (int i = 0; i < temps_count; i++)
  {
    const int reading = analogRead(temps[i]); // getting the voltage reading from the temperature sensor
    voltage[i] = reading * aref_voltage;      // converting that reading to voltage
    voltage[i] /= 1024.0;
    const float temperatureF = (voltage[i] * 1000) - 58;
    temperatureC[i] = (temperatureF - 32) / 1.8; // convert from fahrenheit to celsius

    Serial.print(labels[i]);
    Serial.print(": ");
    Serial.print(voltage[i]);
    Serial.print(" ");
    Serial.print("volts");
    Serial.print(" ");
    Serial.print(temperatureC[i]);
    Serial.println("°C");
  }
  delay(10000);
}