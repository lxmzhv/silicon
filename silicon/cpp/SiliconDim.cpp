// Dimensions

#include <Silicon.h>
#include <SiliconDim.h>
#include <DigitMethods.h>
#include <PorousModel.h>

//
// Fractal dimension
//

int FracDimInit( const PorousModel& model, double& start_eps )
{
  const Point<ulong>& sz = model.GetSize();
  bool  slit_only = (model.ViewOpt().flags & vfCALC_SLIT_DIM) != 0;

  int min_n = Min( sz.y, sz.z );
  if( !slit_only && sz.x != 1 )
    min_n = Min( min_n, (int)sz.x );

  start_eps = 1.0;
  return log2( min_n );
}

double FracDimNextEps( double eps )
{
  return eps * 2;
}

double CalcFractalDim( const PorousModel& model, bool invert, double edge, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  Point<long>         sz( model.GetSize() ), p1, p2, p_max;
  long   max_deep = model.GetMaxDeep();
  bool   found = false;
  ulong  box_cnt = 0, total = 0;
  long   max_z = model.GetSize().z-1;//max_deep;

  if( invert && max_deep <= 1 )
    max_z = model.GetSize().z-1;

  int eps = (int)edge;
  Maximize( eps, 1 );

  int sz_x = x_to - x_from + 1;
  double mul = 100.0 * eps / sz_x * eps / sz.y * eps / (max_z+1); // mix multipliers to avoid overflow
  double perc_mul = (perc_max - perc_min);

  for( p1.x = x_from; p1.x <= x_to; p1.x += eps )
    for( p1.y = 0; p1.y < sz.y; p1.y += eps )
      for( p1.z = 0; p1.z <= max_z; p1.z += eps, ++total )
      {
        if( indic )
          indic->SetState( long(100*perc_min + mul*total*perc_mul) );

        if( eps == 1 ) // to speed up
        {
          if( invert ? !matrix.Value(p1) : matrix.Value(p1) )
            ++box_cnt;
          continue;
        }

        p_max.x = Min( p1.x + eps - 1, x_to     );
        p_max.y = Min( p1.y + eps - 1, sz.y - 1 );
        p_max.z = Min( p1.z + eps - 1, max_z    );

        found = false;

        for( p2.x = p1.x; p2.x <= p_max.x && !found; ++p2.x )
          for( p2.y = p1.y; p2.y <= p_max.y && !found; ++p2.y )
            for( p2.z = p1.z; p2.z <= p_max.z; ++p2.z )
              if( invert ? !matrix.Value(p2) : matrix.Value(p2) )
              {
                found = true;
                ++box_cnt;
                break;
              }
      }

  double res = box_cnt;

  if( edge < 1 )
    res /= edge*edge*edge;

  return res;
}

// Update dimension value when point 'pnt' was changed (pore was created)
// Input/Output: p_value 
// Returns true if p_value was changed
bool UpdateFractalDim( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  Point<long>         sz( model.GetSize() ), p1, p2, p_max;
  long                max_deep = model.GetMaxDeep();
  bool                found = false;
  ulong               box_cnt = 0;
  long                max_z = model.GetSize().z-1;//max_deep;

  if( pnt.x < x_from || pnt.x > x_to )
    return false;

  if( invert && max_deep <= 1 )
    max_z = model.GetSize().z-1;

  int eps = (int)edge;
  Maximize( eps, 1 );

  if( eps == 1 ) // to speed up
  {
    *p_value += (invert ? -1.0 : 1.0);
    return true;
  }

  p1.x = pnt.x - (pnt.x - x_from) % eps;
  p1.y = pnt.y - pnt.y % eps;
  p1.z = pnt.z - pnt.z % eps;

  p_max.x = Min( p1.x + eps - 1, x_to     );
  p_max.y = Min( p1.y + eps - 1, sz.y - 1 );
  p_max.z = Min( p1.z + eps - 1, max_z    );

  found = false;

  for( p2.x = p1.x; p2.x <= p_max.x; ++p2.x )
    for( p2.y = p1.y; p2.y <= p_max.y; ++p2.y )
      for( p2.z = p1.z; p2.z <= p_max.z; ++p2.z )
        if( p2 != pnt && (invert ? !matrix.Value(p2) : matrix.Value(p2)) )
          return false;

  double diff = 1.0;
  if( edge < 1 )
    diff /= edge*edge*edge;

  *p_value += (invert ? -diff : diff);
  return true;
}

