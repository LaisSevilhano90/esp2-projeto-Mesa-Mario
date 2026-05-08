/**
 *  Nome: Pedro Leonel de Lorena, Leonardo Ferrarese Correa, Lais Rodrigues Sevilhano & Luigi Arnosti Reginato
 *  Descrição: Neste projeto haverão dois displays onde serão exibidos os funcionários e se estes estão trabalhando, permitindo que os mesmos registrem seus inícios e finais de turno.
 *  Projeto: Bater ponto - Sistema de presença de funcionários
 *  Data: 06/05/2026
 *  Versão: 0.4
 */

#include <Arduino.h>
#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 20, 4);


// PINOS

const int PINO_LED_RGB = 48;
const int QUANTIDADE_LEDS = 1;
const int PINO_LAMPADA = 15;

// VARIAVEIS TELA
int telaAtual = 0;
unsigned long ultimaTroca = 0;
const unsigned long INTERVALO = 3000;; // 10 segundos

// VARIAVEIS JSON
String funcionario;
String statusDeTrabalho;
String horaDoPontoBatido;

String statusLais;
String horaLais;

String statusLuigi;
String horaLuigi;

String statusLeonardo;
String horaLeonardo;

String statusPedro;
String horaPedro;

// TOPICO 
const char TOPICO_COMANDO[] = "senai134/pedroleonel/esp32/comando";

//  variavel para controlar o estado da lampada
bool lampada = false;


Adafruit_NeoPixel ledRGB(
    QUANTIDADE_LEDS,
    PINO_LED_RGB,
    NEO_GRB + NEO_KHZ800 // constante de configuração
);

//* PROTOTIPOS DAS FUNÇÕES

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarJsonComando(const String &mensagem);
void acenderLampada(bool lampadaParametro);
void trocarTela();


void setup()
{
  pinMode(15, OUTPUT);

  configurarDebug();
  configurarLedRGB();
  conectarWiFi();
  configurarMQTT();
  registrarCallBackMensagem(tratarMensagemRecebida);
  conectarMQTT();
  
  //* LCD 
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Carregando:");
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
  //*LCD
  trocarTela();
}

void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  debugInfo("==================================");
  debugInfo("Mensagem recebida na aplicação");
  debugInfo("==================================");

  if (topico == nullptr)
  {
    debugErro("Tópico MQTT inválido");
    return;
  }

  debugInfo("Tópico: " + String(topico));
  debugInfo("Mensagem " + mensagem);

  debugInfo(String(strcmp(topico, TOPICO_COMANDO)));

  if (strcmp(topico, TOPICO_COMANDO) == 0)
  {
    tratarJsonComando(mensagem);
    return;
  }

  debugErro("Tópico näo tratado: " + String(topico));
}

void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80); // IGUAL O ANALOG WRITE (0 a 255)
  ledRGB.clear();
  ledRGB.show();

  debugInfo("LED RGB configurado no GPIO " + String(PINO_LED_RGB));
}

void alterarCorLedRGB(int vermelho, int verde, int azul)
{
  // essa constrain faz a mesma coisa que um if(vermelho < 0) vermelho = 0 e if(vermelho > 255) vermelho = 255
  vermelho = constrain(vermelho, 0, 255);
  verde = constrain(verde, 0, 255);
  azul = constrain(azul, 0, 255);

  // passar qual é o endereço do led (tipo pensa na fita q tenha 5 leds ele vai falar q começa no primeiro que é o 0)
  ledRGB.setPixelColor(0, ledRGB.Color(vermelho, verde, azul));
  ledRGB.show();

  debugInfo("Cor aplicada no LED RGB");
  debugInfo("R: " + String(vermelho));
  debugInfo("G: " + String(verde));
  debugInfo("B: " + String(azul));
}

