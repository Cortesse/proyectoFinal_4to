#include <WiFi.h>
#include <WiFiMulti.h>
#include <redesWiFi.h>
WiFiMulti wifiMulti;

WiFiServer servidor(80);         // Crea el servidor web en el puerto 80

const int pinSensor = 34;        // Pin de entrada analógica del sensor de peso
const uint32_t TiempoEsperaWiFi = 1000;

unsigned long TiempoActual = 0;
unsigned long TiempoAnterior = 0;
const unsigned long TiempoCancelacion = 1000;  // Tiempo límite para mantener la conexión con el cliente

void setup() {
  Serial.begin(115200);

  // Agrega varias redes WiFi en caso de que una no esté disponible
  wifiMulti.addAP(ssid_1, password_1);
  //wifiMulti.addAP(ssid_2, password_2);
  //wifiMulti.addAP(ssid_3, password_3);
  //wifiMulti.addAP(ssid_4, password_4);

  WiFi.mode(WIFI_STA); // Configura el modo de operación WiFi como estación

  // Intento de conexión a WiFi
  while (wifiMulti.run(TiempoEsperaWiFi) != WL_CONNECTED) {
    Serial.print("."); // Imprime un punto mientras espera la conexión
  }
  Serial.println("\nConectado a WiFi");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  servidor.begin();
}

void loop() {
  WiFiClient cliente = servidor.available(); // Verifica si hay un cliente conectado

  if (cliente) {
    Serial.println("Nuevo Cliente Conectado");
    TiempoActual = millis(); // Guarda el tiempo actual
    TiempoAnterior = TiempoActual; // Inicializa el tiempo anterior 
    String LineaActual = ""; // Inicializa una cadena para almacenar la línea actual de la solicitud

    // Leer la señal analógica del sensor y convertirla a peso
    int valorSensor = analogRead(pinSensor);  // Lee el valor analógico del sensor
    float voltaje = (valorSensor / 4095.0) * 3.3; // Convierte el valor analógico a voltaje
    float peso = ConvertirVaKg(voltaje); // Convierte voltaje a kilogramos

    // Bucle para mantener la conexión con el cliente mientras esté activo
    while (cliente.connected() && (TiempoActual - TiempoAnterior <= TiempoCancelacion)) {
      if (cliente.available()) { // Si hay datos disponibles del cliente
        TiempoActual = millis(); // Actualizar el tiempo actual
        char Letra = cliente.read(); // Leer un carácter del cliente
        LineaActual += Letra; // Agregar el carácter a la línea actual

        // Comprobar si se ha recibido una nueva línea
        if (Letra == '\n') {
          if (LineaActual.startsWith("GET /peso")) { // Ruta para obtener solo el peso
            cliente.println("HTTP/1.1 200 OK");
            cliente.println("Content-type: text/plain");
            cliente.println("Connection: close");
            cliente.println();
            cliente.print(peso); // Enviar solo el valor de peso en respuesta
            break;
          } else if (LineaActual.indexOf("GET /") >= 0) { // Ruta para la página principal
            ResponderCliente(cliente, peso);
            break;
          }
          LineaActual = ""; // Reiniciar la línea actual
        }
      }
    }
    cliente.stop(); // Detener la conexión con el cliente
    Serial.println("Cliente Desconectado"); // Mensaje indicando que el cliente se ha desconectado
    Serial.println(); // Línea en blanco para claridad en el Serial Monitor
  }
}
