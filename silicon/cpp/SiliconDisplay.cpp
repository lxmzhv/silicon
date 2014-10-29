// Painting

#include <SiliconDisplay.h>
#include <points.h>
#include <numbers.h>

SiliconModelTheme NightTheme, LightTheme, ModelTheme;

void InitDisplay()
{
  NightTheme.dark = true;
  NightTheme.bgColor        = RGB( 0x00, 0x00, 0x00 );
  NightTheme.poreColor      = RGB( 0x00, 0xFF, 0x00 );
  NightTheme.pore3dColor    = NightTheme.poreColor;
  NightTheme.poreTipColor   = RGB( 0xFF, 0xFF, 0x00 );
  NightTheme.poreCloudColor = RGB( 0x00, 0x30, 0x00 );
  NightTheme.holeColor      = RGB( 0xFF, 0x00, 0x00 );
  NightTheme.hole2Color     = RGB( 0xFF, 0x00, 0xFF );
  NightTheme.ftorColor      = RGB( 0xFF, 0xFF, 0xFF );
  NightTheme.oxidizedColor  = RGB( 0xFF, 0xC0, 0x00 );
  NightTheme.absorbedColor  = RGB( 0x80, 0x80, 0x80 );
  NightTheme.chargedColor   = RGB( 0xFF, 0xFF, 0xFF );
  NightTheme.equCurveColor  = RGB( 0x00, 0x00, 0xFF );
  NightTheme.dotColor       = RGB( 0xFF, 0xFF, 0xFF );
  NightTheme.lineColor[0]   = RGB( 0x00, 0xFF, 0x00 );
  NightTheme.lineColor[1]   = RGB( 0xFF, 0x00, 0x00 );
  NightTheme.lineColor[2]   = RGB( 0x00, 0x00, 0xFF );
  NightTheme.lineColor[3]   = RGB( 0x00, 0xFF, 0xFF );
  NightTheme.sysCoorColor   = RGB( 0xFF, 0xFF, 0xFF );
  NightTheme.dimColor       = RGB( 0x40, 0x40, 0x40 );
  NightTheme.textColor      = RGB( 0xFF, 0xFF, 0x00 );
  NightTheme.hiliteColor    = RGB( 0xA0, 0xA0, 0xA0 );

  LightTheme.dark = false;
  LightTheme.bgColor        = RGB( 0xFF, 0xFF, 0xFF );
  LightTheme.poreColor      = RGB( 0x00, 0x00, 0x00 );
  LightTheme.pore3dColor    = RGB( 0x00, 0xFF, 0x00 );
//LightTheme.poreTipColor   = RGB( 0xB0, 0x00, 0x00 );
  LightTheme.poreTipColor   = RGB( 0xFF, 0xFF, 0x00 );
  LightTheme.poreCloudColor = RGB( 0xC0, 0xC0, 0xC0 );
  LightTheme.holeColor      = RGB( 0xFF, 0x00, 0x00 );
  LightTheme.hole2Color     = RGB( 0x80, 0x00, 0x00 );
  LightTheme.ftorColor      = RGB( 0x00, 0xB0, 0x00 );
  LightTheme.oxidizedColor  = RGB( 0xFF, 0xC0, 0x00 );
  LightTheme.absorbedColor  = RGB( 0x80, 0x80, 0x80 );
  LightTheme.chargedColor   = RGB( 0x00, 0x00, 0x00 );
  LightTheme.equCurveColor  = RGB( 0xA0, 0xA0, 0xFF );
  LightTheme.dotColor       = RGB( 0x00, 0x00, 0x00 );
  LightTheme.lineColor[0]   = RGB( 0x00, 0x80, 0x00 );
  LightTheme.lineColor[1]   = RGB( 0x80, 0x00, 0x00 );
  LightTheme.lineColor[2]   = RGB( 0x00, 0x00, 0x80 );
  LightTheme.lineColor[3]   = RGB( 0x00, 0x80, 0x80 );
  LightTheme.sysCoorColor   = RGB( 0x00, 0x00, 0x00 );
  LightTheme.dimColor       = RGB( 0xB0, 0xB0, 0xB0 );
  LightTheme.textColor      = RGB( 0x00, 0x00, 0x80 );
  LightTheme.hiliteColor    = RGB( 0x40, 0x40, 0x40 );

  ModelTheme = NightTheme;
}

