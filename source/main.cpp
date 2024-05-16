#include "MicroBit.h"

MicroBit uBit;

const int MOTOR_ADDR = 0x01 << 1;  // I2C address of the STM8S driver (shifted left by 1)
const int MOTOR_REG = 0x02;  // Register for motor control

void setPwmMotor(int mode, int motorL, int motorR) {
    uint8_t data[5] = {MOTOR_REG, static_cast<uint8_t>(mode), static_cast<uint8_t>(motorL), static_cast<uint8_t>(motorR), 0};
    uBit.i2c.write(MOTOR_ADDR, data, sizeof(data));
}
//mode: 1 = Vorwärts | -1 = Rückwerts | 0 = Stopp
// -1, -255, 255 für vorwärts

int getDigitalValue(int pin) {
    NRF_GPIO->PIN_CNF[pin] = (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
                             (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                             (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                             (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                             (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);

    return (NRF_GPIO->IN >> pin) & 1;
}

int main() {
    uBit.init();

    while (true) {
        /*setPwmMotor(1, 255, -255);
        uBit.sleep(2000);
        setPwmMotor(0, 0, 0);
        uBit.sleep(1000);*/

        int leftSensorValue = getDigitalValue(uBit.io.P14.name);

        ManagedString sensorValueString(leftSensorValue);
        uBit.display.print(sensorValueString);

        uBit.sleep(50);

    }
}