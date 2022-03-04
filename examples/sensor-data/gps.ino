// GPS readings from Adafruit Mini GPS PA1010D Module 

#define GPSECHO 1
#define DELAY_TIME 2000
//----------------------------------------------------------------------------------------------------------------------
// GPS
#include <Adafruit_GPS.h>
// what's the name of the hardware serial port?
#define GPSSerial Serial7
// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
uint32_t timer = 0;
//----------------------------------------------------------------------------------------------------------------------
void setup()
{
    // Give time for circuit to stabalize
    delay(1000);

    Serial.begin(115200);
    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);
    delay(100);
    // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    delay(100);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
    delay(100);
    // For the parsing code to work nicely and have time to sort thru the data, and
    // print it out we don't suggest using anything higher than 1 Hz

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
        while(GPS.read()) {};
    }


    // Parse any newly received NMEA sentence.
    if (GPS.newNMEAreceived()) // Check to see if a new NMEA line has been received
    {
        if (GPS.parse(GPS.lastNMEA())) // A successful message was parsed
        {
            if (GPSECHO)
            {
                // https://github.com/adafruit/Adafruit_GPS/blob/96625de7d35302294ee648af6a050332e0d30186/src/NMEA_parse.cpp
                Serial.print(GPS.lastNMEA());
            }
        }
    }


    // approximately every 2 seconds or so, print out the current stats
    if ((millis() - timer) >= DELAY_TIME)
    {
        timer += DELAY_TIME;

        Serial.print("\nTime: ");

        if (GPS.hour < 10) { Serial.print('0'); }

        Serial.print(GPS.hour, DEC); Serial.print(':');

        if (GPS.minute < 10) { Serial.print('0'); }

        Serial.print(GPS.minute, DEC); Serial.print(':');

        if (GPS.seconds < 10) { Serial.print('0'); }

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
    }
}
