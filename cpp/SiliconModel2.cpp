// Модель порообразования в кремнии

#include <SiliconModel2.h>

//
//class SiliconHoleModel
//
SiliconHoleModel::SiliconHoleModel( Point<ulong> sz, Point<ulong> times, double oxid_prob,
                                    double tens, double temp, double lum, double cur_strength, int opt, ModelViewOptions vopt ):
   surface(sz.x, sz.y, times.x, times.y, times.z),
   oxid(sz.x, sz.y,oxid_prob),
   matrix(sz, surface, oxid, opt, vopt, tens, temp, lum, cur_strength),
   step(0)
{
}

void SiliconHoleModel::Step()
{
   surface.Step();
   oxid.Step();
   matrix.Step();
   ++step;
}

void SiliconHoleModel::Init()
{
   surface.Init();
   oxid.Init();
   matrix.Init();
   step = 0;
}

void SiliconHoleModel::Save( File& f )
{
   surface.Save( f );
   oxid.Save( f );
   matrix.Save( f );
   f << step;
}

bool SiliconHoleModel::Load( File& f )
{
   if( surface.Load(f) && oxid.Load(f) && matrix.Load(f) )
   {
      f >> step;
      return true;
   }
   return false;
}
