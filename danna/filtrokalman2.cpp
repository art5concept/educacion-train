#define TRIG1 11
#define ECHO1 12
#define TRIG2 3
#define ECHO2 4

const float MIN_DIST = 2.0;
const float MAX_DIST = 15.0;
const float STEP = 0.1;
const int NUM_BINS = (MAX_DIST - MIN_DIST) / STEP + 1;

float belief[NUM_BINS];
float sigma1 = 0.4;
float sigma2 = 0.4;

unsigned long previousMillis = 0;
bool ledState = false;
unsigned long ledActivatedAt = 0;
const unsigned long ledDuration = 5000;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  init_belief();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= 3) {
    previousMillis = currentMillis;

    float z1 = read_distance(TRIG1, ECHO1);
    float z2 = read_distance(TRIG2, ECHO2);

    adaptive_noise(z1, z2);
    update_belief(z1, z2);
    float estimate = expected_value();

    Serial.print("z1: "); Serial.print(z1);
    Serial.print(", z2: "); Serial.print(z2);
    Serial.print(", estimate: "); Serial.println(estimate);

    if (estimate >= 2.0 && estimate <= 15.0) {
      if (!ledState) {
        digitalWrite(LED_BUILTIN, HIGH);
        ledState = true;
        ledActivatedAt = currentMillis;
      }
    }

    if (ledState && (currentMillis - ledActivatedAt >= ledDuration)) {
      digitalWrite(LED_BUILTIN, LOW);
      ledState = false;
    }
  }
}

float read_distance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  float duration = pulseIn(echoPin, HIGH, 25000);
  float distance = duration * 0.0343 / 2.0;
  return constrain(distance, MIN_DIST, MAX_DIST);
}

void init_belief() {
  for (int i = 0; i < NUM_BINS; i++) {
    belief[i] = 1.0 / NUM_BINS;
  }
}

float gaussian(float x, float mu, float sigma) {
  float exponent = - (pow(x - mu, 2)) / (2 * pow(sigma, 2));
  return (1.0 / (sqrt(2 * PI) * sigma)) * exp(exponent);
}

void update_belief(float z1, float z2) {
  float total = 0;
  for (int i = 0; i < NUM_BINS; i++) {
    float x = MIN_DIST + i * STEP;
    float p1 = gaussian(z1, x, sigma1);
    float p2 = gaussian(z2, x, sigma2);
    belief[i] *= p1 * p2;
    total += belief[i];
  }
  for (int i = 0; i < NUM_BINS; i++) {
    belief[i] /= total;
  }
}

float expected_value() {
  float sum = 0;
  for (int i = 0; i < NUM_BINS; i++) {
    float x = MIN_DIST + i * STEP;
    sum += x * belief[i];
  }
  return sum;
}

void adaptive_noise(float z1, float z2) {
  if ((z1 == MIN_DIST && z2 == MAX_DIST) || (z2 == MIN_DIST && z1 == MAX_DIST)) {
    sigma1 = constrain(sigma1 * 1.1, 0.2, 1.0);
    sigma2 = constrain(sigma2 * 1.1, 0.2, 1.0);
  } else {
    sigma1 = constrain(sigma1 * 0.9, 0.2, 1.0);
    sigma2 = constrain(sigma2 * 0.9, 0.2, 1.0);
  }
}