void PaintGrid( HDC hDC, POINT size, POINT O,
                double min_x, double max_x,
                double min_y, double max_y,
                const char* x_label,
                const char* y_label,
                int x_num, int y_num )
{
  // Compute "good" step and starting x and y

  double x, y;
  double dx = CalcGridStep( min_x, max_x, x_num, &x );
  double dy = CalcGridStep( min_y, max_y, y_num, &y );

  // Set text color

  SetBkColor( hDC, ModelTheme.bgColor );
  SetTextColor( hDC, ModelTheme.textColor );

  // Draw grid

  double mul_x = double(size.x - O.x) / (max_x - min_x);
  double mul_y = double(size.y - O.y) / (max_y - min_y);
  int    ix, iy, len;
  char   buf[100];

  DCObject<HPEN> h_pen( hDC, false );
  h_pen.SetObject( CreatePen( PS_DOT, 1, ModelTheme.dimColor ) );

  for( ; x <= max_x; x += dx )
  {
    ix = O.x + int((x-min_x)*mul_x);
    MoveToEx( hDC, ix, size.y, 0 );
    LineTo( hDC, ix, 0 );
    sprintf( buf, "%g", fabs(x) > NULL_DOUBLE ? x : 0.0 );
    len = (int)strlen(buf);
    TextOut( hDC, ix - CharWidth*len/2, size.y - O.y, buf, len );
  }

  for( ; y <= max_y; y += dy )
  {
    iy = size.y - O.y - int((y-min_y)*mul_y);
    MoveToEx( hDC, 0, iy, 0 );
    LineTo( hDC, size.x, iy );
    sprintf( buf, "%g", fabs(y) > NULL_DOUBLE ? y : 0.0 );
    TextOut( hDC, O.x+2, iy-CharHeight, buf, (int)strlen(buf) );
  }

  // Draw datum lines

  h_pen.SetObject( CreatePen( PS_SOLID, 1, ModelTheme.sysCoorColor ) );

  MoveToEx( hDC, O.x, size.y, 0 );
  LineTo( hDC, O.x, 0 );

  MoveToEx( hDC, 0, size.y-O.y, 0 );
  LineTo( hDC, size.x, size.y-O.y );

  // Draw datum line labels

  len = (int)strlen(x_label);
  TextOut( hDC, size.x-(len+1)*CharWidth, size.y-O.y-CharHeight+2, x_label, len );
  TextOut( hDC, O.x+2, 2, y_label, (int)strlen(y_label) );
}

#define DISPL_X( xx ) (O.x + int( ((xx) - min_p.x)*koeff.x ))
#define DISPL_Y( yy ) (size.y - O.y - int( ((yy) - min_p.y)*koeff.y ))
#define SYS_X( xx ) (double((xx) - O.x)/koeff.x + min_p.x)
#define SYS_Y( yy ) (double(size.y-O.y-(yy))/koeff.y + min_p.y)

