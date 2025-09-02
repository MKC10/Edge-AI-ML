// === Shape Labels (must match your training classes) ===
#define TENNIS     0
#define CYLINDER   1
#define RECTANGLE  2

// === Ultrasonic Sensor Pins ===
#define TRIG_PIN 4
#define ECHO_PIN 39

// === Rolling Statistics ===
const int WINDOW_SIZE = 5;
float distance_buffer[WINDOW_SIZE];
int buffer_index = 0;

// === Read distance in cm using HC-SR04 ===
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout after 30ms
  if (duration == 0) return 0;
  return duration * 0.034 / 2.0; // Convert to cm
}

// === Compute rolling mean of last WINDOW_SIZE readings ===
float getRollingMean() {
  float sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++) {
    sum += distance_buffer[i];
  }
  return sum / WINDOW_SIZE;
}

// === Decision Tree Prediction Function (Depth 3) ===
int predictShape(float reading, float rolling_mean) {
  // This is your trained decision logic (depth = 3)
  if (reading <= 4.1) {
    return TENNIS;
  } else {
    if (rolling_mean <= 4.9) {
      return CYLINDER;
    } else {
      return RECTANGLE;
    }
  }
}

// === Setup Arduino pins and serial ===
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Fill buffer with initial reading
  float first_reading = readDistanceCM();
  for (int i = 0; i < WINDOW_SIZE; i++) {
    distance_buffer[i] = first_reading;
  }

  Serial.println("📡 Real-Time Shape Detection Started...");
}

// === Main loop to read, update, predict and print ===
void loop() {
  float distance = readDistanceCM();

  // Update buffer
  distance_buffer[buffer_index] = distance;
  buffer_index = (buffer_index + 1) % WINDOW_SIZE;

  // Calculate rolling mean
  float rolling_mean = getRollingMean();

  // Predict shape using model
  int predicted_shape = predictShape(distance, rolling_mean);

  // Print the prediction
  Serial.print("Distance: ");
  Serial.print(distance, 2);
  Serial.print(" cm\tRolling Mean: ");
  Serial.print(rolling_mean, 2);
  Serial.print(" cm\tPredicted Shape: ");

  switch (predicted_shape) {
    case TENNIS:    Serial.println("🎾 TENNIS BALL"); break;
    case CYLINDER:  Serial.println("🧯 CYLINDER"); break;
    case RECTANGLE: Serial.println("📦 RECTANGLE"); break;
    default:        Serial.println("❓ UNKNOWN");
  }

  delay(300); // Adjust delay to your sampling speed
}
