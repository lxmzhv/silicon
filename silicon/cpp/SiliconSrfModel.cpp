// Клеточные автомат модели порообразования

#include <points.h>
#include <SiliconSrfModel.h>

//
//class SiliconSrfModel
//

// Вероятности перехода 
double Probs[3][3] = { { 0.1, 0.3, 0.6 },
                       { 0.3, 0.1, 0.6 },
                       { 0.3, 0.6, 0.1 } };

double PSums[3][3]; // Суммы вероятностей p[3][3] смены состояний

SiliconSrfModel::SiliconSrfModel( ulong n, ulong m, ulong time0, ulong time1, ulong time2 ): Model(n,m), waitings(0)
{
   waitTime[sssNO_MOLECULE]          = time0;
   waitTime[sssNOT_CHARGED_MOLECULE] = time1;
   waitTime[sssCHARGED_MOLECULE]     = time2;

   InitWaitings();

// Рассчитываем суммы вероятностей
   double sum[3] = { 0, 0, 0 };
   for( int i = 0; i < 3; i++ )
      for( int j = 0; j < 3; j++ )
         PSums[i][j] = (sum[i] += Probs[i][j]);
}

void SiliconSrfModel::Save( File& f )
{
   Model::Save( f );
   f << waitTime;
   f.Write( (char*)(ulong*)waitings, DataSize()*sizeof(ulong) );
}

bool SiliconSrfModel::Load( File& f )
{
   Model::Load( f ); // Здесь вызовется метод SiliconSrfModel::Init(...)
   f >> waitTime;
   f.Read( (char*)(ulong*)waitings, DataSize()*sizeof(ulong) );
   return true;
}

// Вероятности переходов
const double p0To1_1 = 1;
const double p0To1_2 = .0002;
const double p1To2  = 1;
const double p1To0  = .99;
const double p2To1  = .000005;
const double p2To0 = 1;

int InitAroundArray( int* points, int r )
{
   int n = 0;
   for( long i = -r; i <= r; i++ )
      for( long j = -r; j <= r; j++ )
         if( (i || j) && sqr(i)+sqr(j) <= sqr(r+0.5) )
         {
            points[n++] = i;
            points[n++] = j;
         }
   return n;
}

void SiliconSrfModel::CountArroundStates( long x, long y, char* cnt ) const
{
   const int r = 2;
   static int points[4*(r+1)*(r+1)*2]; // size/2 >= Pi * sqr(R)
   static int* end = 0;
   if( !end )   end = points + InitAroundArray( points, r );

   memset( cnt, 0, 3*sizeof(char) );
   for( int* p = points; p < end; p += 2 )
      cnt[ulong( GetState( (x+*p+size.x)%size.x, (y+*(p+1)+size.y)%size.y) )]++;
}

char SiliconSrfModel::NewState( long x, long y )
{
   static char new_state = 0;
   new_state = GetState(x,y);
   if( GetWaiting(x,y) < waitTime[new_state] )   return new_state;

// Считаем кол-во клеток по состояниям
   static char cnt[3] = {0,0,0};
   CountArroundStates( x, y, cnt );

// Определяем во что должна перейти клетка
   switch( new_state )
   {
      case sssNO_MOLECULE:
         if( (cnt[sssNOT_CHARGED_MOLECULE] >= 1 || cnt[sssCHARGED_MOLECULE] >= 2) && CheckProb(p0To1_1) ||
              cnt[sssNOT_CHARGED_MOLECULE] == 0 && cnt[sssCHARGED_MOLECULE] == 0 && CheckProb(p0To1_2) )
            new_state = sssNOT_CHARGED_MOLECULE;
         break;
      case sssNOT_CHARGED_MOLECULE:
         if( (cnt[sssNOT_CHARGED_MOLECULE] >= 3 || cnt[sssCHARGED_MOLECULE] >= 2) && CheckProb(p1To2) )
            new_state = sssCHARGED_MOLECULE;
         else if( CheckProb(p1To0) )
            new_state = sssNO_MOLECULE;
         break;
      case sssCHARGED_MOLECULE:
         if( (cnt[sssNOT_CHARGED_MOLECULE] >= 3 || cnt[sssCHARGED_MOLECULE] >= 2) && CheckProb(p2To1) )
            new_state = sssNOT_CHARGED_MOLECULE;
         else if( CheckProb(p2To0) )
            new_state = sssNO_MOLECULE;
         break;
   }
   return new_state;
}

void SiliconSrfModel::OnChange( ulong x, ulong y, char new_state )
{
   Model::OnChange( x, y, new_state );

   ulong new_waiting = GetState(x,y) != new_state ? 0 : min( GetWaiting(x,y)+1, waitTime[new_state] );
   SetWaiting( x, y, new_waiting );
}

void SiliconSrfModel::InitWaitings( bool random )
{
   waitings = new ulong[DataSize()+1];
   if( random )
   {
      ulong max_time = max( max( waitTime[sssNO_MOLECULE], waitTime[sssNOT_CHARGED_MOLECULE] ), waitTime[sssCHARGED_MOLECULE] );
      for( long x = 0; x < size.x; x++ )
         for( long y = 0; y < size.y; y++ )
            SetWaiting( x, y, rand()%(max_time+1) );
   }
   else
      memset( waitings, 0, sizeof(ulong)*size.x*size.y );
}

void SiliconSrfModel::InitRandom( char )
{
   Model::InitRandom(3);
   InitWaitings( true );
}

void SiliconSrfModel::Init( ulong n, ulong m, const char* fld )
{
   Model::Init( n, m, fld );
   InitWaitings( true );
}

//
//class OxidModel
// 
char OxidModel::NewState( long x, long y )
{
   char state = GetState( x,y );
   if( state )   return state;
   return CheckProb(prob) ? 1 : 0;
}

void OxidModel::Save( File& f )
{
   Model::Save( f );
   f << prob;
}

bool OxidModel::Load( File& f )
{
   Model::Load( f );
   f >> prob;
   return true;
}
