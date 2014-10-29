
#include <truedrob.h>
#include <matrix.h>
#include <string.h>
#include <funcs.h>

// temp
//#include <stdio.h>

//#define Maximize(a,b) ((b>a) ? a=b : a)

Matrix::Matrix( ulong n_str, ulong n_stlb, Elem* arr ): nStr(1), nStlb(1), data(0)
{
   Init( arr, n_str, n_stlb );
}

Matrix::Matrix( Matrix& m ): nStr(1), nStlb(1), data(0)
{
   Init( m.data, m.nStr, m.nStlb );
}

Matrix& Matrix::operator =( Matrix& m )
{
   if( this == &m )   return *this;
   Init( m.data, m.nStr, m.nStlb );
   return *this;
}

Elem& Matrix::GetElem( ulong d_str, ulong d_stlb )
{
   return data[d_str*nStlb+d_stlb];
}

CElem& Matrix::GetElem( ulong d_str, ulong d_stlb ) const
{
   return data[d_str*nStlb+d_stlb];
}

Matrix Matrix::operator *( Matrix& m )
{
   Matrix res( nStr, m.nStlb );
   if( nStlb != m.nStr )   return res;
   for( ulong i = 0; i < m.nStlb; i++ )
      for( ulong j = 0; j < nStr; j++ )
         for( ulong k = 0; k < nStlb; k++ )
            res.GetElem(j,i) += GetElem(j,k)*m.GetElem(k,i);
   return res;
}

Matrix& Matrix::InitSize( ulong n_str, ulong n_stlb )
{
   if( data && (n_str && n_str != nStr || n_stlb && n_stlb != nStlb) )
      data.Clear();
   if( n_str )    nStr = n_str;
   if( n_stlb )   nStlb = n_stlb;
   if( !data )
      data = new Elem[nStr*nStlb];
   return *this;
}

Matrix& Matrix::Init( Elem* arr, ulong n_str, ulong n_stlb )
{
   InitSize( n_str, n_stlb );
   if( arr )
      //memcpy( data, arr, DataSize() );
      for( int i = 0, cnt = nStr*nStlb; i < cnt; i++ )
         data[i] = arr[i];
   else
      //memset( data, 0, DataSize() );
      for( int i = 0, cnt = nStr*nStlb; i < cnt; i++ )
         data[i] = 0;
   return *this;
}

void Matrix::SwapStr( int s1, int s2, ulong beg_stlb )
{
   if( s1 != s2 )
   {
      Elem elem;
      for( ulong q = beg_stlb; q < nStlb; ++q )
      {
         elem             = GetElem( s1, q );
         GetElem( s1, q ) = GetElem( s2, q );
         GetElem( s2, q ) = elem;
      }
   }
}

Matrix& Matrix::Merge( const Matrix& m1, const Matrix& m2 )
{
   Init( 0, m1.NumStr(), m1.NumStlb() + m2.NumStlb() );
   if( m1.NumStr() == m2.NumStr() )
   {
      const Elem *data1 = &m1[0], *data2 = &m2[0];
      Elem *cur = data;
      ulong stlb1 = m1.NumStlb(), stlb2 = m2.NumStlb();
      for( ulong i = 0, j; i < m1.NumStr(); i++ )
      {
         for( j = 0; j < stlb1; j++ )
            *(cur++) = *(data1++);
         for( j = 0; j < stlb2; j++ )
            *(cur++) = *(data2++);
      }
   }
   return *this;
}

Matrix Merge( const Matrix& m1, const Matrix& m2 )
{
   Matrix m;
   return m.Merge( m1, m2 );
}

void Matrix::Split( Matrix& m1, Matrix& m2, ulong beg_stlb ) const
{
   if( beg_stlb && beg_stlb <= nStlb )
   {
      ulong stlb1 = beg_stlb-1, stlb2 = nStlb-beg_stlb+1;
      m1.Init( 0, nStr, stlb1 );
      m2.Init( 0, nStr, stlb2 );
      Elem* data1 = &m1[0], *data2 = &m2[0], *cur = data;
      for( ulong i = 0, j; i < nStr; i++ )
      {
         for( j = 0; j < stlb1; j++ )
            *(data1++) = *(cur++);
         for( j = 0; j < stlb2; j++ )
            *(data2++) = *(cur++);
      }
   }
   return;
}

Matrix& Matrix::InitEd( ulong n_str, ulong n_stlb )
{
   InitSize( n_str, n_stlb );
   for( ulong i = 0, k = 0; i < nStr; i++ )
      for( ulong j = 0; j < nStlb; j++, k++ )
         data[k] = (i==j ? 1 : 0);
   return *this;
}

bool Matrix::GetInverse( Matrix& res )
{
   if( nStr != nStlb )   return false;
   Matrix m(*this);
   DoGauss( m, res.InitEd( nStr, nStr ) );
// Проверка корректности преобразования (матрица могла быть вырожденной)
// можно сравнить матрицу m с единичной (здесь достаточно взглянуть на её диагональ [!= 0])
   for( ulong i = 0; i < nStr; i++ )
      if( m.GetElem(i,i) == 0 )
         return false;
   return true;
}


// Gauss method

void GaussNullStolb( Matrix& m, ulong stlb, ulong& r )
{
   ulong i = r;
   ulong rows = m.NumStr();
   ulong cols = m.NumStlb();

   while( i < rows && m.GetElem(i,stlb) == 0 )
      ++i;

   if( i == rows )
      return;

   m.SwapStr( r, i );
   Elem& base = m.GetElem(r,stlb);
   if( base != 1 )
   {
      for( ulong w = stlb+1; w < cols; ++w )
         m.GetElem(r,w) /= base;
      base = 1;
   }

   for( i=r+1; i<rows; ++i )
   {
      Elem& first = m.GetElem(i,stlb);
      for( ulong w = stlb+1; w < cols; ++w )
         m.GetElem(i,w) -= first * m.GetElem(r,w);
      first = 0;
   }
   ++r;
}

Matrix& DoGauss( Matrix& m )
{
   /*---------------------------- Creat treugolnik--------------------------*/
   ulong r = 0, j = 0;
   for( ; j < m.NumStlb() && r != m.NumStr(); ++j )
      GaussNullStolb( m, j, r );

   /*----------------------------Obratn Hod---------------------------------*/
   for( long i=r-1, ugol; i >= 0; --i )
   { 
      // Идём до первого ненулевого элемента в строке
      for( j = i; j < m.NumStlb() && m.GetElem(i,j)==0; ) ++j;
      ugol = j;
      // Обнуляем верхние элементы
      for( long ii = i-1; ii >= 0; m.GetElem(ii--,ugol)=0 )
      {
         Elem& first = m.GetElem(ii,ugol);
         for( j = ugol+1; j < m.NumStlb(); ++j )
            m.GetElem(ii,j) -= m.GetElem(i,j)*first;
      }
   }
   return m;
}

// прогоняет метод Гаусса для матрицы m1m2 (т.е. для их объединения)
// объединение | 1 2 3 | и | 2 4 | это | 1 2 3 2 4 |
//             | 4 5 6 |   | 4 5 |     | 4 5 6 4 5 |
Matrix DoGauss( Matrix& m1, Matrix& m2 )
{
   Matrix m;
   m.Merge( m1, m2 );
   DoGauss(m);
   m.Split( m1, m2, m1.NumStlb()+1 );
   return m;
}
