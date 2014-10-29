// Управляющий модуль

#include <crtdbg.h>
#include <view.h>
#include <pointers.h>
#include <file.h>
#include <SiliconModel1.h>
#include <SiliconModel2.h>
#include <SiliconDisplay.h>
#include <Silicon.h>
#include <SiliconDim.h>

// If this macro is defined, the diffusion limited aggregation (DLA) model is used,
// otherwise the cell automat model is used.
#define USE_DLA_MODEL

#ifdef USE_DLA_MODEL
   typedef SiliconHoleModel TModel;
#else
   typedef SiliconModel1 TModel;
#endif

P<TModel> model;
ModelParams params;

const long    DEEP_STEP = 1;
const float   ROTATE_ANGLE = (float)(M_PI/500);
const Point2D<float> DefRotation( ROTATE_ANGLE, 0 );
Point2D<float> Rotation = DefRotation;
Point2D<float> Angle( 0.0f, 0.0f );
Array<double> fractalDimArr, corrDimArr, massDimArr;
double        maxFractalDim, maxCorrDim, maxMassDim;
Array<double> averageDepthArr, poreConcentrationArr, poreAreaArr, poreVolumeArr, porePorosityArr;
double        maxAverageDepth, maxPoreConcentration, maxPoreArea, maxPoreVolume, maxPorePorosity;
double TotalTime = 0.0;
const int LastStepTimeCount = 100;
Array<DWORD> LastStepTimes;
DWORD PrevTickCount = 0;
int CurResult = 0;

const double StopVolumeDef  = 100;
const double StopVolumeStep = 1;
const double StopDepth = 0.8;
double StopVolume = 0;
bool WasStopped = false;

const double DefZoom = 1.0;
double Zoom = DefZoom; // zoom in 3D mode

const double DefViewPosition = 0.5;
double ViewPosition = DefViewPosition; // z-position of a viewer (>0)

bool ShiftDown = false; // Shift key is pressed

ModelViewOptions ViewOpt;
Trf_3d TrfMatrix;

double BrightnessBound = 0.2;

HANDLE HBmp = NULL;
DCObject<HANDLE> HBmpDC( 0, true );

LRESULT CALLBACK WindowProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Params1DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Params2DlgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

Point<ulong> GetSize( int n )
{
  static const int sizes[] = { 100, 300, 500, 700, 1000 };
  int sz = sizes[ max( min( n, 4 ), 0 ) ];
  return Point<ulong>( sz, sz, sz/3 );
}

int GetCharNum( char c )
{
  switch( c )
  {
    case VK_F1:   return 0;
    case VK_F2:   return 1;
    case VK_F3:   return 2;
    case VK_F4:   return 3;
    case VK_F5:   return 4;
  }
  return 0;
}

static TModel* NewSiliconModel( const Point<ulong>& size )
{
  params.size = size;
  params.currentStrength = ComputeCurrentStrength( params.tension, true );

#ifdef USE_DLA_MODEL
  // Diffusion limited aggregation (DLA) model
  return new SiliconHoleModel( params.size, params.lifeTimes, params.oxidProb, params.tension,
                               params.temperature-ABSOLUTE_ZERO, params.luminosity, params.currentStrength,
                               params.holeModelOpt, ViewOpt );
#else
  // Cell automat model
  return new SiliconModel1( params.size, params.lifeTimes, params.oxidProb, ViewOpt,
                            params.probFuncDeg, params.probRange, params.sidewayKoeff );
#endif
}

static TModel* NewSiliconModel( int sz_n = -1 )
{
  Point<ulong> size = params.size;
  if( sz_n >= 0 )
    size = GetSize( sz_n );
  return NewSiliconModel( size );
}

static double GetCurPoreVolumePerc()
{
  Point<ulong> sz = model->GetSize();
  return 100.0*model->GetPoreCount()/sz.x/sz.y/sz.z;
}

static void Stop()
{
  ViewOpt.flags |= vfFREEZE;
}

static void Run()
{
  ViewOpt.flags &= ~vfFREEZE;
  PrevTickCount = GetTickCount();
}

static void PostStep()
{
  double value = model->GetMatrix().GetFractalDim().GetDimension();
  fractalDimArr.Append( 1, &value );
  Maximize( maxFractalDim, value );

  value = model->GetMatrix().GetCorrelationDim().GetDimension();
  corrDimArr.Append( 1, &value );
  Maximize( maxCorrDim, value );

  value = model->GetMatrix().GetMassDim().GetDimension();
  massDimArr.Append( 1, &value );
  Maximize( maxMassDim, value );

  value = model->GetMatrix().GetAverageDepth().GetValue();
  averageDepthArr.Append( 1, &value );
  Maximize( maxAverageDepth, value );

  value = model->GetMatrix().GetPoreConcentration().GetValue();
  poreConcentrationArr.Append( 1, &value );
  Maximize( maxPoreConcentration, value );

  value = model->GetMatrix().GetPoreArea().GetValue();
  poreAreaArr.Append( 1, &value );
  Maximize( maxPoreArea, value );

  value = model->GetMatrix().GetPoreVolume().GetValue();
  poreVolumeArr.Append( 1, &value );
  Maximize( maxPoreVolume, value );

  value = model->GetMatrix().GetPorePorosity().GetValue();
  porePorosityArr.Append( 1, &value );
  Maximize( maxPorePorosity, value );

  if( StopVolume > 0.0 && GetCurPoreVolumePerc() > StopVolume )
  {
    Stop();
    StopVolume += StopVolumeStep;
  }
  if( !WasStopped && model->GetMatrix().GetMaxDeep() >= model->GetMatrix().GetSize().z * StopDepth )
  {
    Stop();
    WasStopped = true;
  }
}

void OnModelChange( HWND hWnd )
{
  trf_init( TrfMatrix );
  TotalTime = 0.0;
  LastStepTimes.Clear();
  StopVolume = StopVolumeDef;
  WasStopped = false;
  PrevTickCount = GetTickCount();
  if( model )
  {
    InitOpenGL_model( hWnd, model->GetMatrix() );
    if( model->GetSize().x == 1 )
      ViewOpt.flags &= ~vfAUTO_ROTATION;

    fractalDimArr.Clear();
    corrDimArr.Clear();
    massDimArr.Clear();
    averageDepthArr.Clear();
    poreConcentrationArr.Clear();
    poreAreaArr.Clear();
    poreVolumeArr.Clear();
    porePorosityArr.Clear();

    maxFractalDim = 0;
    maxCorrDim = 0;
    maxMassDim = 0;
    maxAverageDepth = 0;
    maxPoreConcentration = 0;
    maxPoreArea = 0;
    maxPoreVolume = 0;
    maxPorePorosity = 0;

    model->GetMatrix().RecalcProperties();

    PostStep();
  }
}

