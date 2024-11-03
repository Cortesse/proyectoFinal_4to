#define tiempoMuestreo 5 // El tiempo de muestreo se obtiene como: 1/f_muestreo

const int pinSensor = 34; // pin ADC por el cual se realiza la lectura en el valor analogico del sensor

// Coeficientes de entrada y salida del filtro
const float coefEntrada[5] = {0.0004175, 0.00167, 0.002505, 0.00167, 0.0004175};
const float coefSalida[5] = {1, 3.18, -3.86, 2.111, -0.4381};

// Coeficientes de entrada y salida del filtro inicializados en cero
float arrayEntrada[5] = {0, 0, 0, 0, 0};
float arraySalida[5] = {0, 0, 0, 0, 0};

unsigned long tiempoMedido = 0; // Tiempo de la ultima muestra

// Funcion para leer los datos en la entrada  y almacenarlos en el arrayEntrada
void datosEntrada(){
  tiempoMedido = millis();
  arrayEntrada[0] = float(analogRead(pinSensor));
}

// Función que aplica el filtro digital usando los coeficientes para calcular la salida filtrada
 void filtro(){
  float sumatoria = 0;
  // Calcula la primera parte de la sumatoria del filtro, usando el coeficiente y valor de entrada más reciente
  sumatoria = coefEntrada[0] * arrayEntrada[0];
  // Ciclo para sumar el resto de términos con los coeficientes de entrada y salida
  for(int i = 1; i < 5; i++){
    sumatoria += coefEntrada[i] * arrayEntrada[i] + coefSalida[i] * arraySalida[i];
  }
  // Guarda el resultado de la sumatoria en la primera posición de arraySalida, que es la salida filtrada más reciente
  arraySalida[0] = sumatoria;
}

// Función para desplazar los valores en los arrays de entrada y salida
void corrimientoArray(){
  // Desplaza cada valor a la siguiente posición, para mantener los últimos valores de las entradas y salidas
  for(int i = 4; i > 0; i--){
    arraySalida[i] = arraySalida[i-1];
    arrayEntrada[i] = arrayEntrada[i-1];
  }
}

// Función para inicializar el pin del sensor como entrada
void pines(){
  pinMode(pinSensor, INPUT);
}

// Configuración inicial del programa
void setup() {
  // Llama a la función para configurar los pines
  pines();
  // Inicializa el tiempo de la última lectura
  tiempoMedido = millis();
  // Configura la comunicación serial a 115200 baudios para enviar datos al monitor
  Serial.begin(115200);
}

// Bucle principal del programa
void loop() {
  // Verifica si ha pasado el tiempo de muestreo desde la última lectura
  if(millis() - tiempoMedido >= tiempoMuestreo){
    // Llama a la función para tomar una nueva lectura de entrada
    datosEntrada();
    // Aplica el filtro digital a los valores de entrada y salida
    filtro();
    Serial.print(arrayEntrada[0]);
    Serial.print(" ");
    Serial.println(arraySalida[0]);
    // Llama a la función para desplazar los valores en los arrays
    corrimientoArray();
  }
}