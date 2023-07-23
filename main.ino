#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <Arduino_MKRIoTCarrier.h>

#include "thingProperties.h"
#include "visual.h"

MKRIoTCarrier carrier;

int moistPin;
bool fanOppened;

const int FAN_THRESHOLD = 30;


uint32_t lightsOn = carrier.leds.Color(82, 118, 115);
uint32_t lightsOff = carrier.leds.Color(0, 0, 0);

uint32_t greenLight = carrier.leds.Color(25, 135, 84);
uint32_t yellowLight = carrier.leds.Color(255, 191, 0);
uint32_t redLight = carrier.leds.Color(165, 42, 42);

  
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
  
  moistPin = A6;
  
  carrier.display.setRotation(0);
  carrier.display.fillScreen(ST77XX_BLACK);
  carrier.display.drawBitmap(65, 70, CAT_LOGO, 100, 100, ST77XX_WHITE);
  
  starting_melody();
  delay(1500);
}
 
void loop() {
  ArduinoCloud.update();
  
  temperature = carrier.Env.readTemperature();
  handleFan(temperature);
  delay(100);

  humidity = carrier.Env.readHumidity();
 
  int raw_moisture = analogRead(moistPin);
  moisture = map(raw_moisture, 0, 1023, 100, 0);
  
  if (moisture > 40) {
    carrier.leds.fill(greenLight, 0, 5);
    carrier.leds.show();
  } else if (moisture > 25) {
    carrier.leds.fill(yellowLight, 0, 5);
    carrier.leds.show();
  } else {
    carrier.leds.fill(redLight, 0, 5);
    carrier.leds.show();
    handleMoisture(moisture);
  }

  delay(500);
}

void handleMoisture(int moisture) {
  String event_name = "watering";
  notifyWatering(event_name);
  waterpump = true;
  onWaterpumpChange();
  waterpump = false;
}

void notifyWatering(String event_name){
  String url = "/trigger/" + event_name + "/json/with/key/" + API_KEY;
  client.post(url, "application/json", "{}");

  delay(6000);
}

void circularAnimation() {
  for (int i = 0; i < 5; i++) {
    carrier.leds.fill(lightsOn, i, 1);
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
    // Light animation
    for (int i = 0; i < 5; i++) {
      carrier.Buzzer.beep(800);
      circularAnimation();
      delay(100);
    }
    carrier.Relay2.close();
    delay(500);
  }
}

void handleFan(int temperature) {
  if (temperature > FAN_THRESHOLD && !fanOppened) {
    carrier.Relay1.open();
  }
  else if (temperature <= FAN_THRESHOLD && fanOppened) {
    carrier.Relay1.close();
  }
  fanOppened = !fanOppened;
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
