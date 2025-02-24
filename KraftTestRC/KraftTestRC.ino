/* ESP32S3 Bluetooth adapter for Kraft Apple II joystick

    Copyright (c) Michael Neil and Far Left Lane. All rights reserved.
    Licensed under the MIT license. See LICENSE file in the project root for details.

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/

/* 
    Board XIAO_ESP32S3          https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
    Library ESP32-BLE-Gamepad   https://github.com/lemmingDev/ESP32-BLE-Gamepad
*/

#include <Arduino.h>

#include <BleGamepad.h>


const int sChargePin = D7;
const int sYPin = D9;
const int sXPin = D10;
const int sRedButtonPin = D0;
const int sBlackButtonPin = D2;

BleGamepad bleGamepad;
BleGamepadConfiguration bleGamepadConfig;  


// the setup function runs once when you press reset or power the board
void setup() 
{
  delay(1000);
  Serial.begin(115200);

  Serial.println("Setup");

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(sChargePin, OUTPUT);              //  This pin is used to charge the capacitors 
  digitalWrite(sChargePin, LOW);

  pinMode(sXPin, INPUT);                    //  The axis pins read a Resistor (100 Ohm + 150K pot) / Capacitor network (22nF)
  pinMode(sYPin, INPUT);

  pinMode(sRedButtonPin, INPUT_PULLUP);     //  Pull up the button pins as they switch to GND
  pinMode(sBlackButtonPin, INPUT_PULLUP);

  bleGamepadConfig.setAutoReport(false);    //  We will report all changes at the end of the loop
  bleGamepadConfig.setControllerType(CONTROLLER_TYPE_JOYSTICK); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
  bleGamepadConfig.setVid(0xe502);          //  From the TestAll sample 
  bleGamepadConfig.setPid(0xabcd);
  bleGamepadConfig.setButtonCount(2);
  bleGamepadConfig.setHatSwitchCount(0);
  bleGamepadConfig.setWhichAxes(true, true, false, false, false, false, false, false);

  bleGamepad.begin(&bleGamepadConfig);
}

unsigned long ReadAxis (int inPin)
{
  digitalWrite(sChargePin, HIGH);     //  Charge the capacitor
  delay(10);                          //  wait for a bit
  digitalWrite(sChargePin, LOW);      //  Start the capacitor discharge

  unsigned long startTime = micros();

  digitalWrite(LED_BUILTIN, HIGH);    // turn the LED off by making the voltage HIGH
  while (digitalRead(inPin) != 0)
  {
    //  Loop waiting for the pin to go low
  }

  unsigned long endTime = micros();

  return (endTime - startTime);
}

int sLastRedButton = 0;
int sLastBlackButton = 0;

// the loop function runs over and over again forever
void loop() 
{
  //Serial.println("Loop");

  if (bleGamepad.isConnected())
  {
    digitalWrite(LED_BUILTIN, LOW);     // turn the LED on (LOW is the voltage level)

    unsigned long xAxis = ReadAxis(sXPin);
    unsigned long yAxis = ReadAxis(sYPin);

    Serial.printf("Loop : X : %d Y : %d Red : %d Black : %d\n", xAxis, yAxis , digitalRead(sRedButtonPin), digitalRead(sBlackButtonPin));
 
    uint16_t XJoystickValue;
    uint16_t YJoystickValue;

    //  TODO Add more noise dampening and a center "notch"

    XJoystickValue = map(xAxis, 0, 2200, 0, 32737);
    YJoystickValue = map(yAxis, 0, 2400, 0, 32737);

    bleGamepad.setX(XJoystickValue);
    bleGamepad.setY(YJoystickValue);

    int redButton = digitalRead(sRedButtonPin);
    if (redButton != sLastRedButton)
    {      
      if (digitalRead(redButton) == 0)
        bleGamepad.press(BUTTON_1);
      else 
        bleGamepad.release(BUTTON_1);

      sLastRedButton = redButton;
    }

    int blackButton = digitalRead(sBlackButtonPin);
    if (blackButton != sLastBlackButton)
    {
      if (digitalRead(blackButton) == 0)
        bleGamepad.press(BUTTON_2);
      else 
        bleGamepad.release(BUTTON_2);

      sLastBlackButton = blackButton;
    }

    // Serial.printf("Loop : X : %d Y : %d Red : %d Black : %d\n", XJoystickValue, YJoystickValue , redButton, blackButton);

    bleGamepad.sendReport();

    delay(10);                        //  wait for a bit
  }
  else
  {
    delay(100);                       //  wait for a connection
  }
}
