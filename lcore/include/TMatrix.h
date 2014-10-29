
#pragma once

#include "TrueDrob.h"

template< class T >
class TMatrix
{
   public:
                     TMatrix( ulong n_str=1, ulong n_stlb=1, T* arr=0 ): nRows(1), nCols(1), data(0) { Init(arr,n_str,n_stlb); }
                     TMatrix( TMatrix<T>& m ): nRows(1), nCols(1), data(0) { Init( m.data, m.nRows, m.nCols ); }

               // arr == 0     - заполнить нулями
               // n_str (n_stlb) == 0 - оставить без изменения nRows (nCols)
      TMatrix<T>& Init( T* arr = 0, ulong n_str = 0, ulong n_stlb = 0 );
               // Проинициализировать размер
      TMatrix<T>& InitSize( ulong n_str = 0, ulong n_stlb = 0 );
               // Проинициализировать единичной
      TMatrix<T>& InitEd( ulong n_str = 0, ulong n_stlb = 0 );
      TMatrix<T>& operator =( TMatrix<T>& m ) { if( this!=&m ) Init(m.data,m.nRows,m.nCols); return *this; }
       TMatrix<T> operator *( TMatrix<T>& m );

       virtual T& operator[]( ulong offset ) { return data[offset]; }
 virtual const T& operator[]( ulong offset ) const { return data[offset]; }
       virtual T& GetElem( ulong n_str, ulong n_stlb ) { return data[n_str*nCols+n_stlb]; }
 virtual const T& GetElem( ulong n_str, ulong n_stlb ) const { return data[n_str*nCols+n_stlb]; }
             bool Contains( ulong x, ulong y ) const { return x >= 0 && y >= 0 && x < nRows && y < nCols; }

             void SwapStr( int s1, int s2, ulong beg_stlb = 0 );

            ulong NumRows() const { return nRows; }
            ulong NumCols() const { return nCols; }

                // Сливает две матрицы (столбцы m1 добавляются к m2, а результат в this)
      TMatrix<T>& Merge( const TMatrix<T>& m1, const TMatrix<T>& m2 );
                // Разделяет матрицу на m1 и m2, beg_stlb - первый столбец второй матрицы
             void Split( TMatrix<T>& m1, TMatrix<T>& m2, ulong beg_stlb ) const;

                // Обратную матрицу пометит в res; вернёт false, если операция невозможна
             bool GetInverse( TMatrix<T>& res );

   protected:
      ulong nRows, nCols; // Size of matrix
      VP<T> data;

            ulong DataSize() const { return nRows*nCols*sizeof(T); }
};

// 
// ++++++++++++++++++++++++ Implementation +++++++++++++++++++++++++++++++
//

template< class T >
TMatrix<T> TMatrix<T>::operator *( TMatrix<T>& m )
{
   TMatrix<T> res( nRows, m.nCols );
   if( nCols != m.nRows )   return res;
   for( ulong i = 0; i < m.nCols; i++ )
      for( ulong j = 0; j < nRows; j++ )
         for( ulong k = 0; k < nCols; k++ )
            res.GetElem(j,i) += GetElem(j,k)*m.GetElem(k,i);
   return res;
}

template< class T >
TMatrix<T>& TMatrix<T>::InitSize( ulong n_str, ulong n_stlb )
{
   if( (n_str && n_str != nRows) || (n_stlb && n_stlb != nCols) && data )
      data.Clear();
   if( n_str )    nRows = n_str;
   if( n_stlb )   nCols = n_stlb;
   if( !data )
      data = new T[nRows*nCols];
   return *this;
}

template< class T >
TMatrix<T>& TMatrix<T>::Init( T* arr, ulong n_str, ulong n_stlb )
{
   InitSize( n_str, n_stlb );
   if( arr )
      //memcpy( data, arr, DataSize() );
      for( int i = 0, cnt = nRows*nCols; i < cnt; i++ )
         data[i] = arr[i];
   else
      //memset( data, 0, DataSize() );
      for( int i = 0, cnt = nRows*nCols; i < cnt; i++ )
         data[i] = 0;
   return *this;
}

