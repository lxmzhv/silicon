// Model characteristics

#pragma once

class PorousModel;

class Property
{
   public:
                      Property( const char* prop_name, const char* _units ):
                        name(prop_name), units(_units), valid(false), value(0) {}

                 void Recalc( const PorousModel& model );
                 void Update( const PorousModel& model, SimplePoint<long> pnt );
                 void Invalidate() { valid = false; }

          const char* GetName() const { return name; }
          const char* GetUnits() const { return units; }
               double GetValue() const { return valid ? value : 0.0; }

   private:
        const char* name;
        const char* units;
             double value;
               bool valid;

    virtual double Compute( const PorousModel& model ) = NULL;
    virtual double ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore ) = NULL;
};

class AverageDepth: public Property
{
  public:
           AverageDepth(): Property( "Average pore depth", "m" ) {}

  private:
    virtual double Compute( const PorousModel& model );
    virtual double ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore );
};

class PoreArea: public Property
{
  public:
           PoreArea(): Property( "Pore cluster area", "m^2" ) {}

  private:
    virtual double Compute( const PorousModel& model );
    virtual double ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore );
};

class PoreVolume: public Property
{
  public:
           PoreVolume(): Property( "Pore cluster volume", "m^3" ) {}

  private:
    virtual double Compute( const PorousModel& model );
    virtual double ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore );
};

class PoreConcentration: public Property
{
  public:
           PoreConcentration(): Property( "Concentration of pores", "1/m^3" ) {}

  private:
    virtual double Compute( const PorousModel& model );
    virtual double ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore );
};

class PorePorosity: public Property
{
  public:
           PorePorosity(): Property( "Porosity", "%" ) {}

  private:
    virtual double Compute( const PorousModel& model );
    virtual double ComputeUpdate( const PorousModel& model, double old_val, const Point<ulong>& new_pore );
};