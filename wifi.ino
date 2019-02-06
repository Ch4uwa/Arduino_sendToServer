#include <SoftwareSerial.h>
#include "WiFiEsp.h"
#include "dht.h"
#include "NewPing.h"

#define dht_apin A0
#define TRIG_PIN A1
#define ECHO_PIN A2
#define MAX_DISTANCE 450

// Create WiFi module object on GPIO pin 6 (RX) and 7 (TX)
SoftwareSerial Serial1(6, 7);
WiFiEspClient client;
dht DHT;

NewPing sonar(TRIG_PIN,ECHO_PIN,MAX_DISTANCE);

// Declare and initialise global arrays for WiFi settings
const char ssid[] = "ASUS";
const char pass[] = "*********";
const char server[] = "primat.se";
const String hostname = "primat.se";
const String uri = "/services/data/mamati@sti.se-mamati.json";
const String senduri = "/services/sendform.aspx?xid=mamati&xmail=mamati@sti.se";
const int port = 80;

String data1;
String data2;
String line;
// Declare and initialise variable for radio status
int status = WL_IDLE_STATUS;

void wifiInit()
{
    // Initialize ESP module
    WiFi.init(&Serial1);

    // Check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");

        // Don't continue
        while (true)
            ;
    }

    // Attempt to connect to WiFi network
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network
        status = WiFi.begin(ssid, pass);
    }

    Serial.println("You're connected to the network");
    printWifiStatus();
    Serial.println();
}

void setup()
{
    // Initialize serial for debugging
    Serial.begin(115200);

    // Initialize serial
    Serial1.begin(9600);

    //Delay to let system boot
    delay(500);

    
}

void loop()
{
    tempHumid();
    tempHumid();

    Serial.print("Ping: ");
    Serial.print(readDistance()); // Send ping, get distance in cm and print result (0 = outside set distance range)
    Serial.println("cm");

    /* sendData();
    delay(2000);
    readData();

    // if the server's disconnected, stop the client
    if (!client.connected())
    {
        Serial.println();
        Serial.println("Disconnecting from server...");
        client.stop();
    }

    delay(30000); */
}

int readDistance()
{
    delay(50); // Wait 50ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
    int cm = sonar.ping_cm();
    if (cm==0) {
        cm = MAX_DISTANCE;
    }
    return cm;
}

void tempHumid()
{
    DHT.read11(dht_apin);

    data1 = "&Temperature=" + String(DHT.temperature);
    data2 = "&Humidity=" + String(DHT.humidity);

    delay(5000); //Wait 5 seconds before accessing sensor again.
                 //Fastest should be once every two seconds.
}

void readData()
{
    Serial.println();
    Serial.println("Reading from server...");
    // if you get a connection, report back via serial
    if (client.connect(server, port))
    {
        Serial.println("Connected to server");
        // Make a HTTP request
        client.println("GET " + uri + " HTTP/1.1");
        client.println("Host: " + hostname);
        client.println("Connection: close");
        client.println();
        delay(10);
    }
    /* if there are incoming bytes available
     * from the server, read them and print them */
    while (client.available() == 0)
    {
        delay(5);
    }
    while (client.available())
    {
        char c = client.read();
        Serial.write(c);
    }
}

void sendData()
{
    Serial.println();
    Serial.println("Sending to server...");
    // if you get a connection, report back via serial

    if (client.connect(server, port))
    {
        Serial.println("Connected to server");

        client.println("GET " + senduri + data1 + data2 + " HTTP/1.1");
        client.println("Host: " + hostname);
        client.println("Connection: close");
        client.println();
        delay(10);
    }
    while (client.available() == 0)
    {
        delay(5);
    }
    while (client.available())
    {
        line = client.readString();
    }
    Serial.println(line);
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}
