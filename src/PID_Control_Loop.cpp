// #include <Arduino.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// // config 
// static const int lED_PIN = 2;
// static const int PWM_CH = 0;
// static const int PWM_HZ = 1000;
// static const int PWM_BITS = 8;        // on board led 

// const float loop_hz = 100.0f;     // control rate 
// const float dt  = 1.0f / loop_hz;  // secounds

// volatile float kp = 0.6f;   // pid gains
// volatile float ki = 0.15f;
// volatile float kd = 0.05f;
// volatile float traget_angle = 0.0f; //digitalRead);

// static float angle = 0.0f;  // plant single axise 
// static float rate = 0.0f; 

// static float i_sum = 0.0f;
// static float last_err = 0.0f;  //pid state

// static inline float clampf(float x, float lo, float hi){
//     return x < lo?lo:(x>hi?hi:x);
// }
// void ctrl_task(void*){
//     ledcSetup(PWM_CH,PWM_HZ,PWM_BITS);
//     ledcAttachPin(lED_PIN,PWM_CH);
//     ledcWrite(PWM_CH,0);
    
//     const TickType_t period = pdMS_TO_TICKS((int)(1000.0f/loop_hz));
//     TickType_t last = xTaskGetTickCount();
//     Serial.println("# project 3 : PID stabilazation(100hz)");
//     Serial.println("# commands: KP x | KI x | TGT x | PRINT 0\1");
//     bool do_print = true;

//     for(;;){
//        float gust = random(-3,3) / 100.0f;
//        float err = traget_angle - angle;
//        i_sum += err *dt;
//        i_sum = clampf(i_sum , -200.0f,200.0f);    // anti winup
//        float d_err = (err - last_err) / dt;
//        float u = kp*err + ki*i_sum + kd *d_err;
//        last_err = err;                            // control effort

//        float damping = 0.02f * rate;              // update plant
//        rate += (u- damping + gust)*dt; 
//        angle += rate * dt;
//        float msg = fabsf(u) * 10.0f;
//        int duty = (int)clampf(msg, 0.0f,255.0f);
//        ledcWrite(PWM_CH, duty);
//        if(do_print){
//         Serial.printf("ang=%7.2% tgt=%6.2f err = %7.2f u = %8.3f dusty = %3d kp = %.3f ki = %.3f kd=%.3f\n",
//         angle,traget_angle,err,u,duty,kp,ki,kd);

//        }
//        while(Serial.available()){
//         String line = Serial.readStringUntil('\n');
//         line.trim(); line.toUpperCase();
//         if(line.startsWith("KP")) {kp = line.substring(3).toFloat();}
//         else if (line.startsWith("KD")) {kd = line.substring(3).toFloat();}
//         else if(line.startsWith("TGT")) {traget_angle = line.substring(4).toFloat();}
//         else if(line.startsWith("PRINT")){
//             int v = line.substring(6).toInt();do_print = (v!=0);
//         }else if (line == "RESET"){
//             angle=0;rate=0;i_sum=0;last_err = 0;
//         }
//        }
//        vTaskDelayUntil(&last,period);

//     }
// }
// void setup(){
//     Serial.begin(115200);
//     delay(200);
//     randomSeed(esp_random());
//     xTaskCreatePinnedToCore(ctrl_task,"PID",4096,nullptr,1,nullptr,0);
// }
// void loop(){
    
// }