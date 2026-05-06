//! MqttManager.h

#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H
#include <Arduino.h>

void configurarMQTT();
void conectarMQTT();
void garantirMQTTConectado();
void loopMQTT();

void publicarMensagem(const char* topico, const char* mensagem);
void publicarMensagemNoTopico(int indiceTopico, const char* mensagem);


bool mqttEstaConectado();

// TODO: criar o parametro do tipo int chamado indiceTopico para as funções abaixo

const char* obterTopicoPublicacao(int indiceTopico);
const char* obterTopicoRecebimento();

typedef void (*CallbackMensagemMQTT)(const char* topico, const String& mensagem);

void registrarCallBackMensagem(CallbackMensagemMQTT callback);

int obterTotalTopicosRecebimento();

#endif