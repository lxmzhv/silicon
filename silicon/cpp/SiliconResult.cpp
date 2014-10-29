// Modeling result

#include <SiliconDisplay.h>
#include <points.h>
#include <numbers.h>

const double Voltage[] = { 1, 2, 3, 5, 10, 30, 60, 70, 80, 100 };
const int VoltageCount  = sizeof(Voltage)/sizeof(double);
const char* DimNames[3] = { "Fractal", "Correlation", "Mass fractal" };

struct ModelResult
{
SimplePoint<int> size;
  double volume;
     int valid; // number of valid models
     int count; // total number of models
  double dim[VoltageCount][3]; // Fractal, Correlation and Mass fractal dimensions correspondingly
};

ModelResult Results[] =
{
  { {32,32,16}, 3.0, 7, 9, { {1.47,1.85,0.53}, {1.48,1.85,0.49}, {1.44,1.88,0.47}, {1.52,1.87,0.50}, {1.46,1.87,0.53},
                             {1.39,1.83,0.62}, {1.38,1.70,0.73}, {1.38,1.77,0.75}, {1.41,1.79,0.80} } },
  { {32,32,16}, 4.0, 5, 8, { {1.59,1.94,0.62}, {1.56,1.95,0.59}, {1.55,1.97,0.57}, {1.55,1.96,0.60}, {1.58,1.94,0.60},
                             {1.53,1.84,0.71}, {1.53,1.75,0.84}, {1.53,1.83,0.75} } },
  { {32,32,32}, 2.0, 5, 6, { {1.76,1.92,0.56}, {1.81,1.94,0.52}, {1.75,1.97,0.49}, {1.87,1.91,0.47}, {1.68,1.87,0.52}, {1.67,1.90,0.64} } },
  { {32,32,32}, 3.0, 4, 4, { {1.97,1.99,0.66}, {1.87,2.00,0.64}, {1.85,2.05,0.56}, {1.84,2.02,0.58} } },
  { {64,64,64}, 1.0, 5, 5, { {2.00,2.03,0.45}, {1.99,2.02,0.40}, {2.01,2.04,0.41}, {1.93,1.99,0.42}, {1.77,1.89,0.42} } },
  { {64,64,64}, 1.5, 4, 5, { {2.07,2.14,0.56}, {1.97,2.13,0.51}, {1.97,2.11,0.50}, {1.91,2.02,0.49}, {1.89,2.02,0.49} } },
  { {64,64,64}, 2.0, 3, 4, { {2.02,2.19,0.62}, {2.00,2.16,0.56}, {2.00,2.14,0.55}, {1.99,2.09,0.53} } },
  { {128,128,64}, 0.5, 10, 10, { {1.83,1.83,0.24}, {1.82,1.82,0.21}, {1.82,1.82,0.21}, {1.82,1.82,0.22}, {1.80,1.83,0.25},
                                 {1.57,1.82,0.35}, {1.58,1.74,0.41}, {1.62,1.90,0.44}, {1.59,1.74,0.45}, {1.60,1.77,0.57} } },
  { {128,128,64}, 1.0, 5, 6, { {2.03,2.03,0.44}, {2.02,2.02,0.41}, {2.02,2.02,0.40}, {1.96,2.03,0.40}, {1.84,1.96,0.40}, {1.77,2.04,0.48} } },
  { {128,128,64}, 1.5, 4, 5, { {2.13,2.17,0.57}, {2.02,2.16,0.52}, {2.06,2.16,0.51}, {1.97,2.12,0.48}, {1.92,2.05,0.47} } },
  { {128,128,64}, 2.0, 3, 4, { {2.05,2.25,0.63}, {2.03,2.21,0.58}, {2.06,2.24,0.57}, {2.02,2.17,0.53} } },
  { {128,128,64}, 2.5, 3, 3, { {2.09,2.29,0.66}, {2.07,2.22,0.61}, {2.09,2.28,0.60} } },
  { {128,128,128}, 0.25, 7, 7, { {1.89,1.83,0.20}, {1.88,1.82,0.18}, {1.88,1.83,0.17}, {1.87,1.84,0.17}, {1.87,1.83,0.20}, {1.69,1.74,0.27}, {1.67,1.84,0.35} } },
  { {128,128,128}, 0.50, 5, 6, { {2.09,2.06,0.38}, {2.07,2.04,0.34}, {2.07,2.05,0.33}, {2.04,2.05,0.34}, {1.89,1.93,0.32}, {1.83,1.97,0.36} } },
  { {128,128,128}, 0.75, 4, 4, { {2.16,2.18,0.48}, {2.15,2.18,0.45}, {2.00,2.14,0.43}, {1.95,2.02,0.40} } },
  { {128,128,128}, 1.00, 3, 3, { {2.11,2.25,0.54}, {2.07,2.19,0.49}, {2.04,2.13,0.46} } },
  { {128,128,128}, 1.25, 2, 2, { {2.12,2.25,0.57}, {2.09,2.19,0.52} } }
};
const int ResultCount = sizeof(Results)/sizeof(ModelResult);

