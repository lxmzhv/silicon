// Модель порообразования в кремнии

#include <SiliconModel1.h>

//
//class SiliconModel1
//
SiliconModel1::SiliconModel1( Point<ulong> sz, Point<ulong> times, double oxid_prob, ModelViewOptions opt,
                              double prob_deg, DPoint2D prob_range, double sideway_k, double upward_k ):
   surface(sz.x, sz.y, times.x, times.y, times.z),
   matrix(sz,opt,prob_deg,prob_range,sideway_k,upward_k),
   oxid(sz.x, sz.y,oxid_prob),
   step(0)
{}

void SiliconModel1::Step()
{
   surface.Step();
   oxid.Step();

   POINT sz = surface.GetSize();
   Point<long> p(0,0,-1);
   for( ; p.x < sz.x; p.x++ )
      for( p.y = 0; p.y < sz.y; p.y++ )
         if( surface.GetState(p.x,p.y) == sssCHARGED_MOLECULE && oxid.GetState(p.x,p.y) ) // Условия подходящие
            matrix.WaveDrill( p, surface );
   matrix.ApplyChanges();

   ++step;
}

void SiliconModel1::Init()
{
   surface.Init();
   oxid.Init();
   matrix.Init();
   step = 0;
}

void SiliconModel1::Save( File& f )
{
   surface.Save( f );
   oxid.Save( f );
   matrix.Save( f );
   f << step;
}

bool SiliconModel1::Load( File& f )
{
   if( surface.Load(f) && oxid.Load(f) && matrix.Load(f) )
   {
      f >> step;
      return true;
   }
   return false;
}
