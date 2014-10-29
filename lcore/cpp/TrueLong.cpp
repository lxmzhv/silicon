// Очень большое динамическое беззнаковое число (максимум 0xFFFFFFFF*sizeof(ushort) байт)

#include "TrueLong.h"
#include <stdlib.h>
#include <conio.h>
#include <string.h>

// для отладки
#include <stdio.h>

const ushort MAX_ELEM  = 0xFFFF;
const ulong  LIMIT     = 0x10000; // фактически MAX_ELEM + 1
const int    ESIZE     = 2;//sizeof(ushort);

// делит a на b, при учёте, что b*MAX_ELEM >=a
ushort DivideTwo( TrueLong& a, TrueLong& b )
{
   if( a.IsZero() )   return 0;
   if( b.IsZero() )   return 0;
   ushort min = 0, cur = ushort(LIMIT >> 1);
   ulong max = LIMIT;
   TrueLong c;
   for( ; (max-min) > 1; cur = ushort((max+min)/2) )
   {
      c = b; c *= cur;
      if( c <= a )   min = cur;
      else           max = cur;
   }
   return min;
}

void TrueLong::ShiftUp( ulong cnt )
{
   if( IsZero() )   return;

   ulong word_cnt = cnt >> 4;
   Stretch( meenl + word_cnt + 1 );
   memset( value + meenl, 0, (word_cnt + 1)*2 );
   if( word_cnt )
   {
      memmove( value + word_cnt, value, meenl*2 );
      memset( value, 0, word_cnt*2 );
   }
   meenl += word_cnt;

   uchar bits = uchar(cnt) & 0x0F; // Вот на такое кол-во бит и осталось сдвинуть массив
   if( !bits )   return;

   ushort *beg = value + word_cnt, *cur = value + meenl - 1;
   ushort add = 0;
   for( ulong* curl; cur >= beg; cur-- )
   {
       curl = (ulong*)cur;
       (*curl) <<= bits;
       cur[1] |= add;
       add = *cur;
       *cur = 0;
   }
   cur[1] |= add;
   if( value[meenl] )
      meenl++;
}

void TrueLong::ShiftDown( ulong cnt )
{
   if( IsZero() )   return;

   ulong word_cnt = cnt >> 4;
   if( meenl <= word_cnt  )
   {
      meenl = 0;
      return;
   }

   if( word_cnt )
      memmove( value, value + word_cnt, (meenl-word_cnt)*2 );
   meenl -= word_cnt;

   uchar bits = uchar(cnt) & 0x0F; // Вот на такое кол-во бит и осталось сдвинуть массив
   if( !bits )   return;

   ushort *cur = value, *end = value + meenl - 1;
   ushort add = 0;
   for( ulong* curl; cur < end; cur++ )
   {
       curl = (ulong*)cur;
       (*curl) >>= bits;
       *cur |= add;
       add = cur[1];
       cur[1] = 0;
   }
   add ? *cur |= add : meenl--;
}

_int64 TrueLong::BitsLength()
{
   if( IsZero() )   return 0;
   _int64 res = meenl;
   res <<= 4; // Полное количество бит (с учётом нулевых битов в последнем слове)
// Вычетаем нулевые биты в конце
   ushort lw = value[meenl-1];   // last word
   uchar& lb = ((uchar*)&lw)[1]; // last byte
   for( ; (lb & 0x80) == 0; res-- ) lw <<= 1; // lw != 0, поэтому здесь мы не зациклимся
   return res;
}

ulong TrueLong::ULong()
{
   ulong res = 0;
   if( meenl )
      memcpy( &res, value, meenl==1 ? 2 : 4 );
   return res;
}

_int64 TrueLong::Int64()
{
   _int64 res = 0;
   if( meenl )
   {
      memcpy( &res, value, Min(meenl,ulong(4))*2 );
      res &= 0x7FFFFFFFFFFFFFFF; // Снимаем знак
   }
   return res;
}

