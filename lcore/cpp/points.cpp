
#include "points.h"
#include "matrix.h"

//
//class Plane
//

DPlane::DPlane( DPoint& p1, DPoint& p2, DPoint& p3 )
{
   DPoint v1 = p1 - p2, v2 = p1 - p3;
   a = v1.y*v2.z - v1.z*v2.y;
   b = v1.z*v2.x - v1.x*v2.z;
   c = v1.x*v2.y - v1.y*v2.x;
   d = -(a*p1.x + b*p1.y + c*p1.z);
}

bool DPlane::Cross( DPoint& p1, DPoint& p2, DPoint& res )
{
   DPoint v = p2 - p1;
   if( v.x*a + v.y*b + v.z*c == 0 )   return false;
   double darr[] = { v.y, -v.x,    0, v.y*p1.x - v.x*p1.y,
                     v.z,    0, -v.x, v.z*p1.x - v.x*p1.z,
                       0,  v.z, -v.y, v.z*p1.y - v.y*p1.z,
                       a,    b,    c,                  -d };
   TrueDrob arr[16];
   for( int i = 0; i < 16; i++ )
      arr[i].Init( darr[i] );

   Matrix m( 4, 4, arr );
   DoGauss( m );
   res.x = m[3].Double();
   res.y = m[7].Double();
   res.z = m[11].Double();
   return true;
}

DPoint DPlane::Cross( DPoint& p1, DPoint& p2 )
{
   DPoint res;
   Cross( p1, p2, res );
   return res;
}

double Length( const Point<TrueDrob>& p )
{
   return sqrt( (sqr(p.x)+sqr(p.y)+sqr(p.z)).Double() ); 
}