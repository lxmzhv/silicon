#include <DigitMethods.h>

// Рассчитать прямую среднеквадратично приближенную ко
// множеству точек, т.е. такую прямую, сумма квадратов
// растояний от которой до всех точек минимальна.
Line<double> CalcMiddleSqrLineFunc( int count,
                                    Point2D<double>* points,
                                    bool from_zero )
{
   if( !points || count <= 0 )   return Line<double>();

   // Ищем точку минимума функции
   // G(a,b,c) = Sum( sqr(a*xi + b*yi + c) ),
   // где Sum - суммирование по i
   // G(a,b,c) = sqr(a)*x2_sum + sqr(b)*y2_sum + count*sqr(c) +
   // + 2*a*b*xy_sum + 2*a*c*x_sum + 2*b*c*y_sum
   double x2_sum = 0, y2_sum = 0;
   double xy_sum = 0, x_sum = 0, y_sum = 0;
   for( int i = 0; i < count; i++ )
   {
      x2_sum += sqr( points[i].x );
      y2_sum += sqr( points[i].y );
      xy_sum += points[i].x * points[i].y;
      if( !from_zero )
      {
         x_sum += points[i].x;
         y_sum += points[i].y;
      }
   }

   // Делаем замену a = cos(fi), b = sin(fi),
   // т.к. полагаем, что sqr(a) + sqr(b) == 1.
   // Продифферинцировав G по fi и по с, находим точки,
   // подозрительные на локальный экстремум. Среди них есть
   // хотя бы одна точка локального минимума, т.к.
   // G - ограничена снизу и непрерывна.
   double p = y2_sum - x2_sum + (sqr(x_sum) - sqr(y_sum))/count;
   double q = 2*( xy_sum - x_sum*y_sum/count );
   double fi = IsNullNum(p) ? M_PI/4 : atan(-q/p)/2;

   // Получили четыре точки экстремума: fi + k*Pi/2, k: 0-3
   // Выберем из них точку с минимальным значением функции
   double best_fi = 0, min_value = 0;
   for( int i = 0; i < 4; i++, fi += M_PI/2 )
   {
      double a = cos(fi), b = sin(fi);
      double c = from_zero ? 0 : -(a*x_sum + b*y_sum)/count;
      double val = sqr(a)*x2_sum + sqr(b)*y2_sum +
                   count*sqr(c) + 2*a*b*xy_sum +
                   2*a*c*x_sum + 2*b*c*y_sum;
      if( i==0 || val < min_value )
      {
         best_fi = fi;
         min_value = val;
      }
   }

   // И, наконец, вычисляем коээфф. прямой
   double a = cos(best_fi), b = sin(best_fi);
   double c = from_zero ? 0 : -(a*x_sum + b*y_sum)/count;
   return Line<double>( a, b, c );
}
