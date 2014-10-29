// Взаимодействие с файлами

#pragma once

#include <windows.h>
#include <fstream>
#include <funcs.h>
#include <strings.h>

using namespace std;

const long fomREAD   = 0x01;
const long fomWRITE  = 0x02;
const long fomCREATE = 0x04;
const long fomDEFAULT = fomREAD|fomWRITE;

class DLL_OBJ File
{
   public:
               File( const char* file_name=0, long mode = fomDEFAULT );
               ~File() { Close(); }

          bool Open( const char* file_name, long mode = fomDEFAULT );
          void Close();

          bool Write( const void* data, ulong size );
          bool Read( void* data, long size );

                            bool Write( const char* s ) { return Write( s, (ulong)strlen(s) ); }
        template< class T > bool Write( T& val ) { return Write( &val, (ulong)sizeof(T) ); }
        template< class T > bool Read( T& val ) { return Read( &val, (ulong)sizeof(T) ); }

                            File& operator << ( long val ) { Write( val ); return *this; }
                            File& operator << ( const char* s ) { Write( s ); return *this; }
        template< class T > File& operator << ( T& val ) { Write( val ); return *this; }
        template< class T > File& operator >> ( T& val ) { Read( val ); return *this; }

        //operator HANDLE() { return hFile; }
        //  HANDLE GetHandle() { return hFile; }

   private:
        HANDLE hFile;
        String fileName; // Имя файла - справочно

          void ActionError( const char* msg );
};

bool DLL_OBJ AskFileName( char* file_name, const char* filter, DWORD flags, const char* title=0, HWND h_owner=0 );
void DLL_OBJ ApplyFileExtension( char* file_name, int buf_sz, const char* needed_ext );
bool DLL_OBJ IsFileExists( const char* file_name );

