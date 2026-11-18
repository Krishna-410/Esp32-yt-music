/**
 * @file    main.cpp
 * @author  Er. Krishna Kohli (@krishnakohlidev)
 * @brief   Main application logic for the ESP32 Bluetooth A2DP Sink with TFT UI
 * @version 1.0
 */

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "BluetoothA2DPSink.h"
#include "ui_assets.h"

TFT_eSPI tft = TFT_eSPI();
BluetoothA2DPSink a2dp_sink;

// Track & playback state variables
char currentTitle[64]  = "Not Connected";
char currentArtist[64] = "Pair ESP32 YT Music";
uint32_t totalDurationSec = 0;
uint32_t currentPosSec    = 0;
bool isPlaying            = false;
bool isConnected          = false;

// Flags for thread-safe screen updates
volatile bool needsFullRedraw     = true;
volatile bool needsTimelineRedraw = false;
volatile bool needsControlsRedraw = false;

unsigned long lastTickMs = 0;

/**
 * @brief Formats raw seconds into a "M:SS" string buffer
 */
void formatTime(char *buf, size_t len, uint32_t totalSeconds) {
    uint32_t mins = totalSeconds / 60;
    uint32_t secs = totalSeconds % 60;
    snprintf(buf, len, "%lu:%02lu", (unsigned long)mins, (unsigned long)secs);
}

/**
 * @brief Truncates long strings with ".." to fit the 128px screen width
 */
String fitText(const char *text, uint8_t maxChars) {
    String s = String(text);
    if (s.length() > maxChars) {
        return s.substring(0, maxChars - 2) + "..";
    }
    return s;
}

/**
 * @brief Renders the top status bar and connection indicator
 */
void drawHeader() {
    tft.fillRect(0, 0, 128, 15, COLOR_BG);
    
    // Brand Logo
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(COLOR_YTM_RED, COLOR_BG);
    tft.drawString("YT", 6, 2, 2);
    tft.setTextColor(COLOR_ACCENT, COLOR_BG);
    tft.drawString("Music", 24, 2, 2);

    // Custom Watermark
    tft.setTextDatum(TR_DATUM);
    tft.setTextColor(COLOR_SUBTEXT, COLOR_BG);
    tft.drawString("KK", 108, 4, 1); 

    // Connection indicator dot (top right)
    uint16_t dotColor = isConnected ? TFT_GREEN : TFT_ORANGE;
    tft.fillCircle(118, 8, 4, dotColor);
    tft.drawFastHLine(0, 15, 128, COLOR_CARD);
}

/**
 * @brief Renders the current track title and artist name
 */
void drawTrackText() {
    tft.fillRect(0, 86, 128, 30, COLOR_BG);
    tft.setTextDatum(TC_DATUM);

    tft.setTextColor(COLOR_ACCENT, COLOR_BG);
    tft.drawString(fitText(currentTitle, 18), 64, 86, 2);

    tft.setTextColor(COLOR_SUBTEXT, COLOR_BG);
    tft.drawString(fitText(currentArtist, 21), 64, 103, 1);
}

/**
 * @brief Renders the playback progress bar and timestamps
 */
void drawTimeline() {
    tft.fillRect(0, 116, 128, 18, COLOR_BG);
    tft.drawRoundRect(8, 118, 112, 5, 2, COLOR_SUBTEXT);

    int fillWidth = 0;
    if (totalDurationSec > 0) {
        fillWidth = (currentPosSec * 110) / totalDurationSec;
        if (fillWidth > 110) fillWidth = 110;
    } else if (isPlaying) {
        // Fallback animated progress if duration isn't reported
        fillWidth = (currentPosSec % 60) * 110 / 60;
    }

    if (fillWidth > 0) {
        tft.fillRoundRect(9, 119, fillWidth, 3, 1, COLOR_YTM_RED);
    }

    char posStr[12];
    char durStr[12];
    formatTime(posStr, sizeof(posStr), currentPosSec);
    formatTime(durStr, sizeof(durStr), totalDurationSec);

    tft.setTextColor(COLOR_SUBTEXT, COLOR_BG);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(posStr, 8, 126, 1);

    tft.setTextDatum(TR_DATUM);
    tft.drawString(durStr, 120, 126, 1);
}