//
// Correlation dimension
//

int CorDimInit( const PorousModel& model, double& start_eps )
{
  const Point<ulong>& sz = model.GetSize();
  bool  slit_only = (model.ViewOpt().flags & vfCALC_SLIT_DIM) != 0;
  ulong  sz_x = slit_only ? 1 : sz.x;
  int   max_n = Max( Max( sz_x, sz.y ), sz.z );

  const int deep = 0;
  start_eps = 1.0 / ( ((unsigned __int64)1)<<deep );

  //return (int)(max_n/2 - start_eps);
  return log2( max_n-1 ) + deep /*+ 1*/;
}

double CorDimNextEps( double eps )
{
  return eps*2;
}

double CalcCorrelation( const PorousModel& model, bool invert, double eps, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  Point<long>         sz( model.GetSize() ), p1, p2, p_min, p_max;
  long    max_deep = model.GetMaxDeep();
  ulong   n = 0, cnt = 0, total = 0;
  double  perc_mul = (perc_max - perc_min);
  long    max_z = max_deep;
  long    sz_x = x_to - x_from + 1;

  if( invert )
    max_z = sz.z - 1;

  double mul = 100.0 / sz_x / sz.y / (max_z+1);
  long i_eps = (long)eps;
  Maximize(i_eps, (long)1);

  for( p1.x = x_from; p1.x <= x_to; ++p1.x )
    for( p1.y = 0; p1.y < sz.y; ++p1.y )
      for( p1.z = 0; p1.z <= max_z; ++p1.z, ++total )
      {
        if( indic )
          indic->SetState( long(100*perc_min + mul*total*perc_mul) );
        if( invert ? !matrix.Value(p1) : matrix.Value(p1) )
        {
          ++cnt;

          p_min.x = Max( p1.x - i_eps, x_from );
          p_min.y = Max( p1.y - i_eps, (long)0 );
          p_min.z = Max( p1.z - i_eps, (long)0 );

          p_max.x = Min( p1.x + i_eps, x_to   );
          p_max.y = Min( p1.y + i_eps, sz.y-1 );
          p_max.z = Min( p1.z + i_eps, max_z  );

          for( p2.x = p_min.x; p2.x <= p_max.x; ++p2.x )
            for( p2.y = p_min.y; p2.y <= p_max.y; ++p2.y )
              for( p2.z = p_min.z; p2.z <= p_max.z; ++p2.z )
                if( p1 != p2 )
                  if( invert ? !matrix.Value(p2) : matrix.Value(p2) )
                    ++n;
        }
      }

  return cnt < 2 ? 1.0 : double(n)/(cnt*(cnt-1));
}

// Update dimension value when point 'pnt' was changed (pore was created)
// Input/Output: p_value 
// Returns true if p_value was changed
bool UpdateCorrelationDim( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  Point<long>         sz( model.GetSize() ), p1, p_min, p_max;
  long                max_deep = model.GetMaxDeep();
  ulong               cnt      = model.GetPoreCount(), old_cnt = 0;
  long                max_z    = max_deep, n = 0;
  long                sz_x     = x_to - x_from + 1;

  if( sz_x != sz.x )
  {
    // Not implemented
    *p_value = 0.0;
    return false;
  }

  if( invert )
  {
    max_z = sz.z - 1;
    cnt = sz_x*sz.y*sz.z - cnt;
    old_cnt = cnt + 1;
  }
  else
    old_cnt = cnt - 1;

  if( cnt < 2 || old_cnt < 2 )
  {
    *p_value = 0.0;
    return true;
  }

  long i_eps = (long)edge;
  Maximize(i_eps, (long)1);

  p_min.x = Max( pnt.x - i_eps, x_from );
  p_min.y = Max( pnt.y - i_eps, (long)0 );
  p_min.z = Max( pnt.z - i_eps, (long)0 );

  p_max.x = Min( pnt.x + i_eps, x_to   );
  p_max.y = Min( pnt.y + i_eps, sz.y-1 );
  p_max.z = Min( pnt.z + i_eps, max_z  );

  for( p1.x = p_min.x; p1.x <= p_max.x; ++p1.x )
    for( p1.y = p_min.y; p1.y <= p_max.y; ++p1.y )
      for( p1.z = p_min.z; p1.z <= p_max.z; ++p1.z )
        if( pnt != p1 )
          if( invert ? !matrix.Value(p1) : matrix.Value(p1) )
            n += 2;

  double new_n = (*p_value * old_cnt * (old_cnt-1)) + (invert ? -n : n);
  *p_value = new_n / (cnt * (cnt-1));
  return true;
}

