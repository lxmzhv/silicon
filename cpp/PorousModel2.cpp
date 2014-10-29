#include <PorousModel2.h>

const double PHYS_K  = 1.38e-23; // [Дж/K]
// [k*T] = Дж = кг*м^2/с^2 = Н*м = Вт*с

const double ELECTRON_CHARGE   = -1.6e-19; // [qulon]
const double ELECTRON_MASS     = 9.10953447e-28; // [gramm]

const double HOLE_CHARGE              = -ELECTRON_CHARGE;
const double HOLE_THERM_VELOCITY_MASS = 0.41*ELECTRON_MASS;  // [gramm]
const double HOLE_MASS                = 0.56*ELECTRON_MASS;  // [gramm]
const double HOLE_FREE_RUN_LEN_T273 = 0.04; // = 0.02 - 0.06 [cm], T = 273.15
const double HOLE_MOBILITY_T273  = 500; // Подвижность дырок в кремнии [cm^2/(V*sec)], T = 273.15
const double HOLE_LIFE_TIME_T273 = 2.5e-3;  // Время жизни дырок в кремнии [sec]

const double HOLE_SPEED    = 1.0;  // moves a step
const double FTOR_SPEED    = 0.05; // moves a step
const double HOLE_MAX_CONCENTRATION = 0.5/CELL_VOLUME;

const double SILICON_DIELECTRIC_EPS = 11.68;

const int MAX_Z = int( SILICON_WIDTH / CELL_SIZE );

const double TIP_CHARGE_MUL = 1; // Во сколько раз суммарный заряд ионов на концах пор больше, чем не на концах.
const double PORE_CELL_CHARGE_MUL = 33.0; // Manually tuned coeff (18 corresponds to small potential errors)
const double PORE_CELL_CHARGE_MUL_2D = 3*PORE_CELL_CHARGE_MUL;

/*
Кремний:
Диэлектрическая постоянная: 12
Подвижность электронов: 1300-1400 см^2/(в*c).
Подвижность дырок: 500 см^2/(в*c).
Продолжительность жизни электрона: 50 — 500 мксек
Длина свободного пробега электрона: 0,1 см
Длина свободного пробега дырки: 0,02 — 0,06 см
Концентрация дырок: 1e14 [cm^(-3)]

------------------------------------------------------------------------

Время свободного пробега электрона в кремнии:
  tau = M_n * m / q = 8.54019e-06 sec
Скорость теплового движения электронов:
  V_t = sqrt(3kT/m) = 3714.84 (см?/сек) TODO спросить, нормальная ли это скорость

------------------------------------------------------------------------

Дрейфовая подвижность дырок и электронов в кремнии:
  M_p =  500-600 [cm^2/(B*c)] (holes)
  M_n = 1300-1500 [cm^2/(B*c)] (electrons)
Средняя скорость в направлении поля:
  V_ср = M*E
Напряжение:
  E = U/d, U - разность потенциалов, d - толщина проводника
  E = -grad(fi), fi - потенциал

Usually: 5-10 B
Maximum: 100 B

Intensity of light: J = 10^12 - 10^19 [photon/(cm*cm)] = 10^16 - 10^23 [photon/(m*m)]
With 100B & 10^12 of J there will be straight pores

---------------
ЗАКОН Аррениуса

Оценка времени жизни ионов Фтора в локализованном состоянии (до следующего прыжка)
T = To * exp( Ea - e*E*d ) / k*T
To - характерное время молекулярных колебаний (10^-12 seconds)
Ea - energy of activation (0.7 eV)
e - charge of electron
E - напряженность (fi/width)
d - mesh size (width/N)
k*T = 0.026 eV (T = 300 K)

Examples:
N = 100
if fi = 0, then T = 0.5 sec
if fi = 5, then T = 0.072 sec
if fi = 50, then T = 2 * 10^-9 sec
*/

PorousHoleModel::PorousHoleModel( Point<ulong> sz, SiliconSrfModel& srf, const OxidModel& oxid_model, int opts, ModelViewOptions vopt,
                                  double tens, double temp, double lum, double cur_strength ):
   PorousModel(sz,vopt),
   surface(srf), oxid(oxid_model),
   tipMul(TIP_CHARGE_MUL),
   opt(opts),
   tension(tens),
   temperature(temp),
   luminosity(lum),
   currentStrength(cur_strength)
{
  // Most probable velocity: v = sqrt(8kT/(mPi)))
  // Mean of velocity:       v = sqrt(3kT/m)
  holeThermSpeed = sqrt(3*PHYS_K*temp/(HOLE_THERM_VELOCITY_MASS*0.001)); // [m/sec], mass converted to kg
  holeThermSpeedSigma = 0.30*holeThermSpeed;

  // Mobility of holes and electrons in Silicon:
  // Mu ~ T^(-3/2)
  holeMobility = HOLE_MOBILITY_T273 * 273.15*sqrt(273.15) / (temperature*sqrt(temperature));

  // NOT USED:
  // Похоже, что считать нужно по-другому, т.к. время жизни дырок в
  // кремнии - это 2.5e-3 sec (HOLE_LIFE_TIME), а по этой формуле
  // выходит: (500*0.56*9.10953447e-28)/(1.6e-19) = 1.59417e-06
  //
  // Время свободного пробега дырок в кремнии:
  //holeFreeRunTime = holeMobility * HOLE_MASS / HOLE_CHARGE;
  //stepTime = holeFreeRunTime; // [sec]

  Init();
}

