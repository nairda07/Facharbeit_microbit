#include "MicroBit.h"

MicroBit uBit;

const int MOTOR_ADDR = 0x01 << 1;
const int MOTOR_REG = 0x02;

const int TRIGGER_PIN = MICROBIT_PIN_P16;
const int ECHO_PIN = MICROBIT_PIN_P15;

volatile bool stopMotors = false;
volatile uint64_t echoStartTime = 0;
volatile uint64_t echoEndTime = 0;
volatile bool echoReceived = false;

void setPwmMotor(int mode, int motorL, int motorR) {
    uint8_t data[5] = {MOTOR_REG, static_cast<uint8_t>(mode), static_cast<uint8_t>(motorL), static_cast<uint8_t>(motorR), 0};
    uBit.i2c.write(MOTOR_ADDR, data, sizeof(data));
}

int getDigitalValue(int pin) {
    NRF_GPIO->PIN_CNF[pin] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
                             (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                             (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                             (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                             (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    return (NRF_GPIO->IN >> pin) & 1;
}

void echoISR(MicroBitEvent) {
    if (uBit.io.P15.getDigitalValue()) {
        echoStartTime = system_timer_current_time_us();
    } else {
        echoEndTime = system_timer_current_time_us();
        echoReceived = true;
    }
}

void setupUltrasonic() {
    uBit.io.P16.setDigitalValue(0);
    uBit.io.P15.eventOn(MICROBIT_PIN_EVENT_ON_EDGE);
    uBit.messageBus.listen(MICROBIT_ID_IO_P15, MICROBIT_PIN_EVT_RISE, echoISR);
    uBit.messageBus.listen(MICROBIT_ID_IO_P15, MICROBIT_PIN_EVT_FALL, echoISR);
}

void triggerUltrasonic() {
    uBit.io.P16.setDigitalValue(1);
    uBit.sleep(10);
    uBit.io.P16.setDigitalValue(0);
}

float measureDistance() {
    triggerUltrasonic();

    while (!echoReceived) {
        uBit.sleep(1);
    }

    echoReceived = false;

    uint64_t duration = echoEndTime - echoStartTime;
    float distance = (duration / 2.0) / 29.1;
    return distance;
}

int main() {
    uBit.init();
    setupUltrasonic();

    while (true) {
        float distance = measureDistance();
        if (distance < 20.0) {
            stopMotors = true;
        } else {
            stopMotors = false;
        }
        if (stopMotors) {
            setPwmMotor(0, 0, 0);
        } else {
            int leftSensorValue = getDigitalValue(uBit.io.P13.name);
            int rightSensorValue = getDigitalValue(uBit.io.P14.name);

            if (leftSensorValue == 0 && rightSensorValue == 0) {
                setPwmMotor(-1, -60, 60);
            } else if (leftSensorValue == 1 && rightSensorValue == 0) {
                setPwmMotor(1, 0, 60);
            } else if (leftSensorValue == 0 && rightSensorValue == 1) {
                setPwmMotor(-1, -60, 0);
            } else {
                setPwmMotor(0, 0, 0);
            }
        }
    }
}