//
// Mass dimension
//

int MassDimInit( const PorousModel& model, double& start_eps )
{
  start_eps = 1.0;
  return log2( (int)model.GetSize().z-1 ) + 2;
}

double MassDimNextEps( double eps )
{
  return eps * 2;
}

double CalcMassDim( const PorousModel& model, bool invert, double eps, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  Point<long>    sz( model.GetSize() ), p1, p2;
  long           max_deep = Min( model.GetMaxDeep(), (long)eps );
  long           cnt = 0, total = 0;

  double  mul = 100.0 / (x_to-x_from+1) / sz.y / (eps+1);
  double  perc_mul = (perc_max - perc_min);

  for( p1.z = 0; p1.z <= max_deep; ++p1.z )
  {
    for( p1.y = 0; p1.y < sz.y; ++p1.y, ++total )
      for( p1.x = x_from; p1.x <= x_to; ++p1.x )
        if( matrix.Value(p1) )
          ++cnt;
    if( indic )
      indic->SetState( long(100*perc_min + mul*total*perc_mul) );
  }

  return cnt;
}

// Update dimension value when point 'pnt' was changed (pore was created)
// Input/Output: p_value
// Returns true if p_value was changed
bool UpdateMassDim( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value )
{
  long max_deep = Min( model.GetMaxDeep(), (long)edge );

  if( pnt.z > max_deep )
    return false;

  *p_value += 1.0;
  return true;
}

//
// class Dmension
//

void Dimension::Recalc( const PorousModel& model, PlotUpdateFunc plot_func, HWND hWnd )
{
  static bool calculation = false;

  if( valid || calculation ) // to avoid recursion calls
    return;

  calculation = true;

  P<Indicator> indic;
  if( hWnd )
  {
    indic = new Indicator();
    indic->View::Create( hWnd, 100, 100, 400, 100, "Calculating dimension..." );
    indic->Show();
  }

  // Находим точки

  double eps = 1.0;
  int step_num = init_func( model, eps );
  double perc_mul = 0.0;

  if( step_num > 0 )
  {
    values = new DPoint2D[step_num];
    points = new DPoint2D[step_num];
    perc_mul = 1.0 / step_num;
  }

  points_cnt = 0; // чтобы на время вычислений можно было корректно отрисовывать размерность

  Buf    buf;
  double value = 0.0;

  bool  slit_only = (model.ViewOpt().flags & vfCALC_SLIT_DIM) != 0;
  long  x_from = slit_only ? model.VertSlitDeep() : 0;
  long  x_to   = slit_only ? model.VertSlitDeep() : model.GetSize().x-1;

  for( int i = 0; i < step_num; ++i, eps = next_eps_func(eps) )
  {
    if( indic )
    {
      sprintf_s( buf, sizeof(Buf), "Epsilon: %g (step %d out of %d)", eps, i+1, step_num );
      indic->SetState( -1, buf );
    }

    value = compute_func( model, inverted, eps, x_from, x_to, indic, perc_mul*i, perc_mul*(i+1) );

    values[i].x = eps;
    values[i].y = value;

    points[i].x = epsSign*log2( eps );
    points[i].y = value > NULL_DOUBLE ? log2( value ) : 0.0;

    ++points_cnt;

    // Находим прямую наилучшего приближения
    line = CalcMiddleSqrLineFunc( points_cnt, points, false );

    // Рассчитываем размерность, как угловой коэфф. прямой
    dimension = !IsNullNum(line.b) ? (-line.a/line.b) : LARGE_DOUBLE*Sign(line.a);

    // Отрисовываем вычисленное
    if( plot_func )
      plot_func( hWnd );
  }

  valid = true;
  calculation = false;
}