static void Init( HWND hWnd )
{
  model = NewSiliconModel();
  OnModelChange( hWnd );
}

// Get current step time in seconds (average time of the last 3 steps)
static double GetCurStepTime()
{
  int cnt = LastStepTimes.Size();
  if( cnt == 0 )
    return 0.0;

  DWORD time = 0;
  for( int i = 0; i < cnt; ++i )
    time += LastStepTimes[i];

  return time == 0 ? 0 : cnt*1e3/time;
}

static void SetProcessTitle( HWND h_wnd )
{
  static String title;
  Point<ulong> sz = model->GetSize();

  title.Clear();
  title << "Silicon Model v" << SILICON_VERSION;
  title << " | Size: [ " << sz.x << " x " << sz.y << " x " << sz.z << " ]";
  title << " | Step: " << model->GetStep();
  title << " (" << int(GetCurStepTime()) << " ps)";

  int pore_cnt = model->GetPoreCount();
  Buf buf;
  sprintf( buf, "%.2f", GetCurPoreVolumePerc() );
  title << " | Pores: " << pore_cnt << " (" << buf << " %)";

  switch( ViewOpt.mode )
  {
    case vmHORZ_SLIT:
      title << " | Horizontal slit [ deep: " << model->GetMatrix().SlitDeep() << " ]";
      break;
    case vmVERT_SLIT:
    case vmDIMENSION:
      title << " | Vertical slit [ deep: " << model->GetMatrix().VertSlitDeep() << " ]";
      break;
  }

  title << " | Brightness bound: " << BrightnessBound;

  //title << " | El.field err: ";
  //sprintf( buf, "%.1f", 100*model->CalcMaxPotential1Error() );
  //title << buf << "% (max), ";
  //sprintf( buf, "%.1f", 100*model->CalcAveragePotential1Error() );
  //title << buf << "% (avg)";

  if( ViewOpt.flags&vfFREEZE )
    title << " | [pause]";

  SetWindowText( h_wnd, title );
}

int ModeMenu[vmTOTAL][2] =
{
  { IDM_VIEW_SURFACE,         vmSURFACE        },
  { IDM_VIEW_VERT_SLIT,       vmVERT_SLIT      },
  { IDM_VIEW_HORZ_SLIT,       vmHORZ_SLIT      },
  { IDM_VIEW_DIMENSION,       vmDIMENSION      },
  { IDM_VIEW_PROPERTY,        vmPROPERTY       },
  { IDM_VIEW_3D,              vm3D_MODE        },
  { IDM_VIEW_3D_OPENGL,       vm3D_OPENGL_MODE },
  { IDM_VIEW_DISTR,           vmDISTRIBUTION   },
  { IDM_VIEW_VAH,             vmVAH            },
  { IDM_VIEW_RESULTS,         vmRESULTS        }
};

int ElFieldMenu[efmTOTAL][2] =
{
  { IDM_VIEW_NO_FIELD,        efmNO_FIELD       },
  { IDM_VIEW_OVERALL_FIELD,   efmOVERALL_FIELD  },
  //{ IDM_VIEW_TWO_COMPONENTS,  efmTWO_COMPONENTS },
  //{ IDM_VIEW_PORE_FIELD,      efmPORE_FIELD     },
  //{ IDM_VIEW_FIELD_DIFF,      efmDIFF           },
};

int FlagsMenu[vfTOTAL][2] =
{
  { IDM_PAUSE,                vfFREEZE           },
  { IDM_VIEW_SHOW_HOLES,      vfSHOW_HOLES       },
  { IDM_VIEW_SHOW_TWIN_HOLES_ONLY, vfSHOW_TWIN_HOLES_ONLY },
  { IDM_VIEW_SHOW_EQU_CURVES, vfSHOW_EQU_CURVES  },
  { IDM_VIEW_SHOW_TIPS,       vfSHOW_TIPS        },   
  { IDM_VIEW_SHOW_PORE_CLOUD, vfSHOW_PORE_CLOUD  },
  { IDM_VIEW_AUTO_ROTATION,   vfAUTO_ROTATION    },
  { IDM_VIEW_SHOW_SHADOW,     vfSHOW_SHADOW      },
  { IDM_VIEW_SHOW_BOUNDS,     vfSHOW_BOUNDS      },
  { IDM_VIEW_SHOW_FTOR,       vfSHOW_FTOR        },
  { IDM_VIEW_TRANSPARENT,     vfTRANSPARENCY     },
  { IDM_VIEW_SHOW_OXID,       vfSHOW_OXID        },
  { IDM_CALC_SLIT_DIMENSION,  vfCALC_SLIT_DIM    },
  { IDM_SHOW_DIM_PLOT,        vfSHOW_DIM_PLOT    },
  { IDM_FRACTAL_DIM_ONLY,     vfFRACTAL_DIM      },
  { IDM_CORRELATION_DIM_ONLY, vfCORRELATION_DIM  },
  { IDM_MASS_DIM_ONLY,        vfMASS_DIM         },
  { IDM_AVERAGE_DEPTH,        vfAVERAGE_DEPTH    },
  { IDM_PORE_CONCENTRATION,   vfPORE_CONCENTRATION },
  { IDM_PORE_AREA,            vfPORE_AREA        },
  { IDM_PORE_VOLUME,          vfPORE_VOLUME      },
  { IDM_PORE_POROSITY,        vfPORE_POROSITY    },
};

