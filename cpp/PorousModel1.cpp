// Matrix of silicon model (box) with porous tree

#include <windows.h>
#include <PorousModel1.h>

//
// Tools
//

int Init3dAroundArray( Point<long>* points, int size, int r )
{
   int n = 0, i, j, k;
   for( i = -r; i <= r; ++i )
      for( j = -r; j <= r; ++j )
         for( k = -r; k <= r; ++k )
            if( (i || j || k) && sqr(i)+sqr(j)+sqr(k) <= sqr(r+0.5) )
            {
               if( n > size )
                  return -1;
               points[n].x = i;
               points[n].y = j;
               points[n++].z = k;
            }
   return n;
}

//
//class PorousModel1
//

PorousModel1::PorousModel1( Point<ulong> sz, ModelViewOptions opt, double prob_deg, DPoint2D prob_range, double sideways_k, double upward_k ): 
   PorousModel(sz,opt), visited(sz), probRange(prob_range), sidewayKoeff(sideways_k), upwardKoeff(upward_k), srfInfluence(.0)
{
   Minimize( Maximize( probRange.x, .0 ), 1. );
   Minimize( Maximize( probRange.y, .0 ), 1. );
   Minimize( probRange.x, probRange.y );
   Init( prob_deg );
}

void PorousModel1::Init( double prob_deg )
{
   PorousModel::Init();
   probDeg = prob_deg;
   ClearBuffers();
}

// Группы направлений, для обхода точки
const Point<long> DirsUFDB[8] = { PointUp, PointUp+PointForward, PointForward, PointForward+PointDown, PointDown, 
                                  PointDown+PointBack, PointBack, PointBack+PointUp };

const Point<long> DirsLFRB[8] = { PointLeft, PointLeft+PointForward, PointForward, PointForward+PointRight, 
                                  PointRight, PointRight+PointBack, PointBack, PointBack+PointLeft };

const Point<long> DirsULDR[8] = { PointUp, PointUp+PointLeft, PointLeft, PointLeft+PointDown, PointDown, 
                                  PointDown+PointRight, PointRight, PointRight+PointUp };

// Вернуть массив направлений, перпендикулярных dir
const Point<long>* PerpDirs[6] = { DirsLFRB, DirsLFRB, DirsUFDB, DirsUFDB, DirsULDR, DirsULDR };

inline bool PorousModel1::Value( const Point<long>& p ) const
{
   if( p.z < 0 )   return true; // сверху никогда ничего нет
   if( !matrix.Contains(p) )   return false; // В других направлениях всё заполнено
   return matrix.Value(p);
}

long PorousModel1::CountFilled( const Point<long>& p, const BaseDir3d& dir )
{
   static const Point<long>* dirs = 0, *end = 0;
   static Point<long> cur;
   static long cnt = 0;

   if( p.z < 0 )   return 0; // сверху никогда ничего нет
   if( !matrix.Contains(p) )   return 8; // В других направлениях всё заполнено

   dirs = PerpDirs[dir];
   end = dirs+8;
   for( cnt = 0; dirs < end; dirs++ )
   {
      cur = p;
      cur += *dirs;
      if( !Value(cur) )   cnt++;
   }
   return cnt;
}

inline double PorousModel1::ProbFunction( int& cnt, const int max_cnt )
{
   static double last_deg = -1e+100;
   static DPoint2D last_range( 0, 0 );
   static int last_max_cnt = -1;
   static VP<double> values;  // Кэш значений

   if( max_cnt != last_max_cnt )
      values = new double[(last_max_cnt = max_cnt) + 1];

   if( !IsNullNum(last_deg-probDeg) || last_range != probRange )
   {
      for( int i = 0; i <= max_cnt; i++ )
         values[i] = (probRange.y-probRange.x) * pow( static_cast<double>(i)/max_cnt, probDeg ) + probRange.x;
      last_deg = probDeg;
      last_range = probRange;
   }

   return values[cnt];
}

double PorousModel1::GetProb( const Point<long>& dst_p, const Dir3d& dir )
{
   static Point<long> p;
   static int cells_cnt = 0;

   p = dst_p;
   p -= dir.GetPoint();
   cells_cnt = CountFilled( p, (BaseDir3d)dir.dir1 );

   // Вычисляем вероятность удачного бурения
   return ProbFunction( cells_cnt, 8 ); // Вероятность возникновения отверстия
}

