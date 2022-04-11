#define PIN_SDA 5 //D1
#define PIN_SCL 4 //D2

#define PIN_SU   14 //D5
#define PIN_STBY 12 //D6
#define PIN_H_A1 13 //D7
#define PIN_H_A2 2  //D4

int dir;

void setup() {
  
  delay(2000);
  Serial.begin(115200);
  Serial.println("\n\nBOOT");
  
  pinMode(PIN_SU, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_H_A1, OUTPUT);
  pinMode(PIN_H_A2, OUTPUT);
  
  digitalWrite(PIN_SU, LOW);
  digitalWrite(PIN_STBY, LOW);
  digitalWrite(PIN_H_A1, LOW);
  digitalWrite(PIN_H_A2, LOW);

  dir = 0;
}

// the loop function runs over and over again forever
void loop() {

  Serial.println("");
  Serial.print("LOOP -: dir: ");
  Serial.println(dir);

  if (dir) {
    dir = 0;
  }
  else {
    dir = 1;
  }

  digitalWrite(PIN_SU, HIGH);
  
  Serial.println("Charging...");
  delay(5000);
  Serial.println("Charged!");

  if (dir) {
    digitalWrite(PIN_H_A1, HIGH);
    digitalWrite(PIN_H_A2, LOW);
  }
  else {
    digitalWrite(PIN_H_A1, LOW);
    digitalWrite(PIN_H_A2, HIGH);
  }

  Serial.println("Activation");
  digitalWrite(PIN_STBY, HIGH);
  delay(75);
  Serial.println("Stop");
  
  digitalWrite(PIN_STBY, LOW);
  digitalWrite(PIN_SU, LOW);
  digitalWrite(PIN_H_A1, LOW);
  digitalWrite(PIN_H_A2, LOW);

  Serial.println("Waiting");
  delay(10000);
}
