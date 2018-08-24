/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <GPIO.h>
#include "Keyboard.h"
#include "Display.h"
#include "CardReader.h"
#include "Multiplexer.h"
#include "StateMachine.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/

static const int MUX_FREQ = 100;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

static void Systick_Handler(void);


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//
void App_Init (void)
{
	// Init drivers
	initKeyboard();
	initDisplay();
	initCardReader();
	initMultiplexer(MUX_FREQ);
	//initBuzzer();
}

// 
void App_Run (void)
{
	// Get events from keyboard and magnetic card
	
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void Systick_Handler()
{
	multiplexerPISR();
}