double PorousModel1::GetProbSphere( const Point<long>& dst_p, int radius )
{
   static Point<long> p;
   static int cells_cnt = 0, cells_num = 0;

   p = dst_p;
   cells_cnt = CountAroundFilledCells( p, radius, &cells_num );

   // Вычисляем вероятность удачного бурения
   return ProbFunction( cells_cnt, cells_num ); // Вероятность возникновения отверстия
}

bool PorousModel1::DrillCell( const Point<long>& dst_p, const Dir3d& dir, char srf_state )
{
   static double prob = 0;

   //prob = GetProb( dst_p, dir );
   prob = GetProbSphere( dst_p, 3 );

   // Корректируем вероятность с учетом направления
   if( dir.IsUp() )
      prob *= upwardKoeff;  // Вверх бурить должно быть очень трудно
   if( dir.IsHorizontal() )
      prob *= sidewayKoeff; // вбок бурить должно быть труднее, чем вниз

   // Correct the probability according to surface state
   if( srf_state != sssCHARGED_MOLECULE )
     prob *= srfInfluence;

   // Узнаём судьбу клетки
   if( !CheckProb(prob) )   return false; // Не удалось просверлить

   buf.push_back(dst_p); // Помещаем в буфер для изменения
   return true;
}

int PorousModel1::CountAroundFilledCells( const Point<long>& p, int radius, int* cells_num )
{
   static VP< Point<long> > around;
   static int arr_sz = -1;
   static int last_radius = -1;

   if( last_radius != radius )
   {
      int cube_side = 2*radius + 1;
      int buf_sz = cube_side*cube_side*cube_side; // Коэффициент 3, т.к. для каждой клетки три координаты
      around = new Point<long>[buf_sz];
      arr_sz = Init3dAroundArray( around, buf_sz, radius );
      last_radius = radius;
   }

   static Point<long> pnt, *v;
   
   static int i;
   v = around;
   int filled_cells = 0;
   for( i = 0; i < arr_sz; ++i, ++v )
   {
      pnt = p + *v;
      if( pnt.z < 0 || !Value( pnt ) )
         ++filled_cells;
   }

   *cells_num = arr_sz;
   return filled_cells;
}

void PorousModel1::WaveDrill( const Point<long>& p, const Model& surface )
{
   static DrillPoint dp, *cur_dp = 0;
   static Point<long> dst_p;
   static Dir3d back_dir, cur_dir;
   static char i;

   cur_dir.Set( dirNULL, dirNULL, dirNULL );

   wave.push( dp.Set(p, dirDOWN, dirNULL, dirNULL) );
   for( ; !wave.empty(); wave.pop() )
   {
      cur_dp = &wave.front()->value;
      dst_p = cur_dp->p;
      dst_p += cur_dp->dir.GetPoint();

      if( !matrix.Contains(dst_p) )   continue;

      if( !Value( dst_p ) ) // Отверстия нет - бурим
         DrillCell( dst_p, cur_dp->dir, surface.GetState(dst_p.x, dst_p.y) );
      else
         if( !visited.Value(dst_p) )
         {
            back_dir = cur_dp->dir.Back();
            for( i = 0; i < DIR_3D_CNT; ++i )
            {
               cur_dir = dir3dTools.GetDir(i);
               if( cur_dir != back_dir && cur_dir.IsNullBaseDir() )           
                  wave.push( dp.Set( dst_p, cur_dir ) );
            }
            /*for( i = dirUP; i <= dirLAST; ++i )
            {
               cur_dir.dir1 = i;
               if( cur_dir.dir1 != back_dir.dir1 )           
                  wave.push( dp.Set( dst_p, cur_dir ) );
            }*/
         }

      visited.SetValue(dst_p,true);
   }
}

void PorousModel1::ClearBuffers()
{
   buf.clear();
   visited.Init( GetSize() );
}

void PorousModel1::ApplyChanges()
{
   static ArrPointLong::iterator it;

   for( it=buf.begin(); it<buf.end(); it++ )
      CreatePore( *it );
   
   ClearBuffers();
}

void PorousModel1::Save( File& f )
{
   matrix.Save( f );
   f << probDeg << probRange << sidewayKoeff << upwardKoeff << (ushort&)viewOpt.mode << vertSlitDeep << horzSlitDeep
     << poreCount << maxDeep;
}

bool PorousModel1::Load( File& f )
{
   Init();
   bool ok = matrix.Load( f );
   visited.Init( GetSize() );
   f >> probDeg >> probRange >> sidewayKoeff >> upwardKoeff >> (ushort&)viewOpt.mode >> vertSlitDeep >> horzSlitDeep 
     >> poreCount >> maxDeep;
   return ok;
}