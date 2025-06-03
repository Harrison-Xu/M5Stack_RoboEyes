#define MAINCOLOR TFT_YELLOW

#include <M5Unified.h>
#include <M5GFX.h>
#include <M5Stack_RoboEyes.h>

M5Canvas canvas(&M5.Display);
RoboEyes roboEyes;  // create RoboEyes instance
m5::touch_detail_t touchDetail;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  M5.Display.setFont(&fonts::DejaVu40);

  canvas.createSprite(M5.Display.width(), M5.Display.height());
  canvas.setColorDepth(16);

  roboEyes.begin(&canvas, canvas.width(), canvas.height(), 100);  // max framerate
  roboEyes.setPosition(CT);
  roboEyes.setWidth(60, 60);         // leftEye, rightEye
  roboEyes.setHeight(100, 100);      // leftEye, rightEye
  roboEyes.setBorderradius(20, 20);  // leftEye, rightEye
  roboEyes.setSpacebetween(25);      // can also be negative

  roboEyes.setAutoblinker(ON, 3, 1);  // on/off, interval between each blink in seconds, range for random interval variation in seconds
  roboEyes.setIdleMode(ON, 5, 2);     // on/off, interval between each eye repositioning in seconds, range for random interval variation in seconds
  roboEyes.setCuriosity(ON);
  roboEyes.setMood(NORMAL);  // NORMAL, HAPPY, TIRED, ANGRY
  roboEyes.setHFlicker(OFF, 3);
  roboEyes.setVFlicker(OFF, 3);

  // Serial.begin(115200);
  // Serial.println(" Ready ");
  M5.Display.print(" Ready ");
  delay(500);

  M5.Display.clear();
  roboEyes.close();  // start with closed eyes
}

void loop() {
  M5.update();
  touchDetail = M5.Touch.getDetail();

  if (touchDetail.isPressed()) {
    roboEyes.eyeLxNext = touchDetail.x - roboEyes.eyeLwidthNext - roboEyes.spaceBetweenNext / 2;
    roboEyes.eyeLyNext = touchDetail.y - roboEyes.eyeLheightNext / 2;

    roboEyes.setAutoblinker(OFF, 3, 1);
    roboEyes.setIdleMode(OFF, 5, 2);
    roboEyes.setCuriosity(OFF);
    roboEyes.setMood(HAPPY);
    roboEyes.anim_laugh();
    // roboEyes.anim_confused();
    // roboEyes.blink();
  } else {
    roboEyes.setAutoblinker(ON, 3, 1);
    roboEyes.setIdleMode(ON, 5, 2);
    roboEyes.setCuriosity(ON);
    roboEyes.setMood(NORMAL);
  }

  roboEyes.update();
  canvas.pushSprite(0, 0);
}
