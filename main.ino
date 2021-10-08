/*
+---+------+------+------+----------------------+
| # | MS1  | MS2  | MS3  | Microstep resolution |
+---+------+------+------+----------------------+
| 0 | LOW  | LOW  | LOW  | Full step            |
| 1 | HIGH | LOW  | LOW  | Half step            |
| 2 | LOW  | HIGH | LOW  | Quarter step         |
| 3 | HIGH | HIGH | LOW  | Eighth step          |
| 4 | HIGH | HIGH | HIGH | Sixteenth step       |
+---+------+------+------+----------------------+
*/

// Enc have 24 steps per revolution
// The motor have 800 steps per revolution
// Want: 1 encoder rev = 1 motor rev
// 800 / 24 = 33.3333333333...
// float steps_per_pulse = 33.3333333333;
// 200(стъпки на мотора) / 32(стъпки на енкодера)
// 360/1.8(step angle) = 200
// 200/32 = 6.25

// const int PIN_MS_1 = 8;
// const int PIN_MS_2 = 9;
// const int PIN_MS_3 = 10;

// pinMode(PIN_MS_1, OUTPUT);
// pinMode(PIN_MS_2, OUTPUT);
// pinMode(PIN_MS_3, OUTPUT);

// Serial.println(encoderChar);
// digitalWrite(PIN_MS_1, LOW);
// digitalWrite(PIN_MS_2, LOW);
// digitalWrite(PIN_MS_3, LOW);

#include <AccelStepper.h>
#include <Bounce2.h>

const int PIN_BTN_STOP = 2;
const int PIN_BTN_BACK = 3;
const int PIN_BTN_FORTH = 4;

const int MOTOR_PIN_DIR = 5;
const int MOTOR_PIN_STEP = 6;
const int PIN_MOTOR_ENABLE = 7;

const int ENCODER_PIN_A = 8;
const int ENCODER_PIN_B = 9;

const int PIN_BTN_END_BACK = 10;
const int PIN_BTN_END_FORTH = 11;

int encoder_pin_A_last = LOW;
int encoder_pos = 0;
int n = LOW;

float steps_per_pulse = 80.25;

char encoderChar = '0';
char encoderCharLast = '0';

const unsigned long DELAY_WORK = 1000;

AccelStepper motor(AccelStepper::DRIVER, MOTOR_PIN_STEP, MOTOR_PIN_DIR);
AccelStepper motor_high_speed(AccelStepper::DRIVER, MOTOR_PIN_STEP, MOTOR_PIN_DIR);

Bounce btnBack = Bounce();
Bounce btnForth = Bounce();
Bounce btnStop = Bounce();

Bounce btnEndBack = Bounce();
Bounce btnEndForth = Bounce();

const unsigned long DEBOUNCE_DELAY = 5;

boolean homing, working;

boolean home_position = false;
boolean end_back_position = false;
boolean end_forth_position = false;


void setup(){
  Serial.begin(9600);

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  motor.setMaxSpeed(8500.0); // 800.0
  motor.setAcceleration(8800.0); // 300.0

  motor_high_speed.setMaxSpeed(15000.0); // 800.0
  motor_high_speed.setAcceleration(15000.0); // 300.0

  // intial buttons
  pinMode(PIN_BTN_BACK, INPUT_PULLUP);
  btnBack.attach(PIN_BTN_BACK);
  btnBack.interval(DEBOUNCE_DELAY);

  pinMode(PIN_BTN_FORTH, INPUT_PULLUP);
  btnForth.attach(PIN_BTN_FORTH);
  btnForth.interval(DEBOUNCE_DELAY);

  pinMode(PIN_BTN_STOP, INPUT_PULLUP);
  btnStop.attach(PIN_BTN_STOP);
  btnStop.interval(DEBOUNCE_DELAY);

  pinMode(PIN_BTN_END_BACK, INPUT_PULLUP);
  btnEndBack.attach(PIN_BTN_END_BACK);
  btnEndBack.interval(DEBOUNCE_DELAY);

  pinMode(PIN_BTN_END_FORTH, INPUT_PULLUP);
  btnEndForth.attach(PIN_BTN_END_FORTH);
  btnEndForth.interval(DEBOUNCE_DELAY);

}

void run_motor_sensor() {
  n = digitalRead(ENCODER_PIN_A);

  if ((encoder_pin_A_last == LOW) && (n == HIGH)) {

    digitalWrite(PIN_MOTOR_ENABLE, LOW);

    if (digitalRead(ENCODER_PIN_B) == LOW) {
      encoder_pos--;
      encoderChar = 'B';
    }
    else {
      encoder_pos++;
      encoderChar = 'A';
    }

    if (encoderCharLast == encoderChar) {
      Serial.print("Encoder char: ");
      Serial.println(encoderChar);
      motor.runToNewPosition((long) round(encoder_pos * steps_per_pulse));
    }
  } 

  encoderCharLast = encoderChar;
  encoder_pin_A_last = n;
}

void loop() {

    if (digitalRead(ENCODER_PIN_A) || digitalRead(ENCODER_PIN_B)) {
      // Serial.println("RUN SENSOR: ");
      button_end_update();
      if (home_position){
        run_motor_sensor();
      } else {
        end_back_position = true;
        run_motor_back();
      }
    } 
  
    btnBack.update();
    boolean valBtnBack = btnBack.read();
    if (valBtnBack == LOW && digitalRead(PIN_BTN_END_BACK)) {
      Serial.println("Enable motor back:");
      end_back_position = true;
      run_motor_back();
    } 
    
    btnForth.update();
    boolean valBtnForth = btnForth.read();
    if (valBtnForth == LOW && digitalRead(PIN_BTN_END_FORTH)) {
      Serial.println("Enable motor forth:");
      end_forth_position = true;
      run_motor_forth();
    }

    working = false;
    setPinEnable(working);
}

void run_motor_forth() {
  digitalWrite(PIN_MOTOR_ENABLE, LOW);
  motor_high_speed.setCurrentPosition(0);

  while (digitalRead(PIN_BTN_STOP) && end_forth_position) {
    button_forth_update();
    motor_high_speed.move(100);
    motor_high_speed.run();
  }
  
  working = false;
  setPinEnable(working);
}

void button_forth_update(){
  btnEndForth.update();
  boolean valBtnEndForth = btnEndForth.read();
  if (valBtnEndForth == LOW) {
    Serial.println("BUTTON END BACK:");
    end_forth_position = false;
  }
}

void run_motor_back() {
  motor_high_speed.setCurrentPosition(0);
  digitalWrite(PIN_MOTOR_ENABLE, LOW);
  
  while (digitalRead(PIN_BTN_STOP) && end_back_position) {
    button_end_update();
    motor_high_speed.move(-100);
    motor_high_speed.run();
  }
  
   working = false;
   setPinEnable(working);
}

void button_end_update(){
  btnEndBack.update();
  boolean valBtnEndBack = btnEndBack.read();
  if (valBtnEndBack == LOW) {
    Serial.println("BUTTON END BACK:");
    home_position = true;
    end_back_position = false;
  }
}

void setPinEnable(boolean working) {
  digitalWrite(PIN_MOTOR_ENABLE, !working);
  //digitalWrite(PIN_LED_STATO, working);
}


/*
// example
void run_motor(int dir) {
  
  String moto_dir = "";
  digitalWrite(PIN_MOTOR_ENABLE, LOW);
  motor_high_speed.setCurrentPosition(0);

  while (digitalRead(PIN_BTN_STOP)) {
    motor_high_speed.move(dir);
    motor_high_speed.run();
  }
  
   working = false;
   setPinEnable(working);
}
*/
