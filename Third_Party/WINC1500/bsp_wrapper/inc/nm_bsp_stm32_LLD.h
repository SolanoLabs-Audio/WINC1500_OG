/*****************************************************************************
 * nm_bsp_stm32_LLD.h
 *
 *  Created on: Nov 12, 2025
 *   Copyright (C) 2022 Marco Solano (SOLANOLABS). All rights reserved.
 *   Author: Marco Solano <marco.solano@solanolabs.it>
 *
 *   Made for :
 *
 *****************************************************************************/
#ifndef WINC1500_BSP_WRAPPER_INC_NM_BSP_STM32_LLD_H_
#define WINC1500_BSP_WRAPPER_INC_NM_BSP_STM32_LLD_H_

  #ifdef __cplusplus
    extern "C" {
  #endif

    /*
     * Questo file definisce l'interfaccia hardware a basso livello (Low-Level Driver)
     * per il WINC1500 sulla NUCLEO-H743ZI2.
     * Gestisce l'accesso diretto ai pin e alle periferiche HAL.
     */

    #include "stm32h7xx_hal.h"
    #include "stdio.h" // Per printf

    /* ------------------------------------------------------------------------- */
    /* Handle delle periferiche (definite in main.c, generate da CubeMX)         */
    /* ------------------------------------------------------------------------- */

    /* Handle SPI per il WINC1500 (es. SPI1) */
    extern SPI_HandleTypeDef hspi1;
    #define hspiWinc (hspi1)

    /* Handle UART per il debug (es. USART3) */
    extern UART_HandleTypeDef huart3;
    #define huartDebug (huart3)


    /* ------------------------------------------------------------------------- */
    /* Definizione Pin (basata su NUCLEO-H743ZI2 + ATWINC1500-XPRO)             */
    /* ------------------------------------------------------------------------- */

    /* SPI1 (Header Arduino D11, D12, D13) */
    #define WINC_SPI_PORT           GPIOA
    #define WINC_SPI_PIN_SCK        GPIO_PIN_5
    #define WINC_SPI_PIN_MISO       GPIO_PIN_6
    #define WINC_SPI_PIN_MOSI       GPIO_PIN_7

    /* CS (Header Arduino D10) */
    #define WINC_CS_PORT            GPIOD
    #define WINC_CS_PIN             GPIO_PIN_14

    /* RESET (Header Arduino D4) */
    #define WINC_RESET_PORT         GPIOG
    #define WINC_RESET_PIN          GPIO_PIN_2

    /* CHIP_EN / WIFI_EN (Header Arduino D2) */
    #define WINC_CHIP_EN_PORT       GPIOG
    #define WINC_CHIP_EN_PIN        GPIO_PIN_3

    /* IRQ (Header Arduino D7) */
    #define WINC_IRQ_PORT           GPIOF
    #define WINC_IRQ_PIN            GPIO_PIN_13
    #define WINC_IRQ_EXTI_LINE      EXTI15_10_IRQn


    /* ------------------------------------------------------------------------- */
    /* Prototipi Funzioni LLD                                                    */
    /* ------------------------------------------------------------------------- */

    /**
     * @brief Inizializza l'hardware LLD (principalmente per il debug).
     */
    void WINC_LLD_Init(void);

    /**
     * @brief Controlla il pin CHIP_EN (WIFI_ENABLE).
     * @param state 1 (HIGH) per abilitare, 0 (LOW) per disabilitare.
     */
    void WINC_LLD_ChipEnable(uint8_t state);

    /**
     * @brief Esegue la sequenza di reset hardware del WINC1500.
     */
    void WINC_LLD_Reset(void);

    /**
     * @brief Funzione bloccante per la trasmissione/ricezione SPI tramite DMA.
     * @note  Questa è la funzione LLD centrale. È sincrona (bloccante)
     * ma usa il DMA per l'effettivo trasferimento.
     * @param pTx Buffer di trasmissione
     * @param pRx Buffer di ricezione
     * @param len Lunghezza della transazione in byte
     */
    void WINC_LLD_SpiTxRx(uint8_t* pTx, uint8_t* pRx, uint16_t len);

    /**
     * @brief Wrapper per HAL_Delay.
     * @param ms Millisecondi di attesa.
     */
    void WINC_LLD_Delay(uint32_t ms);

    /**
     * @brief Funzione di gestione chiamata dalla ISR EXTI (in stm32h7xx_it.c).
     * @param GPIO_Pin Il pin che ha generato l'interrupt.
     */
    void WINC_LLD_ExtiCallback(uint16_t GPIO_Pin);

  #ifdef __cplusplus
    }
  #endif
#endif
//WINC1500_BSP_WRAPPER_INC_NM_BSP_STM32_LLD_H_