void Dimension::Update( const PorousModel& model, SimplePoint<long> pnt )
{
  if( !update_func )
    return;

  if( !valid )
  {
    Recalc( model, NULL, NULL );
    return;
  }

  bool  slit_only = (model.ViewOpt().flags & vfCALC_SLIT_DIM) != 0;
  long  x_from = slit_only ? model.VertSlitDeep() : 0;
  long  x_to   = slit_only ? model.VertSlitDeep() : model.GetSize().x-1;

  for( int i = 0; i < points_cnt; ++i )
    if( update_func( model, pnt, inverted, values[i].x, x_from, x_to, &values[i].y ) )
      points[i].y = (values[i].y > NULL_DOUBLE ? log2( values[i].y ) : 0.0);

  // Находим прямую наилучшего приближения
  line = CalcMiddleSqrLineFunc( points_cnt, points, false );

  // Рассчитываем размерность, как угловой коэфф. прямой
  dimension = !IsNullNum(line.b) ? (-line.a/line.b) : LARGE_DOUBLE*Sign(line.a);

  valid = true;
}

//
// Old code
//

//// Растянуть координату x из размерности sz в sz_to
//// x_from, x_to - результат
//inline void Stretch( ulong sz, ulong sz_to, long x, long& x_from, long& x_to )
//{
//   if( sz_to < sz )
//      x_from = x_to = sz_to*( 2*x + 1 ) / (sz<<1); // от 0 до sz_scr.x-1
//   else if( sz_to > sz )
//   {
//      x_from = (2*x*sz_to + sz) / (sz<<1);
//      x_to = (2*(x+1)*sz_to - sz) / (sz<<1);
//   }
//   else
//      x_from = x_to = x;
//}
//
//
//long PorousModel::CalcHolesScale( int n, Indicator* indic, int cur, int max ) const
//{
//   long cnt = 0;
//   BoolMatrix3D m( Point<ulong>(n,n,n) );
//
//   double k = 100./(max+1), indic_beg = cur*k;
//   if( indic )   indic->SetState( int(indic_beg) );
//
//   long x_from, x_to, y_from, y_to, z_from, z_to;
//   Point<long> sz( GetSize().x, GetSize().y, GetSize().z ), p, pm;
//   for( p.x = 0; p.x < sz.x; p.x++ )
//   {
//      for( p.y = 0; p.y < sz.y; p.y++ )
//         for( p.z = 0; p.z <= maxDeep; p.z++ )
//            if( matrix.Value(p) )
//            {
//               // Узнаём, куда (в какие клетки) растягивается текущая клетка
//               Stretch( sz.x, n, p.x, x_from, x_to );
//               Stretch( sz.y, n, p.y, y_from, y_to );
//               Stretch( sz.z, n, p.z, z_from, z_to );
//
//               // Бежим по полученным клеткам, проверяя их значения
//               for( pm.x = x_from; pm.x <= x_to; pm.x++ )
//                  for( pm.y = y_from; pm.y <= y_to; pm.y++ )
//                     for( pm.z = z_from; pm.z <= z_to; pm.z++ )
//                        if( !m.Value(pm) )
//                        {
//                           m.SetValue( pm, true );
//                           cnt++;
//                        }
//            }
//      if( indic )   indic->SetState( int(indic_beg + k*p.x/(sz.x-1)) );
//   }
//   return cnt;
//}
//
//bool PorousModel::CalcNextDimPoints( DPoint2D* points, long& cnt, Indicator* indic ) const
//{
//   static bool calcing = false;
//   if( calcing || !points )   return false;
//   calcing = true;
//
//   long max_n = Max( Max( GetSize().x, GetSize().y ), GetSize().z );
//   Maximize<long>( max_n, 2 );
//   int max_cnt = min( log2(max_n), 28 );
//   if( cnt > max_cnt )   
//   {
//      calcing = false;
//      return false;
//   }
//
//   int n = 1 << (cnt+1);
//   //for( ; cnt <= max_cnt; n <<= 1 )
//   {
//      if( indic )
//      {
//         static char msg[0x100];
//         sprintf_s( msg, 0x100, "Мелкость разбиения: %ld (шаг %ld из %ld)", n, cnt+1, max_cnt+1 );
//         indic->SetState( -1, msg );
//      }
//      points[cnt].x = cnt + 0.5; // == log2( n/sqrt(2) ) == log2(n) - 0.5
//      points[cnt].y = log2( (double)CalcHolesScale(n,indic,cnt,max_cnt) );
//      
//      cnt++;
//   }
//
//   calcing = false;
//   
//return true;
//}
