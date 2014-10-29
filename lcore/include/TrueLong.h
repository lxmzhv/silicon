// Очень большое динамическое беззнаковое число (максимум 0xFFFFFFFF*sizeof(ushort) байт)

#pragma once
#include <funcs.h>
#include <strings.h>

const ulong INIT_LEN = 0x20; // Начальное кол-во элементов в value (для минимизации вызовов malloc)

class DLL_OBJ TrueLong
{
   public:
                TrueLong(): value(0), length(0), meenl(0) { Stretch( INIT_LEN ); }
                TrueLong( long init );
                TrueLong( const TrueLong& init );
                ~TrueLong() { Clear(); }

      TrueLong& operator = ( long init );
      TrueLong& operator = ( const TrueLong& init );
      TrueLong& Init( _int64 init );

      TrueLong& operator *= ( long val );
       TrueLong operator * ( long val );
      TrueLong& operator *= ( TrueLong& val );
       TrueLong operator * ( TrueLong& val );
       TrueLong Pow( ulong deg );     // Возведение в степень (сам не изменяется, вернёт результат)
      TrueLong& PowSelf( ulong deg ); // Возведение в степень (сам и будет результатом)

      TrueLong& operator -= ( TrueLong& val );
       TrueLong operator - ( TrueLong& val );
      TrueLong& operator -= ( long val );
       TrueLong operator - ( long val );

      TrueLong& operator += ( TrueLong& val );
       TrueLong operator + ( TrueLong& val );
      TrueLong& operator += ( long val );
       TrueLong operator + ( long val );

      TrueLong& operator /= ( TrueLong& val );
       TrueLong operator / ( TrueLong& val );
       TrueLong operator % ( TrueLong& val );
      TrueLong& operator /= ( long val );
       TrueLong operator / ( long val );
         ushort Divide( ushort val ); // Возвратит остаток, а само поделиться на val
       TrueLong Divide( TrueLong val, TrueLong& rest ); // Возвратит рез-т, само не измениться, rest - остаток

      TrueLong& operator >>= ( ulong num );    // поделить на LIMIT^num
      TrueLong& operator <<= ( ulong num );    // умножить на LIMIT^num

           bool operator > ( TrueLong& val );
           bool operator < ( TrueLong& val );
           bool operator >= ( TrueLong& val );
           bool operator <= ( TrueLong& val );
           bool operator == ( TrueLong& val );
           bool operator != ( TrueLong& val );

           bool operator > ( long val );
           bool operator < ( long val );
           bool operator >= ( long val );
           bool operator <= ( long val );
           bool operator == ( long val );
           bool operator != ( long val );

      TrueLong& FromString( const char* init ); // "0xNNN" или "NNNN"
          char* ToString( char* buf, ulong size );
        String& GetString( String& str );

           void Clear();
           void Optimize();
           bool IsZero( bool optimize=true );
           // Добавить elem, как новый старший разряд
           void AddHighElem( ushort elem );
           // Удлиннить массив value до len (гарантируется, что length будет >= len)
           void Stretch( ulong len );
 
          ulong ULong();
         _int64 Int64();

                // Длина числа в битовом представлении
         _int64 BitsLength();
                // Умножение на 2^cnt
           void ShiftUp( ulong cnt );
                // Деление на 2^cnt
           void ShiftDown( ulong cnt );

   protected:
      ushort*  value;
      ulong  length; // Длина массива value 
      ulong  meenl;  // Значащая длина массива (всё, что за ней, считается нулём)

      TrueLong& FromHexStr( const char* init ); // "NNNNNNNNNN" без префикса "0x"
};

// Наибольший общий делитель
TrueLong NOD( TrueLong& a, TrueLong& b );