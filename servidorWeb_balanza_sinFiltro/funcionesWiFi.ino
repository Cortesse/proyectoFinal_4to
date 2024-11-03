// Función que toma un valor de voltaje como entrada y lo convierte a kilogramos usando un factor de conversión
float ConvertirVaKg(float voltaje) {
  float kgxV = 30.0 / 3.3;  // Ajusta este valor según tu sensor
  return voltaje * kgxV;
}

/*void ResponderCliente(WiFiClient& cliente, float peso) {
    cliente.println("HTTP/1.1 200 OK");
    cliente.println("Content-type:text/html");
    cliente.println();
    cliente.print("<!DOCTYPE html><html lang='es'><head>");
    cliente.print("<meta charset='UTF-8'>");
    cliente.print("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    cliente.print("<title>Balanza Digital</title>");
    
    // Meta refresh para actualizar la página cada 5 segundos
    cliente.print("<meta http-equiv='refresh' content='5'>");

    cliente.print("<style>");
    cliente.print("body { font-family: Arial, sans-serif; margin: 0; padding: 0; display: flex; justify-content: center; align-items: center; height: 100vh; background-color: #ffcccc; }"); // Fondo claro
    cliente.print(".container { text-align: center; background-color: #ffffff; padding: 40px; border-radius: 12px; box-shadow: 0 4px 20px rgba(0, 0, 0, 0.2); max-width: 90%; }"); // Contenedor en tono claro
    cliente.print("h1 { color: #c72c41; margin-bottom: 20px; }"); // Tono rojo oscuro para el título
    cliente.print(".peso { font-weight: bold; color: #a72a38; font-size: 48px; margin: 20px 0; }"); // Peso en un tono rojo medio
    cliente.print("</style>");
    
    cliente.print("</head><body>");
    cliente.print("<div class='container'>");
    cliente.print("<h1>Balanza Digital</h1>"); // Título
    cliente.print("<p class='peso'>"); // Valor del peso
    cliente.print(peso);
    cliente.print(" kg</p>"); // Imprimir el peso en kg
    cliente.print("</div></body></html>");
}*/

void ResponderCliente(WiFiClient client, float peso) {
  // Enviar cabeceras HTTP para indicar contenido HTML
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  // Iniciar el código HTML
  client.println("<!DOCTYPE html>");
  client.println("<html lang='es'>");
  client.println("<head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>Balanza Digital</title>");
  client.println("<style>");
  
  // Estilos CSS para la página
  client.println("body { display: flex; flex-direction: column; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f2f2f2; font-family: Arial, sans-serif; }");
  client.println("h1 { font-size: 2em; color: #333; margin-bottom: 20px; text-align: center; }");
  client.println(".balanza-container { position: relative; width: 300px; height: 320px; background-color: #333; border-radius: 10px; box-shadow: 0 4px 10px rgba(0, 0, 0, 0.2); text-align: center; }");
  client.println(".medidor { position: absolute; top: 20px; left: 20px; right: 20px; height: 90px; background-color: #87CEFA; border-radius: 8px; display: flex; justify-content: center; align-items: center; }");
  client.println(".aguja { position: absolute; width: 8px; height: 70px; background-color: #000; top: 25px; left: 50%; transform-origin: bottom; transform: rotate(0deg); transition: transform 0.5s ease-out; }");
  client.println(".valor-medicion { position: absolute; bottom: 20px; left: 10%; right: 10%; color: white; font-size: 36px; font-weight: bold; background-color: #4a4a4a; padding: 10px; border-radius: 8px; box-sizing: border-box; }");

  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  
  // Título de la balanza
  client.println("<h1>Balanza Digital</h1>");

  // Contenedor principal de la balanza
  client.println("<div class='balanza-container'>");
  client.println("<div class='medidor'>");
  client.println("<div class='aguja' id='aguja'></div>"); // Aguja que se moverá
  client.println("</div>");
  client.println("<div class='valor-medicion' id='peso'>"); 
  client.print(peso); // Mostrar el valor inicial del peso
  client.println(" kg</div>");
  client.println("</div>");

  // Código JavaScript para actualizar el valor del peso sin recargar la página
  client.println("<script>");
  client.println("function actualizarPeso() {");
  client.println("  fetch('/peso').then(response => response.text()).then(data => {");
  client.println("    document.getElementById('peso').innerText = data + ' kg';");
  client.println("  });");
  client.println("}");
  client.println("setInterval(actualizarPeso, 1000);"); // Actualizar cada segundo
  client.println("</script>");
  
  client.println("</body>");
  client.println("</html>");
  
  // Cerrar conexión con el cliente
  client.println();
  client.stop();
}