void PaintDimensionLine( const PorousModel& model, const Dimension& dim, HWND hWnd, HDC hDC, POINT size )
{
  const DPoint2D* points = dim.GetPoints();
  const Line<double>& line = dim.GetLine();
  char buf[0x100];

  BitBlt( hDC, 0, 0, size.x, size.y, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );

  // Определяем параметры отображения
  DPoint2D max_p, min_p;
  for( int i = 0; i < dim.GetPointsNum(); i++ )
  {
    Maximize( max_p.x, points[i].x );
    Maximize( max_p.y, points[i].y );

    Minimize( min_p.x, points[i].x );
    Minimize( min_p.y, points[i].y );
  }

  if( IsNullNum( max_p.x-min_p.x ) )   max_p.x += 1.0, min_p.x -= 1.0;
  if( IsNullNum( max_p.y-min_p.y ) )   max_p.y += 1.0, min_p.y -= 1.0;

  POINT O = { 10, 20 };
  DPoint2D shift( (max_p - min_p) * 0.05 );
  max_p += shift;
  min_p -= shift;
  DPoint2D dim_size( max_p - min_p );
  DPoint2D koeff( (size.x-O.x)/dim_size.x, (size.y-O.y)/dim_size.y );

  // Draw grid, axis etc
  PaintGrid( hDC, size, O, min_p.x, max_p.x, min_p.y, max_p.y, dim.GetAxis1(), dim.GetAxis2(), 10, 10 );

  // Draw dimension line
  DCObject<HPEN> h_pen( hDC, false );
  h_pen.SetObject( CreatePen( PS_SOLID, 2, ModelTheme.lineColor[0] ) );

  POINT beg = {0,0}, end = {0,0};
  if( !IsNullNum(line.b) )
  {
    beg.x = 0;
    beg.y = DISPL_Y( line.GetY( SYS_X(beg.x) ) );

    end.x = size.x;
    end.y = DISPL_Y( line.GetY( SYS_X(end.x) ) );
  }
  else if( !IsNullNum(line.a) )
  {
    beg.y = size.y;
    beg.x = DISPL_X( line.GetX( SYS_Y(beg.y) ) );

    end.y = 0;
    end.x = DISPL_X( line.GetX( SYS_Y(end.y) ) );
  }
  if( memcmp( &beg, &end, sizeof(beg) ) )
  {
    MoveToEx( hDC, beg.x, beg.y, 0 );
    LineTo( hDC, end.x, end.y );
  }

  // Draw points
  DCObject<HBRUSH> h_brush( hDC, false );
  h_pen.SetObject( CreatePen( PS_SOLID, 1, ModelTheme.dotColor ) );
  h_brush.SetObject( CreateSolidBrush( ModelTheme.dotColor ) );

  SetBkColor( hDC, ModelTheme.bgColor );
  SetTextColor( hDC, ModelTheme.textColor );

  POINT p;
  static String str;
  for( int i = 0; i < dim.GetPointsNum(); i++ )
  {
    p.x = DISPL_X( points[i].x );
    p.y = DISPL_Y( points[i].y );
    Ellipse( hDC, p.x-DotRadius, p.y-DotRadius, p.x+DotRadius, p.y+DotRadius );

    sprintf( buf, "(%.2f, %.2f)", points[i].x, points[i].y );
    TextOut( hDC, p.x+DotRadius, p.y+DotRadius, buf, (int)strlen(buf) );
  }

  // Draw dimension
  (str = dim.GetName()) << ": " << dim.GetDimension();
  TextOut( hDC, size.x/2, size.y*9/10, str, str.Length() );
}

#undef DISPL_X
#undef DISPL_Y
#undef SYS_X
#undef SYS_Y

void PaintGraph( HDC hDC, POINT size, int arr_num, const Array<double>* arrays[], const char* names[],
                 double *p_min, double *p_max, double *p_min_value,
                 const char* x_label, const char* y_label )
{
  BitBlt( hDC, 0, 0, size.x, size.y, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );

  if( arr_num < 1 )
    return;

  int max_n = arrays[0]->Size()-1;
  for( int i = 1; i < arr_num; ++i )
    Minimize( max_n, arrays[i]->Size()-1 );
  if( max_n < 0 )
    return;

  int start_n = 0;
  if( p_min_value != NULL )
  {
    // Find the first value which is greater than given min value
    start_n = max_n;
    for( int i = 0; i < arr_num; ++i )
      for( int j = 0; j <= start_n; ++j )
        if( (*arrays[i])[j] >= *p_min_value )
        {
          start_n = j;
          break;
        }
  }

  if( start_n >= max_n )
    return;

  // Compute Min & Max

  double min =  LARGE_DOUBLE;
  double max = -LARGE_DOUBLE;

  if( p_min != NULL )
    min = *p_min;
  else
  {
    for( int i = 0; i < arr_num; ++i )
      Minimize( min, GetArrayMin( *arrays[i] ) );
  }

  if( p_max != NULL )
    max = *p_max;
  else
  {
    for( int i = 0; i < arr_num; ++i )
      Maximize( max, GetArrayMax( *arrays[i] ) );
  }

  double height = max - min;
  if( height <= NULL_DOUBLE )
    max = min + 1.0;
  else
    max += 0.05*height;

  // Paint grid

  POINT O = { 10, 20 };
  int x_num_labels = 10;
  int y_num_labels = Max( 2, Min( 10, (int)size.y/(2*CharHeight) ) );
  PaintGrid( hDC, size, O, start_n, Max(max_n,1),
             min, max,
             x_label, y_label,
             x_num_labels, y_num_labels );

  // Plot dimensions

  double dx = double(size.x-O.x) / (max_n - start_n);
  double dy = double(size.y-O.y) / (max - min);
  DCObject<HPEN> h_pen( hDC, false );
  DCObject<HBRUSH> h_brush( hDC, false );
  char buf[100];
  double value;
  int i, j, iy;
  VP<int> text_y = new int[arr_num];

  SetBkColor( hDC, ModelTheme.bgColor );
  SetTextColor( hDC, ModelTheme.hiliteColor );

  for( i = 0; i < arr_num; ++i )
  {
    const Array<double>& arr = *arrays[i];

    // Draw current value
    h_pen.SetObject( CreatePen( PS_DASH, 1, ModelTheme.hiliteColor) );

    value = arr[max_n];
    iy = size.y - O.y - int(dy*(value-min));
    MoveToEx( hDC, O.x, iy, 0 );
    LineTo( hDC, O.x + int((max_n-start_n)*dx), iy );

    // Print text for current value
    COLORREF color = ModelTheme.lineColor[i%LineColorNum];
    SetTextColor( hDC, color );

    // Prevent overlap of plot names
    iy -= CharHeight;
    for( j = 0; j < i; )
      if( iy <= text_y[j]+CharHeight && iy > text_y[j]-CharHeight )
      {
        iy -= CharHeight;
        j = 0;
      }
      else
        ++j;

    sprintf( buf, "%s: %g", names[i], fabs(value) > NULL_DOUBLE ? value : 0.0 );
    TextOut( hDC, O.x+10*CharWidth, iy, buf, (int)strlen(buf) );
    text_y[i] = iy;

    // Draw graph
    h_pen.SetObject( CreatePen( PS_SOLID, 1, color ) );
    h_brush.SetObject( CreateSolidBrush(color) );

    const int r = 2;
    int x = O.x, y = size.y - O.y - int(dy*(arr[start_n]-min));
    MoveToEx( hDC, x, y, 0 );
    Ellipse( hDC, x-r, y-r, x+r, y+r );

    for( j = start_n+1; j <= max_n; ++j )
      LineTo( hDC, O.x + int((j-start_n)*dx), size.y - O.y - int(dy*(arr[j]-min)) );
  }
}