void PorousHoleModel::InitModelSize( const Point<ulong>& sz )
{
   holeMap.Init( sz );
   holeMap2.Init( sz );
   tipMap.Init( sz );
   ftorMap.Init( sz );
   poreCloud.Init( sz );
   //porePotentials.Init( sz, 0.0 );
   potentials.Init( sz, 0.0 );
   InitHoleCounts( sz );
}

void PorousHoleModel::InitPotential()
{
   const Point<ulong>& sz = GetSize();
   Point<long> p;

   maxPotential = tension;
   minPotential = tension - sz.z*tension/MAX_Z;
   //maxParticlePotential = NULL_DOUBLE;
   //minParticlePotential = 0.0;
   //maxTopSrfPotential = NULL_DOUBLE;

   for( p.x = 0; p.x < sz.x; ++p.x )
     for( p.y = 0; p.y < sz.y; ++p.y )
       for( p.z = 0; p.z < sz.z; ++p.z )
       {
         potentials.GetElem(p) = BasePotential(p);
         //porePotentials.GetElem(p) = 0.0;
       }
}

void PorousHoleModel::Init()
{
   const Point<ulong>& sz = GetSize();

   InitModelSize( sz );

   PorousModel::Init();

   holes.clear();
   ftor.clear();

   maxZDensity = 0;

   InitPotential();

   /* OBSOLETE: reworked using hole density
   newHoles = 0;
   maxHoles = HOLE_MAX_CONCENTRATION*CELL_VOLUME*sz.z*sz.x*sz.y;
   double dx = CELL_SIZE * sz.x;
   double dy = CELL_SIZE * sz.y;
   const double adjustment = 5e-8;
   holeBurnSpeed = dx*dy * currentStrength * stepTime*adjustment / HOLE_CHARGE;
   if( sz.x == 1 )
      holeBurnSpeed *= 3;

   double one_layer_volume = dx*dy*CELL_SIZE;
   Minimize( holeBurnSpeed, HOLE_MAX_CONCENTRATION*one_layer_volume );*/
}

double PorousHoleModel::CalcHoleDensity( double x )
{
  const double gamma = 14;
  const double tau = HOLE_LIFE_TIME_T273;
  const double eta = 1;
  const double s = 1e3;
  // Коэффициент диффузии дырок
  double d = holeMobility * PHYS_K * temperature / HOLE_CHARGE; // was constant 0.9 before
  double l = sqrt(d*tau);

  double g0 = eta*gamma*luminosity;

  return (g0*tau)/(l*l*gamma*gamma-1) * ((gamma*l*l + s*tau)/(l+s*tau)*exp(-x/l) - exp(-gamma*x));
}

void PorousHoleModel::InitHoleCounts( const Point<ulong>& sz )
{
   int num = Max<int>(sz.z,1);
   double slit_volume = CELL_VOLUME*sz.x*sz.y;

   holeCount = new long[num];
   holeCountNeeded = new double[num];
   holeTotalNeeded = 0;

   for( int i = 0; i < sz.z; ++i )
   {
     holeCount[i] = 0;
     holeCountNeeded[i] = CalcHoleDensity(i*CELL_SIZE)*slit_volume;
     holeTotalNeeded += holeCountNeeded[i];
   }
}

bool PorousHoleModel::DiffuseFtor( const SimplePoint<long>& pnt ) const
{
   if( tipMap.Value( pnt ) )
      return true;
   return CheckProb( 0.001 );
}

bool PorousHoleModel::ChemicalReaction( const SimplePoint<long>& hole_pnt ) const
{
   if( opt & phmoUSE_TWIN_HOLES && !holeMap2.Value(hole_pnt) )
      return false;

   if( hole_pnt.z == 0 )
      return surface.GetState( hole_pnt.x, hole_pnt.y ) == sssCHARGED_MOLECULE &&
             oxid.GetState( hole_pnt.x, hole_pnt.y );

   if( hole_pnt.z-1 <= maxDeep &&
       poreCloud.Value(hole_pnt) &&
       (!(opt & phmoUSE_FTOR) || ftorMap.Value(hole_pnt)) )
      return true;

   return false;
}

