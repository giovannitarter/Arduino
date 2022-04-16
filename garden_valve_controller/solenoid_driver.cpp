#include <ESP8266WiFi.h>

#include "garden_valve_controller.h"
#include "solenoid_driver.h"

SolenoidDriver::SolenoidDriver() {
}


void SolenoidDriver::init(
    uint8_t pin_step_up, 
    uint8_t pin_stby,
    uint8_t pin_h1,
    uint8_t pin_h2
) 
{

    pin_su = pin_step_up;
    pin_stby = pin_stby;
    pin_h1 = pin_h1;
    pin_h2 = pin_h2;
}


void SolenoidDriver::close_valve() {

    pinMode(pin_h1, OUTPUT);
    pinMode(pin_h2, OUTPUT);
    digitalWrite(pin_su, HIGH);

    Serial.println("Charging...");
    delay(CAP_CHARGE_TIME);
    Serial.println("Charged!");

    digitalWrite(pin_h1, HIGH);
    digitalWrite(pin_h2, LOW);

    Serial.println("Activation");
    digitalWrite(pin_stby, HIGH);
    delay(SOLENOID_PULSE_TIME);
    Serial.println("Stop");

    digitalWrite(pin_stby, LOW);
    digitalWrite(pin_su, LOW);
    digitalWrite(pin_h1, LOW);
    digitalWrite(pin_h2, LOW);

}


void SolenoidDriver::open_valve() {

    pinMode(pin_h1, OUTPUT);
    pinMode(pin_h2, OUTPUT);
    digitalWrite(pin_su, HIGH);

    Serial.println("Charging...");
    delay(CAP_CHARGE_TIME);
    Serial.println("Charged!");

    digitalWrite(pin_h1, LOW);
    digitalWrite(pin_h2, HIGH);

    Serial.println("Activation");
    digitalWrite(pin_stby, HIGH);
    delay(SOLENOID_PULSE_TIME);
    Serial.println("Stop");

    digitalWrite(pin_stby, LOW);
    digitalWrite(pin_su, LOW);
    digitalWrite(pin_h1, LOW);
    digitalWrite(pin_h2, LOW);

}
