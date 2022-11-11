// IMU readings from the Adafruit LSM6DSOX + LIS3MDL breakout board

#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>

Adafruit_LIS3MDL magnetometer;
Adafruit_LSM6DSOX imu;

void setup()
{
    delay(5000); // Give time for the Teensy to boot
    Serial.begin(115200);

    magnetometer.begin_I2C();
    imu.begin_I2C();

    // settings for magnetometer
    magnetometer.setPerformanceMode(LIS3MDL_LOWPOWERMODE);
    magnetometer.setDataRate(LIS3MDL_DATARATE_0_625_HZ);
    magnetometer.setRange(LIS3MDL_RANGE_16_GAUSS);
    magnetometer.setOperationMode(LIS3MDL_CONTINUOUSMODE);

    // settings for accelerometer & gyroscope
    imu.setAccelRange(LSM6DS_ACCEL_RANGE_16_G);
    imu.setGyroRange(LSM6DS_GYRO_RANGE_2000_DPS);
    imu.setAccelDataRate(LSM6DS_RATE_6_66K_HZ);
    imu.setGyroDataRate(LSM6DS_RATE_6_66K_HZ);
}

void loop()
{
    // prints accelerometer and gyrocscope readings on the serial monitor

    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;
    imu.getEvent(&accel, &gyro, &temp);

    Serial.print("\t\tTemperature ");
    Serial.print(temp.temperature);
    Serial.println(" deg C");

    /* Display the results (acceleration is measured in m/s^2) */
    Serial.print("\t\tAccel X: ");
    Serial.print(accel.acceleration.x);
    Serial.print(" \tY: ");
    Serial.print(accel.acceleration.y);
    Serial.print(" \tZ: ");
    Serial.print(accel.acceleration.z);
    Serial.println(" m/s^2 ");

    /* Display the results (rotation is measured in rad/s) */
    Serial.print("\t\tGyro X: ");
    Serial.print(gyro.gyro.x);
    Serial.print(" \tY: ");
    Serial.print(gyro.gyro.y);
    Serial.print(" \tZ: ");
    Serial.print(gyro.gyro.z);
    Serial.println(" radians/s ");
    Serial.println();

    // prints magnetometer readings on the serial console
    sensors_event_t event;
    magnetometer.getEvent(&event);
    /* Display the results (magnetic field is measured in uTesla) */
    Serial.print("\tX: ");
    Serial.print(event.magnetic.x);
    Serial.print(" \tY: ");
    Serial.print(event.magnetic.y);
    Serial.print(" \tZ: ");
    Serial.print(event.magnetic.z);
    Serial.println(" uTesla ");
}