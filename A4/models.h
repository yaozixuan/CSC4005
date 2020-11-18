#ifndef _MODELS
#define _MODELS

#include <memory.h>
#include <stdlib.h>
#include "const.h"

#define legal(x, n) ( (x)>=0 && (x)<(n) )

typedef struct TemperatureField
{
	int X_range, Y_range;
	double **t;
	double *storage;
}TemperatureField;

void deleteField(TemperatureField *field);

void newField(TemperatureField *field, int X_range, int Y_range, int sourceX, int sourceY)
{
	TemperatureField temp = *field;
	field->storage = malloc( sizeof(double) * X_range * Y_range );
	field->t = malloc( sizeof(double*) * X_range );
	field->X_range = X_range;
	field->Y_range = Y_range;
	int i, j;
	for (i=0; i<X_range; ++i)
		field->t[i] = &field->storage[i*Y_range];
	if (sourceX)
	{
		double scaleFactorX = (double)sourceX/X_range;
		double scaleFactorY = (double)sourceY/Y_range;
		for (i=0; i<X_range; ++i)
			for (j=0; j<Y_range; ++j)
				field->t[i][j] = temp.t[(int)(i*scaleFactorX)][(int)(j*scaleFactorY)];
		deleteField(&temp);
	}
	else memset(field->storage, 0, sizeof(double)*X_range*Y_range); /* memory set storage to be all 0 */
}

void initField(TemperatureField *field)
{
	int i, j;
	for (i=0; i<field->X_range; ++i)
		for (j=0; j<field->Y_range; ++j)
			field->t[i][j] = 20.0f;
}

void refreshField(TemperatureField *field, int initX, int initY, int thisX, int thisY, int allX, int allY){
	int j;
	for (j=allY*3/10; j<allY*7/10; ++j)
	    if (legal(-initX, thisX)&&legal(j-initY, thisY))
		field->t[-initX][j-initY] = 100.0f;
}

/*
TemperatureField* myClone(TemperatureField *field, int X, int Y)
{
	int i, j;
        TemperatureField *ret = malloc(sizeof(TemperatureField));
	ret->x = X;
	ret->y = Y;
	ret->storage = malloc(sizeof(double)*ret->x*ret->y);
	ret->t = malloc(sizeof(double*)*ret->x);
	for (i=0; i<ret->x; ++i)
		ret->t[i] = &ret->storage[i*ret->y];
	for (i=0; i<X; ++i)
		for (j=0; j<Y; ++j)
			ret->t[i][j] = field->t[i][j];
	return ret;
}
*/
void deleteField(TemperatureField *field)
{
	free(field->t);
	free(field->storage);
	//free(field);
}

#endif
