ESP32 YouTube Music Bluetooth Display
A custom ESP32-based Bluetooth audio receiver (A2DP Sink) featuring a dynamic, YouTube Music-inspired user interface on a 1.8" ST7735 TFT display. This project extracts AVRCP metadata from your smartphone to render real-time track information, playback progress, and a custom vinyl-style album art visual.

Built with C++ using PlatformIO.

✨ Features
Real-Time Metadata Extraction: Displays the current Song Title and Artist using Bluetooth AVRCP.

Live Playback Timeline: Tracks and interpolates playback position with a visual progress bar and MM:SS timestamps.

Dynamic UI States: Play/Pause transport controls update instantly based on the source device's state.

Custom UI Assets: Hardcoded RGB565 color palette matching YouTube Music's dark mode, featuring a drawn vinyl record and clean typography.

Headless Audio Sink: Acts as a fully functional Bluetooth audio receiver (connects as "ESP32 YT Music").

🛠️ Hardware Requirements
ESP32 Development Board (e.g., DOIT DevKit V1)

1.8" ST7735 SPI TFT Display (128x160 resolution)

Jumper wires & Breadboard

Wiring (Pinout)
The display is configured via hardware SPI. Wire your ST7735 to the ESP32 as follows:

TFT Pin	ESP32 GPIO	Description
VCC	3.3V	Power
GND	GND	Ground
CS	15	Chip Select
RESET	4	Reset
A0 / DC	2	Data/Command
SDA / MOSI	23	SPI MOSI
SCK / SCLK	18	SPI Clock
LED	3.3V	Backlight Power
(Note: Audio output requires wiring an I2S DAC to the standard I2S pins, or modifying the sink to output via the internal DAC depending on your audio hardware setup).

💻 Software Setup
This project uses PlatformIO. The platformio.ini file is already configured with the necessary display drivers and build flags, meaning you do not need to manually edit the User_Setup.h file in the TFT_eSPI library.

Clone the repository:

Bash
git clone https://github.com/Krishna-410/esp32-yt-music.git
cd esp32-yt-music
Build and Upload:
If you prefer the terminal (e.g., on Fedora Linux), you can use PlatformIO Core:

Bash
pio run -t upload
Alternatively, open the folder in VS Code with the PlatformIO extension and click the Upload arrow.

Monitor Serial Output (Optional):

Bash
pio device monitor -b 115200
📱 Usage
Power up the ESP32. The display will initialize and show "Not Connected".

Open your smartphone's Bluetooth settings and look for ESP32 YT Music.

Pair and connect. The top-right status dot will turn green.

Open YouTube Music (or any media player), play a track, and the display will instantly sync the title, artist, and timeline.

📦 Dependencies
TFT_eSPI by Bodmer (Graphics & UI)

ESP32-A2DP by Phil Schatzmann (Bluetooth Audio & AVRCP)

Author: Er. Krishna Kohli (@krishnakohlidev)
