// Клеточные автоматы

#pragma once

#include <windows.h>
#include <pointers.h>
#include <vector>
#include <funcs.h>
#include <file.h>

class Model
{
	public:
                 Model( ulong n=1, ulong m=1, const char* fld=0 );
    virtual void Init( ulong n=1, ulong m=1, const char* fld=0 );
    virtual void InitRandom( char states_count = 2 );
            void Step();
    inline  char GetState( long x, long y ) const { return Field()[y*size.x+x]; }
    virtual char NewState( long x, long y ) = 0;
    const POINT& GetSize() const { return size; }
            bool IsValidCoord( int x, int y ) const { return x>=0 && y>=0 && x<size.x && y<size.y; }
  virtual   void Save( File& f );
  virtual   bool Load( File& f );

                 // manually change cell state
    virtual void SetState( long x, long y, char state );

   protected:
            POINT size;  // Размер модели

            virtual   void OnChange( ulong /*x*/, ulong /*y*/, char /*new_state*/ ) {}
            inline    long DataSize() { return size.x*size.y; }

   private:
 	      VP<char> field[2]; // Состояния клеток
            uchar cur;      // текущее поле

     inline char* Field() const { return field[cur]; }
     inline char* NextField() const { return field[1-cur]; }
};

class LifeModel: public Model
{
   public:
                       LifeModel( ulong n=1, ulong m=1 ): Model(n,m) {}
          virtual void InitRandom( char = 2 ) { Model::InitRandom(2); }
          virtual char NewState( long x, long y );
};

class PolyLifeModel: public Model
{
   public:
                       PolyLifeModel( ulong n=1, ulong m=1 ): Model(n,m) {}
          virtual void InitRandom( char = 3 ) { Model::InitRandom(3); }
          virtual char NewState( long x, long y );
};
