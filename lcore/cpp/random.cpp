// Randomize functions

#include <funcs.h>

double uniform_distrib( double a, double b )
{
  return a + (b-a)*double( rand()%(RAND_MAX+1) ) / RAND_MAX;
}

double normal_distrib( double mean, double sigma )
{
  static double x, y, z, s;

  // Using Box-Muller transform
  do
  {
    x = uniform_distrib( -1, 1 );
    y = uniform_distrib( -1, 1 );
    s = x*x + y*y;
  }
  while( s > 1 || s < NULL_DOUBLE );

  z = x * sqrt( -2.0*log(s)/s );

  return mean + z*sigma;
}

// Get random event
// Input:
//    events - array of events' probabilities:
//             1. events[i] >= 0
//             2. sum of events[i] > 0
//    cnt - number of events
// Result:
//    index of event, that has happened
int random_event( const double* events, int cnt )
{
   static double sum, rand_value;
   static int i;

   sum = 0;
   for( i = 0; i < cnt; ++i )
      sum += events[i];

   rand_value = uniform_distrib( 0, sum );

   sum = 0;
   for( i = 0; i < cnt-1; ++i )
   {
      sum += events[i];
      if( rand_value <= sum )
         return i;
   }

   return cnt-1;
}