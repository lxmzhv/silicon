#pragma once

#include <funcs.h>
#include <pointers.h>
#include <points.h>

template< class T >
class Matrix3d
{
   public:
                      Matrix3d() { Init(Point<ulong>(1,1,1)); }
                      Matrix3d( const Point<ulong>& sz, const T& value = 0 ) { Init(sz, value); }
                 void Init( const Point<ulong>& sz, const T& value = 0 );
                 void Init( const T& value = 0 );
            inline T& GetElem( const Point<long>& p ) { return data[(p.z*size.y + p.y)*size.x + p.x]; }
      inline const T& GetElem( const Point<long>& p ) const { return data[(p.z*size.y + p.y)*size.x + p.x]; }

   private:
             VP<T> data;
      Point<ulong> size;

      void operator = ( const Matrix3d<T>& m ) {}
};

template< class T >
void Matrix3d<T>::Init( const Point<ulong>& sz, const T& value )
{
   size = sz;
   data = new T[sz.x * sz.y * sz.z];

   Init( value );
}

template< class T >
void Matrix3d<T>::Init( const T& value )
{
   ulong buf_size = size.x * size.y * size.z;

   for( ulong i = 0; i < buf_size; ++i )
      data[i] = value;
}