// Отобразить разрез на глубине Deep
void PaintSlit( PorousModel& model, HDC hDC, POINT size )
{
  const Point<ulong>& sz = model.GetSize();

  model.SetWindow( hDC );
  if( model.ViewOpt().mode == vmHORZ_SLIT )
    StretchBlt( hDC, 0, 0, size.x, size.y, model.GetDC(), 0, 0, sz.y, sz.x, SRCCOPY );
  else
    StretchBlt( hDC, 0, 0, size.x, size.y, model.GetDC(), 0, 0, sz.y, sz.z, SRCCOPY );
}

inline COLORREF SurfaceColor( SiliconSrfState state, bool oxidized )
{
  COLORREF color = ModelTheme.bgColor;

  switch( state )
  {
    case sssNOT_CHARGED_MOLECULE: color = ModelTheme.absorbedColor; break;
    case sssCHARGED_MOLECULE:     color = ModelTheme.chargedColor;  break;
  }

  if( oxidized )
  {
    COLORREF oxid = ModelTheme.oxidizedColor;
    int red   = (GetRValue(oxid) + 2*GetRValue(color))/3;
    int green = (GetGValue(oxid) + 2*GetGValue(color))/3;
    int blue  = (GetBValue(oxid) + 2*GetBValue(color))/3;
    color = RGB( red, green, blue );
  }

  return color;
}

void PaintField( const SiliconSrfModel& srf_model, const OxidModel* oxid_model, HDC h_dc, POINT size )
{
  POINT sz = srf_model.GetSize();

  // Создаём контекст и битмапку
  static DCObject<HBITMAP> hDC( h_dc );
  static POINT cur_size;
  if( cur_size.x != sz.x || cur_size.y != sz.y || !hDC.Object() )   
  {
    hDC.SetObject( CreateCompatibleBitmap( h_dc, sz.x, sz.y ) );
    cur_size = sz;
  }

  // Отображаем состояние системы
  for( long i = 0; i < long(sz.x); i++ )
    for( long j = 0; j < long(sz.y); j++ )
      SetPixel( hDC, i, j,
                SurfaceColor((SiliconSrfState)srf_model.GetState(i,j),
                             oxid_model ? oxid_model->GetState(i,j)==1 : false) );

  // Копируем на экран
  StretchBlt( h_dc, 0, 0, size.x, size.y, hDC, 0, 0, sz.x, sz.y, SRCCOPY );
}

