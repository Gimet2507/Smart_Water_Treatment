/*---------- Mendefinisikan template ID, Nama, dan Token otentikasi pada Blynk ----------*/
//
#define BLYNK_TEMPLATE_ID "TMPL6pd_Fqf8J"
#define BLYNK_TEMPLATE_NAME "Smart Water Treatment"
#define BLYNK_AUTH_TOKEN "ggEAtbAMfgyHo-akqnnECkyi9wf0iWxm"

/*---------- Mengimport Library yang dibutuhkan ----------*/
#include <ESP8266WiFi.h>                    // Wifi ESP8266
#include <BlynkSimpleEsp8266.h>             // Blynk
#include <Adafruit_ADS1X15.h>               // Modul ADS1115
#include <Servo.h>                          // Motor Servo
#include <LiquidCrystal_I2C.h>              // LCD I2C 16x2
#include <Fuzzy.h>                          // EFLL

/*---------- Mendefinisikan jaringan wifi dan token otentikasi pada Blynk ----------*/
char auth[] = "ggEAtbAMfgyHo-akqnnECkyi9wf0iWxm";
char ssid[] = "Tara";
char pass[] = "00000000";

#define SERVO_PIN 0                         // Pin data D3 atau GPIO 0 untuk motor servo
#define LCD_ADDRESS 0x27                    // alamat LCD I2C 16x2
Servo servoMotor;                           // Mendefinisikan objek Servo sebagai servoMotor
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);  // Mendefinisikan objek LiquidCrystal_I2C sebagai lcd
Adafruit_ADS1015 ads;                       // Mendefinisikan objek Adafruit_ADS1015 sebagai ads
BlynkTimer timer;                           // Mendefinisikan objek BlynkTimer sebagai timer

/*---------- Deklarasi variabel global ----------*/
int16_t adc0, adc1, adc2, adc3;             // Pin pada modul ADS1115
float volts0, volts1, volts2, volts3;       // variabel untuk menampung tegangan pada pin ADS1115

/*----------  Kalibrasi pH pada pengujian pH4 dan pH7 ----------*/
  /* pH 4 memiliki tegangan 3.056v saat melakukan pengujian
     sedangkan pH 7 memiliki tegangan 2.565v
  */
float ph4 = 3.226;
float ph7 = 2.565;

double Vclear = 2.07;                       // Vclear merupakan tegangan saat pengujian sensor turbidity pada air jernih
double ntu1 = 0;                            // Turbidity 1
double ntu2 = 0;                            // Turbidity 2

float phValue;                              // pH output
float phStep;                               // pH kalibrasi

String tds1Terbilang;                       // Turbidity 1 terbilang
String tds2Terbilang;                       // Turbidity 2 terbilang
String phTerbilang;                         // pH terbilang
String servoTerbilang;                      // Motor Servo terbilang

String tds1dservo;                          // Menampilkan TDS1 dan Motor Servo terbilang pada Blynk
String tds2dph;                             // Menampilkan TDS2 dan PH Terbilang pada Blynk

Fuzzy *fuzzy = new Fuzzy();                 // Mendeklarasikan objek Fuzzy kedalam variabel fuzzy

/*---------- Inisialisasi objek pada FUZZY ----------*/
FuzzySet *jernih= new FuzzySet(0, 0, 20, 25);
FuzzySet *normal = new FuzzySet(20,50, 50, 85);
FuzzySet *keruh = new FuzzySet(75, 80, 100, 100);

FuzzySet *asam= new FuzzySet(0, 0, 6, 7);
FuzzySet *sedang = new FuzzySet(6, 7, 7, 8);
FuzzySet *basa = new FuzzySet(7, 8, 14, 14);

FuzzySet *servoTutup = new FuzzySet(180, 180, 180, 180);
FuzzySet *servoBuka = new FuzzySet(0, 0, 0, 0);


