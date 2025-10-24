#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ======== Config ========
#define LOOP_HZ   200.0f           // control loop frequency
#define DT        (1.0f/LOOP_HZ)
#define LED_PINS  {2, 3, 4, 5}     // any 4 PWM-capable pins

// ======== PID structs ========
struct PID {
  float kp, ki, kd;
  float integ = 0, prevErr = 0;
  PID(float _kp = 0.0f, float _ki = 0.0f, float _kd = 0.0f)
    : kp(_kp), ki(_ki), kd(_kd), integ(0.0f), prevErr(0.0f) {}
  float update(float err) {
    integ += err * DT;
    float deriv = (err - prevErr) / DT;
    prevErr = err;
    return kp*err + ki*integ + kd*deriv;
  }
};

// ======== Quad State ========
float pitch=0, roll=0;
float pRate=0, rRate=0;
float tgtPitch=0, tgtRoll=0;

// Outer (angle) and inner (rate) controllers
PID pitchAnglePID{2.0,0.2,0.05};
PID rollAnglePID {2.0,0.2,0.05};
PID pitchRatePID {0.5,0.05,0.02};
PID rollRatePID  {0.5,0.05,0.02};

// ======== Mixer coefficients (X quad) ========
// M1: front-left, M2: front-right, M3: rear-right, M4: rear-left
float mix[4][2] = {
  {+1, +1},   // roll, pitch influence
  {-1, +1},
  {-1, -1},
  {+1, -1}
};

void fc_task(void*) {
  const int leds[4] = LED_PINS;
  for(int i=0;i<4;i++){ ledcSetup(i,1000,8); ledcAttachPin(leds[i],i); }

  const TickType_t period=pdMS_TO_TICKS(1000.0f/LOOP_HZ);
  TickType_t last=xTaskGetTickCount();
  Serial.println("# Project 4: Quad Mixer + Cascaded PID @200 Hz");
  Serial.println("# Commands: ROLL x | PITCH x | KP x | PRINT 0/1");
  bool doPrint=true;

  for(;;){
    // ---- Outer Angle PID → target rates ----
    float errPitch = tgtPitch - pitch;
    float errRoll  = tgtRoll  - roll;
    float tgtPRate = pitchAnglePID.update(errPitch);
    float tgtRRate = rollAnglePID.update(errRoll);

    // ---- Inner Rate PID → control torque ----
    float uPitch = pitchRatePID.update(tgtPRate - pRate);
    float uRoll  = rollRatePID.update(tgtRRate - rRate);

    // ---- Mixer: distribute roll/pitch control to 4 motors ----
    float motor[4];
    for(int i=0;i<4;i++)
      motor[i] = 0.5f + 0.1f*(mix[i][0]*uRoll + mix[i][1]*uPitch);

    // ---- Physics simulation ----
    // simplified 1st-order response + damping + noise
    float gust = random(-2,2)/100.0f;
    pRate += (uPitch - 0.05f*pRate + gust)*DT;
    rRate += (uRoll  - 0.05f*rRate + gust)*DT;
    pitch += pRate*DT;
    roll  += rRate*DT;

    // ---- Clamp & drive LEDs ----
    for(int i=0;i<4;i++){
      motor[i]=constrain(motor[i],0,1);
      ledcWrite(i,(int)(motor[i]*255));
    }

    if(doPrint){
      Serial.printf("p=%.2f r=%.2f uP=%.3f uR=%.3f M=[%.2f %.2f %.2f %.2f]\n",
        pitch,roll,uPitch,uRoll,motor[0],motor[1],motor[2],motor[3]);
    }

    // ---- Serial commands ----
    while(Serial.available()){
      String line=Serial.readStringUntil('\n');
      line.trim(); line.toUpperCase();
      if(line.startsWith("ROLL "))  tgtRoll=line.substring(5).toFloat();
      else if(line.startsWith("PITCH ")) tgtPitch=line.substring(6).toFloat();
      else if(line.startsWith("KP ")) {
        float v=line.substring(3).toFloat();
        pitchAnglePID.kp=rollAnglePID.kp=v;
      }
      else if(line.startsWith("PRINT ")) doPrint=line.endsWith("1");
      else if(line=="RESET"){pitch=roll=pRate=rRate=0;}
    }

    vTaskDelayUntil(&last,period);
  }
}

void setup(){
  Serial.begin(115200);
  randomSeed(esp_random());
  xTaskCreatePinnedToCore(fc_task,"FC",8192,nullptr,1,nullptr,0);
}
void loop(){}

