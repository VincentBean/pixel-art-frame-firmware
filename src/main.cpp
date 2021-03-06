#include <Arduino.h>
#include <FS.h>
#include "SdFat.h"
#include <SPIFFS.h>
#include <SPI.h>

#include "Global.h"
#include "Configuration.hpp"
#include "GifLoader.hpp"
#include "GifPlayer.hpp"
#include "WebServer.hpp"
#include "Wifi.hpp"
#include "MatrixGif.hpp"
#include "MatrixText.hpp"
#include "MatrixTime.hpp"
#include "OTA.h"
#include "Filesystem.hpp"

#define VERSION "1.0"

#define BULB_GIF "/bulb.gif"

#define E_PIN_DEFAULT   18

MatrixPanel_I2S_DMA dma_display;
VirtualMatrixPanel virtualDisp(dma_display, NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, true);

// Flag for the OFF state
bool displayClear = false;

frame_status_t frame_state = STARTUP;
frame_status_t target_state = STARTUP;
frame_status_t lastState = frame_state;
unsigned long lastStateChange = 0;

Config config;

TaskHandle_t scheduled_task;

void handleBrightness()
{
  if (!displayClear)
  {
    virtualDisp.clearScreen();
    displayClear = true;
  }

  ShowGIF("/bulb.gif", true);

  if (millis() - lastStateChange > 2000)
  {
    displayClear = false;
    target_state = PLAYING_ART;
  }
}

void handleScheduled(void *param)
{
  setupWifi();

  initServer();

  setupNTPClient();

  for (;;)
  {
    if (sd_state == MOUNTED)
      handleGifQueue();

    // Time
    if (config.enableTime)
    {
      if (millis() - lastTimeShow > config.timeInterval * 1000)
      {
        lastTimeShow = millis();
        target_state = SHOW_TIME;
      }

      // Update time2 secs before we should show it
      if ((millis() + 2000) - lastTimeShow > config.timeInterval * 1000)
      {
        updateTime();
      }
    }

    vTaskDelay(1 / portTICK_PERIOD_MS); // https://github.com/espressif/esp-idf/issues/1646#issue-299097720
  }
}

// TODO: Write proper display test
void displayTest()
{
  dma_display.clearScreen();
  dma_display.setCursor(0,0);
  dma_display.setTextColor(dma_display.color565(255, 255, 255));
  dma_display.setTextSize(1);
  dma_display.setTextWrap(true);
  dma_display.println("DISPLAY TEST");

  delay(1000);
  dma_display.clearScreen();

  for (int x = 0; x < MATRIX_WIDTH; x++) {
    for (int y = 0; y < MATRIX_HEIGHT; y++) {
        dma_display.drawPixel(x, y, dma_display.color565(255, 0, 0));
        Serial.println("Drawing X:" + String(x) + " Y:" + String(y));
        delay(10);
    }
  }

}

void setup()
{
  Serial.begin(115200);

  if (!mount_fs())
  {
    frame_state = target_state = SD_CARD_ERROR;
  }

  loadSettings();

  dma_display.setPanelBrightness(config.brightness);
  dma_display.setMinRefreshRate(200);

#if FM6126A_PANEL
  dma_display.begin(R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT, A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT, LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT, FM6126A);
#else
  dma_display.begin(R1_PIN_DEFAULT, G1_PIN_DEFAULT, B1_PIN_DEFAULT, R2_PIN_DEFAULT, G2_PIN_DEFAULT, B2_PIN_DEFAULT, A_PIN_DEFAULT, B_PIN_DEFAULT, C_PIN_DEFAULT, D_PIN_DEFAULT, E_PIN_DEFAULT, LAT_PIN_DEFAULT, OE_PIN_DEFAULT, CLK_PIN_DEFAULT);
#endif

  virtualDisp.fillScreen(dma_display.color565(0, 0, 0));

  InitMatrixGif(&virtualDisp);

  xTaskCreatePinnedToCore(
      handleScheduled, /* Function to implement the task */
      "schedule",      /* Name of the task */
      10000,           /* Stack size in words */
      NULL,            /* Task input parameter */
      0,               /* Priority of the task */
      &scheduled_task, /* Task handle. */
      0);              /* Core where the task should run */
}

void handleStartup()
{
  ShowGIF(LOGO_GIF, true);

  if (millis() - lastStateChange > 5000 && frame_state == STARTUP && (frame_state != INDEXING && target_state != INDEXING))
  {
    target_state = PLAYING_ART;
  }
}

bool targetStateValid()
{
  if (target_state == PLAYING_ART && sd_state != MOUNTED)
  {
    return false;
  }

  if (frame_state == INDEXING && target_state != PLAYING_ART)
  {
    return false;
  }

  return true;
}

void loop()
{
  if (sd_state != MOUNTED)
  {
    Serial.println("SD state not mounted in loop()");
    target_state = frame_state = SD_CARD_ERROR;
  }

  if (!gifPlaying && target_state != frame_state && targetStateValid())
  {
    frame_state = target_state;
    lastStateChange = millis();
  }

  if (frame_state == OFF && !displayClear)
  {
    virtualDisp.fillScreen(dma_display.color565(0, 0, 0));
    displayClear = true;
  }

  if (frame_state == STARTUP)
  {
    handleStartup();
  }

  if (frame_state == PLAYING_ART)
  {
    handleGif();
  }

  if (frame_state == SHOW_TEXT)
  {
    handleText();
  }

  if (frame_state == SHOW_TIME)
  {
    handleTime();
  }

  if (frame_state == INDEXING)
  {
    virtualDisp.clearScreen();
    virtualDisp.setCursor(0, 0);
    virtualDisp.println("Indexing\nFiles: " + String(total_files));
    delay(200);
  }

  if (frame_state == CONNECT_WIFI)
  {
    connecting();
  }

  if (frame_state == ADJ_BRIGHTNESS)
  {
    handleBrightness();
  }

  if (config.wifiMode == WIFI_AP_STA)
  {
    handleDns();
  }

  if (frame_state == SD_CARD_ERROR)
  {
    Serial.println("Handling sd card error");
    handle_sd_error();
  }

  if (frame_state == OTA_UPDATE)
  {
    handleOTA();
  }

  if (frame_state == ERROR)
  {
    virtualDisp.fillScreen(virtualDisp.color333(0, 0, 0));
    virtualDisp.setCursor(0, 0);
    virtualDisp.println("ERROR");
  }
}
