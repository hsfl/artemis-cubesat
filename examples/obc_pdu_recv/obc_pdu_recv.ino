/*
 * This code is purposed to run with the Artemis OBC and Artemis PDU.
 * 
 * Functionality:
 * Receive commands from the RFM69 radio and handle switches on the PDU
 * to enable/disable 3.3V, 5V, and 12V switches.
 * 
 */

#include <RFM69.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

/* ----- RADIO FREQ CONFIG ----- */
#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY     RF69_915MHZ

/* ----- RFM69 PIN CONFIG ----- */
#define CS_PIN  38
#define INT_PIN 8
#define RST_PIN 7

/* ----- SPI1 BUS CONFIG ----- */
#define SPI_MISO 39
#define SPI_MOSI 26
#define SPI_SCK 27

/* ----- RFM69 NETWORKING CONFIG ----- */
#define NETWORKID     0   // Must be the same for all nodes
#define MYNODEID      2   // My node ID
#define TONODEID      1   // Destination node ID

#define ENCRYPT       true // Set to "true" to use encryption
#define ENCRYPTKEY    "TOPSECRETPASSWRD" // Use the same 16-byte key on all nodes

#define USEACK        true // Request ACKs or not

/* ----- CREATE RFM69 RADIO OBJECT ----- */
RFM69 rfm69(CS_PIN, INT_PIN, true, &SPI1);

/* ----- CREATE CURRENT SENSOR OBJECTS ----- */
Adafruit_INA219 INA219_1(0x40);

/* ----- GLOBAL VARIABLES ----- */
String RADIO_DATA_IN = "";
String UART1_RX = "";

/* ----- DECLARE FUNCTIONS ----- */
void RESET_RADIO();
bool READ_CMD();
void INA219_INIT();
void INA219_GET_DATA();
void RFM69_INIT();
void RFM69_RECV();
void PDU_UART_SEND();
void PDU_UART_RECV();

/* ----- SETUP ----- */
void setup()
{
  Serial.begin(115200);
  Serial1.begin(115200);

  SPI1.setMISO(SPI_MISO);
  SPI1.setMOSI(SPI_MOSI);
  SPI1.setSCK(SPI_SCK);

  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, LOW);

  RESET_RADIO();
  RFM69_INIT();

  INA219_INIT();
}

/* ----- MAIN LOOP ----- */
void loop()
{
  RFM69_RECV();
  PDU_UART_RECV();
}

/* ----- MANUALLY RESET RADIO ----- */
void RESET_RADIO() {
  digitalWrite(RST_PIN, HIGH);
  delay(10);
  digitalWrite(RST_PIN, LOW);
  delay(10);
}

/* ----- INITIALIZE CURRENT SENSOR ----- */
void INA219_INIT() {
  if(!INA219_1.begin(&Wire2)) {
    Serial.println("Failed to find INA219 with address 0x40");
  } else {
    Serial.println("INA219 0x40 Initialized!");
  }
}

/* ----- GET CURRENT SENSOR DATA ----- */
void INA219_GET_DATA(uint16_t addr) {
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  switch(addr) {
    case 0x40:
      shuntvoltage = INA219_1.getShuntVoltage_mV();
      busvoltage = INA219_1.getBusVoltage_V();
      current_mA = INA219_1.getCurrent_mA();
      power_mW = INA219_1.getPower_mW();
      break;
  }

  loadvoltage = busvoltage + (shuntvoltage / 1000);
  Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
  Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
  Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
  Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
  Serial.println("");
}

/* ----- INITIALIZE RFM69 RADIO ----- */
void RFM69_INIT() {
  while(rfm69.initialize(FREQUENCY, MYNODEID, NETWORKID) == false) {
    Serial.println("RFM69 INIT FAILED. Trying Again ...");
    delay(100);
  }
  rfm69.setHighPower(); // Always use this for RFM69HCW
  
  if (ENCRYPT)
    rfm69.encrypt(ENCRYPTKEY);

  Serial.print("Node ");
  Serial.print(MYNODEID,DEC);
  Serial.println(" successfully initialized!"); 
}

/* ----- RECEIVE DATA FROM RFM69 RADIO ----- */
void RFM69_RECV() {
  if (rfm69.receiveDone()) {
    Serial.print("received from node ");
    Serial.print(rfm69.SENDERID, DEC);
    Serial.print(", message [");

    RADIO_DATA_IN = "";
    for (byte i = 0; i < rfm69.DATALEN; i++) {
      Serial.print((char)rfm69.DATA[i]);
      RADIO_DATA_IN += (char)rfm69.DATA[i];
    }

    // RSSI is the "Receive Signal Strength Indicator", smaller numbers mean higher power.
    Serial.print("], RSSI "); 
    Serial.println(rfm69.RSSI);

    // Check if you want to send msg to PDU
    if(READ_CMD(RADIO_DATA_IN))
      PDU_UART_SEND(RADIO_DATA_IN);

    if (rfm69.ACKRequested())
    {
      rfm69.sendACK();
      Serial.println("ACK sent");
    }
  }
}

/* ----- SEND MESSAGE TO PDU THROUGH UART ----- */
void PDU_UART_SEND(String buf) {
  Serial1.print(buf.c_str());
  Serial1.print('\n');
  Serial.print("SENDING TO PDU: ");
  Serial.print(buf.c_str());
  Serial.print('\n');
}

/* ----- RECEIVE MESSAGE FROM PDU THROUGH UART ----- */
void PDU_UART_RECV() {
  if (Serial1.available() > 0) {
    UART1_RX = Serial1.readString();
    if(UART1_RX.length() > 0) {
      Serial.print("UART RECV: ");
      Serial.println(UART1_RX);
    }
  }
}

/* ----- READ INCOMING MESSAGES FROM RADIO AND CHECK IF IT IS A VALID COMMAND ----- */
bool READ_CMD(String buf) {
  if(buf.indexOf("CMD: READ INA219") == 0) {
    if(buf.indexOf("0x40") > 0) {
      INA219_GET_DATA(0x40);
    } else {
      Serial.println("Please specify a valid address");
    }
  }
  else if(
      buf.indexOf("CMD: GPIO ON ALL") == 0 || 
      buf.indexOf("CMD: GPIO OFF ALL") == 0 || 
      buf.indexOf("CMD: SW_3V3_1") == 0 ||
      buf.indexOf("CMD: SW_3V3_2") == 0 ||
      buf.indexOf("CMD: SW_5V_1") == 0 ||
      buf.indexOf("CMD: SW_5V_2") == 0 ||
      buf.indexOf("CMD: SW_5V_3") == 0 ||
      buf.indexOf("CMD: SW_5V_4") == 0 ||
      buf.indexOf("CMD: SW_12V") == 0 ||
      buf.indexOf("CMD: FATFS") == 0  
    )
    return true;

  return false;
}