int Keys[][2] =
{
  { ' ',  IDM_PAUSE          },
  { '0',  IDM_VIEW_DISTR     },
  { '1',  IDM_VIEW_SURFACE   },
  { '2',  IDM_VIEW_HORZ_SLIT },
  { '3',  IDM_VIEW_VERT_SLIT },
  { '4',  IDM_VIEW_3D        },
  { '5',  IDM_VIEW_3D_OPENGL },
  { '6',  IDM_VIEW_DIMENSION },
  { '7',  IDM_VIEW_PROPERTY  },
  { '8',  IDM_VIEW_RESULTS  },
  { '9',  IDM_VIEW_VAH       },
  { 'a',  IDM_VIEW_TRANSPARENT },    
  { 'b',  IDM_VIEW_SHOW_BOUNDS },
  { 'c',  IDM_VIEW_CHANGE_COLORS },
  { 'd',  IDM_SAVE_DISTR     },
  { 'e',  IDM_VIEW_SHOW_EQU_CURVES },
  { 'E',  IDM_VIEW_HIDE_EQU_CURVES },
  { 'g',  IDM_SHOW_DIM_PLOT  },
  { 'h',  IDM_VIEW_SHOW_HOLES },
  { 'H',  IDM_VIEW_SHOW_TWIN_HOLES_ONLY },
  { 'i',  IDM_VIEW_SHOW_FTOR },
  { 'l',  IDM_LOAD           },
  { 'o',  IDM_PARAMS         },
  { 'r',  IDM_VIEW_AUTO_ROTATION },
  { 's',  IDM_SAVE           },
  { 't',  IDM_VIEW_SHOW_TIPS },
  { 'u',  IDM_VIEW_SHOW_PORE_CLOUD },
  { 'w',  IDM_VIEW_SHOW_SHADOW },
  { 'x',  IDM_VIEW_SHOW_OXID },
  { 'z',  IDM_RESET          },
  {  0,   0 }
};

void SetMenuChecks( HWND hWnd )
{
  HMENU h_menu = GetMenu( hWnd );

  for( int i = 0; i < vmTOTAL; ++i )
    CheckMenuItem( h_menu, ModeMenu[i][0], ViewOpt.mode == ModeMenu[i][1] ? MF_CHECKED : MF_UNCHECKED );

  for( int i = 0; i < efmTOTAL; ++i )
    CheckMenuItem( h_menu, ElFieldMenu[i][0], ViewOpt.field_mode == ElFieldMenu[i][1] ? MF_CHECKED : MF_UNCHECKED );

  for( int i = 0; i < vfTOTAL; ++i )
    CheckMenuItem( h_menu, FlagsMenu[i][0], ViewOpt.flags & FlagsMenu[i][1] ? MF_CHECKED : MF_UNCHECKED );

  CheckMenuItem( h_menu, IDM_ALL_DIMENSIONS, (ViewOpt.flags & vfALL_DIM_ONLY_FLAGS)==0 ? MF_CHECKED : MF_UNCHECKED );
  CheckMenuItem( h_menu, IDM_ALL_PROPERTIES, (ViewOpt.flags & vfALL_PROP_ONLY_FLAGS)==0 ? MF_CHECKED : MF_UNCHECKED );
}

void FullWindowUpdate( HWND h_wnd )
{
  SetProcessTitle( h_wnd );
  UpdateWindow( h_wnd );
  InvalidateRect( h_wnd, 0, false );
  SetMenuChecks( h_wnd );
}

void MainInit()
{
  InitDisplay();
}

void win_main()
{
  MainInit();

  View view( WindowProc, "MyWindow", (LPCSTR)IDC_CELLULAR );
  view.Run( SW_MAXIMIZE );
  model = 0; // дабы мнимой утечки не было
}

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow )
{
  //_CrtSetBreakAlloc( 51 );
  _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
  win_main();
  _CrtDumpMemoryLeaks();
}

void SaveModel( HWND h_wnd )
{
  try
  {
    char file_name[MAX_PATH] = "model.fmd";

    if( !h_wnd )   h_wnd = GetActiveWin();
    if( !AskFileName( file_name, "Fractal models\0*.fmd\0All\0*.*\0", OFN_PATHMUSTEXIST, "Select file to save model.", h_wnd ) )
      return;

    ApplyFileExtension( file_name, sizeof(file_name), ".fmd" );

    Indicator indic;
    indic.Create( h_wnd, 100, 100, "Saving model..." );
    indic.Show();

    File f( file_name, fomWRITE|fomCREATE );
    model->Save( f );
    f.Write( ViewOpt.mode );

    indic.Hide();

    MessageBox( h_wnd, "Model has been saved.", "Finished!", MB_OK );
  }
  catch(...)
  {}
  FullWindowUpdate( h_wnd );
}

void LoadModel( HWND h_wnd )
{
  P<TModel> new_model = NewSiliconModel();
  try
  {
    if( !h_wnd )   h_wnd = GetActiveWin();
    char file_name[MAX_PATH] = "model.fmd";
    if( !AskFileName( file_name, "Fractal models\0*.fmd\0All\0*.*\0", OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, "Select file to load model.", h_wnd ) )
      return;

    Indicator indic;
    indic.Create( h_wnd, 100, 100, "Loading model..." );
    indic.Show();

    File f( file_name, fomREAD );
    new_model->Load( f );
    f >> ViewOpt.mode;

    model = new_model; // модель загрузилась - отдаём
    OnModelChange( h_wnd );

    Stop();
    params.size = model->GetSize();
    params.lifeTimes = model->GetSurface().GetWaitTimes();
    params.oxidProb = model->GetOxid().GetProb();
    //params.probFuncDeg = model->GetMatrix().GetProbDeg();
    //params.probRange = model->GetMatrix().GetProbRange();
    //params.sidewayKoeff = model->GetMatrix().GetSidewayKoeff();

    indic.Hide();

    MessageBox( h_wnd, "Model loaded.", "Finished!", MB_OK );
  }
  catch(...)
  {}
  FullWindowUpdate( h_wnd );
}

int ComputeDepth( HDC h_bmp_dc, double bound, int width, int height )
{
  POINT p = { 0,0 };
  int red, green, blue;
  double brightness, bright_factor = 1.0/(3*0xFF);

  COLORREF color;
  for( p.y = 0; p.y < height; ++p.y )
    for( p.x = 0; p.x < width; ++p.x )
    {
      color = GetPixel( h_bmp_dc, p.x, p.y );

      if( color == CLR_INVALID )
        continue;

      red   = GetRValue(color);
      green = GetGValue(color);
      blue  = GetBValue(color);
      brightness = bright_factor*(red + green + blue);

      if( brightness <= bound )
        return p.y;
    }

  return height-1;
}