/**
 * @brief Executes a full screen wipe and UI redraw
 */
void drawFullUI() {
    tft.fillScreen(COLOR_BG);
    drawHeader();
    drawAlbumArt(tft, 32, 20, isPlaying);
    drawTrackText();
    drawTimeline();
    drawTransportButtons(tft, 144, isPlaying);
}

/* ======================================================================
 * Bluetooth AVRCP Callbacks
 * ====================================================================== */

void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
    if (!text) return;

    switch (id) {
        case ESP_AVRC_MD_ATTR_TITLE:
            if (strncmp(currentTitle, (const char *)text, sizeof(currentTitle) - 1) != 0) {
                strncpy(currentTitle, (const char *)text, sizeof(currentTitle) - 1);
                currentTitle[sizeof(currentTitle) - 1] = '\0';
                currentPosSec = 0; 
            }
            needsFullRedraw = true;
            break;
            
        case ESP_AVRC_MD_ATTR_ARTIST:
            strncpy(currentArtist, (const char *)text, sizeof(currentArtist) - 1);
            currentArtist[sizeof(currentArtist) - 1] = '\0';
            needsFullRedraw = true;
            break;
            
        case ESP_AVRC_MD_ATTR_PLAYING_TIME:
            // Source device sends track length in milliseconds as a string
            uint32_t durationMs = strtoul((const char *)text, NULL, 10);
            totalDurationSec = durationMs / 1000;
            needsTimelineRedraw = true;
            break;
    }
}

void avrc_playstatus_callback(esp_avrc_playback_stat_t playback) {
    bool nowPlaying = (playback == ESP_AVRC_PLAYBACK_PLAYING);
    if (nowPlaying != isPlaying) {
        isPlaying = nowPlaying;
        needsControlsRedraw = true;
    }
}

void connection_state_callback(esp_a2d_connection_state_t state, void *ptr) {
    isConnected = (state == ESP_A2D_CONNECTION_STATE_CONNECTED);
    if (!isConnected) {
        strcpy(currentTitle, "Not Connected");
        strcpy(currentArtist, "Pair ESP32 YT Music");
        isPlaying = false;
        currentPosSec = 0;
        totalDurationSec = 0;
    }
    needsFullRedraw = true;
}

void avrc_rn_play_pos_callback(uint32_t play_pos) {
    currentPosSec = play_pos / 1000; 
    needsTimelineRedraw = true;
}

/* ======================================================================
 * Main Setup & Loop
 * ====================================================================== */

void setup() {
    Serial.begin(115200);

    // Initialize 1.8" 128x160 ST7735 Display in Portrait Mode
    tft.init();
    tft.setRotation(0);
    drawFullUI();

    // Configure AVRCP Metadata Requests
    a2dp_sink.set_avrc_metadata_attribute_mask(
        ESP_AVRC_MD_ATTR_TITLE |
        ESP_AVRC_MD_ATTR_ARTIST |
        ESP_AVRC_MD_ATTR_PLAYING_TIME
    );

    // Bind Callbacks
    a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
    a2dp_sink.set_avrc_rn_playstatus_callback(avrc_playstatus_callback);
    a2dp_sink.set_on_connection_state_changed(connection_state_callback);
    a2dp_sink.set_avrc_rn_play_pos_callback(avrc_rn_play_pos_callback);
    
    // Start Bluetooth Receiver
    a2dp_sink.start("ESP32 YT Music");
}

void loop() {
    // 1. Process pending UI updates from Bluetooth callbacks
    if (needsFullRedraw) {
        needsFullRedraw = false;
        drawFullUI();
    }
    if (needsControlsRedraw) {
        needsControlsRedraw = false;
        drawTransportButtons(tft, 144, isPlaying);
    }
    if (needsTimelineRedraw) {
        needsTimelineRedraw = false;
        drawTimeline();
    }

    // 2. Local timeline interpolation
    if (isPlaying && (millis() - lastTickMs >= 1000)) {
        lastTickMs = millis();
        if (totalDurationSec == 0 || currentPosSec < totalDurationSec) {
            currentPosSec++;
            drawTimeline();
        }
    }
}