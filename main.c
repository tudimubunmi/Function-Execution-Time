/* Description: Program for timing matrix multiplication operation C = A x B.
 *
 * Author: Bunmi Oluwatudimu
 * Date created: February 2020 
 * Date modified:
*******************************************************************************/

/* DriverLib includes. */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard includes. */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/* User-specific includes. */
//#include "uart_functions.h"

uint32_t elapsedTime;
uint32_t start;
uint32_t end;
uint8_t interval;
uint32_t i;

/* Size of the N x N matrix and the min and max values in the matrix. */
#define MATRIX_SIZE 40
#define MIN_VALUE 5
#define MAX_VALUE 10

void matrixMult (int *, int *, int *, unsigned int);
void populateMatrix (int *, int, int, unsigned int);
void tic (void);
void toc (void);
void delay (unsigned int);

/* Data structures for the matrices. */
int A[MATRIX_SIZE * MATRIX_SIZE];
int B[MATRIX_SIZE * MATRIX_SIZE];
int C[MATRIX_SIZE * MATRIX_SIZE];

const eUSCI_UART_Config uartConfig=
{
 EUSCI_A_UART_CLOCKSOURCE_SMCLK,
 78,//BRDIV
 2, //UCxBRF
 0,//UCxBRS
 EUSCI_A_UART_NO_PARITY,
 EUSCI_A_UART_LSB_FIRST,
 EUSCI_A_UART_ONE_STOP_BIT,
 EUSCI_A_UART_MODE,
 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION

};

void EUSCIA0_IRQHandler(void)
    {
        uint32_t status=UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
        UART_clearInterruptFlag(EUSCI_A0_BASE,status);
        if (status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG)
        {
        interval=UART_receiveData(EUSCI_A0_BASE);
        UART_transmitData(EUSCI_A0_BASE, interval);
        UART_transmitData(EUSCI_A0_BASE, '\r');
        UART_transmitData(EUSCI_A0_BASE, '\n');

        }

    }


int
main (void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer ();

    // P1.2 and P1.3 are UART TXD and RXD. These pins must be put in special function mode
        GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN2|GPIO_PIN3,GPIO_PRIMARY_MODULE_FUNCTION);
        //Sets DCO frequency at 12 MHz to get correct baud rate of 9600
        //CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
        MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
        MAP_UART_enableModule(EUSCI_A0_BASE);
        UART_enableInterrupt(EUSCI_A0_BASE,EUSCI_A_UART_RECEIVE_INTERRUPT);
        Interrupt_enableInterrupt(INT_EUSCIA0);
        Interrupt_enableMaster();



    /* Initialize the high-speed clock system. */
    CS_setDCOCenteredFrequency (CS_DCO_FREQUENCY_12); // Set DCO to 12 MHz
    CS_initClockSignal (CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_4);
   // CS_initClockSignal (CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //CS_initClockSignal (CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); // Set SMCLK to 12MHz for UART comms.
    FlashCtl_setWaitState(FLASH_BANK0,2);
    FlashCtl_setWaitState(FLASH_BANK1,2);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ,GPIO_PIN3|GPIO_PIN2,GPIO_PRIMARY_MODULE_FUNCTION);
    Timer32_initModule(TIMER32_0_BASE,TIMER32_PRESCALER_1,TIMER32_32BIT,TIMER32_PERIODIC_MODE);
    Timer32_setCount(TIMER32_0_BASE, 2000000) ;
    Timer32_startTimer(TIMER32_0_BASE,0);

    /* Initialize the UART module. */
   // initUART ();
    //writeString ("Established communication with the board");

    srand (rand()); /* Seed the random number generator. */

    while (1) {
        populateMatrix (A, MIN_VALUE, MAX_VALUE, MATRIX_SIZE); /* Populate matrices with random numbers. */
        populateMatrix (B, MIN_VALUE, MAX_VALUE, MATRIX_SIZE);

       // writeString ("\n\rMultiplying matrices");

        tic(); /* Time the multiplication operation. */
        for (i=0;i<10000;i++); //delay function
        matrixMult(A, B, C, MATRIX_SIZE);
        toc();

        //writeString ("\n\rDone multiplying");

        /* FIXME: Display elapsed time on the terminal. */
        elapsedTime = start - end;

        interval = (uint8_t) (elapsedTime/300000);
        //interval=UART_receiveData(EUSCI_A0_BASE);
        UART_transmitData(EUSCI_A0_BASE, interval+0x30);
        UART_transmitData(EUSCI_A0_BASE, '\r');
        UART_transmitData(EUSCI_A0_BASE, '\n');

        delay (1000); /* Delay 1s before starting again. */
   }
}

/* Function populates the input matrices with random FP values. */
void
populateMatrix (int *M, int min, int max, unsigned int dim)
{
   unsigned int i, j;
   for (i = 0; i < dim; i++) {
      for (j = 0; j < dim; j++) {
         M[i * dim + j] = ceil (min + (max - min) * rand ()/(float) RAND_MAX);
      }
   }
   return;
}

/* Function performs the matrix multiplication C = A x B */
void
matrixMult (int *A, int *B, int *C, unsigned int dim)
{
    unsigned int i, j, k;
    int temp;
    for (i = 0; i < dim; i++) {
        for (j = 0; j < dim; j++) {
            temp = 0.0;
            for (k = 0; k < dim; k++)
                temp += A[i * dim + k] * B[k * dim + j];

            C[i * dim + j] = temp;
        }
    }

    return;
}

/* The tic function. */
void
tic (void)
{
    /* FIXME: Complete the function. */
    start = Timer32_getValue(TIMER32_0_BASE);
    return;
}

/* The toc function. */
void
toc (void)
{
    /* FIXME: Complete the function. */
    end = Timer32_getValue(TIMER32_0_BASE);
    return;
}


/* Function implements a delay for the specified number of millseconds. */
void
delay (unsigned int msecs)
{
    unsigned int i;
    for (i = 0; i < 275 * msecs; i++);
    return;
}