bool PorousHoleModel::GetElFieldColor( const Point<long>& pnt, COLORREF* color ) const
{
   double cur = potentials.GetElem(pnt);
   double max = maxPotential;
   double min = minPotential;
   double tens_range = max - min;
   //double comp1 = porePotentials.GetElem(pnt);
   //double comp2 = BasePotential(pnt);
   //double cur1 = comp1 + comp2;
   //double max1 = maxParticlePotential, max2 = tension;
   //double min1 = minParticlePotential, min2 = tension - GetSize().z*tension/MAX_Z;

   Maximize( max, NULL_DOUBLE );
   //Maximize( max2, NULL_DOUBLE );

   if( viewOpt.flags&vfSHOW_EQU_CURVES )
   {
      int mul = 1 + viewOpt.equ_curves_num/30;
      int step = mul * 100/viewOpt.equ_curves_num;
      int perc_mul = mul * 100;

      int percent_overall;
      //int percent_bottom = int(perc_mul*(tension-cur)/tens_range);
      //int percent_top = percent_bottom < 0 ? int(perc_mul*((maxTopSrfPotential + tension - cur)/maxTopSrfPotential)) : 0;

      bool show = false;

      //bool bottom = (percent_bottom >= 0 && percent_bottom%step == 0);
      //bool top = (percent_bottom == 0) || (percent_top > 0 && percent_top < perc_mul && (percent_top/5)%(step/2) == 0);

      switch( viewOpt.equ_curves_mode )
      {
         case ecmOVERALL:
            percent_overall = int(perc_mul*fabs( (max - cur)/tens_range ));
            show = percent_overall%step == 0;
            break;
         /*case ecmOVERALL1:
            percent_overall = int(perc_mul*fabs( (max - cur1)/tens_range ));
            show = percent_overall%step == 0;
            break;
         case ecmBOTTOM:
            show = bottom;
            break;
         case ecmTOP:
            show = top;
            break;
         case ecmBOTH:
            show = top || bottom;
            break;*/
      }

      if( show )
      {
         *color = ModelTheme.equCurveColor;
         return true;
      }
   }

   switch( viewOpt.field_mode )
   {
      case efmNO_FIELD:
         return false;
      case efmOVERALL_FIELD:
         *color = RGB( 0x00, 0x00, int(0xFF*(Potential(pnt)-min)/(max-min)) );
         break;
      //case efmTWO_COMPONENTS:
      //   *color = RGB( int(0xFF*(comp1-min1)/tens_range), 0x00, int(0xFF*(comp2-min2)/tens_range) );
      //   break;
      //case efmPORE_FIELD:
      //   *color = RGB( 0x00, 0x00, int(0xFF*(comp1-min1)/(max1-min1)) );
      //   break;
      //case efmDIFF:
      //{
      //   if( matrix.Value(pnt) || Potential(pnt) >= tension - NULL_DOUBLE )
      //     return false;

      //   double diff = 2 * fabs(Potential(pnt)-Potential1(pnt)) / (max-min);
      //   Minimize( diff, 1.0 );
      //   *color = RGB( int(0xFF*diff), 0x00, 0x00 );
      //   break;
      //}
   }

   return true;
}

bool PorousHoleModel::GetCellColor( const Point<long>& pnt, COLORREF* color ) const
{
   if( viewOpt.flags&vfSHOW_TIPS && tipMap.Value( pnt ) )
   {
      *color = ModelTheme.poreTipColor;
      return true;
   }

   if( PorousModel::GetCellColor( pnt, color ) )
      return true;

   if( viewOpt.flags&vfSHOW_HOLES && holeMap.Value(pnt) )
   {
      if( opt & phmoUSE_TWIN_HOLES && holeMap2.Value(pnt) )
      {
        *color = ModelTheme.hole2Color;
        return true;
      }
      if( (opt & phmoUSE_TWIN_HOLES) == 0 ||
          (viewOpt.flags&vfSHOW_TWIN_HOLES_ONLY) == 0 )
      {
        *color = ModelTheme.holeColor;
        return true;
      }
   }

   if( (opt & phmoUSE_FTOR) && viewOpt.flags&vfSHOW_FTOR && ftorMap.Value( pnt ) )
   {
      *color = ModelTheme.ftorColor;
      return true;
   }

   if( viewOpt.flags&vfSHOW_PORE_CLOUD && poreCloud.Value( pnt ) )
   {
      *color = ModelTheme.poreCloudColor;
      return true;
   }

   // Show field in slit modes only
   if( viewOpt.mode == vmHORZ_SLIT || viewOpt.mode == vmVERT_SLIT )
      if( GetElFieldColor( pnt, color ) )
         return true;

   return false;
}

