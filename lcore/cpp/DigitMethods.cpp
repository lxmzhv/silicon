#include <DigitMethods.h>

// ���������� ������ ����������������� ������������ ��
// ��������� �����, �.�. ����� ������, ����� ���������
// ��������� �� ������� �� ���� ����� ����������.
Line<double> CalcMiddleSqrLineFunc( int count,
                                    Point2D<double>* points,
                                    bool from_zero )
{
   if( !points || count <= 0 )   return Line<double>();

   // ���� ����� �������� �������
   // G(a,b,c) = Sum( sqr(a*xi + b*yi + c) ),
   // ��� Sum - ������������ �� i
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

   // ������ ������ a = cos(fi), b = sin(fi),
   // �.�. ��������, ��� sqr(a) + sqr(b) == 1.
   // ������������������ G �� fi � �� �, ������� �����,
   // �������������� �� ��������� ���������. ����� ��� ����
   // ���� �� ���� ����� ���������� ��������, �.�.
   // G - ���������� ����� � ����������.
   double p = y2_sum - x2_sum + (sqr(x_sum) - sqr(y_sum))/count;
   double q = 2*( xy_sum - x_sum*y_sum/count );
   double fi = IsNullNum(p) ? M_PI/4 : atan(-q/p)/2;

   // �������� ������ ����� ����������: fi + k*Pi/2, k: 0-3
   // ������� �� ��� ����� � ����������� ��������� �������
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

   // �, �������, ��������� ������. ������
   double a = cos(best_fi), b = sin(best_fi);
   double c = from_zero ? 0 : -(a*x_sum + b*y_sum)/count;
   return Line<double>( a, b, c );
}
