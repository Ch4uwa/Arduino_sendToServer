#include <SoftwareSerial.h>
#include "WiFiEsp.h"
#include "dht.h"
#include <RtcDS3231.h>
#include "U8glib.h"
#include <Wire.h>

#define dht_apin A0

// Create module object on GPIO pin
// wifi 6 (RX) and 7 (TX)

SoftwareSerial Serial1(6, 7);

WiFiEspClient client;
dht DHT;
U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);

RtcDS3231<TwoWire> rtcModule(Wire);

byte hours;
byte minutes;
byte seconds;

short year;
byte month;
byte day;

byte WeekDay;

// Declare and initialise global arrays for WiFi settings
const char ssid[] = "MartinAP";
const char pass[] = "qwertyui";
const char server[] = "primat.se";
const String hostname = "primat.se";
const String uri = "/services/data/mamati@sti.se-mamati.csv";
const String senduri = "/services/sendform.aspx?xdata=mamati@sti.se|mamati|";
const int port = 80;

String dataT;
String dataH;
String line;

// Declare and initialise variable for radio status
int status = WL_IDLE_STATUS;

void setup()
{
    // Initialize serial for debugging
    Serial.begin(115200);
    // Initialize serial
    Serial1.begin(9600);

    // Enable I2C communication
    

    // Update RTC module time to compilation time
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    rtcModule.SetDateTime(compiled);
 
    //Initialize wifi and connect
    //wifiInit();
    tempHumid();
    updateTime();
    updateDate();
}

void loop()
{

    oled.firstPage();
    do
    {
        draw();
    } while (oled.nextPage());
    delay(20);
    updateTime();
    if (!seconds%10) {
        tempHumid();
    }
    if (minutes==00) {
        updateDate();
    }
    
    
    
    //sendData();
    //delay(1000);
    //readData();

    // if the server's disconnected, stop the client
    

    
}

void updateTime()
{
    RtcDateTime now = rtcModule.GetDateTime();
    hours = now.Hour();
    minutes = now.Minute();
    seconds = now.Second();
}

void updateDate()
{
    RtcDateTime now = rtcModule.GetDateTime();
    year = now.Year();
    month = now.Month();
    day = now.Day();
    WeekDay = now.DayOfWeek();
}

void draw()
{
    oled.setFont(u8g_font_timB10);
    oled.setFontPosTop();
    int h = oled.getFontAscent() - oled.getFontDescent();
    
    oled.setPrintPos(0, h*3);
    oled.print(dataT);
    oled.setPrintPos(0, h*4);
    oled.print(dataH + "\%");

    char timeString[10];
    sprintf(timeString, "%02u:%02u:%02u", hours, minutes, seconds);
    int w = (oled.getWidth() - oled.getStrWidth(timeString))/2;
    oled.setPrintPos(w, 0);
    oled.print(timeString);
    char dateString[12];
    sprintf(dateString, "%04u/%02u/%02u", year, month, day);
    w = (oled.getWidth() - oled.getStrWidth(dateString))/2;
    oled.setPrintPos(w, h);
    oled.print(dateString);
}

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

void tempHumid()
{
    DHT.read11(dht_apin);
    dataT = "Temp: " + String(DHT.temperature);
    dataH = "Humid: " + String(DHT.humidity);
    
    //delay(5000); //Wait 5 seconds before accessing sensor again.
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
        delay(50);
        client.println("GET " + senduri + dataT + dataH + " HTTP/1.1");
        client.println("Host: " + hostname);
        client.println("Connection: close");
        client.println();
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
    
    if (!client.connected())
    {
        client.stop();
    }
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