double PorousHoleModel::CalcPoreCellCharge( Point<long> pnt )
{
   // ELECTRON_CHARGE is used here
   // Charge of electron: Qe = -1.6 * 10^(-19) [qulon]
   // Intensity of light: J = 10^19 [photon/(cm*cm)] = 10^23 [photon/(m*m)]
   // Density of charge: G = -Qe * J = 1.6 * 10^4 [qulon/(m*m*m)]
   //const double G = 1.6e4;
   //static double cell_d; // size (diameter) of cell
   //cell_d = SILICON_WIDTH/GetSize().z;
   //static double cell_volume;

   static double G;
   static double cell_q; // charge of cell

   double mul = (GetSize().x == 1 ? PORE_CELL_CHARGE_MUL_2D : PORE_CELL_CHARGE_MUL);
   double g_koeff = mul * tension;

   //G = g_koeff * sqrt((double)pnt.z+1);
   double deep = CELL_SIZE * (pnt.z+1);
   G = g_koeff * ( deep / SILICON_WIDTH );

   cell_q = G * CELL_VOLUME;
   return cell_q;
}

inline double PorousHoleModel::BasePotential( const Point<long>& pnt ) const
{
   // potential == tension, if pnt.z == -1
   // potential == 0, if pnt.z == max_z (GetSize().z-1)
   //return tension * ( (GetSize().z-1) - pnt.z) / GetSize().z;
   return tension * (MAX_Z - pnt.z) / (MAX_Z + 1);
}

inline double PorousHoleModel::Potential( const Point<long>& p ) const
{
  if( !matrix.Contains(p) )
    return BasePotential(p);

  if( matrix.Value(p) )
    return tension;

  return potentials.GetElem(p);
}

double PorousHoleModel::CalcPotentialByNeighbors( const Point<long>& pnt )
{
  const Point<long>& sz = GetSize();
  Point<long> p;
  double sum = 0;
  int num = 0;

  for( int dir = 0; dir <= dirLAST; ++dir )
  {
    p = pnt + Dirs[dir];
    if( p.x >= 0 && p.x < sz.x &&
        p.y >= 0 && p.y < sz.y )
    {
      sum += Potential( p );
      ++num;
    }
  }

  return num > 0 ? sum/num : 0.0;
}

void PorousHoleModel::UpdatePotentialInCell( const Point<long>& pnt )
{
  if( !matrix.Contains(pnt) || matrix.Value(pnt) )
    return;

  double cur = CalcPotentialByNeighbors( pnt );
  potentials.GetElem( pnt ) = cur;

  Minimize( minPotential, cur );
  Maximize( maxPotential, cur);
}

void PorousHoleModel::UpdatePotentialWave( const Point<long>& pnt, bool inverse )
{
  const Point<long>& sz = GetSize();
  Point<long> diff_pnt = sz - pnt, p;
  long max_r = Max( Max3(pnt.x, pnt.y, pnt.z), Max3( diff_pnt.x, diff_pnt.y, diff_pnt.z) );

  for( int radius = 0, r = 0; radius <= max_r; ++radius )
  {
    r = inverse ? (max_r - radius) : radius;

    if( r == 0 )
    {
      UpdatePotentialInCell( pnt );
      continue;
    }

    for( p.x = pnt.x-r; p.x < sz.x && p.x <= pnt.x+r; p.x += 2*r )
      if( p.x >= 0 )
        for( p.y = Max<long>(pnt.y-r,0); p.y < sz.y && p.y <= pnt.y+r; ++p.y )
          for( p.z = Max<long>(pnt.z-r,0); p.z < sz.z && p.z <= pnt.z+r; ++p.z )
            UpdatePotentialInCell( p );

    for( p.y = pnt.y-r; p.y < sz.y && p.y <= pnt.y+r; p.y += 2*r )
      if( p.y >= 0 )
        for( p.x = Max<long>(pnt.x-r,0); p.x < sz.x && p.x <= pnt.x+r; ++p.x )
          for( p.z = Max<long>(pnt.z-r,0); p.z < sz.z && p.z <= pnt.z+r; ++p.z )
          UpdatePotentialInCell( p );

    for( p.z = pnt.z-r; p.z < sz.z && p.z <= pnt.z+r; p.z += 2*r )
      if( p.z >= 0 )
        for( p.x = Max<long>(pnt.x-r,0); p.x < sz.x && p.x <= pnt.x+r; ++p.x )
          for( p.y = Max<long>(pnt.y-r,0); p.y < sz.y && p.y <= pnt.y+r; ++p.y )
            UpdatePotentialInCell( p );
  }
}

void PorousHoleModel::UpdatePotentialFromPoint( const Point<long>& pnt, bool inverse )
{
  do
  {
    UpdatePotentialWave(pnt, inverse);
  } while( CalcMaxPotentialError() > 0.001 );
}

void PorousHoleModel::RecalcPotentialWave()
{
  Point<long> mid_pnt = GetSize()/2;

  for( int i = 0; i < 3; ++i )
    UpdatePotentialWave(mid_pnt, true);

  UpdatePotentialFromPoint(mid_pnt);
}

