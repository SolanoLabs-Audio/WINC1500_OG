#include "winc_if.h"
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "nm_common.h"
#include "m2m_wifi.h"

#define WINC_RECV_BUFFER_SIZE 1500
#define MAX_SOCKETS 10

static struct mg_mgr *s_mgr = NULL;

// Struttura per tenere traccia delle connessioni
typedef struct {
    SOCKET sock;
    struct mg_connection *conn;
    bool is_listener;
    uint8_t recv_buf[WINC_RECV_BUFFER_SIZE];
} winc_conn_t;

static winc_conn_t s_conns[MAX_SOCKETS] = {0};

// Inizializza la tabella delle connessioni
static void init_conn_table(void) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        s_conns[i].sock = -1;
        s_conns[i].conn = NULL;
        s_conns[i].is_listener = false;
    }
}

// Trova una connessione per socket
static winc_conn_t *find_conn(SOCKET sock) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (s_conns[i].sock == sock) {
            return &s_conns[i];
        }
    }
    return NULL;
}

// Trova uno slot libero
static winc_conn_t *find_free_slot(void) {
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (s_conns[i].sock == -1) {
            return &s_conns[i];
        }
    }
    return NULL;
}

static SOCKET create_listener(uint16_t port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Failed to create socket\n");
        return -1;
    }

    // CORREZIONE ALTERNATIVA: usa direttamente 0 per l'indirizzo
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;

    // Usa un array di byte per l'indirizzo (molto probabilmente come è definito nel WINC1500)
    unsigned char *ip_addr = (unsigned char *)&addr.sin_addr;
    ip_addr[0] = 0;
    ip_addr[1] = 0;
    ip_addr[2] = 0;
    ip_addr[3] = 0;

    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        printf("Failed to bind socket %d\n", sock);
        close(sock);
        return -1;
    }

    if (listen(sock, 3) != 0) {
        printf("Failed to listen on socket %d\n", sock);
        close(sock);
        return -1;
    }

    printf("Listener created on socket %d, port %u\n", sock, port);
    return sock;
}

// Invia dati attraverso un socket WINC
static void winc_send_data(struct mg_connection *c, const void *buf, size_t len) {
    SOCKET sock = (SOCKET)(intptr_t)c->fd;
    if (sock < 0) return;

    sint16 ret = send(sock, (void *)buf, len, 0);
    if (ret != M2M_SUCCESS) {
        printf("Send failed on socket %d: %d\n", sock, ret);
        c->is_closing = 1;
    } else {
        printf("Sent %d bytes on socket %d\n", (int)len, sock);
    }
}

// Gestore HTTP semplificato
static void http_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        // CORREZIONE: usa mg_str per stampare l'URI
        printf("HTTP request: %.*s\n", (int)hm->uri.len, hm->uri.buf);

        // CORREZIONE: confronto manuale dell'URI invece di mg_http_match_uri
        if (hm->uri.len == 1 && memcmp(hm->uri.buf, "/", 1) == 0) {
            const char *response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Connection: close\r\n"
                "\r\n"
                "<html><body>"
                "<h1>Ciao da STM32H7 + WINC1500!</h1>"
                "<p>Mongoose funziona con WINC1500!</p>"
                "</body></html>";

            winc_send_data(c, response, strlen(response));
        } else {
            const char *response =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/plain\r\n"
                "Connection: close\r\n"
                "\r\n"
                "404 Not Found";

            winc_send_data(c, response, strlen(response));
        }

        // Chiudi la connessione dopo la risposta
        c->is_closing = 1;
    }
}

