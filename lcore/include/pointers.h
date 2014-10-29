// Работа с указателями

#pragma once

#include <windows.h>
#include <funcs.h>

template< class T >
class P
{
   public:
                  P( T* init=0 ): ptr(init){}
                  ~P() { Clear(); }
     virtual void Clear() { if( ptr ) delete ptr; ptr = 0; }
                  P<T>& operator = ( P<T>& init )
                  {
                     if( &init != this )
                     {
                        Clear();
                        ptr = init.GetPtr();
                     }
                     return *this;
                  }
                  P<T>& operator = ( T* init )
                  { 
                     if( init!=ptr )
                     {
                        Clear();
                        ptr = init;
                     }
                     return *this;
                  }
               T* operator -> () const { return ptr; }
               T& operator *() const { return *ptr; }
                  operator T*() const { return ptr; }
               T* GetPtr() { T* p = ptr; ptr = 0; return p; }

   protected:
               T* ptr;

   private:
                  P( const P<T>& ) {}

};

template< class T >
class VP: public P<T>
{
   public:
                   VP( T* init=0 ): P<T>(init) {}
          virtual ~VP() { Clear(); }
      virtual void Clear() { if( ptr )   delete[] ptr; ptr = 0; }
            VP<T>& operator = ( T* init ) { this->P<T>::operator=(init); return *this; }
            VP<T>& operator = ( VP<T>& init ) { this->P<T>::operator=(init); return *this; }

   private:
                   VP( const VP<T>& ) {}
            VP<T>& operator = ( const P<T>& ) { return *this; }
};

template < class T >
class DCObject
{
   public:
      DCObject( HDC hdc=0, bool create_dc = true ): hDC(0), hObj(0), hObjOld(0), createDC(create_dc) { InitDC( hdc ); }
      ~DCObject() { Clear(); }

      void InitDC( HDC hdc ) { if( !hDC )   hDC = createDC ? CreateCompatibleDC(hdc) : hdc; }
         T Object() { return hObj; }
  operator HDC() { return hDC; }
      void SetObject( T obj )
      {
         ClearObject();
         if( !(hObj = obj) || !hDC )   return;
         HGDIOBJ h_prev = SelectObject( hDC, hObj );
         if( !hObjOld )   hObjOld = h_prev;
      }
      void ClearObject()
      {
         if( hDC && hObjOld )   SelectObject( hDC, hObjOld );
         if( !hObj )   return;
         DeleteObject( hObj );
         hObj = 0;
      }
      void Clear()
      {
         ClearObject();
         if( !hDC )   return;
         if( createDC )   DeleteDC( hDC );
         hDC = 0;
      }

   private:
             HDC hDC;
            bool createDC;
               T hObj;
         HGDIOBJ hObjOld;
};

template<class T>
void InitArg( T* ptr, const T& value )
{
   if( ptr != NULL )
      *ptr = value;
}
