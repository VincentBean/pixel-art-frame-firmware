#ifndef _GLOBAL_
#define _GLOBAL_

#include <queue>
#include <FS.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include "Configuration.hpp"
#include "MatrixText.hpp"
#include "Filesystem.hpp"


// 2 panels are currently supported, 64x64 and 128x32. Set this to false for 64x64 and to true for 128x32
#define PANEL_128_32 false

// Set to true if you're using two 64x32 panels to create 64x64
#define TWO_PANELS false
#define FM6126A_PANEL false

#if PANEL_128_32

#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 2 // Number of INDIVIDUAL PANELS per ROW

#else

#if TWO_PANELS
// Config for 2 64x32 panels chained in a stacked config.
// Change MATRIX_WIDTH to 128 in ESP32-HUB75-MatrixPanel-I2S-DMA.h
#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

#else

#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_RES_Y 64 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW

#endif

#endif

#if PANEL_128_32
#define LOGO_GIF "/logo_128x32.gif"
#else
#define LOGO_GIF "/logo_64x64.gif"
#endif


#define GIF_DIR "/gifs"

#define INDEX_FILENAME_PREFIX "index."
#define INDEX_DIRECTORY "/.index"
#define INDEX_SIZE 250

typedef enum {
    OFF = 0, // LED matrix is off
    PLAYING_ART, // Looping trough art
    SHOW_TEXT,
    SHOW_TIME,
    STARTUP,

    INDEXING,
    CONNECT_WIFI, // Connecting to WiFi
    ADJ_BRIGHTNESS, // Adjusting brightness
    SD_CARD_ERROR,
    
    // Add new states above this, higher states than this cannot be set by the API!

    OTA_UPDATE,
    ERROR

} frame_status_t;

extern sd_state_t sd_state;

extern frame_status_t target_state;
extern unsigned long lastStateChange;

extern Config config;

extern MatrixPanel_I2S_DMA dma_display;
extern VirtualMatrixPanel virtualDisp;

extern bool interruptGif, gifsLoaded;
extern std::vector<String> gifs;
extern bool loadGifFromSpiffs;

extern Text text;

extern int brightness;
extern bool autoPlay;
extern bool gifPlaying;
extern bool allowNextGif;
extern bool queue_populate_requred;
extern unsigned long total_files;
extern String current_gif;

#endif