/* 0 <= bound <= 1 */
void BMP2Model( HWND h_wnd, HDC h_bmp_dc, HANDLE h_bmp, double bound )
{
  if( !h_bmp )
    return;

  BITMAP bmp;
  GetObject( h_bmp, sizeof(bmp), &bmp );
  if( bmp.bmWidth <= 0 || bmp.bmHeight <= 0 )
    Error( "Invalid image size!" );

  int depth = ComputeDepth( h_bmp_dc, bound, bmp.bmWidth, bmp.bmHeight );
  if( depth < 0 || depth >= bmp.bmHeight )
    Error( "Invalid depth!" );

  Point<ulong> sz( 1, bmp.bmWidth, bmp.bmHeight - depth );

  P<TModel> new_model = NewSiliconModel( sz );
  if( !new_model->GetMatrix().LoadBMP( h_bmp_dc, bound, depth ) )
    Error( "Fractal loading failed!" );

  model = new_model; // модель загрузилась - отдаём
  OnModelChange( h_wnd );

  Stop();
  params.size = model->GetSize();
  params.lifeTimes = model->GetSurface().GetWaitTimes();
  params.oxidProb = model->GetOxid().GetProb();

  FullWindowUpdate( h_wnd );
}

void LoadFractal( HWND h_wnd )
{
  try
  {
    if( !h_wnd )   h_wnd = GetActiveWin();

    char file_name[MAX_PATH] = "";
    if( !AskFileName( file_name, "Bitmap images\0*.bmp\0All files\0*.*\0", OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST, "Select file to load fractal.", h_wnd ) )
      return;

    HANDLE h_bmp = LoadImage( 0, file_name, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED | LR_LOADFROMFILE );
    if( !h_bmp )
      Error( "Cannot load specified file." );

    HBmpDC.SetObject( h_bmp );
    HBmp = h_bmp;

    BMP2Model( h_wnd, HBmpDC, HBmp, BrightnessBound );

    MessageBox( h_wnd, "Fractal loaded.", "Finished!", MB_OK );
  }
  catch(...)
  {}
}

void SaveMatrixZDistribution( HWND h_wnd )
{
  try
  {
    char file_name[MAX_PATH] = "z_density.csv";

    if( !h_wnd )   h_wnd = GetActiveWin();
    if( !AskFileName( file_name, "CSV tables\0*.csv\0All\0*.*\0", OFN_PATHMUSTEXIST, "Select file to save distribution density.", h_wnd ) )
      return;

    ApplyFileExtension( file_name, sizeof(file_name), ".csv" );

    Indicator indic;
    indic.Create( h_wnd, 100, 100, "Saving distribution..." );
    indic.Show();

    const BoolMatrix3D& matrix = model->GetMatrix().GetMatrix();
    const Point<ulong>& sz = matrix.GetSize();

    File f( file_name, fomWRITE|fomCREATE );
    String s;
    long n;
    for( long z = 0; z < (long)sz.z; ++z )
    {
      n = matrix.CountValues(z);
      if( n <= 0 )
        break;

      s.Clear();
      s << z << ", " << n << '\n';
      f << (const char*)s;
    }

    indic.Hide();

    MessageBox( h_wnd, "Distribution has been saved.", "Finished!", MB_OK );
  }
  catch(...)
  {}
  FullWindowUpdate( h_wnd );
}

void SetParams( HWND hWnd )
{
  if( DialogBox( GetModuleHandle(0), (LPCTSTR)IDD_PARAMS, hWnd, (DLGPROC)Params2DlgProc ) == IDOK )
  {
    if( params.size.x <= 0 )  params.size.x = 1;
    if( params.size.y <= 0 )  params.size.y = 1;
    if( params.size.z <= 0 )  params.size.z = 1;

    model = NewSiliconModel();
    OnModelChange( hWnd );
  }
}

static void About( HWND hWnd )
{
  DialogBox( GetModuleHandle(0), (LPCTSTR)IDD_ABOUT, hWnd, (DLGPROC)AboutDlgProc );
}

static void SetDimFlag( uint flag )
{
  ViewOpt.flags &= ~vfALL_DIM_ONLY_FLAGS;
  ViewOpt.flags |= flag;
}

static void SetPropFlag( uint flag )
{
  ViewOpt.flags &= ~vfALL_PROP_ONLY_FLAGS;
  ViewOpt.flags |= flag;
}

// Switch dimensions: All -> Fractal -> Correlation -> Mass -> All
static void ShiftDimFlag()
{
  uint flags[] = { 0, vfFRACTAL_DIM, vfCORRELATION_DIM, vfMASS_DIM };
  const int cnt = sizeof(flags)/sizeof(flags[0]);
  uint cur_flag = ViewOpt.flags & vfALL_DIM_ONLY_FLAGS;

  for( int i = 0; i < cnt; ++i )
    if( cur_flag == flags[i] )
    {
      SetDimFlag( flags[(i+1)%cnt] );
      break;
    }
}

static void ShiftPropFlag()
{
  uint flags[] = { 0, vfPORE_POROSITY, vfAVERAGE_DEPTH, vfPORE_CONCENTRATION, vfPORE_AREA, vfPORE_VOLUME };
  const int cnt = sizeof(flags)/sizeof(flags[0]);
  uint cur_flag = ViewOpt.flags & vfALL_PROP_ONLY_FLAGS;

  for( int i = 0; i < cnt; ++i )
    if( cur_flag == flags[i] )
    {
      SetPropFlag( flags[(i+1)%cnt] );
      break;
    }
}

static void SetDimensionMode( HWND hWnd )
{
  // Switch dimensions: All -> Fractal -> Correlation -> Mass -> All
  if( ViewOpt.mode == vmDIMENSION )
    ShiftDimFlag();
  else
    ViewOpt.mode = vmDIMENSION;
}

static void SetPropertyMode( HWND hWnd )
{
  if( ViewOpt.mode == vmPROPERTY )
    ShiftPropFlag();
  else
    ViewOpt.mode = vmPROPERTY;
}

static void ShowEquCurves( bool forward )
{
  if( !(ViewOpt.flags & vfSHOW_EQU_CURVES) )
  {
    ViewOpt.flags ^= vfSHOW_EQU_CURVES;
    ViewOpt.equ_curves_mode = forward ? ecmFIRST : ecmLAST;
    return;
  }

  int shift = 0;

  if( forward && ViewOpt.equ_curves_mode < ecmLAST )
    shift = 1;
  else if( !forward && ViewOpt.equ_curves_mode > ecmFIRST )
    shift = -1;
  else
    ViewOpt.flags ^= vfSHOW_EQU_CURVES;

  if( shift != 0 )
    ViewOpt.equ_curves_mode = (EquCurvesMode)(ViewOpt.equ_curves_mode + shift);
}

