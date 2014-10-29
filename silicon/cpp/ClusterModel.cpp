// Клеточные автоматы

#include <points.h>
#include <ClusterModel.h>

//
//class Model
//
Model::Model( ulong n, ulong m, const char* fld ): cur(0)
{
   Init( n, m, fld );
}

void Model::Save( File& f )
{
   f << size << cur;
   f.Write( Field(), DataSize() );
}

bool Model::Load( File& f )
{
   POINT sz;
   f >> sz;
   Init( sz.x, sz.y );
   f >> cur;
   f.Read( Field(), DataSize() );
   return true;
}

void Model::SetState( long x, long y, char state )
{
   OnChange( x, y, state ); // must be before state change
   Field()[y*size.x+x] = state;
}

void Model::Step()
{
   char* new_fld = NextField();
   for( long i = 0; i < size.x; i++ )
      for( long j = 0; j < size.y; j++ )
      {
         new_fld[j*size.x+i] = NewState( i, j );
         OnChange( i, j, new_fld[j*size.x+i] );
      }
   cur = 1-cur; // меняем текущее поле
}

void Model::Init( ulong n, ulong m, const char* fld )
{
   size.x = n;
   size.y = m;
   field[0] = new char[n*m+1];
   field[1] = new char[n*m+1];
   cur = 0;
   fld ? memcpy(Field(),fld,n*m) : memset(Field(),0,n*m);
   srand( (ulong)GetTickCount() ); // Инициализация генератора случайных чисел
}

void Model::InitRandom( char states_count )
{
   char* fld = Field();
   for( long i = 0; i < size.x; i++ )
      for( long j = 0; j < size.y; j++ )
         fld[j*size.x+i] = rand()%states_count;
}

//
//class LifeModel
//
char LifeModel::NewState( long x, long y )
{
   long cnt = 0;
   for( long i = -1; i < 2; i++ )
      for( long j = -1; j < 2; j++ )
         if( (i || j) && GetState( (x+i+size.x)%size.x, (y+j+size.y)%size.y ) )
            cnt++;
   return cnt > 3 ? 0 : cnt==3 ? 1 : cnt==2 ? GetState(x,y) : 0;
}

char PolyLifeModel::NewState( long x, long y )
{
   char cnt[3] = {0,0,0};
   for( long i = -2; i <= 2; i++ )
      for( long j = -2; j <= 2; j++ )
         if( (i || j) )
            cnt[ulong( GetState( (x+i+size.x)%size.x, (y+j+size.y)%size.y) )]++;
   if( GetState(x,y) )
      cnt[GetState(x,y)]++;
   return cnt[0]==8 ? 0 : cnt[1] > cnt[2] ? 1 : cnt[1] < cnt[2] ? 2 : GetState(x,y);
}
