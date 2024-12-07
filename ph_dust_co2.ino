#include <SoftwareSerial.h>

// 핀 정의
const int DUST_PIN = 8;
const int PH_PIN = A0;
SoftwareSerial mySerial(13, 11);

// CO2 센서 변수
unsigned char Send_data[4] = {0x11,0x01,0x01,0xED};
unsigned char Receive_Buff[8];
unsigned char recv_cnt = 0;
unsigned int PPM_Value;
float co2_sum = 0;

// 먼지 센서 변수
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 3000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float pm25_particles_per_liter = 0;

// 평균 계산 변수
const int MEASURE_COUNT = 30;
int current_measure = 0;
float concentration_sum = 0;
float pm25_sum = 0;
float measurements[30];

// pH 센서 변수
int buf[10], temp;
unsigned long int avgValue;
float phValue;

void Send_CMD(void) {
  for(int i=0; i<4; i++) {
    mySerial.write(Send_data[i]);
    delay(1);
  }
}

unsigned char Checksum_cal(void) {
  unsigned char SUM=0;
  for(unsigned char count=0; count<7; count++) {
    SUM += Receive_Buff[count];
  }
  return 256-SUM;
}

float readPH() {
  for(int i=0; i<10; i++) { 
    buf[i] = analogRead(PH_PIN);
    delay(10);
  }
  for(int i=0; i<9; i++) {
    for(int j=i+1; j<10; j++) {
      if(buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for(int i=2; i<8; i++) {
    avgValue += buf[i];
  }
  float pHVol = (float)avgValue * 5.0 / 1024 / 6;
  return -3.48 * pHVol + 21.34;
}

void setup() {
  pinMode(13, INPUT);
  pinMode(11, OUTPUT);
  pinMode(DUST_PIN, INPUT);
  
  Serial.begin(9600);
  while (!Serial);
  mySerial.begin(9600);
  while (!mySerial);
  
  starttime = millis();
  Serial.println("Starting measurements...");
}

void loop() {// CO2 센서 읽기
    Send_CMD();
    
  while(1) {
    if(mySerial.available()) { 
      Receive_Buff[recv_cnt++] = mySerial.read();
      if(recv_cnt == 8) {
        recv_cnt = 0;
        break;
      }
    }
  } 
  
if(Checksum_cal() == Receive_Buff[7]) {
    PPM_Value = Receive_Buff[3]<<8 | Receive_Buff[4];
    
    // CO2 보정
    float calibration_factor = 0.2;
    float base_co2 = 400.0;
    PPM_Value = (PPM_Value * calibration_factor) + base_co2;
            Serial.print("PPM_Value: ");
        Serial.println(PPM_Value);
    // CO2 값은 먼지센서 측정 타이밍에 맞춰 누적
    if ((millis()-starttime) >= sampletime_ms) {
        co2_sum += PPM_Value;  // 여기로 이동
        Serial.print("CO2 adding to sum: ");
        Serial.println(co2_sum);
    }
}

  // 먼지 센서 읽기
  duration = pulseIn(DUST_PIN, LOW);
  lowpulseoccupancy = lowpulseoccupancy + duration;

  if ((millis()-starttime) >= sampletime_ms) {
    // 먼지 농도 계산
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);
    concentration = 1.1*0.22*pow(ratio,3)-3.8*0.22*pow(ratio,2)+520*0.22*ratio+0.62;
   // concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    pm25_particles_per_liter = concentration/300 ;
     //  or concentration = 0.1*pow(ratio,3) - 0.38*pow(ratio,2) + 52*ratio + 0.62;
    
    
    // pH 측정
    float currentPH = readPH();
    
    // 현재 측정값 저장 및 출력
    measurements[current_measure] = pm25_particles_per_liter;
    concentration_sum += concentration;
    pm25_sum += pm25_particles_per_liter;
        Serial.print("pm25_sum: ");
        Serial.println(pm25_sum);
    
    // 현재 측정 횟수만 출력
    Serial.print("Measurement ");
    Serial.print(current_measure + 1);
    Serial.print(" of 30");
   
    current_measure++;
    
    // 30회 측정 완료시
    if (current_measure >= MEASURE_COUNT) {
      float avg_concentration = concentration_sum / MEASURE_COUNT;
      float avg_pm25 = pm25_sum / MEASURE_COUNT;
      float avg_co2 = co2_sum / MEASURE_COUNT;  // CO2 평균 계산
      
      Serial.println("\n=== Measurement Complete ===");
      Serial.print("Average CO2: ");
      Serial.print(avg_co2);
      Serial.println(" ppm");
      Serial.print("Average PM2.5: ");
      Serial.print(avg_pm25);
      Serial.println(" ug/m3");
      Serial.print("Current pH: ");
      Serial.println(currentPH);
      
      // 변수 초기화
      current_measure = 0;
      concentration_sum = 0;
      pm25_sum = 0;
      co2_sum = 0;
      Serial.println("\nStarting new measurement cycle...\n");
    }
    
    // 초기화
    lowpulseoccupancy = 0;
    starttime = millis();
  }
  
  delay(1000);
}
