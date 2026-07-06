# Load Cell Calibration (NAU7802)

This Arduino sketch provides an interactive serial interface to calibrate a load cell connected via the **NAU7802** 24-bit ADC amplifier. It helps you determine the **Zero Offset** and **Calibration Factor** needed for accurate weight measurements in your main project.

## Features
- **Warm-up Timer**: Enforces a 60-second warm-up period to ensure thermal stability of the load cell and electronics.
- **Interactive Menu**: Simple single-character commands to control the calibration process.
- **EEPROM-ready Values**: Outputs the exact `long` and `float` values you need to copy into your final code.

## Hardware Requirements
- Microcontroller (e.g., Teensy 4.1, ESP32, Arduino Uno)
- **NAU7802** Load Cell Amplifier (e.g., SparkFun Qwiic Scale)
- Load Cell (assembled and mounted)

## Dependencies
You need to install the following library via the Arduino Library Manager:
- **SparkFun Qwiic Scale NAU7802 Arduino Library**

## Wiring
Connect the NAU7802 to your microcontroller's I2C pins:
- **SDA** <-> **SDA**
- **SCL** <-> **SCL**
- **3.3V** <-> **3.3V** (or VCC)
- **GND** <-> **GND**

## How to Use

1. **Upload**: Open `load_cell_calibration.ino` in the Arduino IDE and upload it to your board.
2. **Serial Monitor**: Open the Serial Monitor and set the baud rate to **115200**.
3. **Warm Up**: The system will count down 60 seconds. **Do not touch the scale** during this time.
4. **Calibration Steps**:
    - **Step 1: Set Known Weight (`k`)**: 
        - Type `k` and press Enter.
        - Enter the weight of your calibration object (e.g., `100.0` for 100 grams, `0.5` for 0.5 kg, etc.).
        - *Note: The unit you use here (g, kg, lbs) will be the unit of your final readings.*
    
    - **Step 2: Zero / Tare (`z`)**:
        - Ensure the scale is completely empty.
        - Type `z` and press Enter.
        - The code will calculate and print the **Zero Offset**.

    - **Step 3: Calibrate (`c`)**:
        - Place your known weight on the scale.
        - Type `c` and press Enter.
        - The code will calculate and print the **Calibration Factor**.

5. **Verify**: 
    - Type `w` to read the current weight. It should match your known weight.
    - Remove the weight; it should return close to 0.0.

## Important: Saving Your Calibration
Once you have successfully calibrated, **write down** the following values printed in the Serial Monitor:

1. **Zero Offset** (e.g., `123456`)
2. **Calibration Factor** (e.g., `456.78`)

You will need to hardcode these values into your main project's setup function like this:
```cpp
scale.setZeroOffset(123456);
scale.setCalibrationFactor(456.78);
```
