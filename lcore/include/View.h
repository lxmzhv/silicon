// Визуальные объекты

#pragma once

#include <windows.h>
#include <zmouse.h>
#include <points.h>

class DLL_OBJ View
{
   public:
                 View( WNDPROC proc, const char* class_name = "AlView", LPCTSTR menu=0 );
                 ~View() { if( hWnd ) Hide(); }
            bool Create( HWND owner = 0, int left=100, int top=100, int sz_x=500, int sz_y=400, const char* title = "Окно" );
            void SetText( const char* title ) { Create(); if( title ) SetWindowText(hWnd,title); }
            void Run( int show_mode = SW_SHOWNORMAL );
 virtual    void Show( int show_mode = SW_SHOWNORMAL ) { if( hWnd ) ShowWindow( hWnd, show_mode ); Update(); }
            void Hide() { Show( SW_HIDE ); }
            void Update() { if( hWnd ) UpdateWindow( hWnd ); }
            HWND GetParent() { return hWnd ? ::GetParent(hWnd) : 0; }
            //HWND Handle() { return hWnd; }
            // Принять сообщение винды
            bool DispatchMsg();

   protected:
             long style;
             HWND hWnd;
             char className[0x100];
};

LRESULT CALLBACK IndicProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

const Point2D<int> IndicSize( 300, 100 );

class DLL_OBJ Indicator : public View
{
   public:
                 Indicator();
                 ~Indicator();
            bool Create( HWND owner=0, int left=100, int top=100, const char* title="Индикатор процесса" ) { return View::Create(owner,left,top,IndicSize.x,IndicSize.y,title); }
                 // Установить индикатор на столько-то процентов
            void SetState( long percent, const char* msg = 0 );
            long CurPos() { return curPos; }
           char* Text() { return text; }
 virtual    void Show( int show_mode = SW_SHOWNORMAL );

   protected:
                 long curPos; // текущая позиция в процентах
                 char text[0x100];
    static const long height = 10;

   friend LRESULT CALLBACK IndicProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
};

void DLL_OBJ InvalidateWindow( HWND hWnd, bool erase = false );
void DLL_OBJ WinInitBlack( HWND hWnd );

void DLL_OBJ ErrorMsg( const char* msg );
void DLL_OBJ Error( const char* msg );

HWND DLL_OBJ GetActiveWin();

inline int GET_X_LPARAM( LPARAM lp ) { return LOWORD( lp ); }
inline int GET_Y_LPARAM( LPARAM lp ) { return HIWORD( lp ); }

// Вспомогательные функции для работы с элементами диалогов
double DLL_OBJ GetDlgNumber( HWND hDlg, int idc_item );
void DLL_OBJ SetDlgNumber( HWND hDlg, int idc_item, double value );

template <class P>
void PointToDlg( HWND hDlg, P& p, int cnt, int* IDs )
{
   if( IDs )
      for( int i = 0; i < cnt; i++ )
         SetDlgNumber( hDlg, IDs[i], p[i] );
}

template <class P>
void PointFromDlg( HWND hDlg, P& p, int cnt, int* IDs )
{
   if( IDs )
      for( int i = 0; i < cnt; i++ )
         p[i] = (P::ElemType)GetDlgNumber( hDlg, IDs[i] );
}