TrueLong& TrueLong::operator /= ( TrueLong& val )
{
   *this = *this / val;
   return *this;
}

TrueLong TrueLong::Divide( TrueLong val, TrueLong& rest )
{
   rest = 0;
   if( val.IsZero() )   return TrueLong(0);
   if( IsZero() || *this < val )   return TrueLong(0);
   if( *this == val )   return TrueLong(1);

// Ищем ту "цифру", с которой начнём процесс деления
   ushort *cur = value + meenl - 1;
   ushort *cur_val = val.value + val.meenl - 1, *end;
   bool great = false;
   for( end = cur-val.meenl; cur > end; cur--, cur_val-- )
      if( *cur > *cur_val )
         break;
   if( cur > end )
      cur = end + 1;

// Тепрь cur указывает на последнюю цифру(младший разряд) текущего делимого
   TrueLong res, tl;

// Инициализируем res сами, чтобы с ним было удобно работать
   res.Stretch(1); // Да бы res.value был не ноль
   *res.value = 0;
   res.meenl = 1;

// Копируем первую часть в tmp
   ulong len = ulong(value + meenl - cur);
   rest.Stretch( len );
   memcpy( rest.value, cur, len*ESIZE );
   rest.meenl = len;

// А теперь делим   
   for( ushort num; cur >= value; cur--, rest <<= 1 )
   {
      *rest.value = *cur;
      num = DivideTwo( rest, val );
      res <<= 1;
      *res.value = num;
      tl = val; tl *= num; rest -= tl; // Это операция tmp -= val*num
   }
   rest >>= 1; // теперь это остаток
   res.Optimize();
   rest.Optimize();
   return res;
}

TrueLong TrueLong::operator / ( TrueLong& val )
{
   TrueLong rest;
   return Divide( val, rest );
}

TrueLong TrueLong::operator % ( TrueLong& val )
{
   TrueLong rest;
   Divide( val, rest );
   return rest;
}

bool TrueLong::operator > ( TrueLong& val )
{
   if( IsZero() )   return false;
   if( val.IsZero() )   return true;
   if( meenl != val.meenl ) return meenl > val.meenl;
   ushort *cur=value+meenl-1, *cur_val=val.value+meenl-1;
   for( ; cur >= value; cur--, cur_val-- )
      if( long(*cur)-*cur_val )   
         return (long(*cur)-*cur_val) > 0;
   return false;
}

bool TrueLong::operator < ( TrueLong& val )
{
   return val > *this;
}

bool TrueLong::operator >= ( TrueLong& val )
{
   return !(val > *this);
}

bool TrueLong::operator <= ( TrueLong& val )
{
   return !(*this > val);
}

bool TrueLong::operator == ( TrueLong& val )
{
   Optimize();
   val.Optimize();
   return meenl == val.meenl && !memcmp( value, val.value, meenl*ESIZE );
}

bool TrueLong::operator != ( TrueLong& val )
{
   return !(*this==val);
}

bool TrueLong::operator > ( long val )
{
   return val < 0 ? 1 : *this > TrueLong(val);
}

bool TrueLong::operator < ( long val )
{
   return val <= 0 ? 0 : *this < TrueLong(val);
}

bool TrueLong::operator >= ( long val )
{
   return val < 0 ? 1 : *this >= TrueLong(val);
}

bool TrueLong::operator <= ( long val )
{
   return val < 0 ? 0 : *this <= TrueLong(val);
}

bool TrueLong::operator == ( long val )
{
   return val < 0 ? 0 : *this == TrueLong(val);
}

bool TrueLong::operator != ( long val )
{
   return val < 0 ? 1 : *this != TrueLong(val);
}

TrueLong& TrueLong::operator *= ( TrueLong& val )
{   
   return (*this = *this * val);
}

