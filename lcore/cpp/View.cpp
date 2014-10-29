// Визуальные объекты

#include <view.h>
#include <numbers.h>
#include <stdio.h>

Indicator* CurIndic = 0;

//
// class View
//
View::View( WNDPROC proc, const char* class_name, LPCTSTR menu ): hWnd(0), style(WS_OVERLAPPEDWINDOW|WS_POPUP)
{
   className[0] = 0;
   if( class_name )
   {
      strncpy_s( className, sizeof(className), class_name, 0xFF );
      className[0xFF] = 0;
   }
   WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW|CS_VREDRAW, proc, 0L, 0L,
                     GetModuleHandle(0), 0, 0, (HBRUSH)GetStockObject(WHITE_BRUSH), 
                     menu, className, 0 };
   RegisterClassEx( &wc );
}

bool View::Create( HWND owner, int left, int top, int sz_x, int sz_y, const char* title )
{
   if( hWnd )   return true;
   // Create the application's window.
   hWnd = CreateWindow( className, title, style, left, top, sz_x, sz_y, 
                        owner ? owner : GetDesktopWindow(), 0, GetModuleHandle(NULL), 0 );
   return hWnd != 0;
}

bool View::DispatchMsg()
{
   MSG msg;
   if( !GetMessage(&msg,NULL,0,0) )   return false;
	TranslateMessage(&msg);
	DispatchMessage(&msg);
   return true;
}

void View::Run( int show_mode )
{
   if( !Create() )   return;
   Show( show_mode );
   while( DispatchMsg() );
}

//
//class Indicator
//
Indicator::Indicator():View( IndicProc, "AlIndic" ), curPos(0) 
{
   *text=0; style=WS_CHILD|WS_BORDER|WS_CAPTION; 
}

Indicator::~Indicator()
{
   CurIndic = 0;
}

void Indicator::SetState( long percent, const char* msg_text )
{ 
   if( !msg_text && curPos == percent )
      return;

   if( percent >= 0 )
      curPos = percent;

   if( msg_text )
      strncpy_s( text, sizeof(text), msg_text, 0xFF );

   text[0xFF] = 0;
   Update();
   DispatchMsg(); // Обработка сообщения (для того чтобы окно индикатора не вешало прогу)
}

void Indicator::Show( int show_mode )
{
// Позиционируемся в центре родительского окна
   if( show_mode == SW_SHOWNORMAL )
   {
      HWND h_owner = GetParent();
      RECT rect;
      if( h_owner && GetClientRect(h_owner,&rect) )
      {
         Point2D<int> p( rect.right-rect.left, rect.bottom-rect.top );
         GetWindowRect( hWnd, &rect );
         Point2D<int> sz( rect.right-rect.left, rect.bottom-rect.top );
         p = (p - sz)/2;
         SetWindowPos( hWnd, 0, p.x, p.y, sz.x, sz.y, 0 );
      }
   }
   CurIndic = show_mode == SW_HIDE ? 0 : this;
   View::Show( show_mode );
}


LRESULT CALLBACK IndicProc( HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam )
{
   switch( msg )
   {
      case WM_PAINT:
      {
         HDC hDC = GetDC( hWnd );

         long cur_pos = CurIndic ? CurIndic->CurPos() : -1;
         bool need_perc = cur_pos > 0 && cur_pos <= 100;

         RECT r;
         ::GetClientRect( hWnd, &r );
         POINT d = { (r.right - r.left)/10, (r.bottom + r.top)/2 };

         RECT rc = { r.left+d.x, d.y, r.right-2*d.x, d.y + Indicator::height };
         // Похоже, лишнее...
         //Rectangle( hDC, rc.left, rc.top, rc.right, rc.bottom );

         HBRUSH blue_brush = CreateSolidBrush( RGB( 0x50, 0xA0, 0xFF ) );
         HBRUSH white_brush = CreateSolidBrush( RGB( 0xFF, 0xFF, 0xFF ) );

         // Заливаем индикатор белым
         HBRUSH old_brush = (HBRUSH)SelectObject( hDC, white_brush );
         Rectangle( hDC, r.left, r.top, r.right, r.bottom );

         // Заполняем индикатор завершенности процесса
         if( need_perc ) // Не всегда нужны эти проценты...
         {
            // Рамка индикатора
            Rectangle( hDC, rc.left+1, rc.top+1, rc.right-1, rc.bottom-1 );

            // На сколько процентов завершено
            SelectObject( hDC, blue_brush );
            Rectangle( hDC, rc.left+1, rc.top+1, rc.left+1+(rc.right-rc.left-2)*cur_pos/100, rc.bottom-1 );
         }

         // Информация
         char* txt = CurIndic ? CurIndic->Text() : "";
         TextOut( hDC, rc.left, rc.top-20, txt, (int)strlen(txt) );

         // Проценты
         if( need_perc )
         {
            static char buf[0x40];
            _ltoa_s( cur_pos, buf, 0x40, 10 );
            strcat_s( buf, 0x40, "%" );
            TextOut( hDC, rc.right+5, rc.top - 5, "     ", 5 );
            TextOut( hDC, rc.right+5, rc.top - 5, buf, (int)strlen(buf) );
         }

         SelectObject( hDC, old_brush );
         DeleteObject( blue_brush );
         DeleteObject( white_brush );

         ReleaseDC( hWnd, hDC );
         return 0;
      }
 		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
  }
  return DefWindowProc( hWnd, msg, wParam, lParam );
}

void ErrorMsg( const char* msg )
{
   MessageBox( 0, msg ? msg : "", "Ошибка", MB_OK|MB_ICONEXCLAMATION );
}

void Error( const char* msg )
{
   ErrorMsg( msg );
   throw(1);
}

HWND GetActiveWin()
{
   HWND hwnd = GetActiveWindow();
   return hwnd ? GetLastActivePopup(hwnd) : 0;
}

void InvalidateWindow( HWND hWnd, bool erase )
{
   InvalidateRect( hWnd, 0, erase );
   UpdateWindow( hWnd );
}

void WinInitBlack( HWND hWnd )
{
   PAINTSTRUCT paintStruct;
   HDC hDC = BeginPaint( hWnd, &paintStruct );

   RECT r;
   GetClientRect( hWnd, &r );
   POINT sz = { r.right - r.left, r.bottom - r.top };

   BitBlt( hDC, 0, 0, sz.x, sz.y, 0, 0, 0, BLACKNESS );

   EndPaint( hWnd, &paintStruct );
}

//
// Вспомогательные функции для работы с элементами диалогов
//

double GetDlgNumber( HWND hDlg, int idc_item )
{
   static char buf[0x201];
   GetDlgItemText( hDlg, idc_item, buf, sizeof(buf)-1 );
   return atof( buf );
}

void SetDlgNumber( HWND hDlg, int idc_item, double value )
{
   static char buf[0x201];
   //ftoa( value, buf, sizeof(buf)-1 );
   sprintf_s( buf, sizeof(buf), "%g", value );
   SetDlgItemText( hDlg, idc_item, buf );
}
