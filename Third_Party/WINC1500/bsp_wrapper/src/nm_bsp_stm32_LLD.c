/*****************************************************************************
 * nm_bsp_stm32_LLD.c
 *
 *  Created on: Nov 12, 2025
 *   Copyright (C) 2022 Marco Solano (SOLANOLABS). All rights reserved.
 *   Author: Marco Solano <marco.solano@solanolabs.it>
 *
 *   Made for :
 *
 *****************************************************************************/

#include "nm_bsp_stm32_LLD.h"
#include <string.h> // Per strlen

/* * Flag volatile per la sincronizzazione DMA in bare-metal.
 * 0 = DMA occupato, 1 = DMA completato.
 */
static volatile uint8_t g_spiDmaTxRxCplt = 0;

/* Handle HAL (dichiarati 'extern' nel .h, definiti in main.c) */
SPI_HandleTypeDef hspi1;
UART_HandleTypeDef huart3;

/**
 * @brief Implementazione syscall _write per reindirizzare printf su UART.
 */
#ifdef __GNUC__
int _write(int fd, char* ptr, int len)
{
    HAL_UART_Transmit(&huartDebug, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}
#endif

/* ------------------------------------------------------------------------- */
/* Implementazione Funzioni LLD                                              */
/* ------------------------------------------------------------------------- */

void WINC_LLD_Init(void)
{
    /* L'inizializzazione di HAL, Clock, GPIO, SPI, DMA e UART
     * è già stata eseguita da CubeMX in main.c
     */
    printf("WINC LLD: Inizializzato (NUCLEO-H743ZI2)\r\n");
}

void WINC_LLD_ChipEnable(uint8_t state)
{
    if (state) {
        HAL_GPIO_WritePin(WINC_CHIP_EN_PORT, WINC_CHIP_EN_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(WINC_CHIP_EN_PORT, WINC_CHIP_EN_PIN, GPIO_PIN_RESET);
    }
}

void WINC_LLD_Reset(void)
{
    printf("WINC LLD: Resetting module...\r\n");

    // Sequenza di reset
    WINC_LLD_ChipEnable(0);
    HAL_GPIO_WritePin(WINC_RESET_PORT, WINC_RESET_PIN, GPIO_PIN_RESET);
    WINC_LLD_Delay(50); // T_reset > 10ms

    WINC_LLD_ChipEnable(1);
    WINC_LLD_Delay(10);

    HAL_GPIO_WritePin(WINC_RESET_PORT, WINC_RESET_PIN, GPIO_PIN_SET);
    WINC_LLD_Delay(50); // Attendere che il modulo sia pronto
}

void WINC_LLD_SpiTxRx(uint8_t* pTx, uint8_t* pRx, uint16_t len)
{
    if (len == 0) return;

    /* 1. Imposta la flag di completamento DMA a 'occupato' */
    g_spiDmaTxRxCplt = 0;

    /* 2. Abbassa CS (Attivo Basso) */
    HAL_GPIO_WritePin(WINC_CS_PORT, WINC_CS_PIN, GPIO_PIN_RESET);

    /* 3. Avvia la transazione DMA */
    /* NOTA: Senza D-Cache, non servono funzioni di pulizia/invalidazione cache */
    if (HAL_SPI_TransmitReceive_DMA(&hspiWinc, pTx, pRx, len) != HAL_OK) {
        // Errore grave nell'avvio del DMA
        printf("WINC LLD: Errore avvio SPI DMA!\r\n");
        // Gestire l'errore (es. loop infinito per debug)
        while(1);
    }

    /* 4. Attendi il completamento (polling della flag) */
    /* Questo rende la funzione "bloccante" per il driver WINC,
     * ma la CPU è libera durante il trasferimento DMA.
     */
    while (g_spiDmaTxRxCplt == 0) {
        // In un'app bare-metal, questo è un loop di attesa.
        // In futuro, qui si potrebbero eseguire altre task veloci.
    }

    /* 5. Alza CS */
    HAL_GPIO_WritePin(WINC_CS_PORT, WINC_CS_PIN, GPIO_PIN_SET);
}

void WINC_LLD_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

/* ------------------------------------------------------------------------- */
/* Gestori Callback HAL                                                      */
/* ------------------------------------------------------------------------- */

/**
 * @brief Callback di completamento transazione SPI (chiamata da ISR DMA).
 * @note Questa funzione DEVE essere definita, poiché è una callback HAL __weak.
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef* hspi)
{
    if (hspi->Instance == hspiWinc.Instance) {
        g_spiDmaTxRxCplt = 1; // Imposta la flag 'completato'
    }
}

/**
 * @brief Callback di errore SPI.
 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == hspiWinc.Instance) {
        printf("WINC LLD: Errore SPI/DMA! Codice: 0x%lX\r\n", hspi->ErrorCode);
        while(1);
    }
}
