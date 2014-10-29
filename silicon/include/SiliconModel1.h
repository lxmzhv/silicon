// Model of porous formations in silicon

#pragma once

#include <PorousModel1.h>

class SiliconModel1
{
   public:
                          // Пераметры: см. описание соотв. параметров у каждого из классов
                          SiliconModel1( Point<ulong> sz, Point<ulong> times, double oxid_prob, ModelViewOptions opt, double prob_deg,
                                        DPoint2D prob_range=DPoint2D(.01,.9), double sideway_k=0.5, double upward_k=.05 );
                     void Step();
                      int GetStep() const { return step; }
                     void Init();
   const SiliconSrfModel& GetSurface() { return surface; }
         const OxidModel& GetOxid() { return oxid; }
            PorousModel1& GetMatrix() { return matrix; }
      const Point<ulong>& GetSize() { return matrix.GetSize(); }
                    ulong GetPoreCount() { return matrix.GetPoreCount(); }
                     void Save( File& f );
                     bool Load( File& f );

   private:
               int step;
   SiliconSrfModel surface;
         OxidModel oxid;    // Oxid process on surface
      PorousModel1 matrix;
};