/*---------- Memulai program ----------*/
void setup(void) {
  Serial.begin(9600);

  // Menghubungkan ke WiFi
  Blynk.begin(auth, ssid, pass);

  // Inisialisasi LCD
  lcd.init();
  lcd.clear();
  lcd.backlight();

  // Menampilkan Hello dan Loading pada LCD
  lcd.setCursor(0, 0);
  lcd.print("HELLO!");
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print("LOADING...");
  delay(500);

  /*  melakukan pengecekan wiring modul ADS1115
      Jika modul ADS1115 tidak terhubung, maka tampilkan pesan
      "Failed to initialize ADS" pada serial monitor
  */
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }

  /*  Jika modul ADS1115 terhubung, maka tampilkan pesan
      "ADS1115 OK" pada LCD
  */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ADS1115 OK");
  delay(1000);

  // Setup daya 3V pada channel 0, 1 dan 2 modul ADS1115
  ads.startComparator_SingleEnded(0, 1000);
  ads.startComparator_SingleEnded(1, 1000);
  ads.startComparator_SingleEnded(3, 1000);

  // Inisialisasi motor servo dan mengatur posisi pada 0 derajat
  servoMotor.attach(SERVO_PIN, 500, 2400);
  servoMotor.write(0);

  // Memanggil fungsi keanggotaan fuzzy dan aturan fuzzy
  membershipFunction();
  fuzzyRule();
}

void loop() {
  Blynk.run();  // Menjalankan Blynk
  timer.run();  // Menjalankan BlynkTimer

  // Membaca data ADC channel 0, 1 & 3 pada modul ADS1115
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc3 = ads.readADC_SingleEnded(3);

  // Mengukur tegangan channel 0, 1 & 3
  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts3 = ads.computeVolts(adc3);

  // Memanggil fungsi keanggotaan fuzzy dan aturan fuzzy
  hitungPh();
  delay(1000);
  hitungTds();
  delay(1000);
  hitungFuzzy();
  delay(1000);
  tampilanSistem();
}

/*---------- Fungsi untuk menghitung pH air ----------*/
void hitungPh(){
  phStep = (ph4 - ph7) / 3;
  phValue = 7.00 + ((ph7 - volts3) / phStep);
}

/*---------- Fungsi untuk menghitung kejernihan air sensor turbidity 1 dan 2 ----------*/
void hitungTds(){
  ntu1 = 100.00 - (volts0 / Vclear)*100.00 ;
  ntu2 = 100.00 - (volts1 / Vclear)*100.00 ;
}

/*---------- Fungsi keanggotaan Fuzzy ----------*/
void membershipFunction(){
  //--------------------------------------------------------------------- INPUT-------
  FuzzyInput *sensorTurbidity = new FuzzyInput(1);
  sensorTurbidity->addFuzzySet(jernih);
  sensorTurbidity->addFuzzySet(normal);
  sensorTurbidity->addFuzzySet(keruh);
  fuzzy->addFuzzyInput(sensorTurbidity);

  FuzzyInput *sensorPh = new FuzzyInput(2);
  sensorPh->addFuzzySet(asam);
  sensorPh->addFuzzySet(sedang);
  sensorPh->addFuzzySet(basa);
  fuzzy->addFuzzyInput(sensorPh);
  
  //--------------------------------------------------------------------- OUTPUT-------
  FuzzyOutput *sensorServo = new FuzzyOutput(1);
  sensorServo->addFuzzySet(servoTutup);
  sensorServo->addFuzzySet(servoBuka);
  fuzzy->addFuzzyOutput(sensorServo);
}