void PorousHoleModel::RecalcPotential()
{
  InitPotential();
  //RecalcPorePotential(); // old method
  RecalcPotentialWave(); // new method
}

double PorousHoleModel::CalcMaxPotentialError()
{
  Point<long> pnt;
  double max_err = 0, err = 0;

  for( pnt.x = 0; pnt.x < (long)GetSize().x; ++pnt.x )
    for( pnt.y = 0; pnt.y < (long)GetSize().y; ++pnt.y )
      for( pnt.z = 0; pnt.z < (long)GetSize().z; ++pnt.z )
        if( !matrix.Value( pnt ) )
        {
          err = fabs( Potential(pnt) - CalcPotentialByNeighbors(pnt) );
          Maximize( max_err, err );
        }

  return max_err / tension;
}

void PorousHoleModel::RemoveNearestTips( const Point<long>& hole_pnt )
{
   Point<long> pnt;

   for( int i = 0; i < DIR_3D_CNT; ++i )
   {
      pnt = hole_pnt + dir3dTools.GetDir(i).GetPoint();

      if( !tipMap.Contains(pnt) || !tipMap.Value( pnt ) )
         continue;

      tipMap.SetValue( pnt, false );
      //UpdatePorePotential( pnt, 1-tipMul );
   }
}

bool PorousHoleModel::CreatePore( const Point<long>& pnt )
{
   static Point<long> p;

   if( !PorousModel::CreatePore(pnt) )
      return false;

   for( int i = 0; i < DIR_3D_CNT; ++i )
   {
      p = pnt + dir3dTools.GetDir(i).GetPoint();
      if( poreCloud.Contains(p) )
         poreCloud.SetValue(p, true);
   }

   return true;
}

void PorousHoleModel::Reactions()
{
   ElHole *p_hole;
   static Ion ion;
   ListNode<ElHole> *node, *prev_node;
   SimplePoint<long> pnt;
   double cell_hole_count;

   for( node = holes.front(); node; )
   {
     p_hole = &node->value;
     pnt = p_hole->pnt;
     if( !Value(pnt) )
        if( ChemicalReaction( pnt ) )
        {
           CreatePore( pnt );

           RemoveNearestTips( pnt );
           tipMap.SetValue( pnt, true );
           //UpdatePorePotential( pnt, tipMul );
           UpdatePotentialFromPoint( pnt );

           cell_hole_count = CalcHoleDensity(pnt.z*CELL_SIZE)*CELL_VOLUME;
           holeCountNeeded[pnt.z] -= cell_hole_count;
           holeTotalNeeded        -= cell_hole_count;

           if( (opt & phmoUSE_FTOR) && !ftorMap.Value( pnt ) && DiffuseFtor( pnt ) )
           {
             ftorMap.SetValue( pnt, true );
             ion.initiative = 0.0;
             ion.pnt = pnt;
             ftor.push_back( ion );
           }
        }
        else if( pnt.z > 0 )
        {
           // don't remove element
           node = node->next;
           continue;
        }

     // remove element
     prev_node = node;
     node = node->next;
     --holeCount[prev_node->value.pnt.z];
     holes.erase( &prev_node );
   }
}

double PorousHoleModel::GetPotential( const Point<long>& pnt ) const
{
  return Potential(pnt);
}

void PorousHoleModel::CalcNewHolePosition( SimplePoint<long>& cur_pnt )
{
   static Point<long>       vec, pnt, bounds;
   static Point<double>     v_vec, e_vec, e_vec_sum;
   static int               i, cnt;
   static Dir3d             dir;
   static double            probs[DIR_3D_CNT], potential, min, vec_len;
   static double            phi, cos_theta, sin_theta, cos_angle, max_cos_angle;

   bounds.Init( (long)GetSize().x-1, (long)GetSize().y-1, (long)GetSize().z-1 );

   potential = GetPotential(cur_pnt);
   min = 0.0;
   cnt = 0;
   e_vec_sum.Init(0,0,0);
   for( i = 0; i < DIR_3D_CNT; ++i )
   {
      vec     = dir3dTools.GetDir(i).GetPoint();
      vec_len = Length(vec);
      pnt     = cur_pnt + vec;

      if( !matrix.Contains( pnt ) )
        continue;

      e_vec = vec; // Point<int> => Point<double>
      e_vec /= vec_len;
      e_vec *= (GetPotential(pnt) - potential)/(vec_len*CELL_SIZE); // [V/m]

      e_vec_sum += e_vec;
      ++cnt;
   }

   if( cnt <= 0 )
     return;

   e_vec = e_vec_sum / cnt;

   // Generate random point on a sphere
   phi       = uniform_distrib(0, 2*M_PI);
   cos_theta = uniform_distrib(-1, 1);
   sin_theta = sqrt(1-cos_theta*cos_theta);
   v_vec.Init( sin_theta*cos(phi), sin_theta*sin(phi), cos_theta );
   v_vec /= Length(v_vec); // normalizing (just to be on a safe side)
   v_vec *= normal_distrib( holeThermSpeed, holeThermSpeedSigma ); // [m/sec]

   // Add average hole speed corresponding to given E
   const double adjustment = 300;
   v_vec += e_vec*(holeMobility*1e-4)*adjustment; // converted to [m/sec]

   // Find the closest direction (i.e. minimal angle between v_vec and 3D directions)
   max_cos_angle = -2;
   vec_len = Length(v_vec);
   for( i = 0; i < DIR_3D_CNT; ++i )
   {
     vec = dir3dTools.GetDir(i).GetPoint();
     cos_angle = (v_vec * vec) / ( vec_len * Length(vec));
     if( cos_angle > max_cos_angle )
     {
       max_cos_angle = cos_angle;
       pnt = vec;
     }
   }

   cur_pnt += pnt;
   ValidatePoint< SimplePoint<long> >( cur_pnt, NULL_LONG_PNT, bounds );
}

