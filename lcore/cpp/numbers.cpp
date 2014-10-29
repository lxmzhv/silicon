// Действия над числами
#include <numbers.h>
#include <string.h>

DLL_OBJ char* ftoa( double d, char* buf, ulong sz )
{
   static char buffer[0x100];
   int dec = 0, sign = 0;
   *buf = 0;
   _fcvt_s( buffer, 0x100, d, 7, &dec, &sign );
   char* res = buffer;
   if( res || strlen(res)+2 > sz )
   {
      if( dec <= 0 )
      {
         memset( buf, '0', 2-dec );
         buf[1] = '.';
         strcpy_s( buf+2-dec, sz-(2-dec), res );
      }
      else
      {
         strncpy_s( buf, sz, res, dec );
         buf[dec] = '.';
         strcpy_s( buf+dec+1, sz-(dec+1), res+dec );
      }
      char* end = buf+strlen(buf)-1;
      for( ; end > buf && *end == '0'; end-- )
         *end = 0;
      if( *end == '.' )
         *end = 0;
      if( *buf == 0 )
         strcpy_s( buf, sz, "0" );
      if( sign )
      {
         memmove( buf+1, buf, strlen(buf)+1 );
         *buf = '-';
      }
   }
   return buf;
}

DLL_OBJ double log2( double a )
{
   static const double LOG2 = log(2.0);
   return log(a)/LOG2;
}

DLL_OBJ int log2( int a )
{
   int d = 0;
   for( ; a > 1; ++d )
      a >>= 1;
   return d;
}

double CalcGridStep( double min, double max, int num, double* start )
{
  double len = max - min;
  double step = len / num;

  double scale = 1.0;
  while( step < 1.0 )
  {
    step *= 10;
    scale *= 10;
  }
  while( step >= 10.0 )
  {
    step /= 10;
    scale /= 10;
  }

  step = int(step + 0.5 - NULL_DOUBLE) / scale;
  *start = int(min*scale + 0.5 - NULL_DOUBLE) / scale;

  return step;
}