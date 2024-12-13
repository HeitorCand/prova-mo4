#include <WiFi.h>
#include <HTTPClient.h>

#define led_verde 41 // Pino utilizado para controle do led verda
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led azul

const int botao = 18;  // Numero do pino do botao
int estadoBotao = 0;  // estado do botao

const int ldr = 4;  // numero pino ldr
int mediaClaro = 600;

unsigned long tempoAtual;
unsigned long ultimoTempo1 = 0; // Valor inicial de 0.
unsigned long ultimoTempo2 = 0; // Valor inicial de 0.

bool ligaAmarelo = 0; // desligado
bool ligaVerde = 1;
bool ligaVermelho = 0;

int tempoVermelho = 5000;
int tempoDebounce = 50;
int contador = 0;      // Contador de quantas vezes o botão foi pressionado

bool ultimoEstadoBotao = 0;
unsigned long ultimoTempoDebounce = 0;

void setup() {

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_amarelo, OUTPUT);
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);

  // Inicialização das entradas
  pinMode(botao, INPUT); // Inicializa o botao como input

  digitalWrite(led_amarelo, LOW);
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);

  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST

  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(100);
  //   Serial.print(".");
  // }
  Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

  // Verifica estado do botão
  estadoBotao = digitalRead(botao);
  if (estadoBotao == HIGH) {
    Serial.println("Botão pressionado!");
  } else {
    Serial.println("Botão não pressionado!");
  }

  if (WiFi.status() == WL_CONNECTED) { // Se o ESP32 estiver conectado à Internet
    HTTPClient http;

    String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

    http.begin(serverPath.c_str());

    int httpResponseCode = http.GET(); // Código do Resultado da Requisição HTTP

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      //Serial.println(payload);
      Serial.println("Mensagem recebida!");
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }

  else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  int ldrstatus = analogRead(ldr);
  bool noite = false;
  tempoAtual = millis();

  if (ldrstatus >= mediaClaro) {
    Serial.print("MODO NOTURNO");
    Serial.println(ldrstatus);
    noite = true;


  } else {
    Serial.print("MODO CLARO");
    Serial.println(ldrstatus);
    noite = false;

    digitalWrite(led_amarelo, LOW);
  }

  // Ativa Modo noturno
  if (noite) {
    // Modo intermitente Led Amarelo 1sec
    if (tempoAtual - ultimoTempo1 > 1000) {
      ligaAmarelo = !ligaAmarelo;
      ligaVerde = false;
      ligaVermelho = false;
      ultimoTempo1 = tempoAtual;
    }
  } else {

    // Liga LED Vermelho, verifica se já passou 2sec que o led amarelo estava ligado
    if (tempoAtual - ultimoTempo2 > 2000 && ligaAmarelo) {
      ligaAmarelo = false;
      ligaVerde = false;
      ligaVermelho = !ligaVermelho;
      ultimoTempo2 = tempoAtual;
    }

    // Liga LED Amarelo, verifica se já passou 3sec que o led verde estava ligado
    if (tempoAtual - ultimoTempo2 > 3000 && ligaVerde) {
      ligaAmarelo = !ligaAmarelo;
      ligaVerde = false;
      ligaVermelho = false;
      ultimoTempo2 = tempoAtual;
    }

    // Liga LED Verde, verifica se já passou 5sec que o led vermelho estava ligado
    if (tempoAtual - ultimoTempo2 > tempoVermelho && ligaVermelho) {
      ligaAmarelo = false;
      ligaVerde = !ligaVerde;
      ligaVermelho = false;
      ultimoTempo2 = tempoAtual;
      tempoVermelho = 5000;
    }

    // Reseta o funcionamento a pos a troca de dia e noite
    if (!ligaVermelho && !ligaVerde && !ligaAmarelo) {
      ligaAmarelo = true;
      ligaVerde = false;
      ligaVermelho = false;
    }


    bool btnApertado = digitalRead(botao);

    if (btnApertado&& ligaVermelho) {
      tempoVermelho = 1000;
      ultimoTempo2 = tempoAtual;
    }

  }

  // Controla os led, acendendo e apagando
  digitalWrite(led_amarelo, ligaAmarelo);
  digitalWrite(led_vermelho, ligaVermelho);
  digitalWrite(led_verde, ligaVerde);
}


