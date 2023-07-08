#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char SSID[]     = SECRET_SSID;    // Network SSID (name)
const char PASS[]     = SECRET_OPTIONAL_PASS;  // Network password (use for WPA, or use as key for WEP)
const char API_KEY[]  = SECRET_API;

void onArtificialLightChange();
void onWaterpumpChange();

float humidity;
float temperature;
int light;
int moisture;
bool artificial_light;
bool waterpump;

void initProperties(){

  ArduinoCloud.addProperty(humidity, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(temperature, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(light, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(moisture, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(artificial_light, READWRITE, ON_CHANGE, onArtificialLightChange);
  ArduinoCloud.addProperty(waterpump, READWRITE, ON_CHANGE, onWaterpumpChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
//IFTT API for notifications
WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, "maker.ifttt.com", 80);
