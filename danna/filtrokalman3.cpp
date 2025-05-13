// ============================================================
//  SISTEMA DE DETECCIÓN Y ACTIVACIÓN DE BOOST CON FILTRO KALMAN
//  Para dos sensores HC-SR04 en Arduino UNO (16 MHz, 2 KB SRAM)
//  Distancias en cm, tiempos en ms
// ============================================================

#define TRIG1 11
#define ECHO1 12
#define TRIG2  3
#define ECHO2  4

// ————— Parámetros de distancia —————
const float MIN_DIST           = 2.0;   // cm, mínimo útil
const float MAX_DIST           = 20.0;  // cm, máximo técnico fiable
const float SAFETY_MARGIN      = 1.5;   // cm, margen de seguridad
const float DIFFUSE_ZONE_START = 18.6;  // cm, inicio de zona difusa
const float DIFFUSE_ZONE_END   = 22.0;  // cm, fin de zona difusa

// ————— Parámetros de tiempo —————
const unsigned long READ_INTERVAL   = 10;    // ms entre lecturas
const unsigned long LED_ON_DURATION = 5000;  // ms LED encendido
const unsigned long LED_OFF_DURATION= 1000;  // ms LED apagado

// ————— Estados del LED —————
enum LedState { LED_OFF, LED_ON, LED_WAIT_OFF };
LedState currentLedState = LED_OFF;

// ————— Variables de tiempo —————
unsigned long previousReadMillis = 0;
unsigned long ledStartMillis     = 0;

// ————— Historial para estabilidad —————
const int HISTORY_SIZE = 5;
float estimationHistory[HISTORY_SIZE] = {0};
int historyIndex = 0;

// ————— Variables Kalman 1D —————
float kalman_x  = 10.0;  // estado inicial (cm)
float kalman_p  = 1.0;   // incertidumbre inicial
const float kalman_q0 = 0.01;  // ruido de proceso fijo
float kalman_q  = kalman_q0;
float kalman_r1 = 0.5;   // ruido medición sensor 1
float kalman_r2 = 0.5;   // ruido medición sensor 2

void setup() {
  Serial.begin(9600);
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.println("SISTEMA INICIADO - RANGE SAFE MAX = 20cm, MARGIN = 1.5cm");
}

void loop() {
  unsigned long now = millis();

  // — Lectura y filtro cada READ_INTERVAL ms —
  if (now - previousReadMillis >= READ_INTERVAL) {
    previousReadMillis = now;
    float z1 = read_distance(TRIG1, ECHO1);
    float z2 = read_distance(TRIG2, ECHO2);
    float estimate = update_kalman(z1, z2);

    // Registrar histórico circular
    estimationHistory[historyIndex] = estimate;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;

    // Calcular variación histórica
    float variation = calculate_history_variation();

    // Debug
    Serial.print("z1: "); Serial.print(z1);
    Serial.print("  z2: "); Serial.print(z2);
    Serial.print("  K: ");  Serial.print(estimate);
    Serial.print("  P: ");  Serial.print(kalman_p);
    Serial.print("  Var: ");Serial.print(variation);

    // Condiciones de activación
    float safeMax = MAX_DIST - SAFETY_MARGIN; // 18.5 cm
    bool allValid = true;
    for (int i = 0; i < HISTORY_SIZE; i++) {
      if (estimationHistory[i] > safeMax) {
        allValid = false;
        break;
      }
    }
    bool stable = (variation < 0.2);
    bool lowUncert = (kalman_p < 0.3);

    if (estimate >= MIN_DIST && estimate <= safeMax
        && allValid && stable && lowUncert
        && currentLedState == LED_OFF) {
      // Lecturas individuales también seguras
      if ((z1>0 && z1<=safeMax) || (z2>0 && z2<=safeMax)) {
        Serial.println(" -> ACTIVANDO LED");
        digitalWrite(LED_BUILTIN, HIGH);
        currentLedState = LED_ON;
        ledStartMillis = now;
      }
    } else {
      // Mensajes de por qué no activa
      if (estimate > safeMax)
        Serial.println(" -> FUERA RANGO SAFE");
      else if (!allValid)
        Serial.println(" -> HISTORIAL NO VALIDO");
      else if (!stable)
        Serial.println(" -> LECTURAS INESTABLES");
      else if (!lowUncert)
        Serial.println(" -> INCERTIDUMBRE ALTA");
      else if (currentLedState != LED_OFF)
        Serial.println(" -> LED EN CICLO");
      else
        Serial.println(" -> FUERA RANGO MIN");
    }

    if (z1<0 && z2<0) {
      Serial.println(" -> SIN LECTURA VÁLIDA");
    }
  }

  // — Control ciclo LED (5s ON, 1s OFF) —
  if (currentLedState == LED_ON && now - ledStartMillis >= LED_ON_DURATION) {
    digitalWrite(LED_BUILTIN, LOW);
    currentLedState = LED_WAIT_OFF;
    ledStartMillis = now;
    Serial.println("LED APAGADO - ESPERANDO");
  }
  else if (currentLedState == LED_WAIT_OFF && now - ledStartMillis >= LED_OFF_DURATION) {
    currentLedState = LED_OFF;
    Serial.println("LISTO PARA NUEVA ACTIVACIÓN");
  }
}

// -------------------- Funciones Auxiliares --------------------

float read_distance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration <= 0 || duration >= 25000) return -1;

  float d = duration * 0.0343 / 2.0;  // cm

  // Filtrar zona difusa
  if (d > DIFFUSE_ZONE_START && d < DIFFUSE_ZONE_END) {
    Serial.print("¡ZONA DIFUSA!: "); Serial.print(d); Serial.println(" cm");
    return MAX_DIST + 1;  // fuera de rango
  }

  return constrain(d, MIN_DIST, MAX_DIST);
}

float update_kalman(float z1, float z2) {
  // Predicción
  kalman_p += kalman_q;

  // Actualiza con z1
  if (z1 > 0) {
    float k = kalman_p / (kalman_p + kalman_r1);
    kalman_x += k * (z1 - kalman_x);
    kalman_p *= (1 - k);
  }
  // Actualiza con z2
  if (z2 > 0) {
    float k = kalman_p / (kalman_p + kalman_r2);
    kalman_x += k * (z2 - kalman_x);
    kalman_p *= (1 - k);
  }

  // Ajuste dinámico de ruido de medición
  if (z1>0 && z2>0) {
    float diff = abs(z1 - z2);
    if (diff > 1.0) {
      kalman_r1 = constrain(kalman_r1 * 1.05, 0.1, 2.0);
      kalman_r2 = constrain(kalman_r2 * 1.05, 0.1, 2.0);
    } else {
      kalman_r1 = constrain(kalman_r1 * 0.95, 0.1, 2.0);
      kalman_r2 = constrain(kalman_r2 * 0.95, 0.1, 2.0);
    }
  }

  // Constrain estado
  kalman_x = constrain(kalman_x, MIN_DIST, MAX_DIST);
  // Reset q steady
  kalman_q = kalman_q0;
  return kalman_x;
}

float calculate_history_variation() {
  float maxv = estimationHistory[0], minv = estimationHistory[0];
  for (int i = 1; i < HISTORY_SIZE; i++) {
    if (estimationHistory[i] <= 0) continue;
    maxv = max(maxv, estimationHistory[i]);
    minv = min(minv, estimationHistory[i]);
  }
  return (maxv - minv);
}
