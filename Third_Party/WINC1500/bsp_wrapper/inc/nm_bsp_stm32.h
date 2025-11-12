/*****************************************************************************
 * nm_bsp_stm32.h
 *
 *  Created on: Nov 12, 2025
 *   Copyright (C) 2022 Marco Solano (SOLANOLABS). All rights reserved.
 *   Author: Marco Solano <marco.solano@solanolabs.it>
 *
 *   Made for :
 *
 *****************************************************************************/
#ifndef WINC1500_BSP_WRAPPER_INC_NM_BSP_STM32_H_
#define WINC1500_BSP_WRAPPER_INC_NM_BSP_STM32_H_

  #ifdef __cplusplus
    extern "C" {
  #endif

    /*
     * Questo file definisce l'API BSP (Board Support Package)
     * richiesta dal driver WINC1500. Funge da "traduttore"
     * tra le chiamate del driver e il nostro LLD.
     */

    #include <stdint.h>

    /* Header standard del driver WINC (deve essere nel path di include) */
    /* Contiene la definizione di 'tpfNmBspIsr' */
    #include "nm_bsp.h"

    /* Debug printf (se necessario) */
    #define CONF_WINC_DEBUG       1
    #if CONF_WINC_DEBUG
    #   include "stdio.h"
    #   define NM_BSP_PRINTF      printf
    #else
    #   define NM_BSP_PRINTF(...)
    #endif


    /* ------------------------------------------------------------------------- */
    /* Prototipi Funzioni API BSP                                                */
    /* ------------------------------------------------------------------------- */

    /**
     * @brief Inizializza il BSP. Chiamato da m2m_wifi_init.
     * @return 0 in caso di successo, < 0 in caso di errore.
     */
    int8_t nm_bsp_init(void);

    /**
     * @brief De-inizializza il BSP.
     * @return 0 in caso di successo.
     */
    int8_t nm_bsp_deinit(void);

    /**
     * @brief Mette in pausa l'esecuzione (wrapper per delay).
     * @param u32TimeMsec Millisecondi di attesa.
     */
    void nm_bsp_sleep(uint32_t u32TimeMsec);

    /**
     * @brief Registra la funzione di callback ISR del driver WINC.
     * @param pfIsr Puntatore alla funzione (del driver) da chiamare
     * quando si verifica l'interrupt hardware (IRQ).
     */
    void nm_bsp_register_isr(tpfNmBspIsr pfIsr);

  #ifdef __cplusplus
    }
  #endif
#endif
//WINC1500_BSP_WRAPPER_INC_NM_BSP_STM32_H_