/*---------- Fungsi untuk menetapkan aturan Fuzzy ----------*/
void fuzzyRule(){
  FuzzyRuleAntecedent *ifasamANDkeruh = new FuzzyRuleAntecedent();
  ifasamANDkeruh->joinWithAND(asam, keruh);
  FuzzyRuleConsequent *thenservobuka = new FuzzyRuleConsequent();
  thenservobuka->addOutput(servoBuka);
  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, ifasamANDkeruh, thenservobuka);
  fuzzy->addFuzzyRule(fuzzyRule01);

  FuzzyRuleAntecedent *ifasamANDnormal = new FuzzyRuleAntecedent();
  ifasamANDnormal->joinWithAND(asam, normal);
  FuzzyRuleConsequent *thenservobuka2 = new FuzzyRuleConsequent();
  thenservobuka2->addOutput(servoBuka);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, ifasamANDnormal, thenservobuka2);
  fuzzy->addFuzzyRule(fuzzyRule02);

  FuzzyRuleAntecedent *ifasamANDjernih = new FuzzyRuleAntecedent();
  ifasamANDjernih->joinWithAND(asam, jernih);
  FuzzyRuleConsequent *thenservotutup = new FuzzyRuleConsequent();
  thenservotutup->addOutput(servoTutup);
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, ifasamANDjernih, thenservotutup);
  fuzzy->addFuzzyRule(fuzzyRule03);

  FuzzyRuleAntecedent *ifsedangANDkeruh = new FuzzyRuleAntecedent();
  ifsedangANDkeruh->joinWithAND(sedang, keruh);
  FuzzyRuleConsequent *thenservobuka3 = new FuzzyRuleConsequent();
  thenservobuka3->addOutput(servoBuka);
  FuzzyRule *fuzzyRule04 = new FuzzyRule(4, ifsedangANDkeruh, thenservobuka3);
  fuzzy->addFuzzyRule(fuzzyRule04);

  FuzzyRuleAntecedent *ifsedangANDnormal = new FuzzyRuleAntecedent();
  ifsedangANDnormal->joinWithAND(sedang, normal);
  FuzzyRuleConsequent *thenservobuka4 = new FuzzyRuleConsequent();
  thenservobuka4->addOutput(servoBuka);
  FuzzyRule *fuzzyRule05 = new FuzzyRule(5, ifsedangANDnormal, thenservobuka4);
  fuzzy->addFuzzyRule(fuzzyRule05);

  FuzzyRuleAntecedent *ifsedangANDjernih = new FuzzyRuleAntecedent();
  ifsedangANDjernih->joinWithAND(sedang, jernih);
  FuzzyRuleConsequent *thenservotutup2 = new FuzzyRuleConsequent();
  thenservotutup2->addOutput(servoTutup);
  FuzzyRule *fuzzyRule06 = new FuzzyRule(6, ifsedangANDjernih, thenservotutup2);
  fuzzy->addFuzzyRule(fuzzyRule06);

  FuzzyRuleAntecedent *ifbasaANDkeruh = new FuzzyRuleAntecedent();
  ifbasaANDkeruh->joinWithAND(basa, keruh);
  FuzzyRuleConsequent *thenservobuka5 = new FuzzyRuleConsequent();
  thenservobuka5->addOutput(servoBuka);
  FuzzyRule *fuzzyRule07 = new FuzzyRule(7, ifbasaANDkeruh, thenservobuka5);
  fuzzy->addFuzzyRule(fuzzyRule07);

  FuzzyRuleAntecedent *ifbasaANDnormal = new FuzzyRuleAntecedent();
  ifbasaANDnormal->joinWithAND(basa, normal);
  FuzzyRuleConsequent *thenservobuka6 = new FuzzyRuleConsequent();
  thenservobuka6->addOutput(servoBuka);
  FuzzyRule *fuzzyRule08 = new FuzzyRule(8, ifbasaANDnormal, thenservobuka6);
  fuzzy->addFuzzyRule(fuzzyRule08);

  FuzzyRuleAntecedent *ifbasaANDjernih = new FuzzyRuleAntecedent();
  ifbasaANDjernih->joinWithAND(basa, jernih);
  FuzzyRuleConsequent *thenservotutup3 = new FuzzyRuleConsequent();
  thenservotutup3->addOutput(servoTutup);
  FuzzyRule *fuzzyRule09 = new FuzzyRule(9, ifbasaANDjernih, thenservotutup3);
  fuzzy->addFuzzyRule(fuzzyRule09);
}

