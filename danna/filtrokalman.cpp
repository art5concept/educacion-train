#define TRIG1 11
#define ECHO1 12
#define TRIG2 3
#define ECHO2 4

// Constantes para detección
const float MIN_DIST = 2.0;          // Distancia mínima de activación
const float MAX_DIST = 15.0;         // Distancia máxima de activación - CRÍTICA
const float SAFETY_MARGIN = 0.6;     // Margen de seguridad adicional (14.4 cm es el nuevo límite real)
const float DIFFUSE_ZONE_START = 14.6; // Inicio de zona difusa
const float DIFFUSE_ZONE_END = 17.0;   // Fin de zona difusa

// Variables para control de tiempo
unsigned long previousReadMillis = 0;
unsigned long ledStartMillis = 0;
const unsigned long READ_INTERVAL = 10;      // Lectura cada 10ms
const unsigned long LED_ON_DURATION = 5000;  // 5 segundos encendido
const unsigned long LED_OFF_DURATION = 1000; // 1 segundo apagado

// Estados del LED
enum LedState {LED_OFF, LED_ON, LED_WAIT_OFF};
LedState currentLedState = LED_OFF;

// Histórico de estimaciones para verificación adicional
const int HISTORY_SIZE = 5;
float estimationHistory[HISTORY_SIZE] = {0};
int historyIndex = 0;

// Parámetros del filtro de Kalman (simplificado para 1D)
float kalman_x = 10.0;       // Estimación inicial (cm)
float kalman_p = 1.0;        // Incertidumbre inicial
float kalman_q = 0.01;       // Ruido del proceso (ajustar según velocidad de cambio esperada)
float kalman_r1 = 0.5;       // Ruido de medición sensor 1 (ajustar según precisión del sensor)
float kalman_r2 = 0.5;       // Ruido de medición sensor 2

