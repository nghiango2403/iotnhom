#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <DHT.h>
#include <Wire.h>
#include <U8g2lib.h>
#define DHTTYPE DHT22
#define DHTPIN 14
#define led 2
#define OLED_SDA 13
#define OLED_SCL 12
U8G2_SH1106_128X64_NONAME_F_HW_I2C display (U8G2_R0, U8X8_PIN_NONE);
DHT dht(DHTPIN, DHTTYPE);
unsigned long kiemtracuoi = 0;
bool cudong = false;
const char *ssid = "Wokwi-GUEST";
const char *password = "";
const char *mqtt_server = "10.53.52.163";
const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
const long thoigian = 10000;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Dang ket noi WiFi... ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi da ket noi");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}
void connectBroker()
{
  while (!client.connected())
  {
    Serial.print("Connecting to MQTT ...");
    if (client.connect("MQTT"))
    {
      Serial.println("connected");
      client.subscribe("home/light/cmd");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void hienthiOled(float h, float t) {
  display.clearBuffer();         

  display.setCursor(0, 15);    
  display.print("Nhiet do: ");
  display.print(t, 1);       
  display.print(" C");

  display.setCursor(0, 40);    
  display.print("Do am:    ");
  display.print(h, 1);           
  display.print(" %");

  display.sendBuffer(); 
}
void nhannhietdo()
{
  float h = dht.readHumidity()+random(1, 20);
  float t = dht.readTemperature()+random(1, 20);
  if (isnan(h) || isnan(t))
  {
    client.publish("home/sensor/data", "Loi doc cam bien!");
    return;
  }
  String response = "{\"nhietdo\": " + String(t) + ", \"doam\": " + String(h) + "}";
  hienthiOled(h, t);
  client.publish("home/sensor/data", response.c_str());
  Serial.println("Da phan hoi: " + response);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char)payload[i];
  Serial.println("Massage arived [" + String(topic) + "]: " + incommingMessage);

  if (String(topic) == "home/light/cmd")
  {
    if (incommingMessage=="true")
    {
      digitalWrite(led, HIGH);
      Serial.println("Đèn: BẬT");
    }
    else if (incommingMessage=="false")
    {
      digitalWrite(led, LOW);
      Serial.println("Đèn: TẮT");
    }
  }
  if (String(topic) == "home/sensor")
  {
    nhannhietdo();
  }

}
void setup()
{
  Serial.begin(9600);
  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin();
  display.setFont(u8g2_font_ncenB08_tr);
  setup_wifi();
  pinMode(led, OUTPUT);
  dht.begin();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    connectBroker();
  }
  client.loop();
  unsigned long now = millis();
  if (now - kiemtracuoi >= thoigian || kiemtracuoi==0) {
    kiemtracuoi = now; 
    nhannhietdo(); 
    Serial.println("Da gui du lieu");
  }
}