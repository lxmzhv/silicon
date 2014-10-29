//	Sparse matrices

#pragma once

#include <TrueDrob.h>
#include <array.h>
#include <string.h>
#include <funcs.h>

template<class T>
class DLL_OBJ SMatrix
{
   struct Item
   {
      T      value;
      ulong  col;
   };

   public:
               SMatrix( ulong n_rows = 1, ulong n_cols = 1, ulong average_row_size = 0 );
               //SMatrix( SMatrix& m );

          void SetValue( ulong n_row, ulong n_col, T value );
          bool GetValue( ulong n_row, ulong n_col, T* value );

   private:

      ulong             nRows, nCols;
      VP< Array<Item> > rows;
};

template< class T >
SMatrix<T>::SMatrix<T>( ulong n_rows, ulong n_cols, ulong average_row_size ):
   nCols( n_cols ), nRows( n_rows )
{
   rows = new Array<Item>[nRows];

   if( average_row_size > 0 )
      for( ulong i = 0; i < nRows; ++i )
	 rows[i].Reserve( average_row_size );
}

template< class T >
void SMatrix<T>::SetValue( ulong n_row, ulong n_col, T value )
{
   if( nRows <= n_row )
      return;

   Array<Item>& row = rows[n_row];
   int i = row.Size()-1;

   for( ; i >= 0; --i )
      if( row[i].col > n_col )
         continue;
      else
      {
         if( row[i].col < n_col )
            break;

         row[i].value = value;
         return;
      }

   Item item = { value, n_col };
   row.Ins( i+1, 1, &item );
}

template< class T >
bool SMatrix<T>::GetValue( ulong n_row, ulong n_col, T* value )
{
   if( nRows <= n_row )
      return false;

   Array<Item>& row = rows[n_row];
   int i = row.Size()-1;

   for( ; i >= 0; --i )
      if( row[i].col > n_col )
         continue;
      else
      {
         if( row[i].col == n_col )
         {
            *value = row[i].value;
            return true;
         }
         break;
      }

   *value = 0;
   return false;
}

