// Взаимодействие с файлами

#include <file.h>
#include <view.h>

//
//class File
//
File::File( const char* file_name, long mode ): hFile( INVALID_HANDLE_VALUE )
{
   Open( file_name, mode );
}

void File::ActionError( const char* msg )
{
   Error( String(msg) << " \"" << fileName << "\"!" );
}

bool File::Open( const char* file_name, long mode )
{
   Close();
   fileName = file_name;

   ulong access = 0;
   ulong share = FILE_SHARE_READ|FILE_SHARE_WRITE;
   ulong create = OPEN_EXISTING;
   ulong flags = FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS;
   if( mode&fomREAD )    access |= GENERIC_READ;
   if( mode&fomWRITE )   access |= GENERIC_WRITE;
   if( mode&fomCREATE )   create = CREATE_ALWAYS;

   hFile = CreateFile( file_name, access, share, 0, create, flags, 0 );
   if( hFile != INVALID_HANDLE_VALUE )   return true;

   ActionError( "Не удалось открыть файл" );
   return false;
}

void File::Close()
{
   if( hFile != INVALID_HANDLE_VALUE )
   {
      CloseHandle( hFile );
      hFile = INVALID_HANDLE_VALUE;
   }
}

bool File::Write( const void* data, ulong size ) 
{
   ulong num = 0;
   if( WriteFile( hFile, data, size, &num, 0 ) && num == size )   return true;
   ActionError("Не удалось записать данные в файл" );
   return false;
}

bool File::Read( void* data, long size )
{
   ulong num = 0;
   if( ReadFile( hFile, data, size, &num, 0 ) && num == size )   return true;
   ActionError( "Не удалось прочитать данные из файла" );
   return false;
}


bool AskFileName( char* file_name, const char* filter, DWORD flags, const char* title, HWND h_owner )
{
   OPENFILENAME ofn;
   memset( &ofn, 0, sizeof(ofn) );

   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = h_owner;
   ofn.lpstrFile = file_name;
   ofn.nMaxFile = MAX_PATH;
   ofn.lpstrFilter = filter;
   ofn.lpstrTitle = title;
   ofn.Flags = flags;

   return GetOpenFileName(&ofn) != 0;
}

void ApplyFileExtension( char* file_name, int buf_sz, const char* needed_ext )
{
   char* ext = strrchr( file_name, '.' );
   if( !ext || strcmp( ext, needed_ext ) )
      strcat_s( file_name, buf_sz, needed_ext );
}

bool IsFileExists( const char* file_name )
{
   HANDLE hdl = CreateFile( file_name, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_RANDOM_ACCESS, 0 );

   if( hdl == INVALID_HANDLE_VALUE )
      return false;

   CloseHandle( hdl );
   return true;
}
