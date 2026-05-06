//!secrets.

#ifndef SECRETS_H
#define SECRETS_H

// * CONFIGURAÇÕES DO WIFI

extern const char *WIFI_SSID;
extern const char *WIFI_SENHA;

//======================================
// *MQTT
//======================================

extern const bool USAR_AWS_IOT;

extern const char* MQTT_BROKER;
extern const int MQTT_PORTA;

// o ID do mqtt sempre vai ser diferente
// nunca criar um id igual de alguem pois ele nao permite e fica desconectando um
extern const char* MQTT_CLIENT_ID;

extern const char* MQTT_USUARIO;
extern const char* MQTT_SENHA;

// essa extern const fala q nao estamos usando certificado
// modo de conexao sem segurança
extern const bool MQTT_USAR_TLS;

extern const  char MQTT_CERTIFICADO_CA[];
extern const char* TOPICOS_PUBLICAR[];



extern const int TOTAL_TOPICOS_PUBLICAR;

// 
extern const char* TOPICOS_RECEBER[];

extern const int TOTAL_TOPICOS_RECEBER;

//======================================
// *DEBUG
//======================================

// 0 = sem mensagens
// 1 = apenas erros
// 2 = todas mensagens
extern const int DEBUG_NIVEL_INICIAL;

// Pino usado para forçar todas as mensagens na inicialização
extern const int PINO_HABILITAR_DEBUG_COMPLETO;


#endif