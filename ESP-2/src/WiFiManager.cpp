//! WiFiManager.cpp
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "WiFiManager.h"
#include "secrets.h"
#include "DebugManager.h"


bool wifiEstaConectado()
{
    return WiFi.status() == WL_CONNECTED;
}

void conectarWiFi()
{
  debugInfo("========================");
  debugInfo("Iniciando conexão WiFi...");
  debugInfo("========================"); 

  // Configura o ESP32 como station, ou seja
  // ele vai se conectar a um roteador existente.
  WiFi.mode(WIFI_STA);    

  // Inicia a conexão com SSID e senha 
  WiFi.begin(WIFI_SSID, WIFI_SENHA);
  
  debugInfo("Conectando");

  int tentativasWiFi = 0;
  const int maxTentativasWiFi = 30;

// quando o wifi nao tiver 3(conectado) e Aguarda a conexão por até 30 tentativas.
  while(WiFi.status() != WL_CONNECTED && tentativasWiFi < maxTentativasWiFi)   
  {
    delay(500);
    debugInfoSemLinha(".");                  // vai ficar mandando . toda vez ate o esp32 conectar
    tentativasWiFi++;     
  }

  debugInfo("");

  if(WiFi.status() == WL_CONNECTED)
  {
    debugInfo("WiFi conectado com sucesso!");
    debugInfoSemLinha("[INFO]Endereço IP: ");
    debugInfoSemLinha (WiFi.localIP().toString());
    debugInfo("\n\r");
  }
  
  else
  {
    debugErro("Falha ao conectar com o WiFi.");
    debugErro("Verifique SSID, senha e sinal de rede.");
  }
}

// função q vai ficar no loop
void garantirWiFiConectado()
{
  if(WiFi.status() != WL_CONNECTED)
  {
    debugErro("WiFi desconectado. Tentando reestabeceler a conexão...");
    conectarWiFi();  // tenta conectar dnv
  }

  if(WiFi.status() != WL_CONNECTED)
  {
    debugErro("Não foi possivel reestabelecer a conexão");         // aqui ele so nao conseguiu reconectar msm.
  }
}