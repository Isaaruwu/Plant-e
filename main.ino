#include "thingProperties.h"
#include <Arduino_MKRIoTCarrier.h>
MKRIoTCarrier carrier;
 
int moistPin;
 
String waterPumpState;
String coolingFanState;
String lightState;
 
uint32_t lightsOn = carrier.leds.Color(82, 118, 115);
uint32_t lightsOff = carrier.leds.Color(0, 0, 0);
 
void setup() {
  Serial.begin(9600);
  delay(1500); 
 
  initProperties();
 
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
 
  while (ArduinoCloud.connected() != 1) {
    ArduinoCloud.update();
    delay(500);
  }
  
  delay(500);
  CARRIER_CASE = false;
  carrier.begin();
  moistPin = carrier.getBoardRevision() == 1 ? A5 : A0;
  carrier.display.setRotation(0);
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.drawBitmap(65, 70, CAT_LOGO, 100, 100, ST77XX_WHITE);
  starting_melody();
  updateScreen();
  delay(1500);
}
 
void loop() {
  ArduinoCloud.update();
  
  temperature = carrier.Env.readTemperature();
  humidity = carrier.Env.readHumidity();
 
  int raw_moisture = analogRead(moistPin);
  moisture = map(raw_moisture, 0, 1023, 100, 0);
  
  
  if (moisture > 20) {
    String event_name = "watering";
    notifyWatering(event_name);
    delay(500);
    waterpump = true;
    onWaterpumpChange();
    waterpump = false;
  }

  delay(100);
}

void notifyWatering(String event_name){
  String url = "/trigger/" + event_name + "/json/with/key/" + API_KEY;
  client.post(url, "application/json", "{}");

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);\
  delay(5000);
}

void circularAnimation() {
  for (int i = 0; i < 5; i++) {
    carrier.leds.fill(lightsOn, i, 1);
    carrier.Buzzer.beep(800);
    carrier.leds.show();
    if (i > 0) {
      int prevLed = i-1;
      carrier.leds.fill(lightsOff, prevLed, 1);
      carrier.leds.show();
    }
    delay(100);
  }
  carrier.leds.fill(lightsOff, 4, 1);
  carrier.leds.show();
}

void onWaterpumpChange() {
  if (waterpump) {
    carrier.Relay2.open();
    waterPumpState = "PUMP: ON";
    updateScreen();
    
    // Light animation
    for (int i = 0; i < 10; i++) {
      circularAnimation();
      delay(100);
    }
    
    delay(250);
    carrier.Relay2.close();
    waterpump = false;
    waterPumpState = "PUMP: OFF";
    updateScreen();
  }
}
 
void onCoolingFanChange() {
  if (cooling_fan == true) {
    carrier.Relay1.open();
    coolingFanState = "FAN: ON";
  } else {
    carrier.Relay1.close();
    coolingFanState = "FAN: OFF";
  }
  updateScreen();
}
 
void onArtificialLightChange() {
  if (artificial_light == true) {
    carrier.leds.fill(lightsOn, 0, 5);
    carrier.leds.show();
    lightState = "LIGHTS: ON";
  } else {
    carrier.leds.fill(lightsOff, 0, 5);
    carrier.leds.show();
    lightState = "LIGHTS: OFF";
  }
  updateScreen();
}
 
void updateScreen() {
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.setTextColor(ST77XX_WHITE);
  carrier.display.setTextSize(3);
 
  carrier.display.setCursor(40, 50);
  carrier.display.print(waterPumpState);
  carrier.display.setCursor(40, 90);
  carrier.display.print(coolingFanState);
  carrier.display.setCursor(40, 130);
  carrier.display.print(lightState);
}

void starting_melody() {
  int finalMelody[] = {
    1319, 1568, 2637, 2093, 2349, 3136, 2637, 1568
  };
  int noteDurations[] = {
    4, 8, 8, 4, 4, 4, 4, 4
  };

  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    carrier.Buzzer.sound(finalMelody[thisNote]);
    delay(noteDuration);
    int pauseBetweenNotes = noteDuration * 1.0;
    delay(pauseBetweenNotes);
    carrier.Buzzer.noSound();
  }
}