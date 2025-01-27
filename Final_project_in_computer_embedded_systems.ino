//Final project in computer embedded systems 
//
//Object Tracker with Ultrasonic Sensor 
//
//Madar Eliaou 1954753 
//Baryohay Uzan 916781 

#include <Servo.h>

// Define pin connections
#define TRIG_PIN 9      // Pin for ultrasonic sensor trigger
#define ECHO_PIN 8      // Pin for ultrasonic sensor echo
#define BUTTON_PIN 7    // Pin for button input
#define SERVO_PIN 6     // Pin for servo motor

Servo servo;  // Servo object to control the motor
float targetDistance = 8.0; // Target distance in cm
bool isTargetSet = false;   // Flag to check if the target distance is set
int servoPosition = 90;     // Initial position of the servo (center)
const float tolerance = 15.0; // Tolerance range for distance adjustment (in cm)
const float maxDistanceChange = 15.0; // Threshold to detect lateral movement
const int scanStep = 5; // Degree increment for scanning
String lastDirection = ""; // Last detected direction ("left" or "right")

void setup() {
  pinMode(TRIG_PIN, OUTPUT);   // Set TRIG_PIN as output
  pinMode(ECHO_PIN, INPUT);    // Set ECHO_PIN as input
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set BUTTON_PIN as input with pull-up resistor

  servo.attach(SERVO_PIN);    // Attach servo motor to SERVO_PIN
  servo.write(servoPosition); // Set servo to initial center position

  Serial.begin(9600); // Start serial communication
}

void loop() {
  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {
    targetDistance = getDistance(); // Measure and set target distance
    isTargetSet = true;             // Mark target as set
    lastDirection = "";             // Reset the last direction
    Serial.print("Target distance set: ");
    Serial.print(targetDistance);
    Serial.println(" cm");
    delay(500); // Debounce delay
  }

  if (isTargetSet) {
    float currentDistance = getDistance(); // Measure the current distance

    // Check if the object moved laterally (sudden change in distance)
    if (abs(currentDistance - targetDistance) > maxDistanceChange) {
      Serial.println("Object moved laterally, scanning...");
      scanForObject(); // Initiate scanning to locate the object
      return;
    }

    // Adjust the servo motor if the distance is slightly different
    if (currentDistance > targetDistance + tolerance) {
      // Object is farther away, move servo to the left
      servoPosition = min(servoPosition + 1, 180);
      servo.write(servoPosition);
      lastDirection = "left"; // Remember the direction
    } else if (currentDistance < targetDistance - tolerance) {
      // Object is closer, move servo to the right
      servoPosition = max(servoPosition - 1, 0);
      servo.write(servoPosition);
      lastDirection = "right"; // Remember the direction
    }

    // Print current distance to the serial monitor
    Serial.print("Current distance: ");
    Serial.print(currentDistance);
    Serial.println(" cm");
    delay(20); // Delay to smooth servo movements
  }
}

// Function to measure distance using the ultrasonic sensor
float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distance = duration * 0.034 / 2; // Convert duration to distance in cm
  return distance;
}

// Function to scan and relocate the object
void scanForObject() {
  int scanRange = 45; // Range to scan in degrees
  int originalPosition = servoPosition;

  // Priority to last detected direction
  if (lastDirection == "right") {
    if (scanToDirection(originalPosition, 1)) return;
    if (scanToDirection(originalPosition, -1)) return;
  } else if (lastDirection == "left") {
    if (scanToDirection(originalPosition, -1)) return;
    if (scanToDirection(originalPosition, 1)) return;
  } else {
    // Default scanning in both directions
    if (scanToDirection(originalPosition, -1)) return;
    if (scanToDirection(originalPosition, 1)) return;
  }

  // Return to the original position if the object is not found
  servo.write(originalPosition);
  Serial.println("Object not found after scanning.");
}

// Function to scan in a specific direction
bool scanToDirection(int startPos, int direction) {
  for (int pos = startPos; 
       (direction > 0 ? pos <= startPos + 50 : pos >= startPos - 50); 
       pos += direction * scanStep) {
    if (pos < 0 || pos > 180) break; // Ensure servo position is within bounds
    servo.write(pos);
    delay(80);
    float distance = getDistance();
    if (abs(distance - targetDistance) <= tolerance) {
      servoPosition = pos; // Update the current position
      lastDirection = (direction > 0) ? "right" : "left"; // Record the direction
      Serial.print("Object found in ");
      Serial.println(lastDirection);
      return true;
    }
  }
  return false;
}
