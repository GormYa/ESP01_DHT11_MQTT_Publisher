
#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <DHT.h>


#define DHTTYPE DHT11
#define DHTPIN  2

#define WLAN_SSID       "GormYa.Misafir"
#define WLAN_PASS       "misafir_8536677"

DHT dht(DHTPIN, DHTTYPE); 
 
float sensorNem;
float sensorIsi;
float hissedilen;

unsigned long previousMillis = 0;
const long interval = 2300;

WiFiClient client;
//WiFiClientSecure client;

// MQTT Server Bilgileri (client, ip_address, port, username, password)
Adafruit_MQTT_Client mqtt(&client, "192.168.2.251", 1883, "esp01", "91a324a6d31d5r_a1d0a3s5fe542");
Adafruit_MQTT_Publish sensor1 = Adafruit_MQTT_Publish(&mqtt, "/sensors");

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();

  Serial.println(F("MQTT Üzerinden Sıcaklık Ve Nem Bilgisi Gönderimi"));
  Serial.println();
  Serial.println();
  Serial.print(WLAN_SSID);
  Serial.print(" isimli ağa bağlanılıyor");

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Ağa bağlandı!");
  Serial.print("Alınan IP adresi: ");
  Serial.println(WiFi.localIP());
}

int delayTime = 10000;  //10 saniyede bir sensör bilgisini gönder
int startDelay = 0;

void loop() {
  if (millis() - startDelay > delayTime) {
    MQTT_connect();
    
    sensorNem = dht.readHumidity();
    sensorIsi = dht.readTemperature();
    hissedilen = dht.computeHeatIndex(sensorIsi, sensorNem, false);

    if (!isnan(sensorNem) && !isnan(sensorIsi)) {
      startDelay = millis();
      String stringData = "{\"isi\":\"" + String(sensorIsi, 0) + "\",\"nem\":\"" + String(sensorNem, 0) + "\",\"hissedilen\":\"" + String(hissedilen, 0) + "\"}";
      char gonderilecekData[stringData.length()];
      stringData.toCharArray(gonderilecekData, stringData.length()+1);
      
      if (!sensor1.publish(gonderilecekData)) {
        Serial.println("Veri gönderilemedi!");
      } else {
        Serial.println("Gönderilen sensör bilgisi: " + stringData);
      }
    }
    else{
      Serial.println("Sensör bilgisi okunamadı!");
    }
    if(! mqtt.ping()) {
      mqtt.disconnect();
    }
  }
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.println("MQTT sunucusuna bağlanılıyor...");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("5 saniye sonra bağlantı tekrar denenecek...");
       mqtt.disconnect();
       delay(5000);
       retries--;
       if (retries == 0) {
         while (1); // 3 kere bağlantı denemesinden sonra cihaz resetlensin
       }
  }
  Serial.println("MQTT sunucusuna bağlandı.");
}
