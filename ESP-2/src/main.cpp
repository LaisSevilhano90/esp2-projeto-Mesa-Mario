#include <Arduino.h>
#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"
#include "secrets.h"
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// PINOS
const int PINO_LED_RGB = 48;
const int QUANTIDADE_LEDS = 1;
const int PINO_LAMPADA = 15;

// VARIAVEIS TELA
int telaAtual = 0;
unsigned long ultimaTroca = 0;
const unsigned long INTERVALO = 6000; // 6 segundos
unsigned long ultimaAtualizacaoLCD = 0;
const unsigned long INTERVALO_ATUALIZACAO = 1000; // Atualiza a cada 1 segundo

// VARIAVEIS JSON
String funcionario;
String statusDeTrabalho;
String horaDoPontoBatido;

String statusLais = "Aguardando";
String horaLais = "--:--:--";

String statusLuigi = "Aguardando";
String horaLuigi = "--:--:--";

String statusLeonardo = "Aguardando";
String horaLeonardo = "--:--:--";

String statusPedro = "Aguardando";
String horaPedro = "--:--:--";

// TOPICO
const char TOPICO_COMANDO[] = "senai134/pedroleonel/esp32/comando";

// variavel para controlar o estado da lampada
bool lampada = false;

Adafruit_NeoPixel ledRGB(
    QUANTIDADE_LEDS,
    PINO_LED_RGB,
    NEO_GRB + NEO_KHZ800);

// PROTOTIPOS DAS FUNÇÕES
void tratarMensagemRecebida(const char *topico, const String &mensagem);
void configurarLedRGB();
void tratarJsonComando(const String &mensagem);
void trocarTela();
void atualizarLCD();

void setup()
{
  Serial.begin(9600);

  //* LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Mensagem inicial
  lcd.setCursor(0, 0);
  lcd.print("Conectando...");

  ultimaTroca = millis();
  ultimaAtualizacaoLCD = millis();

  configurarDebug();
  configurarLedRGB();
  conectarWiFi();
  configurarMQTT();
  registrarCallBackMensagem(tratarMensagemRecebida);
  conectarMQTT();
  pinMode(15, OUTPUT);

  // Limpa o LCD e mostra a primeira tela
  lcd.clear();
  atualizarLCD();
}

void loop()
{
  garantirWiFiConectado();
  garantirMQTTConectado();
  loopMQTT();
  trocarTela(); // Troca entre funcionários a cada 6 segundos
}

void tratarMensagemRecebida(const char *topico, const String &mensagem)
{
  debugInfo("==================================");
  debugInfo("Mensagem recebida na aplicacao");
  debugInfo("==================================");

  if (topico == nullptr)
  {
    debugErro("Topico MQTT invalido");
    return;
  }

  debugInfo("Topico: " + String(topico));
  debugInfo("Mensagem: " + mensagem);

  if (strcmp(topico, TOPICO_COMANDO) == 0)
  {
    tratarJsonComando(mensagem);
    return;
  }

  debugErro("Topico nao tratado: " + String(topico));
}

void configurarLedRGB()
{
  ledRGB.begin();
  ledRGB.setBrightness(80);
  ledRGB.clear();
  ledRGB.show();
  debugInfo("LED RGB configurado no GPIO " + String(PINO_LED_RGB));
}

void tratarJsonComando(const String &mensagem)
{
  JsonDocument doc;

  DeserializationError erro = deserializeJson(doc, mensagem);

  if (erro)
  {
    debugErro("Erro ao interpretar o JSON.");
    debugErro(erro.c_str());
    return;
  }

  if (doc["led"].is<JsonObject>())
  {
    int vermelho = doc["led"]["r"].as<int>();
    int verde = doc["led"]["g"].as<int>();
    int azul = doc["led"]["b"].as<int>();
    debugInfo("Comando LED recebido: R=" + String(vermelho) + " G=" + String(verde) + " B=" + String(azul));
  }

  // Processa dados dos funcionários
  if (doc["Funcionario"].is<String>())
  {
    funcionario = doc["Funcionario"].as<String>();
    statusDeTrabalho = doc["Status"].as<String>();
    horaDoPontoBatido = doc["Hora"].as<String>();

    debugInfo("Funcionario: " + funcionario);
    debugInfo("Status: " + statusDeTrabalho);
    debugInfo("Hora: " + horaDoPontoBatido);

    // Atualiza as variáveis do funcionário correspondente
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

    // Atualiza o LCD imediatamente com os novos dados
    atualizarLCD();
  }
}

// Função para atualizar o LCD baseado na tela atual
void atualizarLCD()
{
  lcd.clear();

  switch (telaAtual)
  {
  case 0:
    lcd.setCursor(0, 0);
    lcd.print("Lais:");
    lcd.setCursor(0, 1);
    lcd.print("Status: " + statusLais);
    lcd.setCursor(0, 2);
    lcd.print("Hora: " + horaLais);
    break;

  case 1:
    lcd.setCursor(0, 0);
    lcd.print("Luigi:");
    lcd.setCursor(0, 1);
    lcd.print("Status: " + statusLuigi);
    lcd.setCursor(0, 2);
    lcd.print("Hora: " + horaLuigi);
    break;

  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Leonardo:");
    lcd.setCursor(0, 1);
    lcd.print("Status: " + statusLeonardo);
    lcd.setCursor(0, 2);
    lcd.print("Hora: " + horaLeonardo);
    break;

  case 3:
    lcd.setCursor(0, 0);
    lcd.print("Pedro:");
    lcd.setCursor(0, 1);
    lcd.print("Status: " + statusPedro);
    lcd.setCursor(0, 2);
    lcd.print("Hora: " + horaPedro);
    break;
  }
}

void trocarTela()
{
  unsigned long agora = millis();

  if (agora - ultimaTroca >= INTERVALO)
  {
    ultimaTroca = agora;
    telaAtual = (telaAtual + 1) % 4;
    atualizarLCD(); // Atualiza a tela com os dados do novo funcionário
  }
}