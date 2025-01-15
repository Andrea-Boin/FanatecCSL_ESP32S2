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

const int HandBRAKE_pin = 21; // Define Hand brake pin
const int ShiftUP_pin = 22; // Define shift up pin
const int ShiftDOWN_pin = 23; // Define shift down pin

// Global variables to store the values read from the pins
int Throttle = 0; // Value for the throttle pedal
int Brake = 0;    // Value for the brake pedal
int Clutch = 0;   // Value for the clutch pedal

// Minimum and maximum values for pedal calibration
int ThrottleValoreMIN = 0, ThrottleValoreMAX = 1023; // Throttle pedal range
int BrakeValoreMIN = 0, BrakeValoreMAX = 1023;       // Brake pedal range
int ClutchValoreMIN = 0, ClutchValoreMAX = 1023;     // Clutch pedal range

// Variables for additional controls
int HandBRAKE = 0;  // Handbrake state
int ShiftUP = 0;    // Shift-up button state
int ShiftDOWN = 0;  // Shift-down button state

// Variables to store the last state of the controls
int LastStateHand = 0; // Last state of the handbrake
int LastStateUP = 0;   // Last state of the shift-up button
int LastStateDOWN = 0; // Last state of the shift-down button

// Queue handle for serial communication
QueueHandle_t serialQueue; // Queue to manage data sent via serial communication

// Task declarations
void TaskJoystick(void *pvParameters); // Task for handling joystick inputs
void TaskSeriale(void *pvParameters);  // Task for handling serial communication

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Define serial bauderate
  Joystick.begin(); // Start joystick

  ThrottleValoreMIN = 600;  // Default min value for Throttle
  ThrottleValoreMAX = 1023;  // Default max value for Throttle
  BrakeValoreMIN = 600;     // Default min value for Brake
  BrakeValoreMAX = 1023;     // Default max value for Brake
  ClutchValoreMIN = 600;    // Default min value for Clutch
  ClutchValoreMAX = 1023;    // Default max value for Clutch
// Define type of pin
  pinMode(ThrottlePin, INPUT); 
  pinMode(BrakePin, INPUT);
  pinMode(ClutchPin, INPUT);

  pinMode(HandBRAKE_pin, INPUT_PULLUP); //resistenza da 10k
  pinMode(ShiftUP_pin, INPUT_PULLUP);
  pinMode(ShiftDOWN_pin, INPUT_PULLUP);
// Perform pedal calibration for each pedal
  calibrazionePedale("Throttle", ThrottlePin, ThrottleValoreMIN, ThrottleValoreMAX); // Calibrate the throttle pedal
  calibrazionePedale("Brake", BrakePin, BrakeValoreMIN, BrakeValoreMAX);             // Calibrate the brake pedal
  calibrazionePedale("Clutch", ClutchPin, ClutchValoreMIN, ClutchValoreMAX);         // Calibrate the clutch pedal
// Create the Joystick task to split the workload across tasks
  xTaskCreatePinnedToCore(
    TaskJoystick,    // Task function to execute
    "Joystick",      // Task name
    1024,            // Stack size (in words)
    NULL,            // Parameter passed to the task (not used here)
    1,               // Task priority
    NULL,            // Task handle (not needed in this case)
    0                // Core 0 (the task will be pinned to this core)
  );
// Second task
  xTaskCreatePinnedToCore(
    TaskSeriale,    // Task function to execute
    "Seriale",      // Task name
    1024,           // Stack size (in words)
    NULL,           // Parameter passed to the task (not used here)
    1,              // Task priority
    NULL,           // Task handle (not needed in this case)
    1               // Core 1 (the task will be pinned to this core)
  );
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

}
// Joystick task
void TaskJoystick(void *pvParameters) {
  while (true) {
    // Start Joystick code
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

    vTaskDelay(10 / portTICK_PERIOD_MS); // Pause to prevent CPU overload
  }
}

// Calibration section
void calibrazionePedale(const char* nomePedale, int pin, int &valoreMIN, int &valoreMAX) {
  Serial.println("inizio calibrazione");  // Start calibration message
  delay(2000);  // Wait for 2 seconds
  Serial.print("premere tutto il pedale");  // Instruction to press the pedal fully
  delay(2000);  // Wait for 2 seconds

  unsigned long startMillis = millis();  // Start timer for the first phase of calibration
  while (millis() - startMillis < 3000) {  // Wait for 3 seconds while reading pedal values
    int valore = analogRead(pin);  // Read the current value of the pedal
    Serial.print(nomePedale);  // Print the pedal name
    Serial.print(": ");
    Serial.println(valore);  // Print the pedal value
    delay(10);  // Small delay between readings
  }
  valoreMIN = Serial.readString().toInt();  // Read the minimum value from the serial input

  Serial.println("rilasciare il pedale");  // Instruction to release the pedal
  delay(2000);  // Wait for 2 seconds
  Serial.println("non toccare i pedali");  // Instruction to not touch the pedals

  unsigned long startMillis2 = millis();  // Start timer for the second phase of calibration
  while (millis() - startMillis2 < 3000) {  // Wait for 3 seconds while reading pedal values
    int valore = analogRead(pin);  // Read the current value of the pedal
    Serial.print(nomePedale);  // Print the pedal name
    Serial.print(": ");
    Serial.println(valore);  // Print the pedal value
    delay(10);  // Small delay between readings
  }
  valoreMAX = Serial.readString().toInt();  // Read the maximum value from the serial input

  Serial.println("Calibrazione finita");  // End calibration message
}

void TaskSeriale(void *pvParameters) {
  while (true) {
    // Check if data is available in the serial buffer
    if (Serial.available()) {
      String command = Serial.readString();  // Read the incoming command from the serial monitor

      // Check the command and perform corresponding calibration
      if (command == "Calibrazione_Throttle") {
        calibrazionePedale("Throttle", ThrottlePin, ThrottleValoreMIN, ThrottleValoreMAX); // Calibrate the throttle pedal
      } 
      else if (command == "Calibrazione_Brake") {
        calibrazionePedale("Brake", BrakePin, BrakeValoreMIN, BrakeValoreMAX); // Calibrate the brake pedal
      } 
      else if (command == "Calibrazione_Clutch") {
        calibrazionePedale("Clutch", ClutchPin, ClutchValoreMIN, ClutchValoreMAX); // Calibrate the clutch pedal
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay to avoid CPU overload
  }
}
