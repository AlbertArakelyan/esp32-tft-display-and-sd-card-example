#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ---------------------------------------------------------------- pins
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define SD_CS     5

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// ------------------------------------------------------------- keypad
// TODO: #include <Keypad.h>, keymap, and Keypad object go here.
// Rows: 13, 14, 16, 17, 21   Cols: 22, 25, 26, 27

void setupKeypad() {
  // TODO
}

char readKeypad() {
  // TODO: return keypad.getKey();
  return 0;
}

// ---------------------------------------------------------------- misc
int  tftLine   = 10;              // y cursor for the boot log
unsigned long lastTick = 0;
int  seconds   = 0;

// Print one line to both the display and serial, then advance.
void logLine(const char *msg, uint16_t color) {
  Serial.println(msg);
  tft.setTextColor(color, ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, tftLine);
  tft.println(msg);
  tftLine += 22;
}

// ------------------------------------------------------------- SD test
bool sdWriteReadTest() {
  const char *path = "/hello.txt";

  // --- write ---
  File f = SD.open(path, FILE_WRITE);      // FILE_WRITE truncates; FILE_APPEND adds
  if (!f) {
    logLine("open for write failed", ILI9341_RED);
    return false;
  }
  f.println("Hello from ESP32");
  f.printf("boot at %lu ms\n", millis());
  f.close();
  logLine("wrote hello.txt", ILI9341_GREEN);

  // --- read back ---
  f = SD.open(path, FILE_READ);
  if (!f) {
    logLine("open for read failed", ILI9341_RED);
    return false;
  }

  Serial.println("--- file contents ---");
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(1);
  while (f.available()) {
    String line = f.readStringUntil('\n');
    Serial.println(line);
    tft.setCursor(10, tftLine);
    tft.println(line);
    tftLine += 12;
  }
  Serial.println("---------------------");
  f.close();

  tftLine += 8;
  return true;
}

// ---------------------------------------------------------------- setup
void setup() {
  Serial.begin(115200);
  delay(200);                     // let the serial monitor attach

  tft.begin();
  tft.setRotation(1);             // landscape, 320x240
  tft.fillScreen(ILI9341_BLACK);

  logLine("TFT ok", ILI9341_CYAN);

  setupKeypad();

  delay(5000);

  // SPI.begin(18, 19, 23, SD_CS);
  // SD after the TFT so the display has finished its own init first.
  // 20 MHz; drop toward 4000000 if reads are flaky on long jumpers.
  if (!SD.begin(SD_CS, SPI, 4000000)) {
    logLine("SD init FAILED", ILI9341_RED);
  } else {
    uint8_t type = SD.cardType();
    if (type == CARD_NONE) {
      logLine("no card", ILI9341_RED);
    } else {
      char buf[40];
      snprintf(buf, sizeof(buf), "SD ok, %llu MB", SD.cardSize() / (1024ULL * 1024ULL));
      logLine(buf, ILI9341_GREEN);
      sdWriteReadTest();
    }
  }
}

// ----------------------------------------------------------------- loop
void loop() {
  char key = readKeypad();
  if (key) {
    Serial.println(key);
    tft.fillRect(240, 200, 70, 30, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.setCursor(240, 200);
    tft.print(key);
  }

  if (millis() - lastTick >= 1000) {
    lastTick += 1000;
    tft.setTextColor(ILI9341_YELLOW, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 215);
    tft.printf("Uptime: %d s   ", seconds++);
  }
}