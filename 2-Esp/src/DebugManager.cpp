#include <Arduino.h>
#include "DebugManager.h"
#include "secrets.h"

int nivelDebugAtual = DEBUG_NIVEL_INICIAL;

void debugErro(const String& mensagem)
{
    if(nivelDebugAtual >= DEBUG_ERRO)
    {
        Serial.print("[ERRO] ");
        Serial.println(mensagem);
    }
}

void debugErroSemLinha(const String& mensagem)
{
    if(nivelDebugAtual >= DEBUG_ERRO)
    {
        Serial.println(mensagem);
    }
}

void debugInfo(const String& mensagem)
{
    if(nivelDebugAtual >= DEBUG_ERRO)
    {
        Serial.print("[INFO] ");
        Serial.println(mensagem);
    }
}

void debugInfoSemLinha(const String& mensagem)
{
    if(nivelDebugAtual >= DEBUG_ERRO)
    {
        Serial.println(mensagem);
    }
}

void configurarDebug()      // chamar no setup
{
    Serial.begin(9600);
    delay(1000);

    pinMode(PINO_HABILITAR_DEBUG_COMPLETO, INPUT_PULLUP);
    
    if(digitalRead(PINO_HABILITAR_DEBUG_COMPLETO) == LOW)
    {
        nivelDebugAtual = DEBUG_TUDO;
    }

    else
    {
        nivelDebugAtual = DEBUG_NIVEL_INICIAL;
    }

    for(int i = 0; i < 5; i++)
    {
        // vai tirar o ruido do Serial
        // isso so acontece se o debug tiver ativo
        debugInfoSemLinha("");
    }

    debugInfo("==================================");
    debugInfo(" ESP 32 INICIADO");
    debugInfo(" Sistema de debug ativo");

    if(nivelDebugAtual == DEBUG_ERRO)
    {
    debugInfo(" Debug iniciado em modo apenas erro");
    }
    
    else
    {
    debugInfo(" Debug iniciado em modo completo");
    }

    debugInfo("==================================");
}

// TODO: Criar a função obterNivelDebugAtual