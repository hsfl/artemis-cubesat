/*This code connects to the Adafruit Mini GPS PA1010D module.
It initializes the module then sets the NMEA output and update rate.
It then reads and parses the GPS data, and if it has a fix and is connected, it prints the relevant information to the Serial console.
GPSECHO can be set to true to also echo the raw GPS data to the Serial console.*/

#include <Adafruit_GPS.h>

// Create an instance of the Serial port for the GPS module
#define GPSSerial Serial7
Adafruit_GPS GPS(&GPSSerial);

// Set GPSECHO to "false" or "true" to turn on/off echoing the GPS data to the Serial console
const bool GPSECHO = true;

// Global variable to store GPS connection status
bool gpsConnected = false;

void setup()
{

    // Start the Serial port at 115200 baud rate
    Serial.begin(115200);

    // Start the GPS module at 9600 baud rate
    GPS.begin(9600);

    // Set the NMEA output to RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);

    // Set the NMEA update rate to 1 Hz
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_200_MILLIHERTZ);

    // Give time for the GPS Module to steup and user to open the serial monitor
    delay(5000);

    // Check if GPS module is connected and store the status in the gpsConnected variable
    if (GPS.available() == 0)
    {
        Serial.println("GPS module not detected");
        gpsConnected = false;
    }
    else
    {
        Serial.println("GPS module connected");
        gpsConnected = true;
    }

    // Clear any data from the GPS module
    while (GPS.available())
    {
        GPS.read();
    }
}

void loop()
{

    // Read data from the GPS in the 'main loop'
    char c = GPS.read();

    // If GPSECHO is true above, print the raw GPS data to the Serial console
    if (GPSECHO && c)
    {
        Serial.print(c);
    }

    // If a new NMEA sentence is received, parse it and print the data
    if (GPS.newNMEAreceived())
    {
        if (GPS.parse(GPS.lastNMEA()))
        {
            if (GPS.fix)
            {
                Serial.print("\nTime: ");
                if (GPS.hour < 10)
                {
                    Serial.print('0');
                }
                Serial.print(GPS.hour, DEC);
                Serial.print(':');
                if (GPS.minute < 10)
                {
                    Serial.print('0');
                }
                Serial.print(GPS.minute, DEC);
                Serial.print(':');
                if (GPS.seconds < 10)
                {
                    Serial.print('0');
                }
                Serial.println(GPS.seconds, DEC);
                Serial.print("Date: ");
                Serial.print(GPS.day, DEC);
                Serial.print('/');
                Serial.print(GPS.month, DEC);
                Serial.print("/20");
                Serial.println(GPS.year, DEC);
                Serial.print("Fix: ");
                Serial.println(GPS.fix ? "YES" : "NO");
                Serial.print("Quality: ");
                Serial.println(GPS.fixquality == 0 ? "No fix" : (GPS.fixquality == 1 ? "GPS fix" : (GPS.fixquality == 2 ? "Differential fix" : "Unknown fix status")));
                Serial.print("Satellites: ");
                Serial.println((int)GPS.satellites);
                Serial.print("Location: ");
                Serial.print(GPS.latitudeDegrees);
                Serial.print(", ");
                Serial.println(GPS.longitudeDegrees);
                Serial.print("Speed (knots): ");
                Serial.println(GPS.speed);
                Serial.print("Angle: ");
                Serial.println(GPS.angle);
                Serial.print("Altitude: ");
                Serial.println(GPS.altitude);
            }
        }
    }
}