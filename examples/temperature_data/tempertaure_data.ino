// Temperature readings from TMP36

#define aref_voltage 3.3         
 
//TMP36 Pin Variables
int tempPin = A0;        //the analog pin the TMP36's Vout (sense) pin is connected to
                        //the resolution is 10 mV / degree centigrade with a
                        //500 mV offset to allow for negative temperatures
int tempReading;        // the analog reading from the sensor
 
void setup(void) {
  // We'll send debugging information via the Serial monitor
  Serial.begin(9600);   
}
 
void loop(void) {
 
  tempReading = analogRead(tempPin);  
 
  Serial.print("Temp reading = ");
  Serial.print(tempReading);     // the raw analog reading
 
  // converting that reading to voltage, which is based off the reference voltage
  float voltage = tempReading * aref_voltage;
  voltage /= 1024.0; 
 
  // print out the voltage
  Serial.print(" - ");
  Serial.print(voltage); Serial.println(" volts");
 
  // now print out the temperature in Fahrenheight
  float temperatureF = (voltage * 1000) -58 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((volatge - 500mV) times 100)
  Serial.print(temperatureF); Serial.println(" degrees F");
 
  // now convert to Celcius
  float temperatureC = (temperatureF - 32) / 1.8;
  Serial.print(temperatureC); Serial.println(" degrees C");
 
  delay(1000);
}
