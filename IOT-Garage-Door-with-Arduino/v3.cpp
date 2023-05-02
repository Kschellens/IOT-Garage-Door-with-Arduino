#include <IRremote.h>
#include <Stepper.h>
#include "NewPing.h"

#define IR_RECEIVE_PIN 2
#define TRIGGER_PIN 6
#define ECHO_PIN 3
#define MOSFET_PIN 7

#define MAX_DISTANCE 5

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

enum IrButtonCodes {
  BUTTON_1 = 12,
  BUTTON_2 = 24,
  BUTTON_3 = 94,
  BUTTON_0 = 22
};
int button1Pressed;
int button3Pressed;

const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 8, 10, 9, 11);

const int ledRougePin = 13;
const int ledVertPin = 12;
const int ledBleuPin = 5;

const int LED_BLINK_DURATION = 500;
const int LED_BLINK_TIMES = 2;

void clignoterLED(int ledPin, int duration, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(duration);
    digitalWrite(ledPin, LOW);
    delay(duration);
  }
}

bool objectDetected() {
  int distance = sonar.ping_cm();
  return distance > 0 && distance <= 5;
}

void actionButton1() {
  digitalWrite(MOSFET_PIN, HIGH); // Active le moteur
  Serial.println("Pressed on button 1");
  int numberOfSteps = 10;
  int stepsToPerform = stepsPerRevolution / 4;
  int stepsPerInterval = stepsToPerform / numberOfSteps;
  for (int i = 0; i < numberOfSteps; i++) {
    myStepper.step(stepsPerInterval);
    clignoterLED(ledVertPin, LED_BLINK_DURATION / numberOfSteps, 1);
    
    if (objectDetected() || command == BUTTON_2) {
      actionButton2();
      break;
    }
  }
}

void actionButton2() {
  Serial.println("Pressed on button 2 (Emergency stop)");
  myStepper.step(0);
  digitalWrite(ledRougePin, LOW);
  digitalWrite(ledVertPin, LOW);
  digitalWrite(ledBleuPin, LOW);

  digitalWrite(MOSFET_PIN, LOW); // Coupe le courant du moteur
}

void actionButton3() {
  digitalWrite(MOSFET_PIN, HIGH); // Active le moteur
  Serial.println("Pressed on button 3");
  int numberOfSteps = 10;
  int stepsToPerform = stepsPerRevolution / 4;
  int stepsPerInterval = stepsToPerform / numberOfSteps;
  for (int i = 0; i < numberOfSteps; i++) {
    myStepper.step(-stepsPerInterval);
    clignoterLED(ledRougePin, LED_BLINK_DURATION / numberOfSteps, 1);
    
    if (objectDetected() || command == BUTTON_2) {
      actionButton2();
      break;
    }
  }
}



void actionButton0() {
  Serial.println("Button 1 pressed :" + String(button1Pressed));
  Serial.println("Button 3 pressed :" + String(button3Pressed));
  delay(1000); // Attendez 1 seconde (1000 ms) avant de permettre une nouvelle impression
}

void setup() {
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN);
  myStepper.setSpeed(5);

  pinMode(ledRougePin, OUTPUT);
  pinMode(ledVertPin, OUTPUT);
  pinMode(ledBleuPin, OUTPUT);

  pinMode(MOSFET_PIN, OUTPUT); // Configure la broche MOSFET en tant que sortie
  digitalWrite(MOSFET_PIN, HIGH); // Active le moteur par défaut
}

void loop() {
  if (IrReceiver.decode()) {
    IrReceiver.resume();
    int command = IrReceiver.decodedIRData.command;
    switch (command) {
      case BUTTON_1: {
        actionButton1();
        button1Pressed++;
        break;
      }
      case BUTTON_2: {
        actionButton2();
        break;
      }
      case BUTTON_3: {
        actionButton3();
        button3Pressed++;
        break;
      }
      case BUTTON_0: {
        actionButton0();
        break;
      }
      default: {
        Serial.println("Button not recognized");
      }
    }
  }
}
