#ifndef INC_MONGOOSE_CONFIG_H_
#define INC_MONGOOSE_CONFIG_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

// Configurazione minima
#define MG_ARCH MG_ARCH_CUSTOM
#define MG_ENABLE_POLL 0           // DISABILIAMO il poller interno
#define MG_ENABLE_TLS 0
#define MG_ENABLE_FILE 0
#define MG_ENABLE_IPV6 0
#define MG_ENABLE_LOG 1
#define MG_ENABLE_DAEMON 0
#define MG_ENABLE_BROADCAST 0
#define MG_ENABLE_SOCKET 0         // IMPORTANTE: disabilita socket POSIX

// Buffer sizes
#define MG_MAX_RECV_SIZE 1500
#define MG_IO_SIZE 1500

// AGGIUNGI: Disabilita la definizione di mg_millis in mongoose.c
#define MG_CUSTOM_MILLIS 1

// mg_millis() - Già fornito nel tuo main.c
#include <stdint.h>
extern uint64_t mg_millis(void);

#endif