static void HandleMessage( HWND hWnd, int wmId )
{
  switch( wmId )
  {
    // File
    case IDM_SAVE:   SaveModel( hWnd ); return;
    case IDM_LOAD:   LoadModel( hWnd ); break;
    case IDM_LOAD_FRACTAL: LoadFractal( hWnd ); return;
    case IDM_EXIT:   SendMessage( hWnd, WM_CLOSE, 0, 0 );  return;

    case IDM_SAVE_DISTR: SaveMatrixZDistribution( hWnd ); return;

    // Dialogs
    case IDM_PARAMS:   SetParams( hWnd ); break;
    case IDM_ABOUT:    About( hWnd );  return;

    // Other
    case IDM_VIEW_DIMENSION:
      SetDimensionMode( hWnd );
      break;
    case IDM_VIEW_PROPERTY:
      SetPropertyMode( hWnd );
      break;
    case IDM_RESET:
      model = NewSiliconModel();
      OnModelChange( hWnd );
      break;
    case IDM_VIEW_CHANGE_COLORS:
      ModelTheme = (ModelTheme.dark ? LightTheme : NightTheme);
      break;
    case IDM_VIEW_SHOW_EQU_CURVES:
      ShowEquCurves( true );
      break;
    case IDM_VIEW_HIDE_EQU_CURVES:
      ShowEquCurves( false );
      break;
    case IDM_CALC_SLIT_DIMENSION:
      ViewOpt.flags ^= vfCALC_SLIT_DIM;
      model->GetMatrix().InvalidateProperties();
      break;
    case IDM_PAUSE:
      (ViewOpt.flags & vfFREEZE) ? Run() : Stop();
      break;
    case IDM_FRACTAL_DIM_ONLY:      SetDimFlag( vfFRACTAL_DIM );      break;
    case IDM_CORRELATION_DIM_ONLY:  SetDimFlag( vfCORRELATION_DIM );  break;
    case IDM_MASS_DIM_ONLY:         SetDimFlag( vfMASS_DIM );         break;
    case IDM_ALL_DIMENSIONS:        SetDimFlag( 0 );                  break;
    case IDM_AVERAGE_DEPTH:         SetPropFlag( vfAVERAGE_DEPTH );   break;
    case IDM_PORE_CONCENTRATION:    SetPropFlag( vfPORE_CONCENTRATION ); break;
    case IDM_PORE_AREA:             SetPropFlag( vfPORE_AREA );       break;
    case IDM_PORE_VOLUME:           SetPropFlag( vfPORE_VOLUME );     break;
    case IDM_PORE_POROSITY:         SetPropFlag( vfPORE_POROSITY );   break;
    case IDM_ALL_PROPERTIES:        SetPropFlag( 0 );                 break;
    case IDM_VIEW_RESULTS:
      if( ViewOpt.mode == vmRESULTS )
        ShiftPropFlag();
      else
        ViewOpt.mode = vmRESULTS;
      break;
    default:
    {
     bool found = false;

     // View modes
     for( int i = 0; i < vmTOTAL && !found; ++i )
       if( ModeMenu[i][0] == wmId )
       {
         ViewOpt.mode = (ViewMode)ModeMenu[i][1];
         found = true;
       }

     // Electromagnetic field modes
     for( int i = 0; i < efmTOTAL && !found; ++i )
       if( ElFieldMenu[i][0] == wmId )
       {
         ViewOpt.field_mode = (ElFieldMode)ElFieldMenu[i][1];
         found = true;
       }

     // Flags
     for( int i = 0; i < vfTOTAL && !found; ++i )
       if( FlagsMenu[i][0] == wmId )
       {
         ViewOpt.flags ^= FlagsMenu[i][1];
         found = true;
       }

     if( !found )
       return;

     break;
    }
  }

  if( ViewOpt.mode >= 0 )
    model->GetMatrix().SetViewOpt( ViewOpt );

  FullWindowUpdate( hWnd );
}

static void HandleCommand( HWND hWnd, WPARAM wParam, LPARAM )
{
  int wmId = LOWORD(wParam)/*, wmEvent = HIWORD(wParam)*/; 

  HandleMessage( hWnd, wmId );
}

const Point2D<float> DPnt2dUp( .0f, -1.0f );
const Point2D<float> DPnt2dDown( .0f, 1.0f );
const Point2D<float> DPnt2dLeft( 1.0f, .0f );
const Point2D<float> DPnt2dRight( -1.0f, .0f );
const Point2D<float> DPnt2dNull( .0f, .0f );

const Point2D<float>& GetVkVector( char vkey )
{
  switch( vkey )
  {
    case VK_DOWN:   return DPnt2dDown;
    case VK_UP:     return DPnt2dUp;
    case VK_RIGHT:  return DPnt2dRight;
    case VK_LEFT:   return DPnt2dLeft;
  }
  return DPnt2dNull;
}

static void HandlePageShift( bool forward )
{
  int mode = ViewOpt.mode;

  if( mode == vm3D_MODE || mode == vm3D_OPENGL_MODE )
    ViewPosition *= (forward ? 1.1 : 0.9);
  else if( mode == vmHORZ_SLIT || mode == vmVERT_SLIT )
  {
    ViewOpt.equ_curves_num -= (forward ? 1 : -1);
    Maximize( ViewOpt.equ_curves_num, 1 );
    Minimize( ViewOpt.equ_curves_num, 100 );
    model->GetMatrix().SetViewOpt( ViewOpt );
  }
}

static bool HandleHome()
{
  switch( ViewOpt.mode )
  {
    case vm3D_MODE:
    case vm3D_OPENGL_MODE:
      trf_init( TrfMatrix );
      ViewPosition = DefViewPosition;
      Zoom = DefZoom;
      Rotation = DefRotation;
      Angle.Init( 0.0f, 0.0f );
      return true;
    case vmHORZ_SLIT:
    case vmVERT_SLIT:
      ViewOpt.equ_curves_num = DefaultCurvesNum;
      model->GetMatrix().SetViewOpt( ViewOpt );
      return true;
  }
  return false;
}

