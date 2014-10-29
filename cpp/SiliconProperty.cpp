// Model characteristics

#include <Silicon.h>
#include <SiliconProperty.h>
#include <DigitMethods.h>
#include <PorousModel.h>
#include <Direction3d.h>

//
// Property
//

void Property::Recalc( const PorousModel& model )
{
   if( !valid )
   {
      value = Compute( model );
      valid = true;
   }
}

void Property::Update( const PorousModel& model, SimplePoint<long> pnt )
{
   if( !valid )
      Recalc( model );
   else
      value = ComputeUpdate( model, value, pnt );
}

//
// AverageDepth
//

double AverageDepth::Compute( const PorousModel& model )
{
  const Point<ulong>& sz = model.GetSize();
  Point<long> p;
  double val = 0;
  ulong  cnt = model.GetPoreCount();

  if( cnt <= 0 )
    return 0;

  for( p.z = 0; p.z <= model.GetMaxDeep(); ++p.z )
    for( p.x = 0; p.x < sz.x; ++p.x )
      for( p.y = 0; p.y < sz.y; ++p.y )
        if( model.GetMatrix().Value(p) )
          val += (p.z+1);

  return CELL_SIZE * val / cnt;
}

double AverageDepth::ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore )
{
  ulong cnt = model.GetPoreCount();
  return (old_val*(cnt-1) + CELL_SIZE*(new_pore.z+1)) / cnt;
}

//
// PoreArea
//

double PoreArea::Compute( const PorousModel& model )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  const Point<ulong>& sz = model.GetSize();
  long max_z = model.GetMaxDeep();
  Point<long> p, p2;
  double area = 0;
  ulong  cnt = 0;

  if( model.GetPoreCount() == 0 )
    return 0;

  for( p.x = 0; p.x < sz.x; ++p.x )
    for( p.y = 0; p.y < sz.y; ++p.y )
      for( p.z = 0; p.z <= max_z; ++p.z )
        if( matrix.Value(p) )
        {
          area += 6;

          if( p.x > 0 )
          {
            p2.Init( p.x-1, p.y, p.z );
            if( matrix.Value(p2) )
              area -= 2;
          }
          if( p.y > 0 )
          {
            p2.Init( p.x, p.y-1, p.z );
            if( matrix.Value(p2) )
              area -= 2;
          }
          if( p.z > 0 )
          {
            p2.Init( p.x, p.y, p.z-1 );
            if( matrix.Value(p2) )
              area -= 2;
          }
        }

  return area * CELL_SIZE * CELL_SIZE;
}

double PoreArea::ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore )
{
  const BoolMatrix3D& matrix = model.GetMatrix();
  Point<long> p;
  double area = old_val;

  area += 6 * CELL_SIZE * CELL_SIZE;
  for( int dir = 0; dir <= dirLAST; ++dir )
  {
    p = new_pore;
    p += Dirs[dir];
    if( matrix.Contains(p) && matrix.Value(p) )
      area -= 2 * CELL_SIZE * CELL_SIZE;
  }

  return area;
}

//
// PoreConcentration
//

double PoreConcentration::Compute( const PorousModel& model )
{
  const Point<ulong>& sz = model.GetSize();
  return model.GetPoreCount() / (CELL_VOLUME*sz.x*sz.y*sz.z);
}

double PoreConcentration::ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore )
{
  return Compute( model );
}

//
// PoreVolume
//

double PoreVolume::Compute( const PorousModel& model )
{
  return CELL_VOLUME * model.GetPoreCount();
}

double PoreVolume::ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore )
{
  return Compute( model );
}

//
// PorePorosity
//

double PorePorosity::Compute( const PorousModel& model )
{
  const Point<ulong>& sz = model.GetSize();
  return 100.0*model.GetPoreCount()/sz.x/sz.y/Max<long>(1,model.GetMaxDeep());
}

double PorePorosity::ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore )
{
  return Compute( model );
}