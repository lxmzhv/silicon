// Различные численные методы

#pragma once

#include <math.h>
#include <points.h>

// Рассчитать прямую среднеквадратично приближенную к множеству точек
// Т.е. такую прямую, сумма квадратов растояний от которой до всех точек минимальна
// from_zero - прямая проходит через ноль
DLL_OBJ Line<double> CalcMiddleSqrLineFunc( int count, Point2D<double>* points, bool from_zero = false );