/*---------- Fungsi untuk menghitung nilai Fuzzy ----------*/
void hitungFuzzy(){
  int inputPh = phValue;
  int inputTurbidity = ntu1;
  Serial.println(String("Input : PH -> ")+ inputPh + (String("    | TURBIDITY -> "))+ inputTurbidity);

  fuzzy->setInput(1, inputTurbidity);
  fuzzy->setInput(2, inputPh);

  /*---------- FUZZYFIKASI ----------*/
  fuzzy->fuzzify();

  /*---------- DEFUZZYFIKASI ----------*/
  float output1 = fuzzy->defuzzify(1);
  servoMotor.write(output1);
}

/*---------- Fungsi untuk menampilkan output ----------*/
void tampilanSistem(){
  /*---------- Memanggil value dari Servo dan pH ----------*/
  bool nilaiTutup =(servoTutup->getPertinence());
  bool nilaiBuka = (servoBuka->getPertinence());

  bool nilaiJernih =(jernih->getPertinence());
  bool nilaiNormal =(normal->getPertinence());
  bool nilaiKeruh =(keruh->getPertinence());

  bool nilaiAsam = (asam->getPertinence());
  bool nilaiSedang = (sedang->getPertinence());
  bool nilaiBasa = (basa->getPertinence());

  /*---------- Servo terbilang ----------*/
  if(nilaiTutup == 1){
    servoTerbilang = "TUTUP";
  }
  else if (nilaiBuka == 1){
    servoTerbilang = "BUKA";
  }

  /*---------- Turbidity 1 ----------*/
  if (nilaiJernih == 1){
    tds1Terbilang = "JERNIH";
  }
  else if (nilaiNormal == 1){
    tds1Terbilang = "NORMAL";
  }
  else if (nilaiKeruh == 1){
    tds1Terbilang = "KERUH";
    {
   Blynk.logEvent("notifikasi_keruh", "notifikasi ketika kondisi keruh");
    }
  }

  /*---------- Turbidity 2 ----------*/
  if (ntu2 < 24.70) {
    tds2Terbilang = "JERNIH";
  }
  else if (24.70 < ntu2 < 71.48) {
    tds2Terbilang = "NORMAL";
  }
  else if (ntu2 > 71.48) {
    tds2Terbilang = "KERUH";
  }

  /*---------- pH ----------*/
  if (nilaiAsam == 1){
    phTerbilang = "ASAM";
  }
  else if (nilaiSedang == 1){
    phTerbilang = "SEDANG";
  }
  else if (nilaiBasa == 1){
    phTerbilang = "BASA";
  }

  tds1dservo = "NTU1: " + tds1Terbilang + " | " + " SERVO: " + servoTerbilang;
  tds2dph = "NTU2: " + tds2Terbilang + " | " + " PH AIR: " + phTerbilang;

  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print(" Volt: "); Serial.print(volts0);
  Serial.print(" NTU1: "); Serial.print(ntu1); Serial.print(" NTU "); Serial.println(tds1Terbilang);
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print(" Volt: "); Serial.print(volts1);
  Serial.print(" NTU2: "); Serial.print(ntu2); Serial.print(" NTU "); Serial.println(tds2Terbilang);
  Serial.print("AIN3: "); Serial.print(adc3); Serial.print(" Volt: "); Serial.print(volts3, 3);
  Serial.print(" phValue: "); Serial.println(phValue);
  
  Serial.print("SERVO: "); Serial.println(servoTerbilang);

  /*---------- Menampilkan nilai sensor turbidity pada LCD ----------*/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NTU1: "); lcd.print(ntu1); lcd.print(" NTU");
  lcd.setCursor(0, 1);
  lcd.print("NTU2: "); lcd.print(ntu2); lcd.print(" NTU");
  delay(1000);

  /*---------- Menampilkan nilai sensor pH dan motor servo pada LCD ----------*/
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PH: "); lcd.print(phValue);
  lcd.setCursor(0, 1);
  lcd.print("SERVO: "); lcd.print(servoTerbilang);
  delay(1000);

  /*---------- Mengirim data ke Blynk ----------*/
  Blynk.virtualWrite(V0, ntu1);
  Blynk.virtualWrite(V1, ntu2);
  Blynk.virtualWrite(V2, phValue);
  Blynk.virtualWrite(V3, tds1dservo);
  Blynk.virtualWrite(V4, tds2dph);
  delay(1000);
}
