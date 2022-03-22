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
// Temperature
int sensorPin = A0; //the analog pin the TMP36 is connected to
//----------------------------------------------------------------------------------------------------------------------
// Magnetometer
#include <Wire.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>
Adafruit_LIS3MDL lis3mdl;
//----------------------------------------------------------------------------------------------------------------------
// Accelerometer
#include <Adafruit_LSM6DS33.h>
Adafruit_LSM6DS33 lsm6ds33;
//----------------------------------------------------------------------------------------------------------------------
uint32_t timer = 0;
//----------------------------------------------------------------------------------------------------------------------
// SD CARD
#include <SD.h>
#include <SPI.h>
File myFile;
const int chipSelect = BUILTIN_SDCARD;
//----------------------------------------------------------------------------------------------------------------------
// JSON string library
#include <ArduinoJson.h>
//----------------------------------------------------------------------------------------------------------------------

#include <NativeEthernet.h>
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress remoteIp(192, 168, 1, 161);  // <- EDIT!!!!
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
    Serial.println(F("Failed to initialize Ethernet library"));
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

  lis3mdl.begin_I2C(); // hardware I2C mode, can pass in address & alt Wire
  lis3mdl.setIntThreshold(500);
  lis3mdl.configInterrupt(false, false, true, // enable z axis
                          true, // polarity
                          false, // don't latch
                          true); // enabled!

  lsm6ds33.begin_I2C();

  // SD CARD

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    while (1) { // No SD card, so don't do anything more - stay stuck here
    }
  }

  timer = millis();
}
//----------------------------------------------------------------------------------------------------------------------
void loop() // run over and over again
{
  // Helpful link: https://arduino.stackexchange.com/questions/72637/i-am-having-strange-issues-when-trying-to-read-form-gps-serial-connection


  // This is ALWAYS needed in order to pump new messages. Even if you do not want the messages you still
  // need to loop GPS.read() over & over until it fails every loop() call.
  if (GPS.available())
  {
    while (GPS.read()) {};
  }


  // Parse any newly received NMEA sentence. Make sure to parse all mmessages even if you do not want to print them!
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

    // Temperature

    const int reading = analogRead(sensorPin); // getting the voltage reading from the temperature sensor
    const float voltage = (reading * 3.3) / 1024; // converting that reading to voltage
    Serial.print("OBC Board Temperature\t");
    Serial.print(voltage); Serial.print(" volts\t");
    float temperatureF = (voltage * 1000) - 58; // now print out the temperature
    Serial.print(temperatureF); Serial.println(" °F\t");

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
    lsm6ds33.getEvent(&accel, &gyro, &temp);

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
    Serial.println(" m/s² ");

    /* Display the results (rotation is measured in rad/s) */
    Serial.print("Gyroscope\tX: ");
    Serial.print(gyro.gyro.x);
    Serial.print(" \tY: ");
    Serial.print(gyro.gyro.y);
    Serial.print(" \tZ: ");
    Serial.print(gyro.gyro.z);
    Serial.println(" radians/s ");
    Serial.println();

    // open the file.
    myFile = SD.open("flightdata.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (myFile) {
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

      JsonObject jtemp = doc.createNestedObject("obc temp");
      jtemp["volts"] = voltage;
      jtemp["farenheit"] = temperatureF;

      //serializeJsonPretty(doc, Serial); //uncomment this if you want to see the json strings in the serial monitor
      serializeJson(doc, myFile);
      myFile.println();  // ensures JSON documents are line-delimited
      myFile.close();

      // Send UDP packet
      udp.beginPacket(remoteIp, remotePort);
      serializeJson(doc, udp);
      udp.println();
      udp.endPacket();
    }
  }
}
