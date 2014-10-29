// Вольт-амперные характеристики

#include <SiliconDisplay.h>

const double CurrentKoeff = 10; // mA/cm^2 -> A/m^2

struct Vah
{
   double tens; // tension
   double curr; // current
};

// Dimensions: V, mA/sm^2
Vah VAH[] =
{
   { -4.386792, 0.217617 },
   { -3.891509, 0.362694 },
   { -3.396226, 0.507772 },
   { -2.971698, 0.580311 },
   { -2.617925, 0.797927 },
   { -2.122642, 0.943005 },
   { -1.485849, 1.088083 },
   { -0.849057, 1.450777 },
   { -0.283019, 1.740933 },
   { 0.212264, 2.103627 },
   { 0.636792, 2.538860 },
   { 0.990566, 2.974093 },
   { 1.273585, 3.554404 },
   { 1.556604, 4.207254 },
   { 1.698113, 4.787565 },
   { 1.910377, 5.367876 },
   { 1.981132, 6.020725 },
   { 2.122642, 6.673575 },
   { 2.405660, 28.000000 },
   { 2.688679, 28.435233 },
   { 2.971698, 28.797927 },
   { 3.254717, 29.233161 },
   { 3.820755, 29.523316 },
   { 4.316038, 29.740933 },
   { 4.811321, 29.958549 },
   { 5.377358, 30.103627 },
   { 6.014151, 30.176166 },
   { 7.216981, 30.248705 },
   { 8.419811, 30.393782 },
   { 9.905660, 30.466321 },
   { 11.320755, 30.611399 },
   { 12.806604, 30.683938 },
   { 14.575472, 30.829016 },
   { 16.202830, 30.901554 },
   { 17.759434, 31.046632 },
   { 20.235849, 31.191710 },
   { 22.500000, 31.336788 },
   { 24.551887, 31.626943 },
   { 27.099057, 31.844560 },
   { 29.009434, 31.917098 },
   { 30.778302, 32.134715 },
   { 32.476415, 32.279793 },
   { 34.245283, 32.352332 },
   { 36.084906, 32.497409 },
   { 38.066038, 32.642487 },
   { 40.047170, 32.787565 },
   { 42.240566, 33.005181 },
   { 44.433962, 33.150259 },
   { 47.051887, 33.222798 },
   { 48.820755, 33.440415 },
   { 50.731132, 33.658031 },
   { 52.500000, 33.803109 },
   { 54.764151, 34.020725 },
   { 56.745283, 34.383420 },
   { 58.797170, 34.746114 },
   { 60.849057, 35.036269 },
   { 63.042453, 35.326425 },
   { 64.952830, 35.616580 },
   { 66.792453, 36.124352 },
   { 68.773585, 36.704663 },
   { 70.613208, 37.357513 },
   { 71.816038, 37.937824 },
   { 72.877358, 38.518135 },
   { 73.938679, 39.243523 },
   { 75.141509, 40.113990 },
   { 75.990566, 40.839378 },
   { 76.768868, 41.709845 },
   { 77.617925, 42.580311 },
   { 78.466981, 43.668394 },
   { 79.316038, 44.756477 },
   { 79.811321, 45.481865 },
   { 80.306604, 46.207254 },
   { 80.518868, 46.715026 }
};

const int VAH_NUM = sizeof(VAH) / sizeof(Vah);

double ComputeCurrentStrength( double tension, bool convert_units )
{
   int last = VAH_NUM-1;
   double dev2 = 0.0;

   if( tension > VAH[last].tens )
   {
      double dx[2], df[2], dev[2];

      dx[0] = VAH[last-1].tens - VAH[last-2].tens;
      dx[1] = VAH[last].tens - VAH[last-1].tens;

      df[0] = VAH[last-1].curr - VAH[last-2].curr;
      df[1] = VAH[last].curr - VAH[last-1].curr;

      dev[0] = df[0] / dx[0];
      dev[1] = df[1] / dx[1];

      dev2 = (dev[1] - dev[0]) / dx[0];
   }

   int i = 1;
   while( i < last && tension > VAH[i].tens )
      ++i;

   double d_tens = VAH[i].tens - VAH[i-1].tens;
   double d_curr = VAH[i].curr - VAH[i-1].curr;
   double result = 0;

   if( IsNullNum( d_tens ) )
      result = VAH[i].curr;
   else
   {
      double dev1 = d_curr / d_tens;
      double dt = tension - VAH[i-1].tens;
      result = VAH[i-1].curr + dev1*dt + dev2*dt*dt/2;
   }

   if( convert_units )
      result *= CurrentKoeff;

   return result;
}

