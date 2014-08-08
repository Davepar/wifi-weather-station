/* 
*  Simple WiFi weather station with Arduino, the DHT11 sensor & the CC3000 chip
*  Part of the code is based on the work done by Adafruit on the CC3000 chip & the DHT11 sensor
*  Writtent by Marco Schwartz for Open Home Automation
*/

// Include required libraries
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "DHT.h"

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// WiFi network (change with your settings !)
#define WLAN_SSID       "yourNetwork"        // cannot be longer than 32 characters!
#define WLAN_PASS       "yourPassword"
#define WLAN_SECURITY   WLAN_SEC_WPA2 // This can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2

// dweet.io ID. Change to a unique ID.
#define DWEET_ID        "weatherstation"

// DHT11 sensor pins
#define DHTPIN 7 
#define DHTTYPE DHT11

// Create CC3000 & DHT instances
DHT dht(DHTPIN, DHTTYPE);
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2);

// Server IP
uint32_t ip = 0;

void setup(void)
{
 
  // Initialize DHT sensor
  dht.begin();
  
  Serial.begin(115200);
  Serial.print("Initializing connection...");

  // Initialise the CC3000 module
  if (!cc3000.begin())
  {
    while(1);
  }

  // Connect to  WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println("Connected to WiFi network!");
    
  // Check DHCP
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100);
  }

  char host[] = "dweet.io";
  Serial.print(host);
  Serial.print(F(" -> "));
  while  (ip  ==  0)  {
    if  (!cc3000.getHostByName(host, &ip))  {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }
  cc3000.printIPdotsRev(ip);
  Serial.println();
}

void loop(void)
{
  
    // Measure the humidity & temperature
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    // Convert to fahrenheit tenths of a degree
    t = (t * 9 / 5 + 32) * 10;

    // Transform to string (temp formatted to include tenths of a degree)
    String temp = String((int) t);
    temp = temp.substring(0, temp.length() - 1) + "." + temp[temp.length() - 1];
    String humidity = String((int) h);

    // Print data
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.println("");
    
    // Send request
    String request = String("GET /dweet/for/") + DWEET_ID + "?temp=" + temp + "&hum=" + humidity + " HTTP/1.0";
    send_request(request);

    // Update every three minutes
    delay(180000);
}

// Function to send a TCP request and get the result as a string
void send_request (String request) {
     
    // Connect    
    Serial.println("Starting connection to server...");
    Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);

    // Send request
    if (client.connected()) {
      client.println(request);      
      client.println(F(""));
      Serial.println("Connected & Data sent");
    } 
    else {
      Serial.println(F("Connection failed"));    
    }

    while (client.connected()) {
      while (client.available()) {

        // Read answer
        char c = client.read();
        // Serial.print(c);
      }
    }
    Serial.println();
    Serial.println("Closing connection");
    Serial.println();
    client.close();
    
}
