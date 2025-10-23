// #include <Arduino.h>
// #include <HardwareSerial.h>

// #define VR_RX   4
// #define VR_TX   5
// #define LED_PIN 2
// #define PWM_CH  0

// HardwareSerial VR(1);
// enum { LED_OFF, LED_ON, FAILSAFE };
// uint8_t state = LED_OFF;
// uint8_t brightness = 0;

// void setBrightness(uint8_t v) {
//   v = constrain(v, 0, 255);
//   if (brightness != v) {
//     brightness = v;
//     ledcWrite(PWM_CH, brightness);
//     Serial.printf("BRIGHTNESS: %d\n", brightness);
//   }
// }

// void setState(uint8_t s) {
//   state = s;
//   switch (s) {
//     case LED_ON:   setBrightness(255); Serial.println("STATE: ON");  break;
//     case LED_OFF:  setBrightness(0);   Serial.println("STATE: OFF"); break;
//     case FAILSAFE: setBrightness(0);   Serial.println("STATE: FAILSAFE"); break;
//   }
// }

// void handleCmd(int id) {
//   Serial.printf("CMD:%d\n", id);
//   if (id == 0) setState(LED_ON);
//   else if (id == 1) setState(LED_OFF);
//   else if (state == LED_ON) {
//     if (id == 2) setBrightness(brightness + 30);
//     if (id == 3) setBrightness(brightness - 30);
//   }
// }

// void setup() {
//   Serial.begin(115200);
//   VR.begin(9600, SERIAL_8N1, VR_RX, VR_TX);
//   ledcSetup(PWM_CH, 1000, 8);
//   ledcAttachPin(LED_PIN, PWM_CH);
//   setState(LED_OFF);
//   Serial.println("\n# Voice-controlled LED (Elechouse VR3.1)");
//   Serial.println("# Expected: CMD:0=ON, CMD:1=OFF, CMD:2=UP, CMD:3=DOWN");
// }

// void loop() {
//   static String buf = "";
//   while (VR.available()) {
//     char c = VR.read();
//     buf += c;
//     if (buf.endsWith("\n")) {
//       buf.trim();
//       Serial.printf("RAW: %s\n", buf.c_str());
//       if (buf.startsWith("CMD:")) {
//         int id = buf.substring(4).toInt();
//         handleCmd(id);
//       }
//       buf = "";
//     }
//   }
// }
