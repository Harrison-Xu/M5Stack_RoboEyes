#define MAINCOLOR TFT_SKYBLUE
#define WIFI_SSID "********"
#define WIFI_PSWD "********"
#define NTP_TIMEZONE "UTC-8"  // POSIX standard, in which "UTC+0" is UTC London, "UTC-8" is UTC+8 Beijing, "UTC+5" is UTC-5 New York
#define NTP_SERVER1 "pool.ntp.org"
#define LOCAL_TIME_OFFSET 8  // 0 is UTC London, 8 is UTC+8 Beijing, -5 is UTC-5 New York
#define SPEAKER_VOLUME 255   // 0~255, bigger is louder

#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <M5Unified.h>
#include <M5GFX.h>
#include <M5Stack_RoboEyes.h>

#if __has_include(<esp_sntp.h>)
#include <esp_sntp.h>
#define SNTP_ENABLED 1
#elif __has_include(<sntp.h>)
#include <sntp.h>
#define SNTP_ENABLED 1
#endif
#ifndef SNTP_ENABLED
#define SNTP_ENABLED 0
#endif

#define SD_SPI_SCK_PIN  36
#define SD_SPI_MISO_PIN 35
#define SD_SPI_MOSI_PIN 37
#define SD_SPI_CS_PIN   4
const char* wavFiles[] = {"/01.wav", "/02.wav", "/03.wav", "/04.wav", "/05.wav", "/06.wav", "/07.wav", "/08.wav", "/09.wav", "/10.wav", 
                          "/11.wav", "/12.wav", "/13.wav", "/14.wav", "/15.wav", "/16.wav", "/17.wav", "/18.wav", "/19.wav", "/20.wav", 
                          "/21.wav"};

M5Canvas canvas(&M5.Display);
RoboEyes roboEyes;
// m5::touch_detail_t touchDetail;
HardwareSerial mySerial(2);
bool playTrigger = false;
int wavIndex, wavIndexLast;

void setup() {
  M5.begin();
  mySerial.begin(115200, SERIAL_8N1, 1, 2);  // RX, TX
  M5.Display.setRotation(1);
  M5.Display.setFont(&fonts::FreeMono12pt7b);
  M5.Speaker.setVolume(SPEAKER_VOLUME);  // 0~255, bigger is louder

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    M5.Display.println("microSD not found");
    while (1) delay(1000);
  } else {
    M5.Display.println("microSD found");
  }
  if (!SD.open("/01.wav", FILE_READ, false)) {
    M5.Display.println("01.wav not found");
    while (1) delay(1000);
  } else {
    M5.Display.println("01.wav found");
  }

  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setColorDepth(16);
  roboEyes.begin(&canvas, canvas.width(), canvas.height(), 100);  // max framerate
  roboEyes.setPosition(CT);
  roboEyes.setWidth(60, 60);         // leftEye, rightEye
  roboEyes.setHeight(100, 100);      // leftEye, rightEye
  roboEyes.setBorderradius(20, 20);  // leftEye, rightEye
  roboEyes.setSpacebetween(25);      // can also be negative

  roboEyes.setAutoblinker(ON, 3, 1);  // interval between each blink in seconds, range for random interval variation in seconds
  roboEyes.setIdleMode(ON, 5, 2);     // interval between each eye reposition in seconds, range for random interval variation in seconds
  roboEyes.setCuriosity(ON);
  roboEyes.setMood(NORMAL);           // NORMAL, HAPPY, TIRED, ANGRY

  roboEyes.setHFlicker(OFF, 3);
  roboEyes.setVFlicker(OFF, 3);
  roboEyes.close();  // start with closed eyes
  M5.Display.println("RoboEyes ready");

  if (!M5.Rtc.isEnabled()) {
    M5.Display.println("RTC not found");
    while (1) delay(1000);
  } else {
    M5.Display.println("RTC found");
  }

  M5.Display.print("WiFi:");
  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  while (WiFi.status() != WL_CONNECTED) {
    M5.Display.print(".");
    delay(1000);
  }
  M5.Display.println("\nWiFi connected");

  M5.Display.print("NTP:");
  configTzTime(NTP_TIMEZONE, NTP_SERVER1);
#if SNTP_ENABLED
  while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) {
    M5.Display.print(".");
    delay(1000);
  }
#else
  struct tm timeInfo;
  while (!getLocalTime(&timeInfo, 1000)) {
    M5.Display.print(".");
    delay(1000);
  }
#endif
  M5.Display.println("\nNTP connected");
  time_t t = time(nullptr) + 1;  // Advance one second
  while (t > time(nullptr));
  M5.Rtc.setDateTime(gmtime(&t));
  delay(1000);

  M5.Display.clear();
}

void loop() {
  M5.update();
  auto dt = M5.Rtc.getDateTime();
  int localHours = dt.time.hours + LOCAL_TIME_OFFSET;
  // touchDetail = M5.Touch.getDetail();
  if (mySerial.available()) {
    int byteReceived = mySerial.read();
    if (byteReceived == 0xC7) {
      playTrigger = true;
    }
  }

  if (!M5.Speaker.isPlaying()) {
    if (playTrigger) {
      roboEyes.setAutoblinker(OFF);
      roboEyes.setIdleMode(OFF);
      roboEyes.setCuriosity(OFF);
      roboEyes.setMood(HAPPY);

      roboEyes.anim_laugh();
      timeToVoice(dt, localHours);
      playTrigger = false;
    } else {
      roboEyes.setAutoblinker(ON, 3, 1);
      roboEyes.setIdleMode(ON, 5, 2);
      roboEyes.setCuriosity(ON);
      roboEyes.setMood(NORMAL);
    }
  }

  roboEyes.update();
  canvas.pushSprite(0, 0);
}

void timeToVoice(auto dt, int localHours) {
  if (localHours == 8) {
    if (dt.time.minutes < 40) {
      playRandomWavFile(1, 2);
    } else if (dt.time.minutes < 59) {
      playRandomWavFile(3, 10);
    } else {
      playRandomWavFile(11, 14);
    }
  } else if (localHours == 9) {
    if (dt.time.minutes < 1) {
      playRandomWavFile(11, 14);
    } else if (dt.time.minutes < 30) {
      playRandomWavFile(15, 16);
    }
  } else if (localHours >= 18) {
    playRandomWavFile(17, 21);
  }
}

void playRandomWavFile(int a, int b) {
  while (wavIndex == wavIndexLast) {
    wavIndex = random(a - 1, b);    // wavIndex a-1 ~ b-1
  }
  playWavFile(wavFiles[wavIndex]);  // wavFile a ~ b
  wavIndexLast = wavIndex;
}

void playWavFile(const char* path) {
  File file = SD.open(path, "r");
  if (!file) {
    M5.Display.setCursor(0, 0);
    M5.Display.printf("Failed to open %s\n", path);
    while (1) delay(1000);
  }
  size_t size = file.size();
  uint8_t* buffer = (uint8_t*)malloc(size);
  if (!buffer) {
    M5.Display.setCursor(0, 0);
    M5.Display.println("Memory allocation failed");
    while (1) delay(1000);
  }
  file.read(buffer, size);
  file.close();
  M5.Speaker.playWav(buffer);
  free(buffer);
}
