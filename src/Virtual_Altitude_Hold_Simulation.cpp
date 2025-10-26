// #include<Arduino.h>
// float alt = 0, vel = 0, acc = 0;
// float tgt_alt = 1.0;
// float kp = 1.2,ki = 0.3,kd = 0.15;
// float integ = 0, prev_err = 0;
// float dt = 0.01;

// void setup(){
//     Serial.begin(115200);
//     Serial.println("altitude hold simulation");
// }

// void loop(){
//     static uint64_t last = micros();
//     dt = (micros() - last) / 1e6;last = micros();
//     float trust = random(-5,5) / 200.0;
//     acc = -0.2 * vel - 9.81 + trust;
//     vel += acc*dt;
//     alt += vel*dt;
//     if (alt < 0){
//         alt = 0; vel = 0;
//     }
//     float err = tgt_alt - alt;
//     integ += err*dt;
//     float deriv  = (err - prev_err) / dt;
//     prev_err = err;
//     float u = kp * err + ki * integ + kd *deriv;
//     vel += u*dt;
//     Serial.printf("alt = %.5f  err = %.2f  u = %.2f\n",alt,err,u);
//     delay(10);
// }