TrueLong TrueLong::operator * ( TrueLong& val )
{
   TrueLong res, tmp;
   if( IsZero() || val.IsZero() )   return res = 0;
// Выберем меньшее по длине разрядов число
   TrueLong *min = this, *max = &val;
   if( meenl > val.meenl )
      min = &val, max = this;
// Умножаем
   ushort *beg = min->value, *cur = min->value + min->meenl - 1;
   for( ; cur >= beg; cur-- )
   {
      tmp = *max;
      tmp *= *cur;
      res <<= 1;
      res += tmp;
   }
   res.Optimize();
   return res;
}

TrueLong TrueLong::operator * ( long val )
{
   if( !val )   return *this = 0;
   if( val < 0 )
      val = -val;
   if( IsZero() || val==1 )   return *this;

   if( val >= LIMIT )
      return *this * TrueLong( val );
   TrueLong res = *this;
   return res *= val;
}

TrueLong& TrueLong::operator *= ( long val )
{  
   if( !val )   return *this = 0;
   if( val < 0 )
      val = -val;
   if( IsZero() || val==1 )   return *this;

   if( val >= LIMIT )
      return *this *= TrueLong( val );

   ulong tmp, add = 0;
   ushort* end = value + meenl;
   for( ushort* cur = value; cur<end; cur++ )
   {
      tmp = *cur;
      tmp *= val;
      tmp += add;
      *cur = ushort(tmp % LIMIT);
      add = tmp / LIMIT;
   }
   AddHighElem( (ushort)add );
   Optimize();
   return *this;
}

TrueLong& TrueLong::operator >>= ( ulong num )
{
   if( IsZero() )   return *this;
   if( num >= meenl )   return *this = 0;
   meenl -= num;
   memmove( value, value + num, meenl*ESIZE );
   return *this;
}

TrueLong& TrueLong::operator <<= ( ulong num )
{
   if( IsZero() )   return *this;
   Stretch( num + meenl );
   memmove( value+num, value, meenl*ESIZE );
   memset( value, 0, num*ESIZE );
   meenl += num;
   return *this;
}

void TrueLong::Stretch( ulong len )
{
   if( len <= length )   return;
   ushort* new_value = (ushort*)malloc(ESIZE*len);
   if( length )
   {
      if( meenl )
         memcpy( new_value, value, meenl*ESIZE );
      if( value )   free(value);
   }
   value = new_value;
   length = len;
}

void TrueLong::AddHighElem( ushort elem )
{
   if( !elem )   return;
   Stretch( meenl+1 );
   value[meenl++] = elem;
}

TrueLong& TrueLong::operator += ( long val )
{
   return val < 0 ? *this -= TrueLong(-val) : *this += TrueLong(val);
}

TrueLong TrueLong::operator + ( long val )
{
   TrueLong res = *this;
   return val < 0 ? res -= TrueLong(-val) : res += TrueLong(val);
}

TrueLong TrueLong::operator + ( TrueLong& val )
{
   TrueLong res = *this;
   return res += val;
}

TrueLong& TrueLong::operator += ( TrueLong& val )
{
   if( val.IsZero() )    return *this;
   if( IsZero() )   return *this = val;

// Предоставляем место под сумму
   ulong len = 1 + Max( meenl, val.meenl );
   Stretch( len );
   memset( value + meenl, 0, (len-meenl)*ESIZE ); // Забиваем конец нулями
   meenl = len;

// Складываем
   ulong sum = 0;
   ushort *end_val = val.value + val.meenl, *cur = value, *cur_val = val.value;
   for( ; cur_val < end_val; cur_val++, cur++ )
   {
      sum += *cur;
      sum += *cur_val;
      *cur = ushort(sum % LIMIT);
      sum /= LIMIT;
   }
   for( ; sum; cur++ )
   {
      sum += *cur;
      *cur = ushort(sum % LIMIT);
      sum /= LIMIT;
   }
   Optimize();
   return *this;
}

