#include <Arduino.h>
float accAngle = 0;
float gyro_rate = 0;
float fused_angle = 0;
float target = 0;
float kp = 0.8;
float ki = 0.05;
float kd = 0.03;
float integ = 0;
float prev_err = 0;
float dt = 0.004;

void setup(){
    Serial.begin(115200);
    Serial.println("virtual IMU fusion motor simulation");
}
void loop(){
    static float trueAngle = 0,rate = 0;
    float gust = random(-5,5) / 100.0;
    rate += (-0.2*trueAngle + gust)*dt;        // random drift/noise 
    trueAngle += rate*dt;                      // spring dynamics 
    accAngle = trueAngle + random(-5,5) / 50.0;   // slow noisy accel
    gyro_rate = rate + random(-3,3) / 10.0;       // fast noisy gyro
    fused_angle = 0.98*(fused_angle + gyro_rate * dt) + 0.02*accAngle;
    
    float err = target - fused_angle;
    integ += err * dt;
    float deriv = (err - prev_err) / dt;
    prev_err = err;
    float u = kp * err + ki * integ + kd * deriv;
    float motor[4];
    motor[0] = 0.5 + 0.1 * u;  // front-left
    motor[1] = 0.5 - 0.1 * u;  // front-right
    motor[2] = 0.5 - 0.1 * u;  // rear-right
    motor[3] = 0.5 + 0.1 * u;  // rear-left

    Serial.printf("angle = %.3f fused = %.3f u = %.3f M = [%.2f %.2f %.2f %.2f]\n",
    trueAngle, fused_angle, u, motor[0],motor[1],motor[2],motor[3]);

    delay(4);

}