#pragma once

#include <view.h>
#include <resource.h>
#include <version.h>
#include <funcs.h>

enum PorousHoleModelOpt
{
   phmoEXP_PROB       = 1<<0,
   phmoUSE_FTOR       = 1<<1,
   phmoUSE_TWIN_HOLES = 1<<2,
   phmoDEFAULT = phmoUSE_TWIN_HOLES | phmoEXP_PROB
};

struct ModelParams
{
   Point<ulong> size;

   double tension;
   double temperature;
   double luminosity;
   double currentStrength;

   // Other parameters
   int holeModelOpt;
   double probFuncDeg; //ProbFunc ProbFunction = pow_func<20,1>;
   DPoint2D probRange;
   double sidewayKoeff;

   // Surface options
   Point<ulong> lifeTimes;
   double oxidProb;

   ModelParams()
   {
      // default values of model parameters
      size.Init(1, 200, 120);

      tension     = 10.0;
      temperature = 20;
      luminosity  = 3e20;
      currentStrength = 0.0; 

      holeModelOpt = phmoDEFAULT;
      probFuncDeg = 20;
      probRange.Init( .0001, .2 );
      sidewayKoeff = 0.0;

      // Surface options
      lifeTimes.Init(0,1,1);
      oxidProb = .003;
   }
};

// View modes

enum ViewMode
{
   vmDISTRIBUTION    = 0,
   vmSURFACE         = 1, // Show silicon plate surface
   vmHORZ_SLIT       = 2, // Show horizontal slit of the silicon plate
   vmVERT_SLIT       = 3, // Show vertical slit of the silicon plate
   vm3D_MODE         = 4,
   vm3D_OPENGL_MODE  = 5,
   vmDIMENSION       = 6, // Show cluster dimension(s)
   vmPROPERTY        = 7, // Show cluster properties
   vmRESULTS         = 8, // Modeling results
   vmVAH             = 9, // Volt-ampere charactiristics
   vmTOTAL,
   vmDEFAULT = vmVERT_SLIT
};

const uint vfSHOW_HOLES           = 1 << 0; // Show holes
const uint vfFREEZE               = 1 << 1; // Pause
const uint vfSHOW_EQU_CURVES      = 1 << 2; // Show equipotential curves (surfaces)
const uint vfSHOW_TIPS            = 1 << 3; // Show pore tips
const uint vfAUTO_ROTATION        = 1 << 4; // Auto rotate model in 3D mode
const uint vfSHOW_SHADOW          = 1 << 5; // 3D mode only: show shadow
const uint vfSHOW_BOUNDS          = 1 << 6; // 3D mode only: show bounds of silicon
const uint vfSHOW_FTOR            = 1 << 7; // Show Ftor ions
const uint vfTRANSPARENCY         = 1 << 8;
const uint vfSHOW_OXID            = 1 << 9;
const uint vfCALC_SLIT_DIM        = 1 << 10; // Compute fractal dimension of slit
const uint vfSHOW_DIM_PLOT        = 1 << 11; // Show dimension plot
const uint vfFRACTAL_DIM          = 1 << 12; // Show fractal dimension only
const uint vfCORRELATION_DIM      = 1 << 13; // Show correlation dimension only
const uint vfMASS_DIM             = 1 << 14; // Show fractal dimension only
const uint vfSHOW_PORE_CLOUD      = 1 << 15; // Show pore cloud
const uint vfAVERAGE_DEPTH        = 1 << 16;
const uint vfPORE_CONCENTRATION   = 1 << 17;
const uint vfPORE_AREA            = 1 << 18;
const uint vfPORE_VOLUME          = 1 << 19;
const uint vfPORE_POROSITY        = 1 << 20;
const uint vfSHOW_TWIN_HOLES_ONLY = 1 << 21; // Show only hole couples
const uint vfALL_DIM_ONLY_FLAGS   = vfFRACTAL_DIM | vfCORRELATION_DIM | vfMASS_DIM;
const uint vfALL_PROP_ONLY_FLAGS  = vfAVERAGE_DEPTH | vfPORE_CONCENTRATION | vfPORE_AREA | vfPORE_VOLUME | vfPORE_POROSITY;
const uint vfTOTAL                = 22;
const uint vfDEFAULT = vfSHOW_HOLES | vfSHOW_TWIN_HOLES_ONLY | vfSHOW_BOUNDS |
                       vfAUTO_ROTATION | vfTRANSPARENCY | vfSHOW_PORE_CLOUD | vfSHOW_DIM_PLOT;

enum ElFieldMode
{
   efmNO_FIELD = 0,      // Without electromagnetic field
   efmOVERALL_FIELD,     // Show total field
   efmTOTAL,
   efmDEFAULT = efmNO_FIELD

   // obsolete
   //efmTWO_COMPONENTS,    // Split field on two components: global and local (pores' fields)
   //efmPORE_FIELD,        // Always show pores' fields
   //efmDIFF,              // Diff mode with pore potential
};

enum EquCurvesMode
{
   ecmOVERALL = 0,

   // obsolete:
   ecmOVERALL1, // old method
   ecmBOTTOM,
   ecmTOP,
   ecmBOTH,

   ecmFIRST   = ecmOVERALL,
   ecmLAST    = ecmOVERALL,
   ecmDEFAULT = ecmOVERALL
};

const int DefaultCurvesNum = 10;
const int LineColorNum = 4;

struct SiliconModelTheme
{
   bool     dark;
   COLORREF bgColor;
   COLORREF poreColor;
   COLORREF pore3dColor;
   COLORREF poreTipColor;
   COLORREF poreCloudColor;
   COLORREF holeColor;
   COLORREF hole2Color;
   COLORREF ftorColor;
   COLORREF oxidizedColor;
   COLORREF absorbedColor;
   COLORREF chargedColor;
   COLORREF equCurveColor;
   COLORREF dotColor;
   COLORREF lineColor[LineColorNum];
   COLORREF sysCoorColor;
   COLORREF dimColor;
   COLORREF textColor;
   COLORREF hiliteColor;

   SiliconModelTheme() { memset( this, 0, sizeof(SiliconModelTheme) ); }
};

extern SiliconModelTheme  NightTheme, LightTheme, ModelTheme;

struct ModelViewOptions
{
   ViewMode         mode;
   uint             flags;
   ElFieldMode      field_mode;
   EquCurvesMode    equ_curves_mode;
   int              equ_curves_num;

        ModelViewOptions():
           mode(vmDEFAULT), flags(vfDEFAULT), field_mode(efmDEFAULT),
           equ_curves_mode(ecmDEFAULT), equ_curves_num(DefaultCurvesNum) {}

   bool operator == ( const ModelViewOptions& opt )
   {
      return opt.mode == mode &&
             opt.flags == flags &&
             field_mode == opt.field_mode &&
             equ_curves_mode == opt.equ_curves_mode &&
             equ_curves_num == opt.equ_curves_num;
   }
};

double ComputeCurrentStrength( double tension, bool convert_units );

const double ABSOLUTE_ZERO = -273.15;