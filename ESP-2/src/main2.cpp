/**
 *  Nome: Pedro Leonel de Lorena
 *  Descrição: Este projeto realiza a conexao do esp com o wifi e o protocolo mqtt
 *  Projeto: conexao MQTT
 *  Data: 28/04
 *  Versão: 1.0
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

LiquidCrystal_I2C lcd(0x27, 20,4);

void AtualizarDisplay();

const int PINO_LED_RGB = 48;
const int QUANTIDADE_LEDS = 1;
const int PINO_LAMPADA = 15;


String tempoLocal;

const char TOPICO_COMANDO[] = "senai134/pedroleonel/esp32/comando";

bool lampada = false;

Adafruit_NeoPixel ledRGB(
    QUANTIDADE_LEDS,
    PINO_LED_RGB,
    NEO_GRB + NEO_KHZ800 // constante de configuração
);

void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void alterarCorLedRGB(int vermelho, int verde, int azul);
void tratarJsonComando(const String &mensagem);
void acenderLampada(bool lampadaParametro);
void coletarHora();

void setup()
{
  pinMode(15, OUTPUT);

  configurarDebug();
  configurarLedRGB();
  conectarWiFi();
  configurarMQTT();
  registrarCallBackMensagem(tratarMensagemRecebida);
  conectarMQTT();
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();

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
}

void acenderLampada(bool lampadaParametro)
{
  debugInfo("entrou aqui");

  lampada = lampadaParametro;
  digitalWrite(PINO_LAMPADA, lampada);
  debugInfo("Estado da Lampada: " + String(lampada));
}

void coletarHora()
{
  WiFiClientSecure client;

  client.setInsecure();

  HTTPClient http;

  if (!http.begin(client, URL_API))
  {
    Serial.println();
    Serial.println("Falha ao iniciar a conexão HTTP");
    return;
  }

  http.setTimeout(10000);

  int httpCode = http.GET();

  if (httpCode > 0)
  {
    Serial.println("Codigo HTTP: ");
    Serial.print(httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
      String resposta = http.getString();
      //USAR SE DER ERRO NA API
      Serial.println("resposta bruta da API: ");
      Serial.print(resposta);
      JsonDocument doc;
      DeserializationError erro = deserializeJson(doc, resposta);

      if (!erro)
      {
          tempoLocal = doc["localtime"].as<String>();
      }
    }
    else
    {
      Serial.print("A API respondeu, mas com codigo de erro: "); // se for diferente de 200(Esse 200 foi obtido la no http.GET()) mostra isso
      Serial.println(httpCode);
    }
  }
  else // se httpCode <= 0
  {
    Serial.print("Erro na requisiçao HTTP: ");
    Serial.println(http.errorToString(httpCode));
  }
  http.end();
}

void AtualizarDisplay()
{

  lcd.setCursor(1, 0);
  lcd.print("Lais");
  lcd.setCursor(2, 1);
  lcd.print("Leonardo");
  lcd.setCursor(3, 2);
  lcd.print("Luigi");
  lcd.setCursor(4, 3);
  lcd.print("Pedro");

  switch (coordenada)
  {
  case 0:
    lcd.setCursor(0, 0);
    lcd.print(">");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print(" ");
    break;
  case 1:
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(">");
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print(" ");
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(">");
    lcd.setCursor(0, 3);
    lcd.print(" ");
    break;
  case 3:
    lcd.setCursor(0, 0);
    lcd.print(" ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(" ");
    lcd.setCursor(0, 3);
    lcd.print(">");
    break;
  default:
    coordenada = 0;
    break;
  }
}
