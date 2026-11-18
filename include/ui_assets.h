/**
 * @file    ui_assets.h
 * @author  Er. Krishna Kohli (@krishnakohlidev)
 * @brief   UI components and color definitions for the ESP32 YouTube Music display
 * @version 1.0
 */

#ifndef UI_ASSETS_H
#define UI_ASSETS_H

#include <Arduino.h>
#include <TFT_eSPI.h>

// Custom RGB565 color palette
#define COLOR_BG      0x0841  // Deep dark charcoal
#define COLOR_CARD    0x18E3  // Card background
#define COLOR_YTM_RED 0x001F  // YouTube Music Red
#define COLOR_VINYL   0x2124  // Dark vinyl disc gray
#define COLOR_SUBTEXT 0xAD55  // Light gray for artist & timestamps
#define COLOR_ACCENT  0xFFFF  // Pure white

/**
 * @brief Draws the 64x64 Album Art Card
 * 
 * @param tft Reference to the TFT instance
 * @param x X-coordinate of the album art
 * @param y Y-coordinate of the album art
 * @param isPlaying Current playback state to animate the center icon
 */
inline void drawAlbumArt(TFT_eSPI &tft, int x, int y, bool isPlaying) {
    // Outer rounded album frame
    tft.fillRoundRect(x, y, 64, 64, 8, COLOR_CARD);
    tft.drawRoundRect(x, y, 64, 64, 8, COLOR_SUBTEXT);

    // Vinyl disc body
    int cx = x + 32;
    int cy = y + 32;
    tft.fillCircle(cx, cy, 26, COLOR_VINYL);
    tft.drawCircle(cx, cy, 22, COLOR_BG);
    tft.drawCircle(cx, cy, 18, COLOR_BG);

    // YouTube Music red center label
    tft.fillCircle(cx, cy, 12, COLOR_YTM_RED);
    tft.drawCircle(cx, cy, 9, COLOR_ACCENT);

    // Center play triangle inside album art
    tft.fillTriangle(cx - 3, cy - 4, cx - 3, cy + 4, cx + 5, cy, COLOR_ACCENT);
}

/**
 * @brief Draws the Backward, Play/Pause, and Forward transport controls
 * 
 * @param tft Reference to the TFT instance
 * @param centerY Y-coordinate center for the transport controls
 * @param isPlaying Current playback state to toggle Play/Pause icon
 */
inline void drawTransportButtons(TFT_eSPI &tft, int centerY, bool isPlaying) {
    // Clear button area (smaller height to avoid erasing timestamps)
    tft.fillRect(0, centerY - 10, 128, 20, COLOR_BG);

    // 1. Backward Button (|<<)
    tft.fillRect(20, centerY - 4, 2, 9, COLOR_ACCENT);
    tft.fillTriangle(22, centerY, 28, centerY - 4, 28, centerY + 4, COLOR_ACCENT);
    tft.fillTriangle(28, centerY, 34, centerY - 4, 34, centerY + 4, COLOR_ACCENT);

    // 2. Center Play / Pause Button Circle
    tft.fillCircle(64, centerY, 8, COLOR_YTM_RED); 
    if (isPlaying) {
        // Pause bars (||)
        tft.fillRect(61, centerY - 4, 2, 9, COLOR_ACCENT);
        tft.fillRect(65, centerY - 4, 2, 9, COLOR_ACCENT);
    } else {
        // Play triangle (>)
        tft.fillTriangle(62, centerY - 4, 62, centerY + 4, 68, centerY, COLOR_ACCENT);
    }

    // 3. Forward Button (>>|)
    tft.fillTriangle(106, centerY, 100, centerY - 4, 100, centerY + 4, COLOR_ACCENT);
    tft.fillTriangle(100, centerY, 94, centerY - 4, 94, centerY + 4, COLOR_ACCENT);
    tft.fillRect(107, centerY - 4, 2, 9, COLOR_ACCENT);
}

#endif // UI_ASSETS_H