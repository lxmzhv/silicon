// Строка

#include <strings.h>
#include <numbers.h>
#include <memory.h>

String::String( int len ): size(0)
{
   Init( len );
}

String::String( const char* str ): size(0)
{
   *this = str;
}

//String& String::operator = ( const char* str )
//{
//	if( value != str )
//      if( !str )
//      {
//         Init( 0 );
//         *value = 0;
//      }
//      else
//      {
//         int l = strlen(str);
//         Init( l );
//         strcpy_s( value, l+1, str );
//      }
//	return *this;
//}
//
//String& String::operator = ( char ch )
//{
//   Init( 1 );
//	value[0]=ch;
//	value[1]='\0';
//	return *this;
//}

void String::Init( int sz )
{
   if( !value || size < sz+1 )
      value = new char[ size = sz+1 ];
   *value = 0;
}

void String::Resize( int sz )
{
   if( sz+1 <= size )   return;
   VP<char> s = new char[ size = sz+1 ];
   strcpy_s( s, size, value );
   value = s;
}

String& String::operator += ( const char* str )
{
   if( !str )   return *this;
   int l1 = Length(), l2 = strlen(str), l = l1+l2;
   Resize( l );
   strcat_s( value, l+1, str );
   return *this;
}

String& String::operator += ( char ch )
{
   int l = Length();
   Resize( l+1 );
   value[l] = ch;
   value[l+1] = 0;
   return *this;
}

String& String::operator += ( long val )
{
   const int buf_sz = sizeof(val)*3;
   static char buf[buf_sz];
   *buf = 0;

   ltoa( val, buf, 10 );
   return *this += buf;
}

String& String::operator += ( int val )
{
   const int buf_sz = sizeof(val)*3;
   static char buf[buf_sz];
   *buf = 0;

   itoa( val, buf, 10 );
   return *this += buf;
}

String& String::operator += ( ulong val )
{
   const int buf_sz = sizeof(val)*3;
   static char buf[buf_sz];
   *buf = 0;

   ultoa( val, buf, 10 );
   return *this += buf;
}

String& String::operator += ( double val )
{
   const int buf_sz = sizeof(val)*10;
   static char buf[buf_sz];
   *buf = 0;

   ftoa( val, buf, buf_sz );
   return *this += buf;
}

//String String::operator + ( const char* str ) const
//{
//   String res( Length() + (str ? strlen(str) : 0) );
//   res = value;   
//   return res += str;
//}
//
//String String::operator + ( char ch ) const
//{
//	 String res( Length()+1 );
//    res = value;
//	 return res += ch;
//}
//
String operator + ( char ch, const String &str )
{
   String res( str.Length()+1 );
   res = ch;   
   return res += str;
}
