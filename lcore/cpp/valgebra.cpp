// Vectorial algebra

#include <valgebra.h>

void trf_init( Trf_3d trf )
{
   for( int i = 0; i < 3; ++i )
      for( int j = 0; j < 4; ++j )
         trf[i][j] = i == j ? 1.0f : 0.0f;
}

void trf_copy( Trf_3d trf_out, const Trf_3d trf )
{
   for( int i = 0; i < 3; ++i )
      for( int j = 0; j < 4; ++j )
         trf_out[i][j] = trf[i][j];
}

void trf_x_trf( const Trf_3d trf1, const Trf_3d trf2, Trf_3d trf_out )
{
   static Trf_3d trf;

   for( int i = 0; i < 3; ++i )
      for( int j = 0; j < 3; ++j )
         trf[j][i] = trf1[j][0]*trf2[0][i] + trf1[j][1]*trf2[1][i] + trf1[j][2]*trf2[2][i];

   for( int i = 0; i < 3; ++i )
      trf[i][3] = trf1[i][3] + trf2[i][3];

   trf_copy( trf_out, trf );
}

void trf_rotate( const Trf_3d trf_in, float angle, bool vert, Trf_3d trf_out )
{
   static float cos_a, sin_a;
   static Trf_3d trf;

   if( fabs(angle) < NULL_DOUBLE )
      return;

   trf_init( trf );

   sin_a = (float)sin(angle);
   cos_a = (float)cos(angle);

   if( vert )
   {
      trf[0][0] = trf[2][2] = cos_a;
      trf[0][2] = -sin_a;
      trf[2][0] = sin_a;
   }
   else
   {
      trf[0][0] = trf[1][1] = cos_a;
      trf[0][1] = sin_a;
      trf[1][0] = -sin_a;
   }

   trf_x_trf( trf, trf_in, trf_out );
}

void trf_rotate_hv( const Trf_3d trf, float h_angle, float v_angle, Trf_3d trf_out )
{
   trf_rotate( trf, h_angle, false, trf_out );
   trf_rotate( trf, v_angle, true, trf_out );
}