void PaintBall( HDC hDC, POINT size,
    double x, double y,
    double diameter, double bright,
    uchar red, uchar green, uchar blue )
{
  static const int    layers = 10;
  static const double sqrt_3 = sqrt( 3.0 );
  static double       val, r, r1, r2, radius, shift;
  static int          main_color, i;
  static COLORREF     color;
  static double       cur_bright;
  const COLORREF boundary_color = ModelTheme.dark ? RGB(0,0,0) : RGB(0x40,0x40,0x40);
  const double min_bright = ModelTheme.dark ? 0.05 : 0.30;
  const double bright1 = 0.10;
  const double bright2 = 1.0 - (min_bright + bright1);
  const double layer_step_bright = (bright1 + bright*bright2);

  if( x < 0 || y < 0 || x > size.x || y > size.y )
    return;

  DCObject<HBRUSH> h_brush( hDC, false );
  DCObject<HPEN>   h_pen( hDC, false );
  radius = diameter / 2;

  for( i = 0; i < layers; ++i )
  {
    val = double(i)/layers;

    cur_bright = min_bright + layer_step_bright*val;
    Validate( cur_bright, min_bright, 1.0 );
    color = RGB( int(cur_bright*red), int(cur_bright*green), int(cur_bright*blue) );

    h_pen.SetObject( CreatePen( PS_SOLID, 1, i == 0 ? boundary_color : color ) );
    h_brush.SetObject( CreateSolidBrush( color ) );

    r = radius * sqrt_3 * (1 - val);
    shift = radius * val/2;
    r1 = r + shift;
    r2 = r - shift;
    Ellipse( hDC, int(x - r1), int(y - r1), int(x + r2), int(y + r2) );
  }
}

bool cmp_pore( DisplBall b1, DisplBall b2 )
{
  double d = b1.pnt.z - b2.pnt.z;
  if( d != 0.0 )
    return d < 0.0;

  d = b1.pnt.x - b2.pnt.x;
  if( d != 0.0 )
    return d < 0.0;

  return b1.pnt.y < b2.pnt.y;
}

float DisplParams::GetRealStretch( const FPoint& pnt ) const
{
  float dz = view_z - pnt.z;

  if( fabs(dz) < NULL_DOUBLE )
    return LARGE_FLOAT;

  return stretch * (view_z - proj_z) / dz;
}

void DisplParams::GetDisplPoint( const FPoint& pnt, POINT& displ_pnt ) const
{
  float koef = GetRealStretch( pnt );

  displ_pnt.x = int( 0.5 * size.x + koef * pnt.x );
  displ_pnt.y = int( 0.5 * size.y + koef * pnt.y );
}

void TrfModelPoint( const Point<ulong>& sz, Trf_3d trf, const FPoint& pnt, FPoint& pnt_out )
{
  static FPoint center;

  center.Init( 0.5f*sz.x, 0.5f*sz.y, 0.5f*sz.z );

  trf_x_pnt( trf, pnt - center, pnt_out );

  pnt_out.Init( pnt_out.y, pnt_out.z, pnt_out.x );
}

