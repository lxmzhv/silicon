// Рациональные числа любой точности

#include <math.h>
#include <string.h>
#include <stdio.h>
#include "TrueDrob.h"

TrueDrob::TrueDrob( long pp, long qq )
{
   Init( pp, qq );
}

// Длина мантиссы у double в битах
#define MANTISSA_LENGTH 52
#define MANTISSA_BITS 0x000FFFFFFFFFFFFF
#define HIGH_BIT      0x0010000000000000
#define SIGN_BIT      0x8000000000000000

TrueDrob& TrueDrob::Init( double init )
{
   if( init == 0 )   return *this = 0;

   _int64& _init = *(_int64*)&init;
   sign = (_init & SIGN_BIT) != 0 ? 1 : 0;

   // на 2 в степени deg нужно умножить мантиссу, чтобы получить значение
   long deg = (long)((_init & ~SIGN_BIT) >> MANTISSA_LENGTH) - 1023 - MANTISSA_LENGTH;
   _int64 mant = (_init & MANTISSA_BITS) | HIGH_BIT; // мантисса

   // Повышаем отрицательную степень, пока это возможно (фактически, сокращаем дробь)
   for( ; (deg < 0) && (mant & 1) == 0; deg++ ) // Закончится т.к. mant != 0
      mant >>= 1;

   p.Init( mant );
   q = 1;

   if( deg == 0 )
      return *this;

   q = 1;
   q.ShiftUp( (ulong)(deg > 0 ? deg : -deg) );

   if( deg > 0 )
   {
      p *= q;
      q = 1;
   }
   else // deg < 0
      Optimize();
   return *this;
}

const long MAX_LONG = 0x7FFFFFFF;

double TrueDrob::Double()
{
   if( p.IsZero() )   return 0;
   // p/q = a * 2^n, где 1 <= a < 2   (a - это мантисса)
   // Поэтому n = [log2(p/q)] = [log2(p)-log2(q)], значит,
   // l1 - l2 - 1 < n < l1 - l2 + 1, где l1 - битовая длина p, а l2 - битовая длина q
   _int64 n_min = p.BitsLength() - q.BitsLength() - 1;
   // n = либо n_min, либо n_min+1
   // 1 <= (p/q) * 2^(-n) < 2
   // Берём n = n_min+1, если q <= p * 2^(-n), то это настоящее n, иначе n = n_min
   _int64 n = n_min + 1;
   TrueLong tmp;
   if( n <= 0 ) // Проверяем q <= p * 2^(-n)
   {
      tmp = p;
      tmp.ShiftUp( ulong(-n) );
      if( q > tmp )   
      {
         n--;
         tmp.ShiftUp( 1 );
      }
   }
   else // Проверяем q * 2^n <= p
   {
      tmp = q;
      tmp.ShiftUp( ulong(n) );
      if( tmp > p )
      {
         n--;
         tmp.ShiftDown( 1 );
      }
   }

// Нашли n, теперь формируем double
   _int64 res = (n + 1023) & 0x7FF;
   res <<= MANTISSA_LENGTH;
   // a = 2^(-n) * p/q    или     a * 2^52 = 2^(52-n) * p/q      (MANTISSA_LENGTH == 52)
   if( n < 0 )
   {
      tmp.ShiftUp( MANTISSA_LENGTH );
      tmp /= q;
   }
   else
   {
      TrueLong a = p;
      a.ShiftUp( MANTISSA_LENGTH );
      tmp = a / tmp;
   }
   res |= tmp.Int64() & MANTISSA_BITS;
   if( sign )   res |= SIGN_BIT;
   return *(double*)&res;
}

//double TrueDrob::GetDouble()
//{
//   TrueLong rest;
//   TrueLong val = p.Divide( q, rest );
//   double res = val.ULong();
//   rest *= MAX_LONG;
//   rest /= q;
//   double a = rest.ULong();
//   a /= MAX_LONG;
//   return res+a;
//}
//
TrueDrob& TrueDrob::operator =( long init )
{
   q = 1;
   p = labs(init);
   sign = init < 0;
   return *this;
}

TrueDrob& TrueDrob::operator =( TrueLong& init )
{
   q = 1;
   sign = 0;
   p = init;
   return *this;
}

TrueDrob& TrueDrob::operator =( const TrueDrob& init )
{
   p = init.p;
   q = init.q;
   sign = init.sign;
   return *this;
}

TrueDrob& TrueDrob::Init( TrueLong& pp, TrueLong& qq, char sgn )
{
   p = pp;
   q = qq;
   sign = sgn ? 1 : 0;
   return *this;
}

TrueDrob& TrueDrob::Init( long pp, long qq )
{
   p = labs(pp);
   q = labs(qq);
   sign = (pp < 0 && qq > 0) || (pp >= 0 && qq < 0); // 0 or 1
   return *this;
}

TrueDrob& TrueDrob::operator *=( long mnoj )
{
   p *= labs( mnoj );
   if( Sign( mnoj ) == -1 )
      sign = 1 - sign;
   return Optimize();
}

TrueDrob TrueDrob::operator *( long mnoj )
{
   TrueDrob res(*this);
   return res *= mnoj;
};

TrueDrob& TrueDrob::operator *=( TrueDrob& d )
{
   p *= d.p;
   q *= d.q;
   if( d.sign )
      sign = 1 - sign;
   return Optimize();
}

