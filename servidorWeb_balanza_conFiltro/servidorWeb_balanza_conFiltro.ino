#include <WiFi.h>
#include <WiFiMulti.h>
#include <redesWiFi.h>
#include <esp_adc_cal.h>

#define tiempoMuestreo 2.8 

/* ----- VARIABLES SERVIDOR WEB ----- */

WiFiServer servidor(80);         // Crea el servidor web en el puerto 80
WiFiMulti wifiMulti;

const int pinSensor = 34;
const uint32_t tiempoEsperaWiFi = 5000;

unsigned long tiempoActual = 0;
unsigned long tiempoAnterior = 0;
const unsigned long tiempoCancelacion = 1000;  // Tiempo límite para mantener la conexión con el cliente

/* ----- VARIABLES FILTRO ----- */

esp_adc_cal_characteristics_t *adc_chars; 
const adc_unit_t unit = ADC_UNIT_1;       
const adc_bits_width_t width = ADC_WIDTH_BIT_12;

const float coefEntrada[5] = {0.000051993, 0.000208, 0.000312, 0.000208, 0.000051993};
const float coefSalida[5] = {1, 3.531, -4.7, 2.793, -0.6249};

double arrayEntrada[5] = {0, 0, 0, 0, 0};
double arraySalida[5] = {0, 0, 0, 0, 0};

unsigned long tiempoMedido = 0;
bool flag = true;
float acumuladorSalida = 0; 
int contador = 0;      
float filtradoADC = 0;
float peso = 0;
float muestra = 0;

/* Configuración de IP estática */
IPAddress ip(192, 168, 213, 115);      // IP deseada
IPAddress gateway(192, 168, 213, 27);   // Puerta de enlace de la red
IPAddress subnet(255, 255, 255, 0);  // Máscara de subred

/* ----- FUNCIONES FILTRO ----- */

void datosEntrada() {
  tiempoMedido = millis();
  arrayEntrada[0] = esp_adc_cal_raw_to_voltage(float(analogRead(pinSensor)), adc_chars)/1000.00;
}

void filtro() {
  float sumatoria = 0;
  sumatoria = coefEntrada[0] * arrayEntrada[0];
  for(int i = 1; i < 5; i++) {
    sumatoria += coefEntrada[i] * arrayEntrada[i] + coefSalida[i] * arraySalida[i];
  }
  arraySalida[0] = sumatoria;
}

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

  if(filtradoADC <= 0) {
    filtradoADC = 0;
  }

  peso = (filtradoADC * 36);
}

void corrimientoArray() {
  for(int i = 4; i > 0; i--) {
    arraySalida[i] = arraySalida[i-1];
    arrayEntrada[i] = arrayEntrada[i-1];
  }
}

/* ----- FUNCIONES SERVIDOR WEB ----- */

void responderCliente(WiFiClient cliente, float peso) {
  cliente.println("HTTP/1.1 200 OK");
  cliente.println("Content-type:text/html");
  cliente.println("Connection: close");
  cliente.println();
  
  cliente.println("<!DOCTYPE html>");
  cliente.println("<html lang='es'>");
  cliente.println("<head>");
  cliente.println("<meta charset='UTF-8'>");
  cliente.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  cliente.println("<title>Balanza Digital</title>");
  cliente.println("<style>");
  cliente.println("body { display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f2f2f2; font-family: Arial, sans-serif; }");
  cliente.println("h1 { font-size: 2em; color: #333; margin-bottom: 20px; text-align: center; }");
  cliente.println(".balanza-container { position: relative; width: 300px; height: 320px; background-color: #333; border-radius: 10px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.2); text-align: center; }");
  cliente.println(".medidor { position: absolute; top: 20px; left: 20px; right: 20px; height: 90px; background-color: #87CEFA; border-radius: 8px; display: flex; justify-content: center; align-items: center; }");
  cliente.println(".aguja { position: absolute; width: 8px; height: 70px; background-color: #000; top: 25px; left: 50%; transform-origin: bottom; transform: rotate(0deg); transition: transform 0.5s ease-out; }");
  cliente.println(".valor-medicion { position: absolute; bottom: 20px; left: 10%; right: 10%; color: white; font-size: 36px; font-weight: bold; background-color: #4a4a4a; padding: 10px; border-radius: 8px; box-sizing: border-box; }");
  cliente.println("</style>");
  cliente.println("</head>");
  cliente.println("<body>");
  cliente.println("<h1>Balanza Digital</h1>");
  cliente.println("<div class='balanza-container'>");
  cliente.println("<div class='medidor'>");
  cliente.println("<div class='aguja' id='aguja'></div>");
  cliente.println("</div>");
  cliente.println("<div class='valor-medicion' id='peso'>"); 
  cliente.print(peso);
  cliente.println(" kg</div>");
  cliente.println("</div>");
  cliente.println("<script>");
  cliente.println("function actualizarPeso() {");
  cliente.println("  fetch('/peso').then(response => response.text()).then(data => {");
  cliente.println("    document.getElementById('peso').innerText = data + ' kg';");
  cliente.println("  });");
  cliente.println("}");
  cliente.println("setInterval(actualizarPeso, 500);");
  cliente.println("</script>");
  cliente.println("</body>");
  cliente.println("</html>");
  cliente.println();
  cliente.stop();
}

/* --------------- SETUP --------------- */

void setup() {
  Serial.begin(115200);
  pinMode(pinSensor, INPUT);
  analogReadResolution(12);
  analogSetPinAttenuation(pinSensor, ADC_0db);
  adc_chars = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(unit, ADC_ATTEN_DB_0, width, 1100, adc_chars);

  wifiMulti.addAP(ssid_5, password_5);

  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);  // Asigna la IP estática
  while (wifiMulti.run(tiempoEsperaWiFi) != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  servidor.begin();
  tiempoMedido = millis();
}

/* --------------- LOOP --------------- */

void loop() {
  if (millis() - tiempoMedido >= tiempoMuestreo) {
    datosEntrada();
    filtro();
    calculoPeso(arraySalida[0]);
    corrimientoArray();
  }

  WiFiClient cliente = servidor.available();
  if (cliente) {
    while (cliente.connected()) {
      if (cliente.available()) {
        String LineaActual = cliente.readStringUntil('\r');
        cliente.flush();

        if (LineaActual.startsWith("GET /peso")) {
          cliente.println("HTTP/1.1 200 OK");
          cliente.println("Content-type: text/plain");
          cliente.println("Connection: close");
          cliente.println();
          cliente.print(peso, 3);
          break;
        } else if (LineaActual.indexOf("GET /") >= 0) {
          responderCliente(cliente, peso);
          break;
        }
      }
    }
    cliente.stop();
  }
}