void PorousHoleModel::CalcNewPosition( SimplePoint<long>& cur_pnt, bool positive )
{
   static Point<long>       pnt, bounds;
   static int               i;
   static Dir3d             dir;
   static double            probs[DIR_3D_CNT], potential, min;

   bounds.Init( (long)GetSize().x-1, (long)GetSize().y-1, (long)GetSize().z-1 );

   potential = Potential( cur_pnt );
   min = 0.0;
   for( i = 0; i < DIR_3D_CNT; ++i )
   {
      pnt = cur_pnt + dir3dTools.GetDir(i).GetPoint();

      if( !matrix.Contains( pnt ) )
         probs[i] = 0.0;
      else
      {
         if( positive )
            probs[i] = Potential(pnt) - potential;
         else
            probs[i] = potential - Potential(pnt);
      }

      if( opt & phmoEXP_PROB )
      {
         // i.e. exp( - (Uo - e * Fi) / (k * T) ); e = 1.6e-19; k = 1.38e-23; T = 300; Uo = 0;
         const double adjustment = 10;
         const double koeff = HOLE_CHARGE / (PHYS_K*temperature);
         probs[i] = exp( adjustment * koeff * probs[i] );
      }            

      Minimize( min, probs[i] );
   }

   for( i = 0; i < DIR_3D_CNT; ++i )
      probs[i] -= min - 1e-6;

   i = random_event( probs, DIR_3D_CNT );

   pnt = dir3dTools.GetDir(i).GetPoint();
   //Minimize( pnt.z, (long)0 );

   cur_pnt += pnt;
   ValidatePoint< SimplePoint<long> >( cur_pnt, NULL_LONG_PNT, bounds );
}

void PorousHoleModel::MoveHole( const Point<long>& prev_pnt, const Point<long>& new_pnt )
{
   if( !holeMap.Value( new_pnt ) )
      holeMap.SetValue( new_pnt, true );
   else
      holeMap2.SetValue( new_pnt, true );
   --holeCount[prev_pnt.z];
   ++holeCount[new_pnt.z];
}

void PorousHoleModel::MoveFtor( const Point<long>& prev_pnt, const Point<long>& new_pnt )
{
   ftorMap.SetValue( prev_pnt, false );
   ftorMap.SetValue( new_pnt, true );
}

void PorousHoleModel::MoveParticles( List<Ion>& ions, IonType ion_type, float speed )
{
   bool               positive, is_ftor;
   static Ion        *p_ion, ion;
   static Point<long> prev_pnt;
   ListNode<Ion> *node, *prev_node;
   ulong max_z = GetSize().z-1;

   positive = ion_type == itHOLE;

   for( node = ions.front(); node; )
   {
      p_ion = &node->value;

      if( ion_type == itFTOR )
      {
         is_ftor = ftorMap.Value(p_ion->pnt);
         if( !is_ftor || p_ion->pnt.z == max_z )
         {
            if( is_ftor )
               ftorMap.SetValue( p_ion->pnt, false );
            prev_node = node;
            node = node->next;
            ions.erase( &prev_node );
            continue;
         }
      }

      //remove = false;
      for( p_ion->initiative += speed; p_ion->initiative >= 1.0; p_ion->initiative -= 1.0 )
      {
         prev_pnt = p_ion->pnt;
         if( ion_type == itHOLE )
            CalcNewHolePosition( p_ion->pnt );
         else
            CalcNewPosition( p_ion->pnt, positive );

         if( prev_pnt == p_ion->pnt )
            continue;

         //if( !matrix.Contains(p_ion->pnt) )
         //{
         //   remove = true;
         //   break;
         //}

         if( ion_type == itHOLE )
            MoveHole( prev_pnt, p_ion->pnt );
         else // itFTOR
         {
            MoveFtor( prev_pnt, p_ion->pnt );

            // Create another Ftor ion if it's a pore
            if( Value( prev_pnt ) && DiffuseFtor( prev_pnt ) )
            {
               ion.initiative = 0.0;
               ion.pnt = prev_pnt;
               ions.push_before( ions.front(), ion );
               ftorMap.SetValue( prev_pnt, true );
            }
         }
      }

      //if( remove )
      //{
      //   if( ion_type == itFTOR && ftorMap.Value(prev_pnt) )
      //      ftorMap.SetValue( prev_pnt, false );

      //  // remove element
      //  prev_node = node;
      //  node = node->next;
      //  ions.erase( &prev_node );
      //  continue;
      //}

      node = node->next;
   }
}

