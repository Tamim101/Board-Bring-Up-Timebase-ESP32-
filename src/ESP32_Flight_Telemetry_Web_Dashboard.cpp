#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"  // for esp_wifi_set_max_tx_power()

const char* AP_SSID = "DRONE-FC";
const char* AP_PASS = "12345678";
WebServer server(80);

struct Axis {
  float angle;     // deg
  float rate;      // deg/s
  float target;    // desired angle
  float kp, ki, kd;
  float i;         // integral term
  float prev_err;  // for derivative
};

Axis roll_axis  {0,0,0, 2.0,0.2,0.05, 0,0};
Axis pitch_axis {0,0,0, 2.0,0.2,0.05, 0,0};
Axis yaw_axis   {0,0,0, 1.0,0.1,0.02, 0,0};

float motors[4] = {0,0,0,0};

uint64_t last_update_us = 0;

// loop timing stats
uint32_t last_loop_count_time = 0;
uint32_t loop_counter = 0;
float loop_hz = 0.0f;

// helper: check for bad float and clamp
float safeFloat(float v) {
  if (!isfinite(v)) { // true if NaN or Inf
    return 0.0f;
  }
  return v;
}


float pid_update(Axis &a, float dt) {
  // protect against insane dt (0 or extremely tiny)
  if (dt <= 0.000001f) {
    return 0.0f;
  }

  float err = a.target - a.angle;
  a.i += err * dt;

  float d = (err - a.prev_err) / dt;
  a.prev_err = err;

  float u = a.kp * err + a.ki * a.i + a.kd * d;

  // clean invalid math
  if (!isfinite(u)) u = 0.0f;
  return u;
}


void update_flight_model() {
  uint64_t now = micros();
  if (last_update_us == 0) {
    last_update_us = now;
    return; // skip first frame so dt isn't crazy
  }

  float dt = (now - last_update_us) / 1e6f;
  last_update_us = now;

  // safety: if dt is too huge (stall) or too tiny (timer glitch), clamp it
  if (dt < 0.00005f) dt = 0.00005f;   // 50 microseconds min
  if (dt > 0.050f)   dt = 0.050f;     // 50 ms max

  // 1) world physics for each axis
  for (Axis* a : {&roll_axis, &pitch_axis, &yaw_axis}) {
    float gust = random(-5,5) / 100.0f; // disturbance
    float stiffness = 0.2f;             // self-level tendency

    a->rate  += (-stiffness * a->angle + gust) * dt;
    if (!isfinite(a->rate)) a->rate = 0.0f;

    a->angle += a->rate * dt;
    if (!isfinite(a->angle)) a->angle = 0.0f;
  }

  // 2) run PID per axis
  float u_roll  = pid_update(roll_axis,  dt);
  float u_pitch = pid_update(pitch_axis, dt);
  float u_yaw   = pid_update(yaw_axis,   dt);

  // 3) control pushes the craft back
  roll_axis.rate  += u_roll  * dt;
  pitch_axis.rate += u_pitch * dt;
  yaw_axis.rate   += u_yaw   * dt;

  if (!isfinite(roll_axis.rate))  roll_axis.rate  = 0.0f;
  if (!isfinite(pitch_axis.rate)) pitch_axis.rate = 0.0f;
  if (!isfinite(yaw_axis.rate))   yaw_axis.rate   = 0.0f;

  // 4) motor mixing
  motors[0] = 0.5f + 0.05f*(+u_roll - u_pitch - u_yaw);
  motors[1] = 0.5f + 0.05f*(-u_roll - u_pitch + u_yaw);
  motors[2] = 0.5f + 0.05f*(-u_roll + u_pitch - u_yaw);
  motors[3] = 0.5f + 0.05f*(+u_roll + u_pitch + u_yaw);

  // clamp and sanitize motors
  for (int i=0; i<4; i++) {
    if (!isfinite(motors[i])) motors[i] = 0.0f;
    if (motors[i] < 0) motors[i] = 0;
    if (motors[i] > 1) motors[i] = 1;
  }
}

const char* DASHBOARD_HTML = R"HTML(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
<title>Drone Flight Telemetry</title>
<style>
body {
  font-family:sans-serif;
  background:#0f172a;
  color:#fff;
  padding:20px;
}
.card {
  background:#1e293b;
  padding:16px;
  border-radius:12px;
  margin-bottom:16px;
  max-width:400px;
}
h1 {
  font-size:20px;
  margin:0 0 12px;
  color:#38bdf8;
}
.row {
  display:flex;
  justify-content:space-between;
  font-size:14px;
}
.motorrow {
  display:flex;
  justify-content:space-between;
  gap:4px;
  font-size:14px;
}
.motor {
  flex:1;
  background:#334155;
  border-radius:8px;
  padding:8px;
  text-align:center;
}
.val {
  font-weight:bold;
  color:#fff;
}
.lbl {
  color:#94a3b8;
  font-size:12px;
}
#errbox {
  background:#7f1d1d;
  color:#fff;
  font-size:12px;
  padding:8px;
  border-radius:8px;
  display:none;
  white-space:pre-wrap;
  margin-bottom:16px;
}
</style>
</head>
<body>
<h1>Drone Flight Telemetry</h1>

<div id="errbox"></div>

