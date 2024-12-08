// 먼지 센서 핀 정의
const int DUST_PIN = 8;
// pH 센서 핀 정의
const int PH_PIN = A0;

// 먼지 센서 변수
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 3000;  // 3초 샘플링
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float pm25_particles_per_liter = 0;

// 먼지 센서 평균 계산 변수
const int MEASURE_COUNT = 30;  // 30회 측정
int current_measure = 0;
float concentration_sum = 0;
float pm25_sum = 0;
float measurements[30];  // 개별 측정값 저장 배열

// pH 센서 변수
int buf[10], temp;
unsigned long int avgValue;
float phValue;

void setup() {
  Serial.begin(9600);
  pinMode(DUST_PIN, INPUT);
  starttime = millis();
  Serial.println("Starting measurements...");
}

// pH 값 읽기 함수
float readPH() {
  // 10번 측정하여 버퍼에 저장
  for(int i=0; i<10; i++) { 
    buf[i] = analogRead(PH_PIN);
    delay(10);
  }
  // 버블 정렬
  for(int i=0; i<9; i++) {
    for(int j=i+1; j<10; j++) {
      if(buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  // 평균값 계산 (중간 6개 값만 사용)
  avgValue = 0;
  for(int i=2; i<8; i++) {
    avgValue += buf[i];
  }
  float pHVol = (float)avgValue * 5.0 / 1024 / 6;
  float pH = -3.48 * pHVol + 21.34;
  return pH;
}

void loop() {
  // 먼지 센서 읽기
  duration = pulseIn(DUST_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;

  if ((millis()-starttime) >= sampletime_ms) {
    // 먼지 농도 계산
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    pm25_particles_per_liter = concentration / 3531;
    
    // pH 측정
    float currentPH = readPH();
    
    // 현재 측정값 저장 및 출력
    measurements[current_measure] = pm25_particles_per_liter;
    concentration_sum += concentration;
    pm25_sum += pm25_particles_per_liter;
    
    Serial.print("Measurement ");
    Serial.print(current_measure + 1);
    Serial.print(" of 30 - PM2.5: ");
    Serial.print(pm25_particles_per_liter);
    Serial.print(" ug/m3, pH: ");
    Serial.println(currentPH);
    
    current_measure++;
    
    // 30회 측정 완료시
    if (current_measure >= MEASURE_COUNT) {
      float avg_concentration = concentration_sum / MEASURE_COUNT;
      float avg_pm25 = pm25_sum / MEASURE_COUNT;
      
      Serial.println("\n=== Measurement Complete ===");
      Serial.print("Average concentration = ");
      Serial.print(avg_concentration);
      Serial.println(" pcs/0.01cf");
      Serial.print("Average PM2.5 = ");
      Serial.print(avg_pm25);
      Serial.println(" ug/m3");
      
      Serial.println("\nAll PM2.5 measurements:");
      for(int i = 0; i < MEASURE_COUNT; i++) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(measurements[i]);
        Serial.println(" ug/m3");
      }
      
      // 변수 초기화
      current_measure = 0;
      concentration_sum = 0;
      pm25_sum = 0;
      Serial.println("\nStarting new measurement cycle...\n");
    }
    
    // 다음 측정을 위한 초기화
    lowpulseoccupancy = 0;
    starttime = millis();
  }
}
