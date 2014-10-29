// Dimensions

#pragma once

class PorousModel;

typedef double (*CalcDimFunc)( const PorousModel& model, bool invert, double eps, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max );
typedef int (*InitCalcFunc)( const PorousModel& model, double& start_eps );
typedef double (*NextEpsFunc)( double eps );
typedef void (*PlotUpdateFunc)( HWND );
typedef bool (*UpdateDimFunc)( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value );

class Dimension
{
   public:
                      Dimension( const char* dim_name, bool invert, CalcDimFunc func, InitCalcFunc init_f,
                                 NextEpsFunc eps_func, UpdateDimFunc update_f, int eps_sign, const char* axis_1, const char* axis_2 ):
                        name(dim_name), inverted(invert), compute_func(func), init_func(init_f), next_eps_func(eps_func), update_func(update_f),
                        points_cnt(0), dimension(0.0), valid(false), epsSign(eps_sign), axis1(axis_1), axis2(axis_2) {}

                 void Recalc( const PorousModel& model, PlotUpdateFunc plot_func, HWND hWnd );
                 void Update( const PorousModel& model, SimplePoint<long> pnt );
                 void Invalidate() { valid = false; }

          const char* GetName() const { return name; }
          const char* GetAxis1() const { return axis1; }
          const char* GetAxis2() const { return axis2; }
               double GetDimension() const { return dimension; }
      const DPoint2D* GetPoints() const { return points; }
                  int GetPointsNum() const { return points_cnt; }
  const Line<double>& GetLine() const { return line; }
                  int GetEpsSign() const { return epsSign; }              
                  int InitCalc( const PorousModel& model, double& start_eps );

   private:
               bool inverted; // Calc dimension of pores (false) or solid material - silicon (true)
        const char* name;
        const char* axis1;
        const char* axis2;
       Line<double> line;
       VP<DPoint2D> values; // couples of ( eps, N(eps) ) - needed for fast recompute
       VP<DPoint2D> points; // couples of ( log(eps), log(N(eps)) )
                int points_cnt;
             double dimension;
                int epsSign;
               bool valid;
        CalcDimFunc compute_func;
      UpdateDimFunc update_func;
       InitCalcFunc init_func;
        NextEpsFunc next_eps_func;
};

int FracDimInit( const PorousModel& model, double& start_eps );
double FracDimNextEps( double eps );
double CalcFractalDim( const PorousModel& model, bool invert, double eps, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max );
bool UpdateFractalDim( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value );

int CorDimInit( const PorousModel& model, double& start_eps );
double CorDimNextEps( double eps );
double CalcCorrelation( const PorousModel& model, bool invert, double eps, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max );
bool UpdateCorrelationDim( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value );

int MassDimInit( const PorousModel& model, double& start_eps );
double MassDimNextEps( double eps );
double CalcMassDim( const PorousModel& model, bool invert, double eps, long x_from, long x_to, Indicator* indic, double perc_min, double perc_max );
bool UpdateMassDim( const PorousModel& model, SimplePoint<long> pnt, bool invert, double edge, long x_from, long x_to, double *p_value );