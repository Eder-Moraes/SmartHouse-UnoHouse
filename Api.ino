#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "";
const char* password = "";

WebServer server(80);

//Buzzer
const int buzzer_pin = 13;

// Pinos e variáveis do sensor PIR
const int pirPin = 34;  // Pino de saída do sensor PIR
const int ledSala = 33;
const int ledGaragem = 32;
bool ledSalaState = false;
bool ledGaragemState = false;
bool gateState = false;
bool senhaCorreta = false;
bool movimentoDetectado = false;  // Variável para armazenar o estado do movimento

const int servoPin = 27;  // Pino de controle do servo
Servo gateServo;          // Objeto para controlar o servo

void piscaLed(){
    digitalWrite(ledSala, HIGH);
    digitalWrite(ledGaragem, HIGH);
    tone(buzzer_pin, 1000);
    delay(200);
    noTone(buzzer_pin);
    digitalWrite(ledSala, LOW);
    digitalWrite(ledGaragem, LOW);
    delay(200);
}

// Funções motor
void abrirPortao() {
  Serial.println("Abrindo portão");
  gateServo.write(75);  // Gira o servo para 90 graus para abrir
  delay(1000);
  //gateServo.write(0);   // Retorna à posição inicial
}

void fecharPortao() {
  Serial.println("Fechando portão");
  gateServo.write(0);   // Gira o servo para 0 graus para fechar
}

void handleCORS() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  server.send(204);
}

void handleLedSalaControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String status = server.arg("status");
  if (status == "on") {
    ledSalaState = true;
    digitalWrite(ledSala, HIGH);
  } else if (status == "off") {
    ledSalaState = false;
    digitalWrite(ledSala, LOW);
  }
  server.send(200, "text/plain", ledSalaState ? "LED Ligado" : "LED Desligado");
}

void handleLedGaragemControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String status = server.arg("status");
  if (status == "on") {
    ledGaragemState = true;
    digitalWrite(ledGaragem, HIGH);
  } else if (status == "off") {
    ledGaragemState = false;
    digitalWrite(ledGaragem, LOW);
  }
  server.send(200, "text/plain", ledGaragemState ? "LED Ligado" : "LED Desligado");
}

void handleGateControl() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String status = server.arg("status");
  if (status == "open") {
    gateState = true;
    abrirPortao();
    Serial.println("Portão aberto");
  } else if (status == "close") {
    gateState = false;
    fecharPortao();
    Serial.println("Portão fechado");
  }
  server.send(200, "text/plain", gateState ? "Portão Aberto" : "Portão Fechado");
}

void handlePassword() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  String senha = server.arg("password");
  if (senha == "1234") {
    Serial.println("Senha correta");
    server.send(200, "text/plain", "Senha correta!");
    senhaCorreta = true;
    movimentoDetectado = false;  // Reseta o estado de movimento
    digitalWrite(ledSala, LOW);       // Apaga o LED
    digitalWrite(ledGaragem, LOW);       // Apaga o LED
    noTone(buzzer_pin);
    senhaCorreta = false;
  } else {
    server.send(403, "text/plain", "Senha incorreta.");
    senhaCorreta = false;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzer_pin, OUTPUT);  // Configura o pino do buzzer como saída
  pinMode(ledSala, OUTPUT);
  pinMode(ledGaragem, OUTPUT);
  pinMode(pirPin, INPUT);
  gateServo.attach(servoPin);  // Configura o pino do servo

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao Wi-Fi...");
  }

  Serial.println("Conectado ao Wi-Fi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Rotas do servidor
  server.on("/ledSala", handleLedSalaControl);
  server.on("/ledGaragem", handleLedGaragemControl);
  server.on("/gate", handleGateControl);
  server.on("/password", HTTP_POST, handlePassword);
  server.on("/ledSala", HTTP_OPTIONS, handleCORS);
  server.on("/ledGaragem", HTTP_OPTIONS, handleCORS);
  server.on("/gate", HTTP_OPTIONS, handleCORS);
  server.on("/password", HTTP_OPTIONS, handleCORS);

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();

  // Verifica se o sensor PIR detectou movimento
  if (digitalRead(pirPin) == LOW && !senhaCorreta && !movimentoDetectado) {
    Serial.println("Movimento detectado!");
    movimentoDetectado = true;  // Marca que o movimento foi detectado
  }

  // Se o movimento foi detectado e a senha ainda não foi inserida corretamente, pisca o LED
  if (movimentoDetectado && !senhaCorreta) {
    piscaLed();
    fecharPortao();
    gateState = false;
  }
}
