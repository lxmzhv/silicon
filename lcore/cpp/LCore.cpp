
#include "LCore.h"

BOOL WINAPI DllMain( HINSTANCE, // handle to DLL module 
                     DWORD fdwReason,    // reason for calling function 
                     LPVOID )            // reserved 
{
   switch( fdwReason )
   {
      case DLL_PROCESS_ATTACH:
         //hInstance = hinstDLL;
         return TRUE;
      case DLL_PROCESS_DETACH:
         return TRUE;
   }
   return TRUE;
}