void setup() {
  Serial.begin(9600);
  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Mensaje inicial crítico para depuración
  Serial.println("SISTEMA INICIADO CON FILTRO KALMAN - ZONA CRÍTICA: NO ACTIVAR LED A DISTANCIAS > 14.4 cm");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Controlar sensores y actualizar estimación Kalman
  if (currentMillis - previousReadMillis >= READ_INTERVAL) {
    previousReadMillis = currentMillis;
    
    // Leer distancias
    float z1 = read_distance(TRIG1, ECHO1);
    float z2 = read_distance(TRIG2, ECHO2);
    
    // Actualizar filtro de Kalman y obtener estimación
    float kalman_estimate = update_kalman(z1, z2);
    
    // Añadir a histórico
    estimationHistory[historyIndex] = kalman_estimate;
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    
    // Calcular variación en histórico para evaluar estabilidad
    float variation = calculate_history_variation();
    
    // Imprimir valores para depuración
    Serial.print("z1: "); Serial.print(z1);
    Serial.print(", z2: "); Serial.print(z2);
    Serial.print(", Kalman: "); Serial.print(kalman_estimate);
    Serial.print(", P: "); Serial.print(kalman_p);
    Serial.print(", Var: "); Serial.print(variation);
    
    // CRÍTICO: Verificar si las estimaciones son seguras
    float safeMaxDistance = MAX_DIST - SAFETY_MARGIN; // 14.4 cm
    bool allHistoryValid = true;
    bool stableReading = variation < 0.2; // Variación baja indica estimación estable
    
    // Verificar histórico completo
    for (int i = 0; i < HISTORY_SIZE; i++) {
      if (estimationHistory[i] > safeMaxDistance) {
        allHistoryValid = false;
        break;
      }
    }
    
    // Verificar condiciones para activar LED
    if (kalman_estimate >= MIN_DIST && 
        kalman_estimate <= safeMaxDistance && 
        allHistoryValid &&
        stableReading &&
        kalman_p < 0.3 &&  // Baja incertidumbre en la estimación
        currentLedState == LED_OFF) {
      
      // Verificación adicional crítica: ambos sensores deben estar en rango seguro
      if ((z1 <= safeMaxDistance || z1 < 0) && (z2 <= safeMaxDistance || z2 < 0) && 
          (z1 > 0 || z2 > 0)) {  // Al menos un sensor válido
        Serial.println(" - ACTIVANDO LED [SEGURO]");
        digitalWrite(LED_BUILTIN, HIGH);
        currentLedState = LED_ON;
        ledStartMillis = currentMillis;
      } else {
        Serial.println(" - NO ACTIVANDO: lecturas de sensores inconsistentes");
      }
    } else {
      if (kalman_estimate > safeMaxDistance) {
        Serial.println(" - NO ACTIVANDO: FUERA DE RANGO SEGURO [CRÍTICO]");
      } else if (!allHistoryValid) {
        Serial.println(" - NO ACTIVANDO: histórico reciente fuera de rango seguro");
      } else if (!stableReading) {
        Serial.println(" - NO ACTIVANDO: lecturas inestables");
      } else if (kalman_p >= 0.3) {
        Serial.println(" - NO ACTIVANDO: alta incertidumbre en estimación");
      } else if (currentLedState != LED_OFF) {
        Serial.println(" - LED ya está en ciclo de activación");
      } else {
        Serial.println(" - NO ACTIVANDO: fuera de rango mínimo");
      }
    }
    
    if (z1 < 0 && z2 < 0) {
      Serial.println("Sin lectura confiable de ambos sensores.");
    }
  }
  
  // Control del LED para replicar comportamiento original (5s ON, 1s OFF)
  if (currentLedState == LED_ON && currentMillis - ledStartMillis >= LED_ON_DURATION) {
    digitalWrite(LED_BUILTIN, LOW);
    currentLedState = LED_WAIT_OFF;
    ledStartMillis = currentMillis;
    Serial.println("LED apagado - entrando en tiempo de espera");
  } 
  else if (currentLedState == LED_WAIT_OFF && currentMillis - ledStartMillis >= LED_OFF_DURATION) {
    currentLedState = LED_OFF;
    Serial.println("Ciclo LED completado - listo para nueva activación");
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
  
  // CRÍTICO: Filtrar explícitamente valores que están en la zona difusa (14.6-17)
  if (distance > DIFFUSE_ZONE_START && distance < DIFFUSE_ZONE_END) {
    Serial.print("¡ALERTA! Lectura en zona difusa: ");
    Serial.print(distance);
    Serial.println(" cm - Tratando como fuera de rango.");
    return MAX_DIST + 1; // Forzar fuera de rango
  }
  
  return constrain(distance, MIN_DIST, MAX_DIST);
}

// Implementación simplificada del filtro de Kalman unidimensional
float update_kalman(float z1, float z2) {
  // 1. Predicción (sin cambio en x porque asumimos objeto estacionario)
  // x = x (no cambia)
  kalman_p = kalman_p + kalman_q;
  
  // 2. Actualización con la primera medición (si es válida)
  if (z1 > 0) {
    float k = kalman_p / (kalman_p + kalman_r1);
    kalman_x = kalman_x + k * (z1 - kalman_x);
    kalman_p = (1 - k) * kalman_p;
  }
  
  // 3. Actualización con la segunda medición (si es válida)
  if (z2 > 0) {
    float k = kalman_p / (kalman_p + kalman_r2);
    kalman_x = kalman_x + k * (z2 - kalman_x);
    kalman_p = (1 - k) * kalman_p;
  }
  
  // Ajustar ruido de medición dinámicamente
  if (z1 > 0 && z2 > 0) {
    float diff = abs(z1 - z2);
    if (diff > 1.0) {
      // Si los sensores difieren mucho, aumentar el ruido
      kalman_r1 = constrain(kalman_r1 * 1.05, 0.1, 2.0);
      kalman_r2 = constrain(kalman_r2 * 1.05, 0.1, 2.0);
    } else {
      // Si son consistentes, reducir el ruido
      kalman_r1 = constrain(kalman_r1 * 0.95, 0.1, 2.0);
      kalman_r2 = constrain(kalman_r2 * 0.95, 0.1, 2.0);
    }
  }
  
  // Aplicar restricciones a la estimación
  kalman_x = constrain(kalman_x, MIN_DIST, MAX_DIST);
  
  return kalman_x;
}

float calculate_history_variation() {
  if (estimationHistory[0] == 0) return 0; // Histórico no inicializado
  
  float max_val = estimationHistory[0];
  float min_val = estimationHistory[0];
  
  for (int i = 1; i < HISTORY_SIZE; i++) {
    if (estimationHistory[i] == 0) continue; // Ignorar valores no inicializados
    
    if (estimationHistory[i] > max_val) max_val = estimationHistory[i];
    if (estimationHistory[i] < min_val) min_val = estimationHistory[i];
  }
  
  return max_val - min_val; // Variación máxima en el histórico
}