static bool HandleArrow( HWND h_wnd, char key )
{
  Point2D<float> vec = GetVkVector( key );

  switch( ViewOpt.mode )
  {
    case vm3D_MODE:
    case vm3D_OPENGL_MODE:
      if( ShiftDown )
      {
        if( (ViewOpt.flags & vfAUTO_ROTATION) == 0 )
        {
          ViewOpt.flags |= vfAUTO_ROTATION;
          Rotation.Init( 0, 0 );
        }
        Rotation += vec * ROTATE_ANGLE;
      }
      else
      {
        vec *= 10.0*ROTATE_ANGLE;
        trf_rotate_hv( TrfMatrix, vec.x, vec.y, TrfMatrix );
        Angle += vec;
      }
      return true;

    case vmVERT_SLIT:
    case vmDIMENSION:
      if( key == VK_UP )
        BrightnessBound += 0.01;
      else if( key == VK_DOWN )
        BrightnessBound -= 0.01;

      Maximize( BrightnessBound, 0.0 );
      Minimize( BrightnessBound, 1.0 );

      BMP2Model( h_wnd, HBmpDC, HBmp, BrightnessBound );
      return true;
  }
  return false;
}

static void HandleKeyDown( HWND hWnd, WPARAM wParam, LPARAM )
{
  char key = (char)wParam;

  switch( wParam )
  {
    case VK_SHIFT:
      ShiftDown = true;
      return;
    case VK_PRIOR:
      HandlePageShift( false );
      break;
    case VK_NEXT:
      HandlePageShift( true );
      break;
    case VK_DOWN:
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
      if( !HandleArrow( hWnd, key ) )
        return;
      break;
    case VK_HOME:
      if( !HandleHome() )
        return;
      break;
    default:
      return;
  }

  FullWindowUpdate( hWnd );
}

static void HandleKeyUp( HWND hWnd, WPARAM wParam, LPARAM )
{
  char key = (char)wParam;

  switch( wParam )
  {
    case VK_SHIFT:
      ShiftDown = false;
      break;
  }
}

static void HandleShift( int sign )
{
  switch( ViewOpt.mode )
  {
    case vmHORZ_SLIT:
    case vmVERT_SLIT:
      model->GetMatrix().ShiftSlitDeep( DEEP_STEP*sign );
      if( ViewOpt.flags & vfCALC_SLIT_DIM )
        model->GetMatrix().InvalidateProperties();
      break;
    case vmDIMENSION:
      model->GetMatrix().ShiftVertSlitDeep( DEEP_STEP*sign );
      if( ViewOpt.flags & vfCALC_SLIT_DIM )
        model->GetMatrix().InvalidateProperties();
      break;
    case vm3D_MODE:
    case vm3D_OPENGL_MODE:
      Zoom *= 1 + 0.1*sign;
      break;
    case vmRESULTS:
      CurResult = (CurResult + sign + ResultGroupCount) % ResultGroupCount;
      break;
  }
}

static void UpdateStepTimeStat()
{
  DWORD tick_count = GetTickCount();
  DWORD step_time = tick_count - PrevTickCount;

  TotalTime += step_time;
  PrevTickCount = tick_count;

  int cnt = LastStepTimes.Size();
  if( cnt < LastStepTimeCount )
    LastStepTimes.Append( 1, &step_time );
  else
  {
    for( int i = 0; i < cnt-1; ++i )
      LastStepTimes[i] = LastStepTimes[i+1];
    LastStepTimes[cnt-1] = step_time;
  }
}

static void Step()
{
  UpdateStepTimeStat();
  model->Step();
  PostStep();
}

static void HandleChar( HWND hWnd, WPARAM wParam, LPARAM )
{
  switch( wParam )
  {
    //case 'q':
    //   model = NewSiliconModel( GetCharNum( (char)wParam ) );
    //   OnModelChange( hWnd );
    //   break;

    case 'p':
      Stop();
      break;
    case 'n':
      Step();
      Stop();
      break;

    case '=':
    case '+':
      HandleShift( 1 );
      break;
    case '_':
    case '-':
      HandleShift( -1 );
      break;

    case 'f':
      ViewOpt.field_mode = ElFieldMode( (ViewOpt.field_mode+1)%efmTOTAL );
      break;

    default:
      for( int i = 0; Keys[i][0]; ++i )
        if( Keys[i][0] == wParam )
        {
          HandleMessage( hWnd, Keys[i][1] );
          break;
        }
      return;
  }

  if( ViewOpt.mode >= 0 )
    model->GetMatrix().SetViewOpt( ViewOpt );
  FullWindowUpdate( hWnd );
}

static void PaintDimension( HDC hDC, POINT sz, HWND hWnd )
{
  PorousModel& matrix = model->GetMatrix();
  double min = 0.0, max = 3.0, min_value = NULL_DOUBLE;
  Dimension* dim  = NULL;
  const Array<double>* dim_arr = 0;

  switch( ViewOpt.flags & vfALL_DIM_ONLY_FLAGS )
  {
    case 0:
    {
      const int dim_num = 3;
      const Array<double>* dim_arrays[dim_num] = { &fractalDimArr, &corrDimArr, &massDimArr };
      const char* dim_names[dim_num] =
      {
        matrix.GetFractalDim().GetName(),
        matrix.GetCorrelationDim().GetName(),
        matrix.GetMassDim().GetName()
      };

      max = Max( maxFractalDim, Max( maxCorrDim, maxMassDim ) ) + 0.2;
      PaintGraph( hDC, sz, dim_num, dim_arrays, dim_names, &min, &max, &min_value, "Step", "Dimension" );
      return;
    }
    case vfFRACTAL_DIM:
      dim = &matrix.GetFractalDim();
      dim_arr = &fractalDimArr;
      break;
    case vfCORRELATION_DIM:
      dim = &matrix.GetCorrelationDim();
      dim_arr = &corrDimArr;
      break;
    case vfMASS_DIM:
      dim = &matrix.GetMassDim();
      dim_arr = &massDimArr;
      break;
    default:
      return;
  }

  if( ViewOpt.flags & vfSHOW_DIM_PLOT )
  {
    const char* graph_name = dim->GetName();
    PaintGraph( hDC, sz, 1, &dim_arr, &graph_name, &min, &max, &min_value, "Step", "Dimension" );
  }
  else
  {
    dim->Recalc( matrix, FullWindowUpdate, hWnd );
    PaintDimensionLine( matrix, *dim, hWnd, hDC, sz );
  }
}

