#include <math.h>

// --- Đọc nhiệt độ từ cảm biến NTC ---
#define ANALOG_PIN A1         // NTC nối giữa A1 và điện trở 5k
#define VCC 5.0
#define R_FIXED 5000.0

// --- Biến đo xung RPM ---
volatile unsigned long lastRiseTime = 0;
volatile unsigned long period = 0;
volatile bool newData = false;

const int pwmInputPin = 2; // Chân nhận tín hiệu RPM từ cảm biến

// --- Biến dữ liệu gửi ---
int AguiMT, BguiMT;
int D3guiMT, D4guiMT, D5guiMT, D6guiMT, D7guiMT, D8guiMT, D9guiMT;
int RPMguiMT, ThermitguiMT;

// --- Hàm gửi dữ liệu ---
void guiLV(int soA, int soB, bool soC, bool soD, bool soE, bool soF, bool soG, bool soH, bool soI, int soJ, int soK) {
  Serial.print("a"); Serial.print(soA);
  Serial.print("b"); Serial.print(soB);
  Serial.print("c"); Serial.print(soC);
  Serial.print("d"); Serial.print(soD);
  Serial.print("e"); Serial.print(soE);
  Serial.print("f"); Serial.print(soF);
  Serial.print("g"); Serial.print(soG);
  Serial.print("h"); Serial.print(soH);
  Serial.print("i"); Serial.print(soI);
  Serial.print("j"); Serial.print(soJ); // RPM
  Serial.print("k"); Serial.print(soK); // Nhiệt độ
  Serial.print("#");
  Serial.println();
}

void setup() {
  Serial.begin(19200);

  pinMode(pwmInputPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pwmInputPin), risingEdgeISR, RISING);

  pinMode(3, INPUT); pinMode(4, INPUT); pinMode(5, INPUT);
  pinMode(6, INPUT); pinMode(7, INPUT); pinMode(8, INPUT); pinMode(9, INPUT);
}

void loop() {
  // --- Đọc giá trị analog và digital ---
  AguiMT = analogRead(A0);
  BguiMT = analogRead(A1);
  D3guiMT = digitalRead(3); D4guiMT = digitalRead(4); D5guiMT = digitalRead(5);
  D6guiMT = digitalRead(6); D7guiMT = digitalRead(7); D8guiMT = digitalRead(8); D9guiMT = digitalRead(9);

  // --- Tính RPM nếu có xung mới ---
  if (newData) {
    noInterrupts();
    unsigned long currentPeriod = period;
    newData = false;
    interrupts();

    if (currentPeriod > 0) {
      float frequency = 1e6 / currentPeriod;
      RPMguiMT = (int)(frequency * 60.0);
    } else {
      RPMguiMT = 0;
    }
  }

  // --- Tính nhiệt độ từ cảm biến NTC ---
  int analogValue = analogRead(ANALOG_PIN);  
  float voltage = (analogValue / 1023.0) * VCC;
  float R_NTC = ((VCC * R_FIXED) / voltage) - R_FIXED;
  float R_kohm = R_NTC / 1000.0;
  float temperature = -29.14 * log(R_kohm) + 93.358;
  ThermitguiMT = round(temperature);

  // --- Gửi dữ liệu nếu có lệnh từ máy tính ---
  while (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if (incomingByte == 'r') {
      guiLV(AguiMT, BguiMT, D3guiMT, D4guiMT, D5guiMT, D6guiMT, D7guiMT, D8guiMT, D9guiMT, RPMguiMT, ThermitguiMT);
      break;
    }
  }

  delay(10);
}

void risingEdgeISR() {
  unsigned long currentTime = micros();
  period = currentTime - lastRiseTime;
  lastRiseTime = currentTime;
  newData = true;
}