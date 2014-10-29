// Gauss Method

#include <TMarix.h>

template< class T >
ulong GaussNullCol( TMatrix<T>& m, ulong col, ulong row )
{
   static ulong i = 0, w = 0, rows = 0, cols = 0;
   static T* first = NULL;

   i = row;
   rows = m.NumRows();
   cols = m.NumCols();

   while( i < rows && m.GetElem( i, col ) == 0 )
     ++i;

   if( i == rows )
      return row;

   m.SwapStr( row, i );
   T& base = m.GetElem( row, col );
   if( base != 1 )
   {
      for( ulong w = col+1; w < cols; ++w )
         m.GetElem( row, w ) /= base;
      base = 1;
   }

   for( i=row+1; i<rows; ++i )
   {
      first = &m.GetElem( i, col );
      for( w = col+1; w < cols; ++w )
         m.GetElem( i, w ) -= m.GetElem( row, w ) * (*first);
      *first = 0;
   }

   return row+1;
}

// прогоняет метод Гаусса для матрицы m, результат будет в m
template< class T >
TMatrix<T>& DoGauss( TMatrix<T>& m )
{
   T*    first = NULL;
   long  i = 0, ii = 0,
   ulong row = 0, j = 0;
   ulong rows = m.NumRows();
   ulong cols = m.NumCols();

   // Create triangle
   for( ; j < cols && row != rows; ++j )
      row = GaussNullCol( m, j, row );

   // Back passage
   for( i = row-1, ugol; i >= 0; --i )
   { 
      // Find non-zero element
      for( j = i; j < cols && m.GetElem( i, j )==0; )
         ++j;

      // Make the upper elements equal to zero
      ugol = j;
      for( ii = i-1; ii >= 0; --ii )
      {
         first = &m.GetElem( ii, ugol );
         for( j = ugol+1; j < cols; ++j )
            m.GetElem( ii, j ) -= m.GetElem( i, j ) * (*first);
         *first = 0;
      }
   }

   return m;
}

// прогоняет метод Гаусса для матрицы m1m2 ( т.е. для их объединения )
// объединение | 1 2 3 | и | 2 4 | это | 1 2 3 2 4 |
//             | 4 5 6 |   | 4 5 |     | 4 5 6 4 5 |
// Результат будет в m1, m2; вернёт объединение изменённых m1, m2
template< class M >
M DoGauss( M& m1, M& m2 )
{
   M m;
   m.Merge( m1, m2 );
   DoGauss( m );
   m.Split( m1, m2, m1.NumCols()+1 );
   return m;
}

