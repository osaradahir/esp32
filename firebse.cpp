#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

int pinPotenciometro = 34;
int ledPin = 13;

String serverName = "https://iot-proyecto-175f6-default-rtdb.firebaseio.com/iot"; // Quitado "/dispositivos" de la URL

unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // Aumentado el tiempo de espera a 5 segundos
int lastValorPotenciometro = -1;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a la red WiFi con la dirección IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Temporizador configurado a 5 segundos (variable timerDelay), tomará 5 segundos antes de publicar la primera lectura.");
}

void loop() {
  int valorPotenciometro = analogRead(pinPotenciometro);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Actualizar potenciómetro
    String serverPathPotenciometro = serverName + "/dispositivos/2/valor.json"; // Ruta para el potenciómetro

    http.begin(serverPathPotenciometro.c_str());
    http.addHeader("Content-Type", "application/json");

    // Enviar una solicitud HTTP PUT con el valor del potenciómetro
    int httpResponseCodePotenciometro = http.PUT(String("{\"valor\":") + String(valorPotenciometro) + "}");

    if (httpResponseCodePotenciometro > 0) {
      Serial.print("Código de respuesta HTTP (Potenciómetro): ");
      Serial.println(httpResponseCodePotenciometro);
      Serial.print("Valor del potenciómetro enviado: ");
      Serial.println(valorPotenciometro);
    } else {
      Serial.print("Código de error (Potenciómetro): ");
      Serial.println(httpResponseCodePotenciometro);
      Serial.print("Valor del potenciómetro no enviado. Error.");
    }

    http.end();

    // Actualizar LED
    String serverPathLED = serverName + "/dispositivos/1/valor.json"; // Ruta para el LED

    http.begin(serverPathLED.c_str());
    int httpResponseCodeLED = http.GET();

    if ((millis() - lastTime) > timerDelay) {
      if (httpResponseCodeLED > 0) {
        String payloadLED = http.getString();
        Serial.print("HTTP Response code LED: ");
        Serial.println(httpResponseCodeLED);
        Serial.print("Payload LED: ");
        Serial.println(payloadLED);

        // Buscar el índice de "valor" en el JSON
        if (payloadLED.indexOf("\"valor\":\"0\"") != -1) {
          digitalWrite(ledPin, LOW);  // Apaga el LED si el valor es "0"
          Serial.println("LED apagado");
        } else {
          digitalWrite(ledPin, HIGH);  // Enciende el LED si el valor no es "0"
          Serial.println("LED encendido");
        }
      } else {
        Serial.print("Error code LED: ");
        Serial.println(httpResponseCodeLED);
      }
      http.end();
      lastTime = millis();
    }
  } else {
    Serial.println("WiFi Desconectado");
  }
}
