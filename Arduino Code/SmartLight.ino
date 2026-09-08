

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "AXS15231B.h"
#include <Preferences.h>

static const int W = 640;
static const int H = 180;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

int lastState = HIGH;
int currentState;

static void lightOn()
{
  backlightOn();
  sprite.fillSprite(TFT_WHITE);
  sprite.setTextColor(TFT_BLACK, TFT_WHITE);
  sprite.drawString("Light on!", 20, 20 ,4);
  lcd_PushColors_rotated_90(0, 0, W, H, (uint16_t *)sprite.getPointer());
}

static void lightOff()
{
  backlightOn();
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.drawString("Light off!", 20, 20, 4);
  lcd_PushColors_rotated_90(0, 0, W, H, (uint16_t *)sprite.getPointer());
}

void setup() {
  // put your setup code here, to run once:
  axs15231_init();
  panelBlankAndBacklightOff();
  sprite.createSprite(W, H);
  sprite.setSwapBytes(1);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  lightOff();
}

bool lightState = false;
bool lastLightState = true;

void loop() {
  // put your main code here, to run repeatedly:
  currentState = digitalRead(PIN_BUTTON_1);
  if(lastState == LOW && currentState == HIGH)
  {
    lightState = !lightState;
  }

  lastState = currentState;

  if(lightState != lastLightState)
  {
    if(lightState)
    {
      lightOn();
    }
    else
    {
      lightOff();
    }
    lastLightState = lightState;
  }
}