void Paint3D( const PorousModel& model, HDC hDC, POINT size, Trf_3d trf, double zoom, double view_pos )
{
  const static FPoint radius( 0.5, 0.5, 0.5 );
  static Point<long> pnt;
  static FPoint float_p, bounds[2][2][2];
  static Point<ulong> sz;
  static DisplBall pore;
  static std::vector<DisplBall> pores;
  static ulong i, j, k, ii, jj, kk, n;
  static int flags;
  static bool show_shadow, show_bounds;

  flags = model.ViewOpt().flags;
  show_shadow = (flags & vfSHOW_SHADOW) != 0;
  show_bounds = (flags & vfSHOW_BOUNDS) != 0;

  sz = model.GetSize();

  // Collect array of pores
  pores.clear();
  for( pnt.x = 0; pnt.x < long(sz.x); ++pnt.x )
    for( pnt.y = 0; pnt.y < long(sz.y); ++pnt.y )
      for( pnt.z = 0; pnt.z < long(sz.z); ++pnt.z )
        if( model.GetCellColor( pnt, &pore.color ) )
        {
          copy_vector( float_p, pnt );
          float_p += radius;
          TrfModelPoint( sz, trf, float_p, pore.pnt );

          pores.push_back( pore );
        }

  for( i = 0; i <= 1; ++i )
    for( j = 0; j <= 1; ++j )
      for( k = 0; k <= 1; ++k )
      {
        float_p.Init( i*sz.x, j*sz.y, k*sz.z );

        TrfModelPoint( sz, trf, float_p, bounds[i][j][k] );
      }

  // Sort balls by deep
  std::sort( pores.begin(), pores.end(), cmp_pore );

  // Display model

  static DisplBall *p_pore;
  static const int shift = 1;
  static double koef;
  static double diameter, min_z, max_z, bright;
  static POINT displ_pnt, displ_pnt2;
  static DisplParams prms;

  // Calculate stretch and position

  diameter = Length( sz );
  min_z = -diameter/2;
  max_z = diameter/2;

  prms.proj_z = 0;
  prms.view_z = max_z + view_pos*diameter;
  prms.size = size;
  prms.stretch = zoom * Min( size.x, size.y ) / (diameter + 2*shift);

  // Clear context
  BitBlt( hDC, 0, 0, size.x, size.y, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );

  // Display bounds

  if( show_bounds )
  {
    DCObject<HPEN> h_pen( hDC, false );
    COLORREF grid_color = RGB( 0x60, 0x60, 0x60 );
    h_pen.SetObject( CreatePen( PS_SOLID, 1, grid_color ) );

    for( i = 0; i <= 1; ++i )
      for( j = 0; j <= 1; ++j )
        for( k = 0; k <= 1; ++k )
        {
          prms.GetDisplPoint( bounds[i][j][k], displ_pnt );

          //SetPixel( hDC, x, y, grid_color );

          for( ii = 0; ii <= 1; ++ii )
            for( jj = 0; jj <= 1; ++jj )
              for( kk = 0; kk <= 1; ++kk )
              {
                n = 0;
                if( i != ii )  ++n;
                if( j != jj )  ++n;
                if( k != kk )  ++n;
                if( n == 1 )
                {
                  MoveToEx( hDC, displ_pnt.x, displ_pnt.y, 0 );

                  prms.GetDisplPoint( bounds[ii][jj][kk], displ_pnt2 );

                  LineTo( hDC, displ_pnt2.x, displ_pnt2.y );
                }
              }
        }
  }

  // Paint balls
  for( i = 0; i < pores.size(); ++i )
  {
    p_pore = &pores[i];

    prms.GetDisplPoint( p_pore->pnt, displ_pnt );
    koef = prms.GetRealStretch( p_pore->pnt );

    bright = show_shadow ? (p_pore->pnt.z - min_z)/diameter : 1.0;

    PaintBall( hDC, size, displ_pnt.x, displ_pnt.y, koef, bright,
        GetRValue(p_pore->color), GetGValue(p_pore->color), GetBValue(p_pore->color) );
  }
}

void PaintDistributions( PorousModel& model, HDC hDC, POINT sz )
{
  BitBlt( hDC, 0, 0, sz.x, sz.y, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );
  model.PaintDistributions( hDC, sz );
}

void PaintMatrixZDistribution( const BoolMatrix3D& matrix, long* dens, long max_dens,
    HDC hDC, POINT displ_sz, COLORREF color )
{
  const Point<ulong>& sz = matrix.GetSize();

  DCObject<HPEN>   h_pen( hDC, false );
  DCObject<HBRUSH> h_brush( hDC, false );

  h_pen.SetObject( CreatePen( PS_SOLID, 2, color ) );
  h_brush.SetObject( CreateSolidBrush( color ) );

  if( max_dens <= 0 || sz.z == 0 )
  {
    MoveToEx( hDC, 0, 0, 0 );
    LineTo( hDC, 0, displ_sz.y );
    return;
  }

  POINT O = { 10, 20 };
  PaintGrid( hDC, displ_sz, O, 0, sz.z-1, 0, max_dens, "Deep", "Density", 10, 10 );

  DPoint2D scale( double(displ_sz.x-O.x)/(sz.z-1), double(displ_sz.y-O.y)/max_dens );

  MoveToEx( hDC, O.x, displ_sz.y - O.y - int(scale.y*dens[0]), 0 );
  for( ulong z = 1; z < sz.z; ++z )
    LineTo( hDC, O.x + int(scale.x*z), displ_sz.y - O.y - int(scale.y*dens[z]) );
}
