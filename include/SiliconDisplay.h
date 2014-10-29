// Painting

#pragma once

#include <view.h>
#include <valgebra.h>
#include <SiliconModel2.h>
#include <SiliconDim.h>
#include <Array.h>

extern const COLORREF DOT_COLOR;
extern const COLORREF LINE_COLOR;
extern const COLORREF SYS_COOR_COLOR;
extern const int ResultCount;
extern const int ResultGroupCount;
const int CharHeight = 18;
const int CharWidth  = 8;
const int DotRadius  = 2;

struct DisplBall
{
   FPoint pnt;
   COLORREF color;

   void SetColor( uchar r, uchar g, uchar b ) { color = RGB(r,g,b); }
};

struct DisplParams
{
   POINT size;
   float stretch;
   float view_z; // z coordinate of viewer
   float proj_z; // z coordinate of projection plane

    float GetRealStretch( const FPoint& pnt ) const;
     void GetDisplPoint( const FPoint& pnt, POINT& displ_pnt ) const;
};

void PaintGraph( HDC hDC, POINT size, int arr_num, const Array<double>* dim_arr[], const char* dim_names[],
                 double* p_min, double *p_max, double *p_min_value, const char* x_label, const char* y_label );
void PaintGrid( HDC hDC, POINT size, POINT O, double min_x, double max_x, double min_y, double max_y,
                const char* x_label, const char* y_label, int x_num, int y_num );
void PaintDimensionLine( const PorousModel& model, const Dimension& dim, HWND hWnd, HDC hDC, POINT size );
void PaintDistributions( PorousModel& model, HDC hDC, POINT size );
void PaintSlit( PorousModel& model, HDC hDC, POINT size );
void PaintField( const SiliconSrfModel& srf_model, const OxidModel* oxid_model, HDC h_dc, POINT size );
void Paint3D( const PorousModel& model, HDC h_dc, POINT size, Trf_3d trf, double zoom, double view_pos );
void PaintVAH( HDC hDC, POINT displ_sz, double tension );
void PaintResult( HDC hDC, POINT size, int num, uint dim_flag );

void InitDisplay();
void InitOpenGL( HWND hWnd );
void InitOpenGL_model( HWND hWnd, const PorousModel& model );
void ReleaseOpenGL( HWND hWnd );
void OpenGLDraw( HWND hWnd, const PorousModel& model, float zoom, float h_angle, float v_angle );

void FullWindowUpdate( HWND h_wnd );