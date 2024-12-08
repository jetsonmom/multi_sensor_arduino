
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 3000;//sampe 30s&nbsp;;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float particles_per_0_01cf=0;
float pm25_particles_per_liter =0;
void setup() {
  Serial.begin(9600);
  pinMode(8,INPUT);
  starttime = millis();//get the current time;
}

void loop() {
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
// if the sampel time = = 30s
  if ((millis()-starttime) >= sampletime_ms) { 
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  
    concentration = 1.1*0.22*pow(ratio,3)-3.8*0.22*pow(ratio,2)+520*0.22*ratio+0.62; // using spec sheet curve

pm25_particles_per_liter = concentration / 300 ; // # 1리터 당 입자 수
    Serial.print("concentration = ");
    Serial.print(concentration);
    Serial.println(" pcs/0.01cf");
    Serial.println("\n");
    Serial.print("dust = ");
    Serial.print(pm25_particles_per_liter);
 Serial.println(" ug/m3");

    lowpulseoccupancy = 0;
    starttime = millis();
  }
}
