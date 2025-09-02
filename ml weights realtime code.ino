#define TRIG_PIN 4
#define ECHO_PIN 39
#define NUM_READINGS 20

float distances[NUM_READINGS];
int readingIndex = 0;

// Neural network weights and biases (from 63.6% model)
float W1[2][10] = {
  {-0.3785, -0.0921,  0.3809,  0.1272, -0.0811,  0.0415,  0.2452, -0.0207,  0.0255,  0.1220},
  {-0.2486, -0.1244,  0.3796,  0.0418, -0.1192, -0.0464,  0.1158, -0.2141,  0.1522, -0.1124}
};

float b1[10] = {0.0592, -0.0024, -0.0735, 0.0422, 0.0055, -0.0114, 0.0201, -0.0305, -0.0081, 0.0129};

float W2[10][3] = {
  {-0.3093, 0.3191, 0.1747},
  {0.0139, -0.0393, -0.1078},
  {-0.1271, 0.1964, -0.0980},
  {-0.2615, 0.0303, -0.0440},
  {-0.0472, 0.0254, -0.0626},
  {0.0875, -0.0162, -0.0629},
  {-0.0729, 0.0297, 0.0091},
  {0.0192, 0.0266, 0.0306},
  {0.0088, -0.0505, 0.0615},
  {0.0493, -0.0216, 0.0131}
};

float b2[3] = {0.0098, -0.0069, -0.0029};

// Activation functions
float sigmoid(float x) {
  return 1.0 / (1.0 + exp(-x));
}

float softmax(float* x, int size, int& maxIdx) {
  float maxVal = x[0];
  for (int i = 1; i < size; i++) if (x[i] > maxVal) maxVal = x[i];

  float sum = 0.0;
  float expVals[3];
  for (int i = 0; i < size; i++) {
    expVals[i] = exp(x[i] - maxVal);
    sum += expVals[i];
  }

  float maxProb = 0.0;
  for (int i = 0; i < size; i++) {
    x[i] = expVals[i] / sum;
    if (x[i] > maxProb) {
      maxProb = x[i];
      maxIdx = i;
    }
  }
  return maxProb;
}

float readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  float duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  float distance = duration * 0.0343 / 2;
  return distance;
}

float computeMean(float* arr, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) sum += arr[i];
  return sum / size;
}

float computeMedian(float* arr, int size) {
  float temp[size];
  memcpy(temp, arr, sizeof(float) * size);
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (temp[j] > temp[j + 1]) {
        float t = temp[j];
        temp[j] = temp[j + 1];
        temp[j + 1] = t;
      }
    }
  }
  return (size % 2 == 0) ? (temp[size/2 - 1] + temp[size/2]) / 2 : temp[size/2];
}

void predictShape(float mean, float median) {
  float input[2] = {mean, median};

  float hidden[10];
  for (int i = 0; i < 10; i++) {
    hidden[i] = b1[i];
    for (int j = 0; j < 2; j++) hidden[i] += input[j] * W1[j][i];
    hidden[i] = sigmoid(hidden[i]);
  }

  float output[3];
  for (int i = 0; i < 3; i++) {
    output[i] = b2[i];
    for (int j = 0; j < 10; j++) output[i] += hidden[j] * W2[j][i];
  }

  int predictedClass = 0;
  softmax(output, 3, predictedClass);

  Serial.print("Prediction: ");
  if (predictedClass == 0) Serial.println("Circle");
  else if (predictedClass == 1) Serial.println("Cylinder");
  else if (predictedClass == 2) Serial.println("Rectangle");
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println("Started...");
}

void loop() {
  float distance = readUltrasonic();
  if (distance > 1 && distance <= 10) {
    distances[readingIndex++] = distance;
  }

  if (readingIndex >= NUM_READINGS) {
    float mean = computeMean(distances, NUM_READINGS);
    float median = computeMedian(distances, NUM_READINGS);

    Serial.print("Mean: "); Serial.print(mean);
    Serial.print(", Median: "); Serial.println(median);

    predictShape(mean, median);

    readingIndex = 0;
    delay(1000); // delay after prediction
  }

  delay(50);
}
