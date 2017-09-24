#include <ESP8266WiFi.h>


//#define ESSID "GNET"
#define ESSID "GNET3"
#define PSW "aqua@332345678"
#define AP_CHANNEL 0
#define MAX_ESSID_LEN 20
#define RSSI_UNDEF -200
#define NETWORK_SCAN_RETRIES 5

void reconnect() {

  char tmp[MAX_ESSID_LEN];
  int32_t n, max_rssi, rssi, channel;
  char bssid[6];
  uint8_t i;

  Serial.println("Reconnect");

  max_rssi = RSSI_UNDEF;
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();


  for (i=0; i<NETWORK_SCAN_RETRIES; i++) {
  
    Serial.printf("Scanning networks... (%d/%d)\n", i+1, NETWORK_SCAN_RETRIES);
    n = WiFi.scanNetworks();
    Serial.println("Scan done");

  
    if (n > 0) {

      for (int i = 0; i < n; ++i) {

        String cssid = WiFi.SSID(i);
        rssi = WiFi.RSSI(i);
        cssid.toCharArray(tmp, MAX_ESSID_LEN);
      
      
        if (strcmp(tmp, ESSID) == 0 && rssi > max_rssi) {
        
          Serial.println("Network found");
        
          max_rssi = rssi;
          channel = WiFi.channel();
          strncpy(bssid, (const char *)WiFi.BSSID(), 6); 
        }
      }
    }

    if (max_rssi != RSSI_UNDEF) {
      break;
    }
    
  }

  if (n == 0 || max_rssi == RSSI_UNDEF) {

    Serial.println("No network found!");
    Serial.println("Setting up an AP");
    configure_AP(ESSID, PSW, AP_CHANNEL);
  }
  else {
    configure_STA(ESSID, PSW, channel, bssid);
  }
  
  Serial.println("");
}


void configure_STA(char ssid[], char password[], int32_t channel, char bssid[6]) {

  Serial.printf("Connecting in STA mode to network %s\n", ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password, channel, (const uint8_t *)bssid, true);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");

  
}


void configure_AP(char ssid[], char password[], int32_t channel) {

  IPAddress local_IP(192,168,4,1);
  IPAddress gateway(192,168,4,1);
  IPAddress subnet(255,255,255,0);
  IPAddress dhcp_start(192,168,4,10);
  IPAddress dhcp_end(192,168,4,20);
  
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  //WiFi.softAPConfig(local_IP, gateway, subnet, dhcp_start, dhcp_end);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ESSID, PSW, channel, false);
  
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());

}


void setup() {
  
  Serial.begin(115200);
  Serial.println();

  Serial.println("\n\n\n\rBOOT");

  delay(100);
  reconnect();
}


void loop() {
  Serial.printf("Connection status: %d\n", WiFi.status());
  delay(5000);
}