void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;

  deserializeJson(doc, mensagem);

  DeserializationError erro = deserializeJson(doc, mensagem);

  if (erro)
  {
    debugErro("Erro ao interpretar o JSON.");
    debugErro(erro.c_str());
    return;
  }

  if (!doc["led"].is<JsonObject>())
  {
    if (!doc["led"]["r"].is<int>() || !doc["led"]["g"].is<int>() || !doc["led"]["b"].is<int>())
    {
      debugErro("JSON Inválido. Use led.r, led.g e led.b");
      return;
    }
  }
  else
  {
    int vermelho = doc["led"]["r"].as<int>();
    int verde = doc["led"]["g"].as<int>();
    int azul = doc["led"]["b"].as<int>();

    debugInfo("entrou no else para chamar alterarCorLedRGB");

    alterarCorLedRGB(vermelho, verde, azul);
  }
  if (doc["lampada"].is<bool>())
  {
    bool lampada = doc["lampada"].as<bool>();

    acenderLampada(lampada);
  }
  else
  {
    debugErro("Checar a conexão da lampamda com o LED");
    debugErro("Checar erro no JSON");
  }

  //* Deserialize do Json do nosso MQTT do esp1

  if (doc["Funcionario"].is<String>())
  {
    funcionario = doc["Funcionario"].as<String>();
    debugInfo("Funcionario : " + funcionario);
  }
  else
  {
    debugErro("Checar se o nome do funcionario está correto no JSON");
    debugErro("Checar se há erro no JSON");
  }
   if (doc["Status"].is<String>())
  {
    statusDeTrabalho = doc["Status"].as<String>();
    debugInfo("Status: " + statusDeTrabalho);
  }
  else
  {
    debugErro("Checar se escreveu errado o nome no doc do status de trabalho");
    debugErro("Checar se há erro no JSON");
  }
   if (doc["Hora"].is<String>())
  {
    horaDoPontoBatido = doc["Hora"].as<String>();
    debugInfo("Hora do Ponto Batido : " + horaDoPontoBatido);
  }
  else
  {
    debugErro("Checar se escreveu errado o nome no doc do Hora do ponto batido");
    debugErro("Checar erro no JSON");
  }

  if (funcionario == "Lais")
  { 
    statusLais = statusDeTrabalho;
    horaLais = horaDoPontoBatido;
  }
  else if (funcionario == "Luigi")
  {
    statusLuigi = statusDeTrabalho;
    horaLuigi = horaDoPontoBatido;
  }
  else if (funcionario == "Leonardo")
  {
    statusLeonardo = statusDeTrabalho;
    horaLeonardo = horaDoPontoBatido;
  }
  else if (funcionario == "Pedro")
  {
    statusPedro = statusDeTrabalho;
    horaPedro = horaDoPontoBatido;
  }

}

void acenderLampada(bool lampadaParametro)
{
  debugInfo("entrou aqui");

  lampada = lampadaParametro;
  digitalWrite(PINO_LAMPADA, lampada);
  debugInfo("Estado da Lampada: " + String(lampada));
}

//!FUNCAO TROCAR TELA LCD

void trocarTela() {
    unsigned long agora = millis();
    
    if (agora - ultimaTroca >= INTERVALO) {
        ultimaTroca = agora;
        telaAtual = (telaAtual + 1) % 4; // cicla entre 0, 1, 2 e 3
        lcd.clear(); // só limpa quando troca de tela
        delay(50);
        switch (telaAtual) {
            case 0:
                lcd.setCursor(0, 0);
                lcd.print("Lais:");
                lcd.setCursor(0, 1);
                lcd.print("Status: " + statusLais);
                lcd.setCursor(0, 2);
                lcd.print("Hora do Ponto: " + horaLais);
                break;

            case 1:
                lcd.setCursor(0, 0);
                lcd.print("Luigi:");
                lcd.setCursor(0, 1);
                lcd.print("Status: " + statusLuigi);
                lcd.setCursor(0, 2);
                lcd.print("Hora do Ponto: " + horaLuigi);
                break;

            case 2:
                lcd.setCursor(0, 0);
                lcd.print("Leonardo:");
                lcd.setCursor(0, 1);
                lcd.print("Status: " + statusLeonardo);
                lcd.setCursor(0, 2);
                lcd.print("Hora do Ponto: " + horaLeonardo);
                break;

            case 3:
                lcd.setCursor(0, 0);
                lcd.print("Pedro:");
                lcd.setCursor(0, 1);
                lcd.print("Status: " + statusPedro);
                lcd.setCursor(0, 2);
                lcd.print("Hora do Ponto: " + horaPedro);
                break;
        }
    }
}