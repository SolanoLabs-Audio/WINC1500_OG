/*****************************************************************************
 * nm_bsp_stm32.c
 *
 *  Created on: Nov 12, 2025
 *   Copyright (C) 2022 Marco Solano (SOLANOLABS). All rights reserved.
 *   Author: Marco Solano <marco.solano@solanolabs.it>
 *
 *   Made for :
 *
 *****************************************************************************/

#include "nm_bsp_stm32.h"
#include "nm_bsp_stm32_LLD.h"

/* Header standard del WINC (devono essere nel path di include) */
#include "common/inc/nm_common.h"
#include "bus_wrapper/inc/nm_bus_wrapper.h"

/*
 * Puntatore globale alla funzione ISR del driver WINC.
 * Viene "registrato" tramite nm_bsp_register_isr()
 * e "chiamato" da WINC_LLD_ExtiCallback().
 */
static tpfNmBspIsr gpfIsr = NULL;


/* ------------------------------------------------------------------------- */
/* Funzioni locali di "glue" per il BUS WRAPPER (nm_spi_rw, ecc.)            */
/* ------------------------------------------------------------------------- */
/* * Il "bus_wrapper" (un file .c del driver Microchip) si aspetta
 * che noi forniamo queste funzioni con nomi specifici.
 */

/**
 * @brief Chiamata dal bus_wrapper per inizializzare l'hardware SPI.
 */
static int8_t
spi_init(void){

    /* L'init HAL/GPIO/SPI/DMA è già fatto da CubeMX.
     * Eseguiamo solo il reset hardware del modulo.
     */
    WINC_LLD_Reset();
    /* Abilita l'interrupt EXTI (già fatto in CubeMX) */
    return M2M_SUCCESS;
}
//



/**
 * @brief Chiamata dal bus_wrapper per de-inizializzare l'SPI.
 */
static int8_t
spi_deinit(void){

    /* Disabilita il modulo */
    WINC_LLD_ChipEnable(0);
    return M2M_SUCCESS;
}
//


/**
 * @brief Chiamata dal bus_wrapper per eseguire una R/W SPI.
 * Questa è la funzione "colla" più importante.
 */
static int8_t
spi_rw(uint8_t* pu8Mosi, uint8_t* pu8Miso, uint16_t u16Sz){

    /* * Il driver WINC riutilizza lo stesso buffer per MOSI e MISO
     * quando fa una lettura (MISO != NULL).
     * Se MISO è NULL, è solo una scrittura.
     * Se MOSI è NULL, è solo una lettura (ma MISO sarà usato come buffer TX/RX).
     *
     * WINC_LLD_SpiTxRx gestisce pTx e pRx separati.
     */

    if(pu8Miso){
        // Transazione R/W o R-only
        // Il driver WINC si aspetta che i dati di TX (pu8Mosi)
        // e i dati di RX (pu8Miso) siano nello stesso buffer (pu8Miso)
        // per risparmiare RAM.

        // Se pu8Mosi e pu8Miso puntano a buffer diversi (caso R/W)
        if (pu8Mosi != pu8Miso) {
             // Copia i dati da inviare nel buffer di ricezione,
             // perché HAL_SPI_TransmitReceive_DMA sovrascriverà pRx
             // mentre legge pTx.
             // Se il driver WINC è standard, pTx e pRx sono diversi.
             WINC_LLD_SpiTxRx(pu8Mosi, pu8Miso, u16Sz);
        } else {
             // Caso Read-Only (pu8Mosi == pu8Miso)
             // Dobbiamo inviare byte fittizi (spesso il primo è un comando)
             // e ricevere nello stesso buffer.
             WINC_LLD_SpiTxRx(pu8Miso, pu8Miso, u16Sz);
        }

    } else {
        // Transazione Write-Only (pu8Miso è NULL)
        // Dobbiamo fornire un buffer RX fittizio
        // (La libreria HAL H7 non ama un pRxData NULL nel DMA)
        // Usiamo lo stesso buffer TX, i dati ricevuti verranno scartati.
        WINC_LLD_SpiTxRx(pu8Mosi, pu8Mosi, u16Sz);
    }

    return M2M_SUCCESS;
}
//






/* ------------------------------------------------------------------------- */
/* Implementazione API BSP (chiamate dal driver WINC)                        */
/* ------------------------------------------------------------------------- */

int8_t
nm_bsp_init(void){

    /* 1. Inizializza l'hardware a basso livello (stampe, ecc.) */
    WINC_LLD_Init();

    /* * 2. Registra le nostre funzioni "glue" (spi_init, spi_rw)
     * all'interno del bus_wrapper del driver Microchip.
     */
    tstrNmBusCallbacks sstrBusCb = {
        .pfnSpiInit = spi_init,
        .pfnSpiDeinit = spi_deinit,
        .pfnSpiRw = spi_rw,
        // .pfnSpiWrite? .pfnSpiRead? -> nm_spi_rw gestisce tutto
    };

    /* 3. Inizializza il bus_wrapper */
    int8_t ret = nm_bus_init(NULL, &sstrBusCb);
    if (ret != M2M_SUCCESS) {
        NM_BSP_PRINTF("nm_bsp_init: Fallimento nm_bus_init\r\n");
        return ret;
    }

    return M2M_SUCCESS;
}


int8_t
nm_bsp_deinit(void){

    spi_deinit();
    return M2M_SUCCESS;
}
//



void
nm_bsp_sleep(uint32_t u32TimeMsec)
	{WINC_LLD_Delay(u32TimeMsec);}

void
nm_bsp_register_isr(tpfNmBspIsr pfIsr){

    NM_BSP_PRINTF("WINC BSP: Registrazione callback ISR\r\n");
    gpfIsr = pfIsr;
}
//

/* ------------------------------------------------------------------------- */
/* Gestione IRQ (Chiamato dal LLD)                                           */
/* ------------------------------------------------------------------------- */

/**
 * @brief Chiamato da WINC_LLD_ExtiCallback (nel LLD)
 * quando scatta l'interrupt EXTI.
 */
void
WINC_LLD_ExtiCallback(uint16_t GPIO_Pin){

    if(GPIO_Pin == WINC_IRQ_PIN){

        if(gpfIsr){
            /* * Chiama la funzione ISR del driver WINC
             * (es. hif_isr, che è stata registrata)
             */
            gpfIsr();

        }else
        	{NM_BSP_PRINTF("WINC BSP: IRQ ricevuto ma gpfIsr è NULL!\r\n");}
    }
}
//


















