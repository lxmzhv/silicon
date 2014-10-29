// Other variant of silicon matrix model

#pragma once

#include <BoolMatrix3d.h>
#include <PorousModel.h>
#include <Matrix3d.h>

using namespace std;

struct Ion
{
   SimplePoint<long> pnt;
               float initiative;
};

typedef Ion ElHole;

enum IonType
{
   itHOLE = 0,
   itFTOR = 1
};

class PorousHoleModel: public PorousModel
{
   public:
                     PorousHoleModel( Point<ulong> sz, SiliconSrfModel& srf, const OxidModel& oxid, int opt, ModelViewOptions vopt,
                                      double tension, double temperature, double luminosity, double cur_strength );

                void Init();
        inline  bool Value( const Point<long>& p ) const { return matrix.Contains(p) && matrix.Value(p); }
        inline  bool Path( const Point<long>& p ) const { return holeMap.Value(p); }
        inline  bool Tips( const Point<long>& p ) const { return tipMap.Value(p); }
        inline  bool Ftor( const Point<long>& p ) const { return ftorMap.Value(p); }
        inline const BoolMatrix3D& GetHoleMap() const { return holeMap; }
        inline const BoolMatrix3D& GetFtorMap() const { return ftorMap; }

                void Step();

                void Save( File& f );
                bool Load( File& f );
                bool LoadBMP( HDC hDC, double bound, int depth );

       virtual  bool GetCellColor( const Point<long>& pnt, COLORREF* color ) const;
       virtual  void PaintDistributions( HDC hDC, POINT size );

   protected:
                bool GetElFieldColor( const Point<long>& pnt, COLORREF* color ) const;
        virtual bool CreatePore( const Point<long>& pnt );

   private:
  SiliconSrfModel& surface;
  const OxidModel& oxid;

  Matrix3d<double> potentials;     // overall electrical field

      BoolMatrix3D poreCloud;

            double newHoles;
            double maxHoles;
      List<ElHole> holes;        // electrical holes
      BoolMatrix3D holeMap, holeMap2;
          VP<long> holeCount;       // number of holes by horizontal slits
        VP<double> holeCountNeeded; // required number of holes by horizontal slits
            double holeTotalNeeded; // required total number of holes
      BoolMatrix3D tipMap;      // pore spikes

         List<Ion> ftor;         // ions (e.g. F+: ion of the Ftor)
      BoolMatrix3D ftorMap;     // Ftor ion positions

            double tipMul;       // multiplier for tip charge (and potential)
            double minPotential; // minimum potential
            double maxPotential; // maximum potential
            double minParticlePotential; // minimum particle potential
            double maxParticlePotential; // maximum particle potential
            double maxTopSrfPotential; // maximum potential on the surface z == 0
            double tension;         // external voltage
            double temperature;
            double luminosity;
            double currentStrength;
            double holeBurnSpeed; // hole creation speed
            double holeThermSpeed;
            double holeThermSpeedSigma;
            double holeMobility;
            //double holeFreeRunTime;
            //double stepTime;
              long maxZDensity;
               int opt;           // PorousHoleModelOpt

               void InitModelSize( const Point<ulong>& sz );
               bool ChemicalReaction( const SimplePoint<long>& hole_pnt ) const;
               void Reactions();
               void CalcNewPosition( SimplePoint<long>& cur_pnt, bool positive );
               void CalcNewHolePosition( SimplePoint<long>& cur_pnt );
               void MoveParticles( List<Ion>& ions, IonType ion_type, float speed );
               void MoveHole( const Point<long>& prev_pnt, const Point<long>& new_pnt );
               void MoveFtor( const Point<long>& prev_pnt, const Point<long>& new_pnt );
               void CreateFtorIons();
               //void InitFtorIons();
               void CreateHoles();
               void InitHoleCounts( const Point<ulong>& sz );

             double CalcPoreCellCharge( Point<long> pnt );
      inline double Potential( const Point<long>& pnt ) const;
      inline double GetPotential( const Point<long>& pnt ) const;
      inline double BasePotential( const Point<long>& pnt ) const;

               void RemoveNearestTips( const Point<long>& pnt );
               bool DiffuseFtor( const SimplePoint<long>& pnt ) const;
             double CalcHoleDensity( double x );

               void InitPotential();
               void RecalcPotential();
               void UpdatePotentialFromPoint( const Point<long>& pnt, bool inverse = false );
               void UpdatePotentialWave( const Point<long>& pnt, bool inverse = false );
               void UpdatePotentialInCell( const Point<long>& pnt );
             double CalcPotentialByNeighbors( const Point<long>& pnt );
               void RecalcPotentialWave();
             double CalcMaxPotentialError();

   //const bool usePorePotential;
   //Matrix3d<double> porePotentials; // pore electrical field
   //void UpdatePotentials( const Point<long>& pnt, double q );
   //void UpdatePorePotential( const Point<long>& pnt, double mul );
   //void RecalcPorePotential();
   //inline double ParticlePotential( const Point<long>& particle, const Point<long>& pnt, double q );
   //inline double Potential1( const Point<long>& pnt ) const;
   //double CalcPotential1Error( bool average );
   //double CalcAveragePotential1Error();
   //double CalcMaxPotential1Error();
};