// Callback principale per i socket WINC
void winc_sock_cb(SOCKET sock, uint8_t u8MsgType, void *pvMsg) {
    winc_conn_t *wc = find_conn(sock);

    printf("WINC CB: sock=%d, type=%d\n", sock, u8MsgType);

    switch (u8MsgType) {
        case SOCKET_MSG_ACCEPT: {
            tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
            if (pstrAccept && pstrAccept->sock >= 0) {
                SOCKET new_sock = pstrAccept->sock;
                printf("ACCEPT: new socket %d from listener %d\n", new_sock, sock);

                // Crea una nuova connessione Mongoose
                struct mg_connection *c = mg_alloc_conn(s_mgr);
                if (c) {
                    // Configura la connessione
                    c->fd = (void *)(intptr_t)new_sock;
                    c->is_udp = 0;
                    c->pfn = http_handler;  // Assegna l'handler HTTP
                    c->mgr = s_mgr;

                    // Trova uno slot libero
                    winc_conn_t *new_wc = find_free_slot();
                    if (new_wc) {
                        new_wc->sock = new_sock;
                        new_wc->conn = c;
                        new_wc->is_listener = false;

                        // Inizia a ricevere dati
                        recv(new_sock, new_wc->recv_buf, sizeof(new_wc->recv_buf), 0);

                        printf("New connection established on socket %d\n", new_sock);
                    } else {
                        printf("No free slots for new connection\n");
                        close(new_sock);
                        free(c);
                    }
                } else {
                    printf("Failed to allocate connection\n");
                    close(new_sock);
                }
            }
            break;
        }

        case SOCKET_MSG_RECV: {
            tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
            if (pstrRecv && pstrRecv->s16BufferSize > 0 && wc && wc->conn) {
                printf("RECV: %d bytes on socket %d\n", pstrRecv->s16BufferSize, sock);

                // Crea un messaggio HTTP
                struct mg_http_message hm;
                memset(&hm, 0, sizeof(hm));

                // Copia i dati nel buffer del messaggio
                size_t len = pstrRecv->s16BufferSize;
                if (len > sizeof(hm.message.buf) - 1) {
                    len = sizeof(hm.message.buf) - 1;
                }
                memcpy(hm.message.buf, pstrRecv->pu8Buffer, len);
                hm.message.len = len;
                hm.message.buf[len] = '\0';

                // Copia anche l'URI (semplificato)
                hm.uri.buf = hm.message.buf;
                // Trova l'URI nel messaggio (ricerca semplificata)
                char *uri_start = strstr((char*)hm.message.buf, " /");
                if (uri_start) {
                    uri_start += 1; // Salta lo spazio
                    char *uri_end = strchr(uri_start, ' ');
                    if (uri_end) {
                        hm.uri.buf = uri_start;
                        hm.uri.len = uri_end - uri_start;
                    }
                }

                // Segnala a Mongoose che c'è un messaggio HTTP
                mg_call(wc->conn, MG_EV_HTTP_MSG, &hm);

                // Continua a ricevere
                recv(sock, wc->recv_buf, sizeof(wc->recv_buf), 0);
            } else if (pstrRecv && pstrRecv->s16BufferSize <= 0 && wc) {
                printf("Connection closed on socket %d\n", sock);
                if (wc->conn) {
                    wc->conn->is_closing = 1;
                }
                close(sock);
                wc->sock = -1;
            }
            break;
        }

        case SOCKET_MSG_SEND: {
            printf("SEND completed on socket %d\n", sock);
            // I dati sono stati inviati, possiamo procedere
            if (wc && wc->conn) {
                mg_call(wc->conn, MG_EV_WRITE, NULL);
            }
            break;
        }

        default:
            break;
    }
}

void winc_if_init(struct mg_mgr *mgr) {
    s_mgr = mgr;
    init_conn_table();
    printf("WINC interface initialized\n");
}

void winc_if_poll(struct mg_mgr *mgr, int timeout_ms) {
    // Gestisce gli eventi WINC
    m2m_wifi_handle_events(NULL);

    // Pulisce le connessioni chiuse
    for (int i = 0; i < MAX_SOCKETS; i++) {
        if (s_conns[i].sock != -1 && s_conns[i].conn && s_conns[i].conn->is_closing) {
            printf("Closing socket %d\n", s_conns[i].sock);
            close(s_conns[i].sock);
            s_conns[i].sock = -1;
            s_conns[i].conn = NULL;
        }
    }
}

// Crea un server HTTP
bool winc_create_http_server(uint16_t port) {
    SOCKET sock = create_listener(port);
    if (sock < 0) {
        return false;
    }

    // Registra il listener
    winc_conn_t *wc = find_free_slot();
    if (wc) {
        wc->sock = sock;
        wc->conn = NULL;  // I listener non hanno connessione Mongoose diretta
        wc->is_listener = true;

        printf("HTTP server created on port %u\n", port);
        return true;
    }

    close(sock);
    return false;
}
