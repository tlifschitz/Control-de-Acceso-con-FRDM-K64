#ifndef __CIRCULARBUFFER_H_
#define __CIRCULARBUFFER_H_

#include <stdbool.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//               ITBA - Laboratorio de Microprocesadores - 2018
//                                   Grupo 3
//
//
//
// Source: https://embedjournal.com/implementing-circular-buffer-embedded-c/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
typedef struct 
{
	char * const buffer;
	int head;
	int tail;
	const int maxlen;
} CircularBuffer;



// Macro to allocate memory and create structure
#define CIRCULAR_BUFFER_NEW(name,size) __CIRCULAR_BUFFER_NEW(name,size)


// Push data to buffer, return false if full, true instead.
bool push(CircularBuffer *this,char data);

// Pop data from the buffer, return false if empty, true.
bool pop(CircularBuffer *this,char * data); 

// Return true if the buffer is empty
bool isEmpty(CircularBuffer *this);

// Return true if the buffer is full
bool isFull(CircularBuffer *this);

#endif // __CIRCULARBUFFER_H_

