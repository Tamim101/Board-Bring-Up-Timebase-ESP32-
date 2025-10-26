// #include <Arduino.h>
// struct Axis{
//     float angle = 0;
//     float rate = 0;
//     float acc = 0;
//     float gyro = 0;
//     float fused = 0;
//     float target = 0;
//     float kp = 2.0,ki = 0.2,kd = 0.05,i=0,prev_err = 0;
//     float update(float dt){
//         float err = target - fused;
//         i += err*dt;
//         float d = (err - prev_err) /dt;
//         prev_err = err;
//         return kp * err + ki* i +kd*d;
//     }
// };
// Axis roll,pitch,yaw;
// float dt = 0.004;
// void setup(){
//     Serial.begin(115200);
//     Serial.println("virtual 3 axis flight control simulation");
// }
// void loop(){
//     static uint64_t last = micros();
//     dt = (micros() - last) / 1e6;last = micros();
//     for (Axis* a : {&roll, &pitch,&yaw}){
//         float gust = random(-5,5) / 100.0;
//         a->rate += (-0.2*a->angle+gust)*dt ;
//         a-> angle += a->rate *dt;
//         a-> acc = a->angle + random(-3,3)/ 100.0;
//         a->gyro = a->rate + random(-5,5) / 10.0;
//         a->fused = 0.98*(a->fused + a->gyro*dt) + 0.02* a-> acc;
//       }
//     float ur = roll.update(dt);
//     float up = roll.update(dt);
//     float uy = yaw.update(dt);
//     float mix[4][3] = {
//         {+1,-1,-1},{-1,-1,+1},{-1,+1,-1},{+1,+1,+1}
//     };
//     float m[4];
//     for(int i = 0; i<4;i++){
//         m[i] = 0.5 + 0.05*(mix[i][0]*ur + mix[i][1]*up + mix[i][2]*uy);

//     }
//     for(int i = 0; i < 4;i++){
//         m[i] = 0.5+0.05*(mix[i][0]*ur + mix[i][1]*up + mix[i][2]*uy);
//     }
//     for(int i = 0; i < 4; i++){
//         m[i] = constrain(m[i],0,1);
//     }
//     Serial.printf("R = %.5f P = %.5f Y = %.5f | ur = %.5f up = %.5f uy = %.5f | M = [%.5f %.5f %.5f %.5f]\n",
//     roll.fused,pitch.fused,yaw.fused,ur,up,uy,m[0],m[1],m[2],m[3]);
// }