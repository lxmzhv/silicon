// Vectorial algebra

#pragma once

#include <points.h>

typedef float Trf_3d[3][4]; // transformation

void DLL_OBJ trf_init( Trf_3d trf );
void DLL_OBJ trf_copy( Trf_3d trf_out, const Trf_3d trf );
void DLL_OBJ trf_x_trf( const Trf_3d trf1, const Trf_3d trf2, Trf_3d trf_out );
void DLL_OBJ trf_rotate( const Trf_3d trf, float angle, bool vert, Trf_3d trf_out );
void DLL_OBJ trf_rotate_hv( const Trf_3d trf, float h_angle, float v_angle, Trf_3d trf_out );

template < class V1, class V2 >
void copy_vector( V1& v1, const V2& v2 )
{
   v1[0] = v2[0];
   v1[1] = v2[1];
   v1[2] = v2[2];
}

template<class V1, class V2>
void trf_x_pnt( const Trf_3d trf, const V1& pnt, V2& pnt_out )
{
   FPoint p;

   for( int i = 0; i < 3; ++i )
      p[i] = trf[i][0]*pnt[0] + trf[i][1]*pnt[1] + trf[i][2]*pnt[2] + trf[i][3];

   copy_vector( pnt_out, p );
}