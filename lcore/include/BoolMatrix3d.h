#pragma once

#include <pointers.h>
#include <points.h>
#include <funcs.h>
#include <file.h>

// 3D matrix of bool values: 0 or 1
class DLL_OBJ BoolMatrix3D
{
  public:
    BoolMatrix3D() { Init( Point<ulong>(1,1,1) ); }
    BoolMatrix3D( Point<ulong> sz, bool def_value = false );
    void Init( Point<ulong> sz, bool value = false );
    void Init( bool value = false );
    inline bool Value( const SimplePoint<long>& p ) const;
    inline void SetValue( const SimplePoint<long>& p, bool value );
    inline bool Contains( const SimplePoint<long>& p ) const;
    inline const Point<ulong>& GetSize() const { return size; }
    long CountValues( long z ) const;

    virtual     void Save( File& f );
    virtual     bool Load( File& f );

  private:
    VP<ulong> data;
    ulong dataSize;
    Point<ulong> size;
};

//
// Inlines
//

inline bool BoolMatrix3D::Value( const SimplePoint<long>& p ) const
{
  static ulong offset, n, mask;
  offset = ulong( size.x*( size.y*p.z + p.y ) + p.x );
  n = Min( offset>>5, dataSize );
  mask = 1<<(offset&31);
  return (data[n] & mask) != 0;
}

inline void BoolMatrix3D::SetValue( const SimplePoint<long>& p,
                                    bool value )
{
  static ulong offset, n, mask;
  offset = ulong( size.x*( size.y*p.z + p.y ) + p.x );
  n = min( offset>>5, dataSize );
  mask = 1<<(offset&31);
  if( value )   data[n] |= mask;
  else          data[n] &= ~mask;
}

inline bool BoolMatrix3D::Contains( const SimplePoint<long>& p ) const
{
  return !( p.x<0 || p.y<0 || p.z<0 || p.x>=long(size.x) ||
            p.y>=long(size.y) || p.z>=long(size.z) );
}