static void PaintSingleProperty( HDC hDC, POINT sz, uint prop_type )
{
  PorousModel& matrix = model->GetMatrix();
  double     min = 0.0, max = 3.0, min_value = NULL_DOUBLE;
  Property*  prop = NULL;
  const Array<double>* arr = 0;

  switch( prop_type )
  {
    case vfAVERAGE_DEPTH:
      prop = &matrix.GetAverageDepth();
      arr = &averageDepthArr;
      max = 1.1*maxAverageDepth;
      break;
    case vfPORE_CONCENTRATION:
      prop = &matrix.GetPoreConcentration();
      arr = &poreConcentrationArr;
      max = 1.1*maxPoreConcentration;
      break;
    case vfPORE_AREA:
      prop = &matrix.GetPoreArea();
      arr = &poreAreaArr;
      max = 1.1*maxPoreArea;
      break;
    case vfPORE_VOLUME:
      prop = &matrix.GetPoreVolume();
      arr = &poreVolumeArr;
      max = 1.1*maxPoreVolume;
      break;
    case vfPORE_POROSITY:
      prop = &matrix.GetPorePorosity();
      arr = &porePorosityArr;
      max = 1.1*maxPorePorosity;
      break;
  }

  const char* graph_name = prop->GetName();
  const char* y_label = prop->GetUnits();

  PaintGraph( hDC, sz, 1, &arr, &graph_name, &min, &max, &min_value, "Step", y_label );
}

static void PaintProperty( HDC h_dc, POINT size )
{
  uint prop_type = ViewOpt.flags & vfALL_PROP_ONLY_FLAGS;

  if( prop_type != 0 )
    PaintSingleProperty( h_dc, size, prop_type );
  else
  {
     POINT sz = { size.x/2, size.y/2 };

     DCObject<HBITMAP> hDC( h_dc );
     hDC.SetObject( CreateCompatibleBitmap( h_dc, sz.x, sz.y ) );

     PaintSingleProperty( hDC, sz, vfAVERAGE_DEPTH );
     BitBlt( h_dc, 0, 0, sz.x, sz.y, hDC, 0, 0, SRCCOPY );

     PaintSingleProperty( hDC, sz, vfPORE_CONCENTRATION );
     BitBlt( h_dc, 0, sz.y, sz.x, sz.y, hDC, 0, 0, SRCCOPY );

     PaintSingleProperty( hDC, sz, vfPORE_AREA );
     BitBlt( h_dc, sz.x, 0, sz.x, sz.y, hDC, 0, 0, SRCCOPY );

     PaintSingleProperty( hDC, sz, vfPORE_VOLUME );
     BitBlt( h_dc, sz.x, sz.y, sz.x, sz.y, hDC, 0, 0, SRCCOPY );
  }
}

static void PaintWindow( HWND hWnd )
{
  PorousModel& matrix = model->GetMatrix();

  PAINTSTRUCT paintStruct;
  HDC h_dc = BeginPaint( hWnd, &paintStruct );

  RECT r;
  GetClientRect( hWnd, &r );
  POINT sz = { r.right - r.left, r.bottom - r.top };

  static POINT cur_size = { 0, 0 };
  static DCObject<HBITMAP> hDC( h_dc );

  // Create bitmap in given context
  if( cur_size.x != sz.x || cur_size.y != sz.y || !hDC.Object() )   
  {
    hDC.SetObject( CreateCompatibleBitmap( h_dc, sz.x, sz.y ) );
    cur_size.x = sz.x;
    cur_size.y = sz.y;
  }

  switch( ViewOpt.mode )
  {
    case vmSURFACE:
    {
      bool show_oxid = (ViewOpt.flags & vfSHOW_OXID) != 0;
      PaintField( model->GetSurface(), show_oxid ? &model->GetOxid() : NULL, hDC, sz ); 
      break;
    }
    case vm3D_MODE:
      Paint3D( matrix, hDC, sz, TrfMatrix, Zoom, ViewPosition );
      break;
    case vm3D_OPENGL_MODE:
      OpenGLDraw( hWnd, matrix, Zoom, Angle.x, Angle.y );
      break;
    case vmDISTRIBUTION:
      PaintDistributions( matrix, hDC, sz );
      break;
    case vmVERT_SLIT:
    case vmHORZ_SLIT:
      PaintSlit( matrix, hDC, sz );
      break;
    case vmVAH:
      PaintVAH( hDC, sz, params.tension );
      break;
    case vmRESULTS:
      PaintResult( hDC, sz, CurResult, ViewOpt.flags & vfALL_PROP_ONLY_FLAGS );
      break;
    case vmDIMENSION:
      PaintDimension( hDC, sz, hWnd );
      break;
    case vmPROPERTY:
      PaintProperty( hDC, sz );
      break;
  }

  // Copy into display
  if( ViewOpt.mode != vm3D_OPENGL_MODE )
    BitBlt( h_dc, 0, 0, sz.x, sz.y, hDC, 0, 0, SRCCOPY );

  EndPaint( hWnd, &paintStruct );
}

void RotateModel( int x_from, int y_from, int x_to, int y_to )
{
  int dx = x_to - x_from;
  int dy = y_to - y_from;

  trf_rotate_hv( TrfMatrix, -ROTATE_ANGLE*dx, ROTATE_ANGLE*dy, TrfMatrix );
}

static bool HandleMouseMove( HWND hWnd, WPARAM wParam, LPARAM lParam )
{
  static bool initialized = false;
  static int xx = -1, yy = -1;

  bool handled = false;
  int x = GET_X_LPARAM( lParam ); 
  int y = GET_Y_LPARAM( lParam ); 

  if( (wParam & MK_LBUTTON) && initialized )
  {
    RotateModel( xx, yy, x, y );
    FullWindowUpdate( hWnd );
    handled = true;
  }

  xx = x; 
  yy = y; 
  initialized = true;

  return handled;
}

static void HandleTimer( HWND hWnd )
{
  bool redraw = false;

  if( !(ViewOpt.flags & vfFREEZE) )
  {
    Step();
    redraw = true;
  }

  if( ViewOpt.flags & vfAUTO_ROTATION && (ViewOpt.mode == vm3D_MODE || ViewOpt.mode == vm3D_OPENGL_MODE) )
  {
    trf_rotate_hv( TrfMatrix, Rotation.x, Rotation.y, TrfMatrix );
    Angle += Rotation;
    redraw = true;
  }

  if( redraw )
    FullWindowUpdate( hWnd );
}

