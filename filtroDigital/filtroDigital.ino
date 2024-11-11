#include "esp_adc_cal.h"
#define tiempoMuestreo 2.8 // Tiempo en milisegundos

const int pinSensor = 34; // Pin ADC para la lectura analógica del sensor
esp_adc_cal_characteristics_t *adc_chars; 
const adc_unit_t unit = ADC_UNIT_1;       
const adc_bits_width_t width = ADC_WIDTH_BIT_12;

const float coefEntrada[5] = {0.000051993, 0.000208, 0.000312, 0.000208, 0.000051993};
const float coefSalida[5] = {1, 3.531, -4.7, 2.793, -0.6249};
double arrayEntrada[5] = {0, 0, 0, 0, 0};
double arraySalida[5] = {0, 0, 0, 0, 0};

unsigned long tiempoMedido = 0; // Tiempo de la última muestra

// Variables para el promedio
bool flag = true;
float acumuladorSalida = 0; // Acumulador para las salidas
int contador = 0;            // Contador de muestras
float filtradoADC = 0;
float peso = 0;
float muestra = 0;

// Función para leer los datos en la entrada y almacenarlos en arrayEntrada
void datosEntrada() {
  tiempoMedido = millis();
  arrayEntrada[0] = (esp_adc_cal_raw_to_voltage(analogRead(pinSensor), adc_chars))/1000.00;
}

// Función que aplica el filtro digital usando los coeficientes para calcular la salida filtrada
void filtro() {
  float sumatoria = coefEntrada[0] * arrayEntrada[0];
  for(int i = 1; i < 5; i++) {
    sumatoria += coefEntrada[i] * arrayEntrada[i] + coefSalida[i] * arraySalida[i];
  }
  arraySalida[0] = sumatoria;
}

// Función para calcular el peso y el valor filtrado
void calculoPeso(float salidaADC) {
  if(flag && millis() > 2000) {
    acumuladorSalida += salidaADC;
    contador++;
    if(contador >= 50 && millis() > 5000) {
      muestra = acumuladorSalida / contador;
      flag = false;
    }
  }

  filtradoADC = (salidaADC - muestra);

  if(filtradoADC <= 0){
    filtradoADC = 0;
  }

  peso = (filtradoADC * 36);

  Serial.print("Señal entrada: ");
  Serial.print(arrayEntrada[0]);
  Serial.print(" --- Señal salida: ");
  Serial.print(arraySalida[0]);
  Serial.print(" --- Muestra: ");
  Serial.print(muestra, 2);
  Serial.print(" --- salidaADC-Muestra: ");
  Serial.print(arraySalida[0] - muestra, 2);
  Serial.print(" --- FiltradoADC: ");
  Serial.print(filtradoADC, 2);
  Serial.print(" --- Peso: ");
  Serial.println(peso, 2);
}

// Función para desplazar los valores en los arrays de entrada y salida
void corrimientoArray() {
  for(int i = 4; i > 0; i--) {
    arraySalida[i] = arraySalida[i - 1];
    arrayEntrada[i] = arrayEntrada[i - 1];
  }
}

// Configuración inicial del programa
void setup() {
  Serial.begin(115200); // Configura la comunicación serial
  pinMode(pinSensor, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(pinSensor, ADC_0db);

  adc_chars = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, ADC_ATTEN_DB_0, width, 1100, adc_chars);

  tiempoMedido = millis();
}

// Bucle principal del programa
void loop() {
  // Verifica si ha pasado el tiempo de muestreo desde la última lectura
  if(millis() - tiempoMedido >= tiempoMuestreo) {
    // Llama a la función para tomar una nueva lectura de entrada
    datosEntrada();
    filtro();
    calculoPeso(arraySalida[0]);
    corrimientoArray();
  }
}
