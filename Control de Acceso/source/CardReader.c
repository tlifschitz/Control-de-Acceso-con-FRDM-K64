/////////////////////////////////////////////////////////////////////////////////
//                             Included header files                           //
/////////////////////////////////////////////////////////////////////////////////
#include "CardReader.h"
#include "GPIO.h"
#include <stdbool.h>
#include <stdlib.h>
/////////////////////////////////////////////////////////////////////////////////
//                       Constants and macro definitions                       //
/////////////////////////////////////////////////////////////////////////////////
#define SS_char 0x0B
#define FS_char 0x0D
#define ES_char 0x0F
#define DATA_BUFFER_LENGTH 40
#define MIN_BUFFER_DATA 7	// minimum data that needs to be copied to the buffer before error
#define WORD_SIZE 5	// size in bits of each word
#define WORD_PARITY_BIT(x) (1 & ((x)>>(WORD_SIZE-1)) )

/////////////////////////////////////////////////////////////////////////////////
//                   Local function prototypes ('static')                      //
/////////////////////////////////////////////////////////////////////////////////
static void enableCallback(void);
static void newBit(void);
static void dataSignalChanged(void);
static void processWord(uint8_t w);
static uint8_t checkOddParity(uint8_t w);
static void processRawData(void);
static _Bool checkLRCparity();
void setError(_Bool s);
uint64_t myPow(uint64_t b, uint64_t e);
/////////////////////////////////////////////////////////////////////////////////
//                   Local variable definitions ('static')                     //
/////////////////////////////////////////////////////////////////////////////////

typedef enum crState_ENUM {
	idle,
	searchingSS,
	savingData,
	waitingLRC,
	dataReady
} CR_STATE;

static uint8_t buffer[DATA_BUFFER_LENGTH];
static CR_STATE crState;
static	int8_t shiftCount;
static	uint8_t word;
static	uint8_t buffIndex;
static 	uint8_t dataValue;	//Value of the last input bit
static 	_Bool error;



void initCardReader()
{

	pinMode(PIN_ENABLE, INPUT);
	pinMode(PIN_CLOCK, INPUT);
	pinMode(PIN_DATA, INPUT);

	pinConfigureIRQ (PIN_ENABLE, IRQC_INTERRUPT_EITHER	, enableCallback );
	pinConfigureIRQ (PIN_CLOCK, IRQC_INTERRUPT_RISING	, newBit);
	pinConfigureIRQ (PIN_DATA, IRQC_INTERRUPT_FALLING	, dataSignalChanged );
	crState = idle;
}

CR_DATA isDataReady(void)
{
	CR_DATA retVal = _crNoData;

	if(error == true)
		retVal = _crError;
	else if(crState == dataReady)
		retVal = _crDataOk;

	error = false;
	return retVal;
}

void setError(_Bool s)		// for debugging
{
	if(s == true)
		error = true;
	else
		error = false;
}

_Bool cardInserted(void)
{
	_Bool retVal = false;
	if(digitalRead(PIN_ENABLE)==LOW)
		retVal = true;

	return retVal;
}

static void enableCallback(void)
{
	if(digitalRead(PIN_ENABLE)==HIGH)//in the raising edge case the raw data block obtained is processed
	{
		processRawData();
	}
	else	//in the falling edge case the card is starting to be passed, so the driver resets
	{
		crState = searchingSS;
		shiftCount=0;		// to fix the offset of the first clock of the card
		buffIndex=0;
		word=0;
		dataValue=0;
		setError(false);
		for(uint8_t i=0 ; i<DATA_BUFFER_LENGTH ; i++)
			buffer[i]=0;
	}
}


static void newBit(void)
{
	if(crState == searchingSS)
	{
		word = word>>1;
		word |= (dataValue & 1) << 4;
		if( (word & 0x0F) == SS_char)
		{
			crState = savingData;
			processWord(word);
			word = 0;
		}
	}
	else if(crState == savingData || crState == waitingLRC)
	{
		if(shiftCount == WORD_SIZE )
		{
			shiftCount=0;
			word=0;
		}
		word |= (dataValue & 1) << shiftCount;

		if(++shiftCount == WORD_SIZE)
		{
			processWord(word);
			if(crState == savingData && (word & 0x0F) == ES_char)	// if the End Sentinel was saved, stop saving data and wait for LRC
				crState = waitingLRC;
			else if(crState == waitingLRC)
				crState = dataReady;
		}
	}
	dataValue = 0;
}

static void dataSignalChanged(void)
{
	dataValue=1;//if there was a transition between rising edges the corresponding data bit is 1
}

static void processWord(uint8_t w)
{
	if(checkOddParity(w))
	{
		w &= 0x0F;	//clear MS nibble
		if(buffIndex < 40)
			buffer[buffIndex++]=w;
		else
			setError(true);
	}
	else
		setError(true);

}

static uint8_t checkOddParity(uint8_t w)
{
	uint8_t parity = 1;
	uint8_t word2check = w;
	for(uint8_t i=1 ; i<5 ; i++)
		word2check ^= w>>i;

	parity &= word2check;

	return parity;
}


static void processRawData(void)
{
	if(buffIndex < MIN_BUFFER_DATA ||checkLRCparity() == false)
		setError(true);
}

static _Bool checkLRCparity()
{
	_Bool retVal = true;

	uint8_t parityAcc = 0;
	for(uint8_t i = 0 ; i < DATA_BUFFER_LENGTH - 1 ; i++)
	{
		parityAcc ^= buffer[i];
	}
	if(parityAcc != buffer[DATA_BUFFER_LENGTH-1])
		retVal = false;

	return retVal;
}

uint8_t getCardString(uint8_t * array)
{
	uint8_t i = 0;
	if(array != NULL && crState == dataReady && error == false)
	{
		while( buffer[i+1] != FS_char && i < DATA_CARD_NUMBER_LENGTH)
		{
			++i;
			array[i-1] = buffer[i];
		}
	}

	crState = idle;
	return i;
}

uint64_t getCardNumber()
{
	char array[20];

	uint8_t i = 0;
	if(array != NULL && crState == dataReady && error == false)
	{
		while( buffer[i+1] != FS_char && i < DATA_CARD_NUMBER_LENGTH)
		{
			++i;
			array[i-1] = buffer[i];
		}
	}
	array[i]='\0';
	crState = idle;

	uint64_t n=0;
	uint8_t len=i;
	uint64_t exp;
	while(i>0)
	{
		exp = myPow(10,i-1);
		n += (uint64_t)array[len-i]*exp;
		i--;
	}

	return(n);
}

uint64_t myPow(uint64_t b, uint64_t e)
{
	if(e==0)
		return 1;
	uint64_t n=b;
	for(int i=0; i+1<e; i++)
		n*=b;
	return n;
}