LRESULT CALLBACK WindowProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  switch( msg )
  {
    case WM_CREATE:
      SetCursor( ::LoadCursor(NULL,IDC_ARROW) );
      ShowWindow( hWnd, SW_SHOWMAXIMIZED );
      WinInitBlack( hWnd );
      InitOpenGL( hWnd );

      Init( hWnd );
      SetParams( hWnd );

      FullWindowUpdate( hWnd );
      SetTimer( hWnd, 1, 0, 0 );
      break;
    case WM_PAINT:
      PaintWindow( hWnd );
      return 0;
    case WM_SIZE:
      if( model )
        InitOpenGL_model( hWnd, model->GetMatrix() );
      break;
    case WM_COMMAND:
      HandleCommand( hWnd, wParam, lParam );
      break;
    case WM_CHAR:
      HandleChar( hWnd, wParam, lParam );
      break;
    case WM_KEYDOWN:
      HandleKeyDown( hWnd, wParam, lParam );
      break;
    case WM_KEYUP:
      HandleKeyUp( hWnd, wParam, lParam );
      break;
    case WM_TIMER:
    case WM_ENTERIDLE:
      HandleTimer( hWnd );
      break;
      /*case WM_LBUTTONUP:
        Freeze = !Freeze;
        FullWindowUpdate( hWnd );
        return 0;*/
    case WM_MOUSEWHEEL:
      model->GetMatrix().ShiftSlitDeep( DEEP_STEP*Sign((short)HIWORD(wParam)) );   
      FullWindowUpdate( hWnd );
      return 0;
    case WM_MOUSEMOVE:
      if( HandleMouseMove( hWnd, wParam, lParam ) )
        return 0;
      break;
    case WM_DESTROY:
      ReleaseOpenGL( hWnd );
      PostQuitMessage(0);
      return 0;
  }
  return DefWindowProc(hWnd,msg,wParam,lParam);
}

LRESULT CALLBACK Params1DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
  int id_size[] = { IDC_X, IDC_Y, IDC_Z }, id_life_times[] = { IDC_TIME1, IDC_TIME2, IDC_TIME3 };
  int id_prob[] = { IDC_MIN_PROB, IDC_MAX_PROB };
  switch( msg )
  {
    case WM_INITDIALOG:
      PointToDlg( hDlg, params.size, 3, id_size );
      PointToDlg( hDlg, params.lifeTimes, 3, id_life_times );
      SetDlgNumber( hDlg, IDC_OXID_PROB, params.oxidProb );
      PointToDlg( hDlg, params.probRange, 2, id_prob );
      SetDlgNumber( hDlg, IDC_SIDEWAY, params.sidewayKoeff );
      SetDlgNumber( hDlg, IDC_DEG, params.probFuncDeg );
      return TRUE;
    case WM_COMMAND:
      switch( LOWORD(wParam) )
      {
        case IDOK:
          PointFromDlg( hDlg, params.size, 3, id_size );
          PointFromDlg( hDlg, params.lifeTimes, 3, id_life_times );
          params.oxidProb = GetDlgNumber( hDlg, IDC_OXID_PROB );
          PointFromDlg( hDlg, params.probRange, 2, id_prob );
          params.sidewayKoeff = GetDlgNumber( hDlg, IDC_SIDEWAY );
          params.probFuncDeg = GetDlgNumber( hDlg, IDC_DEG );
          // Без break'a!
        case IDCANCEL:
          EndDialog( hDlg, LOWORD(wParam) );
          return TRUE;
      }
      break;
  }
  return FALSE;
}

LRESULT CALLBACK Params2DlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
  int id_size[] = { IDC_X, IDC_Y, IDC_Z }, id_life_times[] = { IDC_TIME1, IDC_TIME2, IDC_TIME3 };
  switch( msg )
  {
    case WM_INITDIALOG:
      {
        PointToDlg( hDlg, params.size, 3, id_size );
        PointToDlg( hDlg, params.lifeTimes, 3, id_life_times );
        SetDlgNumber( hDlg, IDC_OXID_PROB, params.oxidProb );
        SetDlgNumber( hDlg, IDC_TENSION,     params.tension );
        SetDlgNumber( hDlg, IDC_TEMPERATURE, params.temperature );
        SetDlgNumber( hDlg, IDC_LUMINOSITY,  params.luminosity );
        CheckDlgButton( hDlg, chx1, (params.holeModelOpt&phmoUSE_TWIN_HOLES) != 0 );
        CheckDlgButton( hDlg, chx2, (params.holeModelOpt&phmoUSE_FTOR) != 0 );
        //HWND hwnd_chx1 = GetDlgItem( hDlg, chx1 );
        return TRUE;
      }
    case WM_COMMAND:
      switch( LOWORD(wParam) )
      {
        case IDOK:
          PointFromDlg( hDlg, params.size, 3, id_size );
          PointFromDlg( hDlg, params.lifeTimes, 3, id_life_times );
          params.oxidProb    = GetDlgNumber( hDlg, IDC_OXID_PROB );
          params.tension     = GetDlgNumber( hDlg, IDC_TENSION );
          params.temperature = GetDlgNumber( hDlg, IDC_TEMPERATURE );
          Maximize( params.temperature, ABSOLUTE_ZERO );
          params.luminosity  = GetDlgNumber( hDlg, IDC_LUMINOSITY );
          IsDlgButtonChecked( hDlg, chx1 ) ? (params.holeModelOpt |= phmoUSE_TWIN_HOLES) : (params.holeModelOpt &= ~phmoUSE_TWIN_HOLES);
          IsDlgButtonChecked( hDlg, chx2 ) ? params.holeModelOpt |= phmoUSE_FTOR : params.holeModelOpt &= ~phmoUSE_FTOR;
          // Без break'a!
        case IDCANCEL:
          EndDialog( hDlg, LOWORD(wParam) );
          return TRUE;
      }
      break;
  }
  return FALSE;
}

LRESULT CALLBACK AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
  switch( msg )
  {
    case WM_COMMAND:
      switch( LOWORD(wParam) )
      {
        case IDOK:
        case IDCANCEL:
        case IDCLOSE:
          EndDialog( hDlg, LOWORD(wParam) );
          return TRUE;
      }
      break;
  }
  return FALSE;
}
