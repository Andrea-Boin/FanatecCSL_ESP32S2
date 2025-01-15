#include <Joystick_ESP32S2.h> // joystick library only for esp32 s2
#include <Arduino.h> // 

//configurazione joystick
//Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, // setup joystick with default report id
//                  JOYSTICK_TYPE_JOYSTICK, // type of joystick, like multi axis or gamepad
//                  0, // number of button 0 - 32
//                  0, // number of siwtch 0 -2
//                  true, // x axis
//                  true, // y axis
//                  true, // z axis
//                  false, // Rotation x axis
//                  false, // Rotation y axis
//                  false, // Rotation z axis
//                  false, // Rudder
//                  false, // Throttle
//                  true, // Accelerator
//                  true, // Brake
//                  false); // Steering

Joystick_ Joystick; // This string create a defaul joystick without custom setup

const int ThrottlePin = 32; // Define Throttle pin
const int BrakePin = 33; // Define Brake pin
const int ClutchPin = 34; // Define Clutch pin
// change this value for manual calibration, watch serial monitor for value
int ThrottleValoreMIN = 600;  // Default min value for Throttle
int ThrottleValoreMAX = 1023;  // Default max value for Throttle
int BrakeValoreMIN = 600;     // Default min value for Brake
int BrakeValoreMAX = 1023;     // Default max value for Brake
int ClutchValoreMIN = 600;    // Default min value for Clutch
int ClutchValoreMAX = 1023;    // Default max value for Clutch

const int HandBRAKE_pin = 21; // Define Hand brake pin
const int ShiftUP_pin = 22; // Define shift up pin
const int ShiftDOWN_pin = 23; // Define shift down pin

// Variables for additional controls
int HandBRAKE = 0;  // Handbrake state
int ShiftUP = 0;    // Shift-up button state
int ShiftDOWN = 0;  // Shift-down button state

// Variables to store the last state of the controls
int LastStateHand = 0; // Last state of the handbrake
int LastStateUP = 0;   // Last state of the shift-up button
int LastStateDOWN = 0; // Last state of the shift-down button

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Define serial bauderate
  Joystick.begin(); // Start joystick
// Define type of pin
  pinMode(ThrottlePin, INPUT); 
  pinMode(BrakePin, INPUT);
  pinMode(ClutchPin, INPUT);

  pinMode(HandBRAKE_pin, INPUT_PULLUP); // pullup with 10k ohm
  pinMode(ShiftUP_pin, INPUT_PULLUP);
  pinMode(ShiftDOWN_pin, INPUT_PULLUP);

// Wait for the program to connect only when the keyword is detected
  while (true) {
    if (Serial.available()) {
      String command = Serial.readString(); // Read the incoming command
      if (command == "ArduFan_CSL") {
        // Respond with a confirmation message
        Serial.println("ArduFan_CSL_ready");
        break; // Exit the loop and start sending data
      }
    }
  }


}

void loop() {
  // put your main code here, to run repeatedly:
    int Throttle = analogRead(ThrottlePin);  // Read throttle pedal value
    int Brake = analogRead(BrakePin);        // Read brake pedal value
    int Clutch = analogRead(ClutchPin);      // Read clutch pedal value

    int HandBRAKE = !digitalRead(HandBRAKE_pin);  // Read handbrake state (inverted logic)
    int ShiftUP = !digitalRead(ShiftUP_pin);      // Read shift-up button state (inverted logic)
    int ShiftDOWN = !digitalRead(ShiftDOWN_pin);  // Read shift-down button state (inverted logic)

    // Update joystick button states only if there's a change
    if (HandBRAKE != LastStateHand) {
      Joystick.setButton(0, HandBRAKE);  // Set handbrake button state on joystick
      LastStateHand = HandBRAKE;         // Update last state
    }

    if (ShiftUP != LastStateUP) {
      Joystick.setButton(1, ShiftUP);    // Set shift-up button state on joystick
      LastStateUP = ShiftUP;             // Update last state
    }

    if (ShiftDOWN != LastStateDOWN) {
      Joystick.setButton(2, ShiftDOWN);  // Set shift-down button state on joystick
      LastStateDOWN = ShiftDOWN;         // Update last state
    }

    // Set the range for the pedals based on the calibrated values
    Joystick.setXAxisRange(ThrottleValoreMIN, ThrottleValoreMAX); // Min and max for throttle pedal
    Joystick.setXAxis(Throttle); // Current value of the throttle pedal

    Joystick.setYAxisRange(BrakeValoreMIN, BrakeValoreMAX); // Min and max for brake pedal
    Joystick.setYAxis(Brake); // Current value of the brake pedal

    Joystick.setZAxisRange(ClutchValoreMIN, ClutchValoreMAX); // Min and max for clutch pedal
    Joystick.setZAxis(Clutch); // Current value of the clutch pedal

    Serial.println(Throttle);
    Serial.print(" - ");
    Serial.print(Brake);
    Serial.print(" - ");
    Serial.print(Clutch);
    Serial.print();

}