// Read temperature data from all seven TMP36 temperature sensors

#define TEMPS_COUNT 7  // The total number of temperature sensors
#define aref_voltage 3.3

// Analog Pin temperature sensors are connected to
const int TEMPS[TEMPS_COUNT] = { A0, A1, A6, A7, A8, A9, A17 };
const char *temp_sen_names[TEMPS_COUNT] = { "obc_temp", "pdu_temp", "battery_board_temp", "solar_panel_1_temp", "solar_panel_2_temp", "solar_panel_3_temp", "solar_panel_4_temp" };
int tempReading;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {

  // Temperature Sensors

  float voltage[TEMPS_COUNT] = { 0 }, temperatureC[TEMPS_COUNT] = { 0 };

  for (int i = 0; i < TEMPS_COUNT; i++) {
    const int reading = analogRead(TEMPS[i]);  // getting the voltage reading from the temperature sensor
    voltage[i] = reading * aref_voltage;       // converting that reading to voltage
    voltage[i] /= 1024.0;
    const float temperatureF = (voltage[i] * 1000) - 58;
    temperatureC[i] = (temperatureF - 32) / 1.8;  // convert from fahrenheit to celsius
    Serial.printf("Temperature %d: %.2f volts %.2f°C\n", i, voltage[i], temperatureC[i]);
  }
  delay(5000);
}
