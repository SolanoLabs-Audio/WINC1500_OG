#ifndef WINC_IF_H
#define WINC_IF_H

#include "mongoose.h"
#include "socket.h"

// Inizializza l'interfaccia WINC per Mongoose
void winc_if_init(struct mg_mgr *mgr);

// Gestisce gli eventi di rete (da chiamare nel loop principale)
void winc_if_poll(struct mg_mgr *mgr, int timeout_ms);

// Callback per i socket WINC
void winc_sock_cb(SOCKET sock, uint8_t u8MsgType, void *pvMsg);

// Crea un server HTTP - AGGIUNTA QUESTA DICHIARAZIONE
bool winc_create_http_server(uint16_t port);

#endif