void PorousHoleModel::CreateFtorIons()
{
   if( !(opt & phmoUSE_FTOR) )  return;

   Ion ion;
   ion.initiative = 0.0;
   ion.pnt.z = 0;

   for( ion.pnt.x = 0; ion.pnt.x < (long)GetSize().x; ++ion.pnt.x )
      for( ion.pnt.y = 0; ion.pnt.y < (long)GetSize().y; ++ion.pnt.y )
         if( !ftorMap.Value( ion.pnt ) )
            if( surface.GetState( ion.pnt.x, ion.pnt.y ) == sssCHARGED_MOLECULE &&
                oxid.GetState( ion.pnt.x, ion.pnt.y ) && DiffuseFtor( ion.pnt ) )
            {
               // move ftor from surface into volume
               ftor.push_back( ion );
               ftorMap.SetValue( ion.pnt, true );
               surface.SetState( ion.pnt.x, ion.pnt.y, sssNO_MOLECULE );
            }
}

//void PorousHoleModel::BurnFtorIons()
//{
//   static const double burn_speed = 10;
//   static double burn = 0.0;
//   static Ion ion;
//
//   ion.initiative = 0.0;
//   ion.pnt.z = 0;
//   for( burn += burn_speed; int(burn) > 0; burn -= 1 )
//   {
//      ion.pnt.x = rand() % GetSize().x;
//      ion.pnt.y = rand() % GetSize().y;
//      ftor.push_back( ion );
//      ftorMap.SetValue( ion.pnt, true );
//   }
//}

/*void PorousHoleModel::CreateHoles()
{
   static ElHole hole;

   if( holes.size() >= maxHoles )
     return;

   hole.initiative = 0.0;
   hole.pnt.z = GetSize().z-1;

   for( newHoles += holeBurnSpeed; int(newHoles) > 1 && holes.size() < maxHoles; newHoles -= 1 )
   {
      hole.pnt.x = rand() % GetSize().x;
      hole.pnt.y = rand() % GetSize().y;
      hole.pnt.z = rand() % GetSize().z;
      holes.push_back( hole );
   }
}*/

void PorousHoleModel::CreateHoles()
{
   static ElHole hole;
   const  Point<ulong>& sz = GetSize();
   double hole_cnt;

   hole.initiative = 0.0;

   // Generate holes only on bottom face now
   //for( hole.pnt.z = 0; hole.pnt.z < sz.z; ++hole.pnt.z )
   hole.pnt.z = sz.z-1;
   {
      hole_cnt = holeCountNeeded[hole.pnt.z] - holeCount[hole.pnt.z];
      //hole_total_cnt = holeTotalNeeded - holes.size();
      for( ; hole_cnt > 0; hole_cnt -= 1 )
      {
         if( hole_cnt < 1 && !CheckProb( hole_cnt ) )
            continue;

         hole.pnt.x = rand() % sz.x;
         hole.pnt.y = rand() % sz.y;
         if( !Value(hole.pnt) )
         {
            holes.push_back( hole );
            ++holeCount[hole.pnt.z];
         }
      }
   }
}

void PorousHoleModel::Step()
{
   holeMap.Init();
   holeMap2.Init();

   CreateHoles();
   CreateFtorIons();

   MoveParticles( holes, itHOLE, HOLE_SPEED );
   if( opt & phmoUSE_FTOR )
      MoveParticles( ftor, itFTOR, FTOR_SPEED );

   Reactions();
}

void PorousHoleModel::Save( File& f )
{
   PorousModel::Save( f );
   tipMap.Save( f );
   f << tipMul << tension;
}

bool PorousHoleModel::Load( File& f )
{
   Init();

   bool ok = PorousModel::Load( f );

   InitModelSize( GetSize() );

   tipMap.Load( f );
   f >> tipMul >> tension;

   RecalcPotential();

   return true;
}

bool PorousHoleModel::LoadBMP( HDC hDC, double bound, int depth )
{
   Init();

   bool ok = PorousModel::LoadBMP( hDC, bound, depth );

   InitModelSize( GetSize() );
   RecalcPotential();

   surface.Init( GetSize().y, GetSize().x );

   return ok;
}