#define DISPL_X( xx ) (O.x + int( ((xx) - v_min)*scale.x ))
#define DISPL_Y( yy ) (displ_sz.y - O.y - int( ((yy) - f_min)*scale.y ))
#define SYS_X( xx ) (double(xx-O.x)/scale.x + v_min)
#define SYS_Y( yy ) (double(displ_sz.y-O.y-(yy))/scale.y + f_min)

void PaintVAH( HDC hDC, POINT displ_sz, double tension )
{
   const double v_min = -5.0, v_max = 81.0, v_step = 0.1, v_null = 0.0;
   const double v_len = v_max - v_min;
   double v = 0.0, f = 0.0, f_min = 0.0, f_max = 0.0;
   POINT p;
   String str;

   // Compute min, max, display scales

   f_min = f_max = ComputeCurrentStrength( v_min, false );
   for( v = v_min + v_step; v <= v_max + NULL_DOUBLE; v += v_step )
   {
      f = ComputeCurrentStrength( v, false );

      Maximize( f_max, f );
      Minimize( f_min, f );
   }
   if( IsNullNum( f_max - f_min ) )
   {
      f_max += 1.0;
      f_min -= 1.0;
   }
   double f_len = f_max - f_min;
   f_max += f_len/20;
   f_min -= f_len/20;
   f_len = f_max - f_min;

   POINT O = { 10, 20 };
   DPoint2D scale( double(displ_sz.x-O.x)/v_len, double(displ_sz.y-O.y)/f_len );

   // Clear display

   BitBlt( hDC, 0, 0, displ_sz.x, displ_sz.y, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );

   // Draw grid, axis, etc

   PaintGrid( hDC, displ_sz, O, v_min, v_max, f_min, f_max, "V", "J [mA/cm^2]", 10, 10 );

   // Some common variables

   SetBkColor( hDC, ModelTheme.bgColor );
   SetTextColor( hDC, ModelTheme.textColor );

   DCObject<HPEN> h_pen( hDC, false );
   DCObject<HBRUSH> h_brush( hDC, false );

   // Draw function

   h_pen.SetObject( CreatePen( PS_SOLID, 2, ModelTheme.lineColor[0] ) );

   f = ComputeCurrentStrength( v_min, false );
   MoveToEx( hDC, DISPL_X(v_min), DISPL_Y(f), 0 );
   for( v = v_min + v_step; v <= v_max + NULL_DOUBLE; v += v_step )
   {
      f = ComputeCurrentStrength( v, false );
      LineTo( hDC, DISPL_X(v), DISPL_Y(f) );
   }

   // Draw base points

   int dot_r = 2;

   h_pen.SetObject( CreatePen( PS_SOLID, 1, ModelTheme.dotColor ) );
   h_brush.SetObject( CreateSolidBrush( ModelTheme.dotColor ) );

   for( int i = 0; i < VAH_NUM; ++i )
   {
      v = VAH[i].tens;
      p.x = DISPL_X( v );
      p.y = DISPL_Y( ComputeCurrentStrength(v, false) );

      Ellipse( hDC, p.x-dot_r, p.y-dot_r, p.x+dot_r, p.y+dot_r );
   }

   // Draw current point

   dot_r = 3;

   COLORREF red = RGB( 0xFF, 0, 0 );
   h_pen.SetObject( CreatePen( PS_SOLID, 1, red ) );
   h_brush.SetObject( CreateSolidBrush( red ) );

   f = ComputeCurrentStrength( tension, false );
   p.x = DISPL_X( tension );
   p.y = DISPL_Y( f );

   Ellipse( hDC, p.x-dot_r, p.y-dot_r, p.x+dot_r, p.y+dot_r );

   (str = "(") << tension << ", " << f << ")";
   TextOut( hDC, p.x+dot_r, p.y+dot_r, str, str.Length() );
}

#undef DISPL_X
#undef DISPL_Y
#undef SYS_X
#undef SYS_Y