TrueLong& TrueLong::operator -= ( long val ) 
{
   return val < 0 ? *this += TrueLong(-val) : *this -= TrueLong(val); 
}

TrueLong TrueLong::operator - ( long val )
{
   TrueLong res = *this;
   return val < 0 ? res += TrueLong(-val) : res -= TrueLong(val);
}

TrueLong TrueLong::operator - ( TrueLong& val )
{
   TrueLong res = *this;
   return res -= val;
}

TrueLong& TrueLong::operator -= ( TrueLong& val )
{
   if( IsZero() || val.IsZero() )   return *this;
   if( val.meenl>meenl )   return *this = 0;

   ushort *end = value + meenl, *end_val = val.value + val.meenl;
   ulong dolg = 0;
   for( ushort *cur = value, *cur_val = val.value; cur<end; cur++ )
   {
      if( dolg && (*cur)-- )  dolg = 0;
      if( cur_val == end_val )   
         if( dolg )   continue;
         else         break;
      if( *cur < *cur_val )   dolg++;
      *cur -= *(cur_val++);
   }
   return dolg ? *this = 0 : *this;
}

bool TrueLong::IsZero( bool optimize )
{
   if( optimize )   Optimize();
   return ( value==0 || meenl==0 || (meenl==1 && *value==0) );
}

char* TrueLong::ToString( char* buf, ulong size )
{
   if( !buf || (size--) <= 2 )   return buf;
   *buf = buf[1] = 0;
   TrueLong tl;
   tl = *this;
   size--;
   for( ulong cnt=1; !tl.IsZero(); cnt++ )
   {
      memmove( buf+1, buf, Min( cnt, size ) );
      *buf = char(tl.Divide(10)) + '0';
   }
   if( !*buf )   *buf = '0';
   return buf;
}

String& TrueLong::GetString( String& str )
{
   TrueLong tl;
   tl = *this;

   for( str.Clear(); !tl.IsZero(); )
      str += char('0' + tl.Divide(10));

   int len = str.Length();
   if( !len )
   {
      str = '0';
      return str;
   }

   /* reverse */
   char *beg = str, *end = str+(len-1), c;
   for( ; beg < end; ++beg, --end )
   {
      c = *beg;
      *beg = *end;
      *end = c;
   }

   return str;
}

void TrueLong::Optimize()
{
   if( value )
      for( ushort* cur = value+meenl-1; meenl>1 && !*cur; meenl-- )   cur--;
}

TrueLong TrueLong::operator / ( long val )
{
   TrueLong res = *this;
   return res /= val;
}

ushort TrueLong::Divide( ushort val )
{
   ushort  base_whole  = MAX_ELEM / val;
   ushort base_rest  = MAX_ELEM % val;
   if( ++base_rest==val )
      base_whole++, base_rest = 0;

   ushort rest=0; 
   ulong tmp;  
   for( ushort* cur = value+meenl-1; cur>=value; cur-- )
   {
       tmp = base_rest; tmp *= rest; tmp += *cur%val; // Дабы не выйти за пределы short'a
       *cur = ushort(*cur/val + base_whole*rest + tmp/val);
       rest = ushort(tmp % val);
   }
   Optimize();
   return rest;
}

TrueLong& TrueLong::operator /= ( long val )
{
   if( !val )   return *this = 0;
   if( val < 0 )
      val = -val;
   if( IsZero() || val==1 )   return *this;

   if( val >= LIMIT )
      return *this /= TrueLong( val );

   Divide( ushort(val) );
   return *this;
}

TrueLong::TrueLong( long init ): length(0), meenl(0)
{
   Stretch( INIT_LEN );
   *this = init;
}

TrueLong::TrueLong( const TrueLong& init ): length(0), meenl(0)
{
   Stretch( INIT_LEN );
   *this = init;
}

ushort HexDigitToNumber( char c )
{
   if( c <= '9' )   return ushort(c-'0');
   if( c <= 'f' && c >= 'a' )   return ushort(c-'a'+10);
   return ushort(c-'A'+10);
}

