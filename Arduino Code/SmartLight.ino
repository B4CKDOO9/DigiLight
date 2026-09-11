
#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TFT_eSPI.h>
#include "AXS15231B.h"
#include <WebServer.h>
#include <Preferences.h>

WebServer server(80);

JsonDocument doc;

struct Light
{
    String lightStateValue;
    String brightness;
};

const char* ssid = "M5";
const char* password = "LAPATAPI";

static const int W = 640;
static const int H = 180;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

int lastState = HIGH;
int currentState;


static void panelBlankAndBacklightOff()
{
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW); // OFF
  // Clear panel (portrait coords in driver)
  lcd_fill(0, 0, 180, 640, 0x0000);
}

static void backlightOn()
{
  digitalWrite(TFT_BL, HIGH);
}

bool flag = true;

void lightOn(short unsigned int dynamicColor)
{
  backlightOn();
  sprite.fillSprite(dynamicColor);
  sprite.setTextColor(TFT_BLACK, dynamicColor);
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

void parse_JSON_request(const  String &input, Light &p)
{
    deserializeJson(doc, input);
    p.lightStateValue = doc["lightState"] | "";
    p.brightness = doc["brightness"] | "";
}

void handlePortal()
{
  server.send(200, "text/html", 
  R"rawliteral(
      <!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        
        <style>
            body {
                margin: 0;
                min-height: 100vh;
                background-color: #59CDE9;  /* fallback for old browsers */
                background-image: -webkit-linear-gradient(to bottom, #0A2A88, #59CDE9);  /* Chrome 10-25, Safari 5.1-6 */
                background-image: linear-gradient(to bottom, #0A2A88, #59CDE9); /* W3C, IE 10+/ Edge, Firefox 16+, Chrome 26+, Opera 12+, Safari 7+ */
                background-repeat: no-repeat;
                background-attachment: fixed;
                background-position: center center;
                background-size: cover;
                display:flex; 
                align-items:center; 
                justify-content:center; 
                font-family:sans-serif;
            }
            .container {
                display: flex;
                align-items: center;
                background: Whitesmoke;
                padding: 3rem;
                border-radius: 5px;
                margin: 5rem auto;
                width: 50%;
                box-shadow: 0px 32px 40px -19px rgba(0,0,0,0.25);
            }
            .control-group {
                width: 100%;
                display: flex;
                flex-direction: column;
                align-items: center;
                gap: 1.25rem;
            }
            .slider-row {
                width: 100%;
                display: flex;
                align-items: center;
                gap: 1rem;
            }
            .slider {
                appearance: none;
                width: 100%;
                height: 8px;
                outline: none;
                border-radius: 5px;
                background: linear-gradient(to right, #1868e3 50%, #d3d3d3 50%);

            }
            .slider::-webkit-slider-thumb {
                appearance: none;
                width: 20px;
                height: 20px;
                background: #1868e3;
                cursor: pointer;
                border-radius: 50%;
            }
            .slider::-moz-range-thumb {
                appearance: none;
                width: 20px;
                height: 20px;
                background: #1868e3;
                cursor: pointer;
                border-radius: 50%;
            }
            .value {
                display: flex;
                justify-content: center;
                align-items: center;
                font-size: 20px;
                margin-left: 1rem;
                background: #1868e3;
                color: #ffffff;
                width: 80px;
                height: 40px;
                border-radius: 5px;
            }

                        /* Toggle switch styles (from Uiverse snippet) */
                        .toggle-container {
                            --active-color: #1868e3;
                            --inactive-color: #d3d3d6;
                            position: relative;
                            aspect-ratio: 292 / 142;
                            height: 3.25rem; /* a bit larger to match your slider */
                            width: 60%;
                            max-width: 320px;
                        }

                        .toggle-input {
                            appearance: none;
                            margin: 0;
                            position: absolute;
                            z-index: 1;
                            top: 0;
                            left: 0;
                            width: 100%;
                            height: 100%;
                            cursor: pointer;
                        }

                        .toggle {
                            width: 100%;
                            height: 100%;
                            overflow: visible;
                            display: block;
                        }

                        .toggle-background {
                            fill: var(--inactive-color);
                            transition: fill .4s;
                        }

                        .toggle-input:checked + .toggle .toggle-background {
                            fill: var(--active-color);
                        }

                        .toggle-circle-center {
                            transform-origin: center;
                            transition: transform .6s;
                        }

                        .toggle-input:checked + .toggle .toggle-circle-center {
                            transform: translateX(150px);
                        }

                        .toggle-circle {
                            transform-origin: center;
                            transition: transform .45s;
                            backface-visibility: hidden;
                        }

                        .toggle-circle.left { transform: scale(1); }
                        .toggle-input:checked + .toggle .toggle-circle.left { transform: scale(0); }
                        .toggle-circle.right { transform: scale(0); }
                        .toggle-input:checked + .toggle .toggle-circle.right { transform: scale(1); }

                        .toggle-icon { transition: fill .4s; }
                        .toggle-icon.on { fill: var(--inactive-color); }
                        .toggle-input:checked + .toggle .toggle-icon.on { fill: #fff; }
                        .toggle-icon.off { fill: #eaeaec; }
                        .toggle-input:checked + .toggle .toggle-icon.off { fill: var(--active-color); }
        </style>
    </head>
    <body>
        <div class="container">
            <div class="control-group">
                <div class="toggle-container">
                    <input type="checkbox" class="toggle-input" aria-label="toggle" id="switch">
                    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 292 142" class="toggle">
                        <path
                            d="M71 142C31.7878 142 0 110.212 0 71C0 31.7878 31.7878 0 71 0C110.212 0 119 30 146 30C173 30 182 0 221 0C260 0 292 31.7878 292 71C292 110.212 260.212 142 221 142C181.788 142 173 112 146 112C119 112 110.212 142 71 142Z"
                            class="toggle-background"></path>
                        <rect rx="6" height="64" width="12" y="39" x="64" class="toggle-icon on"></rect>
                        <path
                            d="M221 91C232.046 91 241 82.0457 241 71C241 59.9543 232.046 51 221 51C209.954 51 201 59.9543 201 71C201 82.0457 209.954 91 221 91ZM221 103C238.673 103 253 88.6731 253 71C253 53.3269 238.673 39 221 39C203.327 39 189 53.3269 189 71C189 88.6731 203.327 103 221 103Z"
                            fill-rule="evenodd" class="toggle-icon off"></path>
                        <g filter="url('#goo')">
                            <rect fill="#fff" rx="29" height="58" width="116" y="42" x="13" class="toggle-circle-center"></rect>
                            <rect fill="#fff" rx="58" height="114" width="114" y="14" x="14" class="toggle-circle left"></rect>
                            <rect fill="#fff" rx="58" height="114" width="114" y="14" x="164" class="toggle-circle right"></rect>
                        </g>
                        <filter id="goo">
                            <feGaussianBlur stdDeviation="10" result="blur" in="SourceGraphic"></feGaussianBlur>
                            <feColorMatrix result="goo" values="1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 18 -7" mode="matrix" in="blur">
                            </feColorMatrix>
                        </filter>
                    </svg>
                </div>

                <div class="slider-row">
                    <input type="range" class="slider" id="range" min="0" max="100" value="100">
                    <div class="value">100%</div>
                </div>
            </div>
        </div>
        <script>
            window.addEventListener("DOMContentLoaded", () => {
                const slider = document.getElementById("range");
                const value = document.querySelector(".value");
                const toggle = document.getElementById("switch");
                let lightStateValue = "0";

                function calcValue() {
                    if (!slider) return;

                    let valuePercentage = (slider.value / slider.max) * 100;
                    slider.style.background = `linear-gradient(to right, #1868e3 ${valuePercentage}%, #d3d3d3 ${valuePercentage}%)`;
                    value.innerHTML = `${slider.value}%`;
                }

                calcValue();

                if (slider) {
                    slider.addEventListener("input", calcValue);
                }

                if (toggle) {
                    toggle.addEventListener("change", () => {
                        if (toggle.checked) {
                            lightStateValue = "1";
                            sendData();
                        } else {
                            lightStateValue = "0";
                            sendData();
                        }
                    });
                }

                if (slider) {
                    slider.addEventListener("input", () => {
                        sendData();
                    });
                }

                const sendData = async () => {
                    if (!slider || !toggle) return;

                    const response = await fetch("/light", {
                        method: "POST",
                        headers: { "Content-Type": "application/json" },
                        body: JSON.stringify({
                            lightState: lightStateValue,
                            brightness: slider.value
                        })
                    });

                    return response;
                };
            });
        </script>
    </body>
</html>
    )rawliteral");
}


Light incoming;

void handleLight()
{
    Serial.println("HANDLER");
    Serial.println(incoming.brightness);
    String body = server.arg("plain");
    parse_JSON_request(body, incoming);
    server.send(200, "application/json", "{\"success\":true}");
}

void setup()
{
  Serial.begin(115200);
  axs15231_init();
  panelBlankAndBacklightOff();
  sprite.createSprite(W, H);
  sprite.setSwapBytes(1);
  sprite.setColorDepth(16);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  lightOff();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
  server.on("/", HTTP_GET, handlePortal);  
  server.on("/light", HTTP_POST, handleLight);
  server.begin();
}

bool lightState = false;
bool lastLightState = true;
unsigned int lastWebLightState = incoming.lightStateValue.toInt();
unsigned int WebLightState = 1;
unsigned int currentSliderValue;
unsigned int lastSliderValue = incoming.brightness.toInt();
const float lightConst = 2.55;

void loop() 
{
  // put your main code here, to run repeatedly:
  server.handleClient();
  currentState = digitalRead(PIN_BUTTON_1);
  currentSliderValue = incoming.brightness.toInt();
  WebLightState = incoming.lightStateValue.toInt();

  if(currentSliderValue != lastSliderValue)
  {
    unsigned int calculatedBrightness = ceil(currentSliderValue * lightConst);
    short unsigned int dynamicColor = tft.color565(calculatedBrightness, calculatedBrightness, calculatedBrightness);
    lightOn(dynamicColor);
    Serial.printf("cur=%u last=%u target=%u\n", currentSliderValue, lastSliderValue, calculatedBrightness);
  }

  lastSliderValue = currentSliderValue;

  if(lastState == LOW && currentState == HIGH)
  {
    lightState = !lightState;
  }

  lastState = currentState;
  
  if (WebLightState != lastWebLightState)
  {
    lightState = (WebLightState == 1);
  }

  lastWebLightState = WebLightState;

  if(lightState != lastLightState)
  {
    if(lightState)
    {
        lightOn(TFT_WHITE);
    }
    else
    {
      lightOff();
    }
    lastLightState = lightState;
  }
}
