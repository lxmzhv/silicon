// Model of porous formations in silicon

#pragma once

#include <SiliconSrfModel.h>
#include <PorousModel2.h>

class SiliconHoleModel
{
   public:
                          SiliconHoleModel( Point<ulong> sz, Point<ulong> times, double oxid_prob,
                                            double tens, double temp, double lum, double cur_strength,
                                            int opt, ModelViewOptions vopt );
                     void Step();
                      int GetStep() const { return step; }
                     void Init();
   const SiliconSrfModel& GetSurface() const { return surface; }
         const OxidModel& GetOxid() const { return oxid; }
         PorousHoleModel& GetMatrix() { return matrix; }
      const Point<ulong>& GetSize() const { return matrix.GetSize(); }
                    ulong GetPoreCount() const { return matrix.GetPoreCount(); }
                     void Save( File& f );
                     bool Load( File& f );
                   //double CalcMaxPotential1Error()     { return matrix.CalcMaxPotential1Error(); }
                   //double CalcAveragePotential1Error() { return matrix.CalcAveragePotential1Error(); }

   private:
               int step;
   SiliconSrfModel surface;
         OxidModel oxid;    // Oxid process on surface
   PorousHoleModel matrix;
};
