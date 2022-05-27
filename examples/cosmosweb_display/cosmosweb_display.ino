#define GPSECHO 1
#define DELAY_TIME 2000
//----------------------------------------------------------------------------------------------------------------------
// GPS
#include <Adafruit_GPS.h>
// Serial port the GPS is connected to
#define GPSSerial Serial7
// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
//----------------------------------------------------------------------------------------------------------------------
// Temperature Sensors
#define TEMPS_COUNT 7 // The total number of temperature sensors
// Analog Pin temperature sensors are connected to
const int TEMPS[TEMPS_COUNT] = { A0, A1, A6, A7, A8, A9, A17 };
//----------------------------------------------------------------------------------------------------------------------
// Magnetometer
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>
Adafruit_LIS3MDL lis3mdl;
//----------------------------------------------------------------------------------------------------------------------
// Accelerometer & Gyroscope
#include <Adafruit_LSM6DSOX.h>
Adafruit_LSM6DSOX sox;
//----------------------------------------------------------------------------------------------------------------------
uint32_t timer = 0;
//----------------------------------------------------------------------------------------------------------------------
#include <SPI.h>
//----------------------------------------------------------------------------------------------------------------------
// JSON string library
#include <ArduinoJson.h>
//----------------------------------------------------------------------------------------------------------------------
#include <NativeEthernet.h>
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress remoteIp(192, 168, 1, 161);  // Edit to your own IP address
unsigned short remotePort = 10091;
unsigned short localPort = 10091;
EthernetUDP udp;

