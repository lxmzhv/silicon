// Действия над числами
#include <funcs.h>


// Перевести вещественное число в строку
DLL_OBJ char* ftoa( double d, char* buf, ulong sz );

DLL_OBJ double log2( double a );
DLL_OBJ int log2( int a );

DLL_OBJ double CalcGridStep( double min, double max, int num, double* start );