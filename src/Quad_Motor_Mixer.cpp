#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define LOOP_HZ 200.0f    // control loop
#define DT (1.0f/LOOP_HZ)
#define LED_PINS {2,3,4,5} // any 4 pwm capable

struct PID{
    float kp, ki, kd;
    float integ = 0,prev_err = 0;
    PID(float _kp = 0.0f, float _ki = 0.0f, float _kd = 0.0f):
    kp(_kp),ki(_ki),kd(_kd),integ(0.0f),prev_err(0.0f){}
    float update(float err){
        integ += err * DT;
        float deriv = (err - prev_err) / DT;
        prev_err = err;
        return kp*err + ki*integ + kd*deriv;
    }
};

float pitch = 0, roll = 0;         //quad state
float p_rate = 0, r_rate = 0;
float tgt_pitch =0, tgt_roll = 0;

PID pitchAnglePID{2.0,0.2,0.05};
PID rollAnglePID{2.0,0.2,0.05};
PID pitchRatePID{0.5,0.05,0.02};
PID rollRatePID{0.5,0.05,0.02};     // outer angle and inner rate controllers

float mix[4][2] = {     // // M1: front-left, M2: front-right, M3: rear-right, M4: rear-left
    {+1, +1},
    {-1, +1},
    {-1, -1},        // roll pitch influence
    {+1, -1},
};

void fc_task(void*){
    const int leds[4] = LED_PINS;
    for(int i = 0; i < 4; i++){
     ledcSetup(i,1000,8); ledcAttachPin(leds[i],i);
    }
    const TickType_t period = pdMS_TO_TICKS(1000.0f/LOOP_HZ);
    TickType_t last = xTaskGetTickCount();
    Serial.println("# quad mixer + cascaded pid @200hz");
    Serial.printf("#comands: ROLL x | PITCH x | KP x | PRINT 0/1");
    bool doprint = true;

    while(1){
        float errPitch = tgt_pitch - pitch;
        float errRoll = tgt_roll - roll;
        float tgtPRate = pitchAnglePID.update(errPitch);
        float tgtRRate = pitchRatePID.update(errRoll);  // pid angle target rates
        float uPitch = pitchRatePID.update(tgt_pitch - p_rate);
        float uRoll = rollAnglePID.update(tgtRRate - r_rate);

        float motor[4];
        for(int i = 0; i < 4; i++)  // distrabute roll pitch 4 motor 
        motor[i] = 0.5f + 0.1f*(mix[i][0] * uRoll + mix[i][1]*uPitch);
        float gust = random(-2,2)/100.0f;
        p_rate += (uPitch - 0.05f*p_rate + gust)*DT;
        r_rate += (uRoll - 0.05f*r_rate + gust)*DT;
        pitch += p_rate*DT;
        roll += r_rate*DT;       // order respnsed + damping + noise
        
        for(int i = 0 ;  i < 4; i++){
            motor[i] = constrain(motor[i],0,1);
            ledcWrite(i,(int)(motor[i]*255));

        }
        if(doprint){
            Serial.printf("p=%.2f r=%.2f uP=%.3f uR=%.3f M=[%.2f %.2f %.2f %.2f]\n",
            pitch,roll,uPitch,uRoll,motor[0],motor[1],motor[2],motor[3]);
        }
        while(Serial.available()){
            String line = Serial.readStringUntil('\n');
            line.trim();
            line.toUpperCase();
            if(line.startsWith("ROLL")) {
                tgt_roll = line.substring(5).toFloat();
            }else if(line.startsWith("PITCH")){
                tgt_pitch= line.substring(6).toFloat();
            }else if(line.startsWith("kp")){
                float v = line.substring(3).toFloat();
                pitchAnglePID.kp = rollAnglePID.kp = v;
            }
            else if(line.startsWith("PRINT ")){
                doprint = line.endsWith("1");
            }else if (line == "RESET"){
                pitch= roll=p_rate=r_rate=0;
            }
            
        }
        
    }

}
void setup(){
    Serial.begin(115200);
    randomSeed(esp_random());
    xTaskCreatePinnedToCore(fc_task,"FC",8192,nullptr,1,nullptr,0);
}
void loop(){

}