void PorousHoleModel::PaintDistributions( HDC hDC, POINT size )
{
   VP<long> dens, ftor_dens;
   long max_dens[3] = {0, 0, 0};

   dens = GetMatrixZDensity( matrix, &max_dens[0] );

   if( viewOpt.flags & vfSHOW_HOLES )
      for( long z = GetSize().z-1; z >= 0; --z )
         Maximize( max_dens[1], holeCount[z] );

   if( viewOpt.flags & vfSHOW_FTOR )
      ftor_dens = GetMatrixZDensity( ftorMap, &max_dens[2] );

   maxZDensity = Max(Max(Max(max_dens[0], max_dens[1]), max_dens[2]), maxZDensity);

   PaintMatrixZDistribution( matrix, dens, maxZDensity, hDC, size, ModelTheme.poreColor );
   if( viewOpt.flags & vfSHOW_HOLES )
      PaintMatrixZDistribution( holeMap, holeCount, maxZDensity, hDC, size, ModelTheme.holeColor );
   if( viewOpt.flags & vfSHOW_FTOR )
      PaintMatrixZDistribution( ftorMap, ftor_dens, maxZDensity, hDC, size, ModelTheme.ftorColor );
}

/*
Old method of electrical field calculation, not required already

void PorousHoleModel::UpdatePotentials( const Point<long>& p, double q )
{
   static Point<long> pnt;
   static double* ptr;

   maxPotential = maxParticlePotential = NULL_DOUBLE;
   minPotential = minParticlePotential = LARGE_DOUBLE;
   maxTopSrfPotential = NULL_DOUBLE;

   long max_z = (long)GetSize().z-1;
   double cur, base_potential;

   for( pnt.z = 0; pnt.z < (long)GetSize().z; ++pnt.z )
   {
      base_potential = BasePotential( pnt );
      
      for( pnt.x = 0; pnt.x < (long)GetSize().x; ++pnt.x )
         for( pnt.y = 0; pnt.y < (long)GetSize().y; ++pnt.y )
         {
            ptr = &porePotentials.GetElem( pnt );

            if( pnt != p )
               *ptr += ParticlePotential( p, pnt, q );

            cur = *ptr;

            Minimize( minParticlePotential, cur );
            Maximize( maxParticlePotential, cur );
            if( pnt.z == 0 )
               Maximize( maxTopSrfPotential, cur );

            Minimize( minPotential, cur + base_potential );
            Maximize( maxPotential, cur + base_potential);
         }
   }
}

void PorousHoleModel::UpdatePorePotential( const Point<long>& pnt, double mul )
{
   double q = CalcPoreCellCharge( pnt );
   UpdatePotentials( pnt, mul*q );
}

void PorousHoleModel::RecalcPorePotential()
{
   Point<long> pnt;
   double* ptr = NULL;

   for( pnt.x = 0; pnt.x < (long)GetSize().x; ++pnt.x )
      for( pnt.y = 0; pnt.y < (long)GetSize().y; ++pnt.y )
         for( pnt.z = 0; pnt.z < (long)GetSize().z; ++pnt.z )
            if( matrix.Value( pnt ) )
              UpdatePorePotential( pnt, tipMap.Value(pnt) ? tipMul : 1 );
}

inline double PorousHoleModel::ParticlePotential( const Point<long>& particle, const Point<long>& pnt, double q )
{
   //const double k = 1.0 / ( 4 * M_PI * Eps0 );
   static const double k = 9e9; // (System C: meter, qulon)

   static Point<long> r;
   static double d_fi;

   r = particle - pnt;
   d_fi = k * q / SILICON_DIELECTRIC_EPS;

   return d_fi / (Length(r) * CELL_SIZE);
}

double PorousHoleModel::CalcPotential1Error( bool average )
{
  const Point<long>& sz = GetSize();
  Point<long> pnt;
  double max_err = 0, err = 0, sum_err = 0;
  unsigned long long num = 0;

  for( pnt.x = 0; pnt.x < sz.x; ++pnt.x )
    for( pnt.y = 0; pnt.y < sz.y; ++pnt.y )
      for( pnt.z = 0; pnt.z < sz.z; ++pnt.z )
        if( !matrix.Value(pnt) && Potential(pnt) < tension - NULL_DOUBLE )
        {
          err = fabs( Potential(pnt) - Potential1(pnt) );
          sum_err += err;
          Maximize( max_err, err );
          ++num;
        }

  err = (average ? sum_err/num : max_err);
  return err / tension;
}

double PorousHoleModel::CalcMaxPotential1Error()
{
  return CalcPotential1Error( false );
}

double PorousHoleModel::CalcAveragePotential1Error()
{
  return CalcPotential1Error( true );
}

inline double PorousHoleModel::Potential1( const Point<long>& pnt ) const
{
   return BasePotential( pnt ) + porePotentials.GetElem( pnt );
}
*/