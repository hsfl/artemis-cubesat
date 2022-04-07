#include <ArduinoJson.h>
#include <NativeEthernet.h>
#include <SPI.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress remoteIp(000, 000, 0, 000);  // <- Edit this to the IP address of your computer
unsigned short remotePort = 10091;
unsigned short localPort = 10091;
EthernetUDP udp;

extern unsigned long _heap_end;
extern char *__brkval;

int freeram() {
  return (char *)&_heap_end - __brkval;
}

void setup() {
  // Initialize serial port
  Serial.begin(9600);
  while (!Serial) continue;

  // Initialize Ethernet libary
  if (!Ethernet.begin(mac)) {
    Serial.println(F("Failed to initialize Ethernet library"));
    return;
  }

  // Enable UDP
  udp.begin(localPort);

  while (!Serial) ;
  Serial.print("freeram = ");
  Serial.println(freeram());
  volatile char *p = (char *)malloc(152000);
  *p = 0;
  Serial.print("freeram = ");
  Serial.println(freeram());

}

void loop() {
  // Allocate a temporary JsonDocument
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<500> doc;

 JsonObject freeram = doc.createNestedObject(511904);


  // Log
  Serial.print(F("Sending to "));
  Serial.print(remoteIp);
  Serial.print(F(" on port "));
  Serial.println(remotePort);
  serializeJson(doc, Serial);

  // Send UDP packet
  udp.beginPacket(remoteIp, remotePort);
  serializeJson(doc, udp);
  udp.println();
  udp.endPacket();

  // Wait
  delay(10000);
}