template< class T >
void TMatrix<T>::SwapStr( int s1, int s2, ulong beg_stlb )
{
   if( s1 != s2 )
   {
      T elem;
      for( ulong q = beg_stlb; q < nCols; ++q )
      {
         elem             = GetElem( s1, q );
         GetElem( s1, q ) = GetElem( s2, q );
         GetElem( s2, q ) = elem;
      }
   }
}

template< class T >
TMatrix<T>& TMatrix<T>::Merge( const TMatrix<T>& m1, const TMatrix<T>& m2 )
{
   Init( 0, m1.NumRows(), m1.NumCols() + m2.NumCols() );
   if( m1.NumRows() == m2.NumRows() )
   {
      const T* data1 = &m1[0], *data2 = &m2[0];
      T* *cur = data;
      ulong stlb1 = m1.NumCols(), stlb2 = m2.NumCols();
      for( ulong i = 0, j; i < m1.NumRows(); i++ )
      {
         for( j = 0; j < stlb1; j++ )
            *(cur++) = *(data1++);
         for( j = 0; j < stlb2; j++ )
            *(cur++) = *(data2++);
      }
   }
   return *this;
}

template< class T >
void TMatrix<T>::Split( TMatrix<T>& m1, TMatrix<T>& m2, ulong beg_stlb ) const
{
   if( beg_stlb && beg_stlb <= nCols )
   {
      ulong stlb1 = beg_stlb-1, stlb2 = nCols-beg_stlb+1;
      m1.Init( 0, nRows, stlb1 );
      m2.Init( 0, nRows, stlb2 );
      T* data1 = &m1[0], *data2 = &m2[0], *cur = data;
      for( ulong i = 0, j; i < nRows; i++ )
      {
         for( j = 0; j < stlb1; j++ )
            *(data1++) = *(cur++);
         for( j = 0; j < stlb2; j++ )
            *(data2++) = *(cur++);
      }
   }
}

template< class T >
TMatrix<T>& TMatrix<T>::InitEd( ulong n_str, ulong n_stlb )
{
   InitSize( n_str, n_stlb );
   for( ulong i = 0, k = 0; i < nRows; i++ )
      for( ulong j = 0; j < nCols; j++, k++ )
         data[k] = (i==j ? 1 : 0);
   return *this;
}

template< class T >
bool TMatrix<T>::GetInverse( TMatrix<T>& res )
{
   if( nRows != nCols )   return false;

   TMatrix<T> m(*this);
   DoGauss( m, res.InitEd( nRows, nRows ) );
// Проверка корректности преобразования (матрица могла быть вырожденной)
// можно сравнить матрицу m с единичной (здесь достаточно взглянуть на её диагональ [!= 0])
   for( ulong i = 0; i < nRows; i++ )
      if( m.GetElem(i,i) == 0 )
         return false;
   return true;
}

// 
// ++++++++++++++++++++++++++++ Utilities +++++++++++++++++++++++++++++++++++++++++
//
template< class M >
void PrintTMatrix( M& m, const char* file_name = "c:\\temp\\debug.log" )
{
   FILE* f = fopen( file_name, "a" );
   if( !f )   return;

   fprintf( f, "\n" );
   char buf[1001];
   for( ulong i = 0; i < m.NumRows(); i++, fprintf( f, "\n" ) )
      for( ulong j = 0; j < m.NumCols(); j++ )
         fprintf( f, "%s ", m.GetElem(i,j).ToString( buf, 1000 ) );

   fclose( f );
}

template< class T >
TMatrix<T> Merge( TMatrix<T>& m1, TMatrix<T>& m2 )
{
   TMatrix<T> m;
   return m.Merge( m1, m2 );
}

