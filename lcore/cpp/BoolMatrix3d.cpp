// 3D matrix of bool values: 0 or 1

#include <BoolMatrix3d.h>
#include <view.h>

//
//class BoolMatrix3D
//

BoolMatrix3D::BoolMatrix3D( Point<ulong> sz, bool def_value )
{  
  Init( sz, def_value );
}

void BoolMatrix3D::Init( Point<ulong> sz, bool value )
{
  if( !data || sz != size )
  {
    size = sz;
    dataSize = Max( (sz.x*sz.y*sz.z+31)>>5, ulong(1) );
    data = new ulong[dataSize];
    if( !data )
      Error( "Невозможно выделить требуемое количество памяти!" );
  }
  Init( value );
}

void BoolMatrix3D::Init( bool value )
{
  memset( data, value ? 0xFF : 0, dataSize<<2 );
}

void BoolMatrix3D::Save( File& f )
{
  f << size;
  f.Write( (char*)(ulong*)data, dataSize*sizeof(ulong) );
}

bool BoolMatrix3D::Load( File& f )
{
  Point<ulong> sz;
  f >> sz;
  Init( sz );
  f.Read( (char*)(ulong*)data, dataSize*sizeof(ulong) );
  return true;
}

long BoolMatrix3D::CountValues( long z ) const
{
  SimplePoint<long> p;
  long cnt = 0;

  p.z = z;
  for( p.x = 0; p.x < (long)size.x; ++p.x )
    for( p.y = 0; p.y < (long)size.y; ++p.y )
      if( Value( p ) )
        ++cnt;

  return cnt;
}
