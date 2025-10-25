// #include <Arduino.h>
// float angle = 0;
// float rate  = 0;
// float tgt  = 0;
// float kp = 0.8;
// float ki  = 0.05;
// float kd = 0.03;
// float integ = 0;
// float prev_err = 0;

// void setup(){
//     Serial.begin(115200);
//     Serial.println("virtual IMU running.............");
// }
// void loop(){
//     static uint64_t last = micros();
//     float dt = (micros() - last) / 1e6;
//     last = micros();
//     float gust = random(-5, 5) / 100.0;
//     rate += (-0.1 * angle + gust) * dt;
//     angle += rate *dt;                  //spring damping  noise

//     float err = tgt - angle;
//     integ += err * dt;
//     float deriv = (err - prev_err) / dt;
//     prev_err = err;
//     float u = kp * err + ki * integ + kd * deriv;  // pid control 

//     rate += u * dt;             // simulate actuator response 
//     Serial.printf("ang = %.3f err = %.3f u = %.3f\n", angle,err,u);
//     delay(10);
// }

