#define TRIG1 11
#define ECHO1 12
#define TRIG2 3
#define ECHO2 4

// Constantes para detección
const float MIN_DIST = 2.0;
const float MAX_DIST = 15.0;
const float STEP = 0.1;
const int NUM_BINS = (MAX_DIST - MIN_DIST) / STEP + 1;

// Definir PI si no está disponible
#ifndef PI
#define PI 3.14159265358979323846
#endif

// Variables para filtro bayesiano
float belief[NUM_BINS];
float sigma1 = 0.4;
float sigma2 = 0.4;

// Variables para control de tiempo
unsigned long previousReadMillis = 0;
unsigned long ledStartMillis = 0;
const unsigned long READ_INTERVAL = 10;  // Lectura cada 50ms en lugar de 3ms
const unsigned long LED_ON_DURATION = 5000;  // 5 segundos encendido
const unsigned long LED_OFF_DURATION = 1000; // 1 segundo apagado

// Estados del LED
enum LedState {LED_OFF, LED_ON, LED_WAIT_OFF};
LedState currentLedState = LED_OFF;

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
  
  // Controlar sensores y actualizar creencia
  if (currentMillis - previousReadMillis >= READ_INTERVAL) {
    previousReadMillis = currentMillis;
    
    // Leer distancias
    float z1 = read_distance(TRIG1, ECHO1);
    float z2 = read_distance(TRIG2, ECHO2);
    
    // Ajustar ruido y actualizar creencia
    adaptive_noise(z1, z2);
    update_belief(z1, z2);
    
    // Calcular valor esperado
    float estimate = expected_value();
    
    // Imprimir valores para depuración
    Serial.print("z1: "); Serial.print(z1);
    Serial.print(", z2: "); Serial.print(z2);
    Serial.print(", estimate: "); Serial.println(estimate);
    
    // Iniciar secuencia LED si se detecta objeto
    if (estimate >= MIN_DIST && estimate < MAX_DIST && currentLedState == LED_OFF) {
      digitalWrite(LED_BUILTIN, HIGH);
      currentLedState = LED_ON;
      ledStartMillis = currentMillis;
    }
    if (z1 < 0 || z2 < 0) {
    	Serial.println("Sin lectura confiable de al menos un sensor.");
  	}
  }
  
  // Control del LED para replicar comportamiento original (5s ON, 1s OFF)
  if (currentLedState == LED_ON && currentMillis - ledStartMillis >= LED_ON_DURATION) {
    digitalWrite(LED_BUILTIN, LOW);
    currentLedState = LED_WAIT_OFF;
    ledStartMillis = currentMillis;
  } 
  else if (currentLedState == LED_WAIT_OFF && currentMillis - ledStartMillis >= LED_OFF_DURATION) {
    currentLedState = LED_OFF;
  }
}

float read_distance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  float duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0 || duration >= 25000) {
    return -1;  // Sin eco, ignora esta lectura
  }

  
  float distance = duration * 0.0343 / 2.0;
  return constrain(distance, MIN_DIST, MAX_DIST);
}

void init_belief() {
  // Inicializar con distribución uniforme
  for (int i = 0; i < NUM_BINS; i++) {
    belief[i] = 1.0 / NUM_BINS;
  }
}

float gaussian(float x, float mu, float sigma) {
  float exponent = -pow(x - mu, 2) / (2 * pow(sigma, 2));
  return (1.0 / (sqrt(2 * PI) * sigma)) * exp(exponent);
}

void update_belief(float z1, float z2) {
  if (z1 < 0 || z2 < 0) return;
  
  float total = 0;
  
  // Aplicar Bayes: creencia posterior ∝ verosimilitud * creencia previa
  for (int i = 0; i < NUM_BINS; i++) {
    float x = MIN_DIST + i * STEP;
    float p1 = gaussian(z1, x, sigma1);
    float p2 = gaussian(z2, x, sigma2);
    belief[i] *= p1 * p2;
    total += belief[i];
  }
  
  // Normalizar para asegurar que sumen 1
  if (total > 0) {
    for (int i = 0; i < NUM_BINS; i++) {
      belief[i] /= total;
    }
  } else {
    // Si todas las probabilidades son muy bajas, reiniciar
    init_belief();
  }
}

float expected_value() {
  float sum = 0;
  float weight_sum = 0;
  
  for (int i = 0; i < NUM_BINS; i++) {
    float x = MIN_DIST + i * STEP;
    sum += x * belief[i];
    weight_sum += belief[i];
  }
  
  // Protección contra división por cero
  if (weight_sum > 0) {
    return sum / weight_sum;
  } else {
    return (MIN_DIST + MAX_DIST) / 2.0;  // Valor medio como fallback
  }
}

void adaptive_noise(float z1, float z2) {
  // Incrementar sigma cuando las lecturas son inconsistentes
  if ((z1 == MIN_DIST && z2 == MAX_DIST) || (z2 == MIN_DIST && z1 == MAX_DIST)) {
    sigma1 = constrain(sigma1 * 1.1, 0.2, 1.0);
    sigma2 = constrain(sigma2 * 1.1, 0.2, 1.0);
  } else {
    // Reducir sigma cuando las lecturas son consistentes
    sigma1 = constrain(sigma1 * 0.9, 0.2, 1.0);
    sigma2 = constrain(sigma2 * 0.9, 0.2, 1.0);
  }
}