// Берёт первые num символа и переводит их в ushort (из hex-представления)
ushort UShortHex( const char* str, char num = 4 )
{
   if( !str )   return 0;
   ushort res = 0;
   for( const char* end = str + num; str < end; str++ )
   {
      res <<= 4;
      res += HexDigitToNumber(*str);
   }
   return res;
}

TrueLong& TrueLong::FromHexStr( const char* init )
{
   if( !init )   return *this = 0;
   ulong str_len = (long)strlen( init );

   ulong len = (str_len+3)>>2;  // Длина массива ushort'ов
   meenl = 0; // Чтобы в Stretch() не копировать мусор
   Stretch( len );

   const char* s = init + str_len-4;
   ushort* cur = value;
   for( ; s >= init; s-=4, cur++ )
      *cur = UShortHex( s );

   char rest = char(str_len & 0x3); // str_len%4
   if( rest )
      *cur = UShortHex( init, rest );

   meenl = len;

   return *this;
}

ushort UShort( const char* str )
{
   if( !str )   return 0;
   ushort res = 0;
   for( const char* end = str + 4; str < end && *str; str++ )
   {
      res *= 10;
      res += *str-'0';
   }
   return res;
}

TrueLong& TrueLong::FromString( const char* init )
{
   ulong len = ulong( init ? strlen(init) : 0 );
   if( !len )   return *this = 0; 

   if( !strncmp( init, "0x", 2 ) )
      return FromHexStr( init+2 );

   meenl = 0;
   ushort mul[4] = { 10, 100, 1000, 10000 };
// Берём из строки по 4 знака и переводим их в число
   for( const char *s = init, *end = init + len; s < end; s += 4 )
   {
      *this *= mul[Min(end-s-1,3)];
      *this += UShort( s );
   }
   return *this;
}

TrueLong& TrueLong::operator = ( long init )
{
   if( init < 0 )
      init *= -1;
   Stretch(2);
   meenl = 2;
   memcpy( value, &init, 2*ESIZE );
   Optimize();
   return *this;
}

TrueLong& TrueLong::Init( _int64 init )
{
   if( init < 0 )
      init *= -1;
   Stretch(4);
   meenl = 4;
   memcpy( value, &init, 4*ESIZE );
   Optimize();
   return *this;
}

TrueLong& TrueLong::operator = ( const TrueLong& init )
{
   if( &init == this )   return *this;
   meenl = init.meenl;

   if( meenl )
   {
      if( meenl > length )
      {
         if( length && value )   free(value);
         length = meenl;
         value = (ushort*)malloc( ESIZE*length );
      }
      memcpy( value, init.value, meenl*ESIZE );
   }
   else
      if( length )
         meenl++, *value = 0;
   return *this;
}

void TrueLong::Clear()
{ 
   if( length && value )
      free(value);
   length = 0;
   meenl = 0;
}

//
// Вспомогательные функции
//

// Наибольший общий делитель
TrueLong NOD( TrueLong& a, TrueLong& b )
{
   char a_is_min = (a < b);
   TrueLong r = ( a_is_min ? b%a : a%b );
   if( r.IsZero() )
      return a_is_min ? a : b;
   return NOD( a_is_min ? a : b, r );
}

/* Алгоритм Евклида.*/
//long nod(long a,long b)
//{
//  while (a!=b)
//  {  if (a<b) b-=a;
//     else a-=b;
//  }
//  return a;
//}

TrueLong& TrueLong::PowSelf( ulong deg )
{
   TrueLong a(*this);
   if( deg )
      for( ulong i = 1; i < deg; i++ )
         *this *= a;
   else
      *this = 1;
   return *this;
}

TrueLong TrueLong::Pow( ulong deg )
{
   TrueLong res(1);
   if( deg )
      for( ulong i = 0; i < deg; i++ )
         res *= *this;
   return res;
}