const int ResultGroupIdx[] = { 0, 2, 4, 7, 12 }; // Indices inss Results array
const int ResultGroupCount = sizeof(ResultGroupIdx)/sizeof(int);

void PaintResult( HDC hDC, POINT size, int num, uint dim_flag )
{
  BitBlt( hDC, 0, 0, size.x, size.y, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );

  if( num < 0 || num >= ResultGroupCount )
    return;

  int idx1 = ResultGroupIdx[num];
  if( idx1 < 0 || idx1 >= ResultCount )
    return;

  int idx2 = (num == ResultGroupCount-1) ? ResultCount-1 : ResultGroupIdx[num+1]-1;
  if( idx2 < 0 || idx2 >= ResultCount )
    return;

  int dim_from = 0, dim_to = 0;
  switch( dim_flag )
  {
    case 0:                      dim_from = 0; dim_to = 2; break;
    case vfFRACTAL_DIM:     dim_from = dim_to = 0; break;
    case vfCORRELATION_DIM: dim_from = dim_to = 1; break;
    case vfMASS_DIM:        dim_from = dim_to = 2; break;
  }

  int count = 0;
  for( int idx = idx1; idx <= idx2; ++idx )
    Maximize( count, Results[idx].count );

  if( count <= 0 )
    return;

  double min_voltage = 1;
  double max_voltage = Voltage[count-1]; // Voltage[VoltageCount-1];
  double add_voltage = max_voltage*0.05;
  min_voltage -= add_voltage;
  max_voltage += add_voltage;
  double voltage_range = max_voltage-min_voltage;

  // Compute Min & Max

  double min =  LARGE_DOUBLE;
  double max = -LARGE_DOUBLE;
  const ModelResult* res = NULL;

  for( int idx = idx1; idx <= idx2; ++idx )
  {
    res = &Results[idx];
    for( int j = 0; j < res->count; ++j )
      for( int i = dim_from; i <= dim_to; ++i )
        {
        Minimize( min, res->dim[j][i] );
        Maximize( max, res->dim[j][i] );
        }
  }

  double height = max - min;
  if( height <= NULL_DOUBLE )
    max = min + 1.0;
  else
    max += 0.05*height;

  // Paint grid

  POINT O = { 10, 20 };
  PaintGrid( hDC, size, O, min_voltage, max_voltage, min, max, "Voltage", "Dimension", 10, 10 );

  // Plot dimensions

  double dx = double(size.x-O.x) / voltage_range;
  double dy = double(size.y-O.y) / (max - min);
  DCObject<HPEN>   h_pen( hDC, false );
  DCObject<HBRUSH> h_brush( hDC, false );
  int i, j, x, y, len, color_idx;
  String info;
  
  h_brush.SetObject( CreateSolidBrush( ModelTheme.dotColor ) );
  SetBkColor( hDC, ModelTheme.bgColor );

  for( int idx = idx1; idx <= idx2; ++idx )
  {
    res = &Results[idx];
    for( i = dim_from; i <= dim_to; ++i )
    {
      // Draw graph
      SetTextColor( hDC, ModelTheme.lineColor[i%LineColorNum] );

      MoveToEx( hDC, O.x + int(dx*(Voltage[0]-min_voltage)), size.y - O.y - int(dy*(res->dim[0][i]-min)), 0 );
      for( j = 0; j < res->count; ++j )
      {
        x = O.x + int(dx*(Voltage[j]-min_voltage));
        y = size.y - O.y - int(dy*(res->dim[j][i]-min));

        color_idx = (j < res->valid ? i : 3);
        int line_type = (j < res->valid ? PS_SOLID : PS_DASH);
        h_pen.SetObject( CreatePen( line_type, 1, ModelTheme.lineColor[color_idx%LineColorNum]) );
        LineTo( hDC, x, y );

        h_pen.SetObject( CreatePen( PS_SOLID, 1, ModelTheme.dotColor ) );
        Ellipse( hDC, x-DotRadius, y-DotRadius, x+DotRadius, y+DotRadius );
      }

      info = DimNames[i];
      info << " (volume: " << res->volume << "%)";

      len = info.Length();
      j = res->count-1;
      x = O.x + int(dx*(Voltage[j]-min_voltage));
      x = Min<int>( x, size.x - CharWidth*len );
      TextOut( hDC, x, size.y - O.y - int(dy*(res->dim[j][i]-min)) - CharHeight, info, len );
    }
  }

  res = &Results[idx1];

  SetTextColor( hDC, ModelTheme.textColor );

  info.Clear();
  info << "Size: " << res->size[0] << "x" << res->size[1] << "x" << res->size[2];
  len = info.Length();
  x = size.x-(len+1)*CharWidth;
  TextOut( hDC, x, size.y/2, info, len );
}