TrueDrob TrueDrob::operator *( TrueDrob& d )
{ 
   TrueDrob res(*this);
   return res *= d;
};

TrueDrob& TrueDrob::operator /=( long val )
{
   q *= labs(val);
   if( val < 0 )
      sign = 1 - sign;
   return Optimize();
}

TrueDrob TrueDrob::operator /( long val )
{
   TrueDrob res(*this);
   return res /= val;
}

TrueDrob& TrueDrob::operator /=( TrueDrob& d )
{
   p *= d.q;
   q *= d.p;
   if( d.sign )
      sign = 1 - sign;
   return Optimize();
}

TrueDrob TrueDrob::operator /( TrueDrob& d )
{  
   TrueDrob res(*this);
   return res /= d;
};

TrueDrob& TrueDrob::operator +=( long val )
{
   return *this += TrueDrob(val);
}

TrueDrob TrueDrob::operator +( long val )
{
   TrueDrob res(*this);
   return res += TrueDrob(val);
}

TrueDrob& TrueDrob::operator +=( TrueDrob& d )
{
   TrueLong s1(p), s2(d.p);
   s1 *= d.q;
   s2 *= q;
   q *= d.q;
   if( sign == d.sign )
      p = (s1 += s2);
   else
      if( s1 > s2 )
         p = (s1 -= s2);
      else
      {
         p = (s2 -= s1);
         sign = d.sign;
      }
   return Optimize();
}

TrueDrob TrueDrob::operator +( TrueDrob& d )
{
   TrueDrob res(*this);
   return res += d;
};

TrueDrob& TrueDrob::operator -=( long val )
{
   return *this -= TrueDrob(val);
}

TrueDrob TrueDrob::operator -( long val )
{
   TrueDrob res(val);
   return res -= val;
}

TrueDrob& TrueDrob::operator -=( TrueDrob& d )
{
   d.sign = 1 - d.sign;
   *this += d;
   d.sign = 1 - d.sign;
   return *this;
}

TrueDrob TrueDrob::operator -( TrueDrob& d )
{
   TrueDrob res(*this);
   return res -= d;
};

char TrueDrob::operator <( TrueDrob d )
{
   d -= *this;
   return d.IsZero() ? 0 : (1 - d.sign);
}

char TrueDrob::operator >( TrueDrob d )
{ 
   d -= *this;
   return d.IsZero() ? 0 : d.sign;
}

char TrueDrob::operator <=( TrueDrob d )
{
   d -= *this;
   return d.IsZero() ? 1 : (1-d.sign);
}

char TrueDrob::operator >=( TrueDrob d )
{
   d -= *this;
   return d.IsZero() ? 1 : d.sign;
}

char TrueDrob::operator ==( TrueDrob& d )
{
   if( d.IsZero() || IsZero() )
      return d.IsZero() && IsZero();
   Optimize();
   d.Optimize();
   return (p == d.p && q == d.q && sign == d.sign) ? 1 : 0;
}

char TrueDrob::operator !=( TrueDrob& d )
{
   return (*this == d) ? 0 : 1;
}

char TrueDrob::operator <( long val )
{
   return *this < TrueDrob(val);
}

char TrueDrob::operator >( long val )
{
   return *this > TrueDrob(val);
}

char TrueDrob::operator <=( long val )
{
   return *this <= TrueDrob(val);
}

char TrueDrob::operator >=( long val )
{
   return *this >= TrueDrob(val);
}

char TrueDrob::operator ==( long val )
{
   if( val==0 || IsZero() )
      return val==0 && IsZero();
   return *this == TrueDrob(val);
}

char TrueDrob::operator !=( long val )
{
   return *this != TrueDrob(val);
}

char* TrueDrob::ToString( char* buf, ulong size )
{
   if( !buf || (--size) <= 0 ) return buf;
   *buf = 0;
   if( p.IsZero() )
      strcpy_s( buf, size, "0" );
   else
   {  
      Optimize();
      if( sign && size )	  strcpy_s( buf, size, "-" ), size--;
      p.ToString( buf + sign, size );
      long sz = (long)strlen(buf+sign);
      if( q != TrueLong(1) && (size -= sz ) > 0 )
      {
         char* s = buf+sign+sz;
         strcpy_s( s++, size-(s-buf), "/" );
         q.ToString( s, --size );
      }
   }
   return buf;
}

// Сокращение дроби
TrueDrob& TrueDrob::Optimize()
{
   if( p.IsZero() ) // Здесь для ускорения необходимо максимально уменьшить знаменатель
   {
      q = 1;
      return *this;
   }
   if( q.IsZero() || p == TrueLong(1) || q == TrueLong(1) )
      return *this;
   TrueLong den( NOD( p, q ) );
   p /= den;
   q /= den;
   return *this;
}

char TrueDrob::IsZero()
{
   return p.IsZero() ? 1 : 0;
}

char TrueDrob::IsLess( double d )
{
   TrueDrob drb;
   return *this < drb.Init(d);
}

char TrueDrob::IsGreat( double d )
{
   TrueDrob drb;
   return *this > drb.Init(d);
}

char TrueDrob::IsAbsLess( double eps )
{
   if( eps <= 0 )   return 0;
   return sign ? IsGreat(-eps) : IsLess(eps);
}