<div class="card">
  <div class="row"><div>Roll:</div><div id="roll" class="val">0</div></div>
  <div class="row"><div>Pitch:</div><div id="pitch" class="val">0</div></div>
  <div class="row"><div>Yaw:</div><div id="yaw" class="val">0</div></div>
</div>

<div class="card">
  <div class="motorrow">
    <div class="motor"><div class="val" id="m0">0</div><div class="lbl">M0</div></div>
    <div class="motor"><div class="val" id="m1">0</div><div class="lbl">M1</div></div>
    <div class="motor"><div class="val" id="m2">0</div><div class="lbl">M2</div></div>
    <div class="motor"><div class="val" id="m3">0</div><div class="lbl">M3</div></div>
  </div>
</div>

<div class="card">
  <div class="row"><div>Loop Hz:</div><div id="hz" class="val">0</div></div>
</div>

<script>
// grab all elements once
var rollEl  = document.getElementById("roll");
var pitchEl = document.getElementById("pitch");
var yawEl   = document.getElementById("yaw");
var m0El    = document.getElementById("m0");
var m1El    = document.getElementById("m1");
var m2El    = document.getElementById("m2");
var m3El    = document.getElementById("m3");
var hzEl    = document.getElementById("hz");
var errBox  = document.getElementById("errbox");

// use XMLHttpRequest so even old browsers work
function refreshOnce() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/telemetry", true);
  xhr.onreadystatechange = function() {
    if (xhr.readyState === 4) { // DONE
      if (xhr.status === 200) {
        try {
          var data = JSON.parse(xhr.responseText);

          rollEl.textContent  = data.roll.toFixed(2);
          pitchEl.textContent = data.pitch.toFixed(2);
          yawEl.textContent   = data.yaw.toFixed(2);

          m0El.textContent = data.motors[0].toFixed(2);
          m1El.textContent = data.motors[1].toFixed(2);
          m2El.textContent = data.motors[2].toFixed(2);
          m3El.textContent = data.motors[3].toFixed(2);

          hzEl.textContent = data.hz.toFixed(1);

          errBox.style.display = "none";
          errBox.textContent = "";
        } catch(e) {
          errBox.style.display = "block";
          errBox.textContent = "JSON parse error:\n" + e + "\n\nraw:\n" + xhr.responseText;
        }
      } else {
        errBox.style.display = "block";
        errBox.textContent = "HTTP error " + xhr.status;
      }
    }
  };
  xhr.send();
}

// poll 5 times/sec
setInterval(refreshOnce, 200);
</script>
</body>
</html>
)HTML";

// ==================== HTTP Handlers ====================
void handle_telemetry() {
  // clean values before sending so JSON is valid
  float r  = safeFloat(roll_axis.angle);
  float p  = safeFloat(pitch_axis.angle);
  float y  = safeFloat(yaw_axis.angle);
  float m0 = safeFloat(motors[0]);
  float m1 = safeFloat(motors[1]);
  float m2 = safeFloat(motors[2]);
  float m3 = safeFloat(motors[3]);
  float hz = safeFloat(loop_hz);

  char out[256];
  snprintf(out, sizeof(out),
    "{\"roll\":%.2f,\"pitch\":%.2f,\"yaw\":%.2f,"
    "\"motors\":[%.2f,%.2f,%.2f,%.2f],"
    "\"hz\":%.1f}",
    r,p,y,m0,m1,m2,m3,hz
  );

  Serial.print("[WEB] /telemetry -> ");
  Serial.println(out);

  server.send(200, "application/json", out);
}

void handle_dashboard() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

void not_found_handler() {
  Serial.print("[WEB] 404 for path: ");
  Serial.println(server.uri());
  server.send(404, "text/plain", "Not found");
}

// ==================== Wi-Fi / Web bring-up ====================
void setupWeb() {
  Serial.println("[WEB] bringing up WiFi AP...");

  WiFi.mode(WIFI_AP);
  delay(200);
  WiFi.softAP(AP_SSID, AP_PASS);

  // reduce RF power to avoid brownout on C3
  esp_wifi_set_max_tx_power(40); // ~10 dBm

  IPAddress ip = WiFi.softAPIP();
  Serial.print("[WEB] AP IP: ");
  Serial.println(ip);

  server.on("/",          handle_dashboard);
  server.on("/telemetry", handle_telemetry);
  server.onNotFound(not_found_handler);

  server.begin();
  Serial.println("[WEB] Server started");
}

// ==================== Arduino setup() ====================
void setup() {
  Serial.begin(115200);
  delay(500); // stabilize 3V3 rail before WiFi

  Serial.println("#Flight Telemetry Web Dashboard");

  randomSeed(esp_random());

  setupWeb();

  last_loop_count_time = millis();
}

// ==================== Arduino loop() ====================
void loop() {
  // run simulated drone physics + PID
  update_flight_model();

  // update loop frequency counter
  loop_counter++;
  uint32_t nowMs = millis();
  if (nowMs - last_loop_count_time >= 1000) {
    loop_hz = (float)loop_counter * 1000.0f / (float)(nowMs - last_loop_count_time);
    last_loop_count_time = nowMs;
    loop_counter = 0;

    Serial.printf("[FC] roll=%.2f pitch=%.2f yaw=%.2f hz=%.1f\n",
                  roll_axis.angle, pitch_axis.angle, yaw_axis.angle, loop_hz);
  }

  // respond to HTTP
  server.handleClient();

  delay(2);
}