void setup()
{
  // Give time for the Teensy to boot
  delay(1000);
  Serial.begin(115200);

  // Initialize Ethernet libary
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Ethernet connection not established"));
  }
  // Enable UDP
  udp.begin(localPort);

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS
  GPS.begin(9600);
  delay(100);
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  delay(100);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  delay(100);
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Magnetometer
  lis3mdl.begin_I2C(); // hardware I2C mode, can pass in address & alt Wire
  lis3mdl.setIntThreshold(500);
  lis3mdl.configInterrupt(false, false, true, // enable z axis
                          true, // polarity
                          false, // don't latch
                          true); // enabled!
  // Accelerometer & Gyroscope
  sox.begin_I2C();

  timer = millis();
}
//----------------------------------------------------------------------------------------------------------------------
void loop() // run over and over again
{
  if (GPS.available())
  {
    while (GPS.read()) {};
  }
  // Parse any newly received NMEA sentence.
  if (GPS.newNMEAreceived()) // Check to see if a new NMEA line has been received
  {
    if (GPS.parse(GPS.lastNMEA())) // A successful message was parsed
    {
      if (GPSECHO)
      {
        // https://github.com/adafruit/Adafruit_GPS/blob/96625de7d35302294ee648af6a050332e0d30186/src/NMEA_parse.cpp
        //Serial.print("GPS Message: ");
        //Serial.print(GPS.lastNMEA());
      }
    }
  }

  // approximately every 2 seconds or so, print out the current stats
  if ((millis() - timer) >= DELAY_TIME)
  {
    timer += DELAY_TIME;

    // GPS

    Serial.print("Time: ");

    if (GPS.hour < 10) {
      Serial.print('0');
    }

    Serial.print(GPS.hour, DEC); Serial.print(':');

    if (GPS.minute < 10) {
      Serial.print('0');
    }

    Serial.print(GPS.minute, DEC); Serial.print(':');

    if (GPS.seconds < 10) {
      Serial.print('0');
    }

    Serial.print(GPS.seconds, DEC); Serial.print('.');

    if (GPS.milliseconds < 10)
    {
      Serial.print("00");
    }
    else if (GPS.milliseconds > 9 && GPS.milliseconds < 100)
    {
      Serial.print("0");
    }

    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

    if (GPS.fix)
    {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", ");
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }

    // Temperature Sensors

    float voltage[TEMPS_COUNT] = { 0 }, temperatureC[TEMPS_COUNT] = { 0 };

    for (int i = 0; i < TEMPS_COUNT; i++)
    {
      const int reading = analogRead(TEMPS[i]); // getting the voltage reading from the temperature sensor

      voltage[i] = reading * (3.3 / 1024); // converting that reading to voltage

      const float temperatureF = (voltage[i] * 1000) -58 ;

      temperatureC[i] = (temperatureF - 32) / 1.8; // convert from fahrenheit to celsius 

      char buf[64];
      sprintf(buf, "Temperature %d: %.2f volts %.2f °C", i, voltage[i], temperatureC[i]);
      Serial.print(buf);
    }

    // Magnetometer

    // new sensor event, normalized to uTesla
    sensors_event_t event;
    lis3mdl.getEvent(&event);
    // Display the results (magnetic field is measured in uTesla)
    Serial.print("Magnetometer");
    Serial.print(" \tX: ");
    Serial.print(event.magnetic.x);
    Serial.print(" \tY: ");
    Serial.print(event.magnetic.y);
    Serial.print(" \tZ: ");
    Serial.print(event.magnetic.z);
    Serial.println(" µTesla");

    // Accelerometer-Gyroscope

    // Get a new normalized sensor event
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    sox.getEvent(&accel, &gyro, &temp);

    Serial.print("IMU Temperature ");
    Serial.print(temp.temperature);
    Serial.println(" °C");
    // Serial.printf("IMU Temperature %f °C\n", temp.temperature);

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("Acceleration\tX: ");
    Serial.print(accel.acceleration.x);
    Serial.print(" \tY: ");
    Serial.print(accel.acceleration.y);
    Serial.print(" \tZ: ");
    Serial.print(accel.acceleration.z);
    Serial.println(" m/s.² ");

    /* Display the results (rotation is measured in rad/s) */
    Serial.print("Gyroscope\tX: ");
    Serial.print(gyro.gyro.x);
    Serial.print(" \tY: ");
    Serial.print(gyro.gyro.y);
    Serial.print(" \tZ: ");
    Serial.print(gyro.gyro.z);
    Serial.println(" radians/s ");
    Serial.println();

    // UDP Doc

    StaticJsonDocument<512> doc;  // Increase this value if values are missing in the JSON document
    // To compute the appropriate capacity, go to https://arduinojson.org/v6/assistant/

    doc["time"] = millis();
    doc["node_name"] = "artemis";

    JsonObject gps = doc.createNestedObject("gps");
    gps["long"] = GPS.longitude;
    gps["lat"] = GPS.latitude;
    gps["altitude"] = GPS.altitude;
    gps["quality"] = GPS.fixquality;
    gps["speed"] = GPS.speed;
    gps["angle"] = GPS.angle;
    gps["satellites"] = GPS.satellites;

    char timestr[32];
    sprintf(timestr, "%04d-%02d-%02dT%02d:%02d:%02d.%dZ", GPS.year, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds, GPS.milliseconds);
    gps["time"] = timestr;

    JsonObject imu = doc.createNestedObject("imu");
    imu["temp"] = temp.temperature;

    JsonArray imu_accel = imu.createNestedArray("accel");
    imu_accel.add(accel.acceleration.x);
    imu_accel.add(accel.acceleration.y);
    imu_accel.add(accel.acceleration.z);

    JsonArray imu_gyro = imu.createNestedArray("gyro");
    imu_gyro.add(gyro.gyro.x);
    imu_gyro.add(gyro.gyro.y);
    imu_gyro.add(gyro.gyro.z);

    JsonArray imu_magn = imu.createNestedArray("magn");
    imu_magn.add(event.magnetic.x);
    imu_magn.add(event.magnetic.y);
    imu_magn.add(event.magnetic.z);


    // Add temperatures from the array to the JSON object

    JsonObject jtemp0 = doc.createNestedObject("obc temp");
    jtemp0["volts"] = voltage[0];
    jtemp0["celsius"] = temperatureC[0];

    JsonObject jtemp1 = doc.createNestedObject("pdu temp");
    jtemp1["volts"] = voltage[1];
    jtemp1["celsius"] = temperatureC[1];

    JsonObject jtemp7 = doc.createNestedObject("battery board temp");
    jtemp7["volts"] = voltage[7];
    jtemp7["celsius"] = temperatureC[7];

    JsonObject jtemp6 = doc.createNestedObject("solar panel 1 temp");
    jtemp6["volts"] = voltage[6];
    jtemp6["celsius"] = temperatureC[6];

    JsonObject jtemp8 = doc.createNestedObject("solar panel 2 temp");
    jtemp8["volts"] = voltage[8];
    jtemp8["celsius"] = temperatureC[8];

    JsonObject jtemp9 = doc.createNestedObject("solar panel 3 temp");
    jtemp9["volts"] = voltage[9];
    jtemp9["celsius"] = temperatureC[9];

    JsonObject jtemp17 = doc.createNestedObject("solar panel 4 temp");
    jtemp17["volts"] = voltage[17];
    jtemp17["celsius"] = temperatureC[17];

    //serializeJsonPretty(doc, Serial); //uncomment this if you want to see the json strings in the serial monitor

    // Send UDP packet
    udp.beginPacket(remoteIp, remotePort);
    serializeJson(doc, udp);
    udp.println();
    udp.endPacket();
  }
}
