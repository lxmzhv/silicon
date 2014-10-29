// Matrix of silicon model (box) with porous tree

#include <windows.h>
#include <PorousModel.h>
#include <DigitMethods.h>

//
//class PorousModel
//

PorousModel::PorousModel( Point<ulong> sz, ModelViewOptions view_opt ):
   matrix(sz),
   viewOpt(view_opt),
   vertSlitDeep(0),
   horzSlitDeep(0),
   fractalDim( "Pore fractal dimension", false, CalcFractalDim, FracDimInit, FracDimNextEps, UpdateFractalDim, -1, "-log2(eps)", "log2(N)" ),
   correlationDim( "Correlation dimension", false, CalcCorrelation, CorDimInit, CorDimNextEps, UpdateCorrelationDim, 1, "log2(eps)", "log2(P)" ),
   massDim( "Mass fractal dimension", false, CalcMassDim, MassDimInit, MassDimNextEps, UpdateMassDim, 1, "log2(eps)", "log2(M)" )
{
   Init();
}

// Fill out full volume
static void InitCube( BoolMatrix3D& matrix, ulong& count, long max_deep )
{
   const Point<ulong>& sz = matrix.GetSize();

   matrix.Init(true);
   count = sz.x*sz.y*sz.z;
   max_deep = sz.z-1;
}

void PorousModel::InvalidateProperties()
{
  fractalDim.Invalidate();
  correlationDim.Invalidate();
  massDim.Invalidate();
  averageDepth.Invalidate();
  poreConcentration.Invalidate();
  poreArea.Invalidate();
  poreVolume.Invalidate();
  porePorosity.Invalidate();
}

void PorousModel::RecalcProperties()
{
  fractalDim.Recalc( *this, NULL, NULL );
  correlationDim.Recalc( *this, NULL, NULL );
  massDim.Recalc( *this, NULL, NULL );
  averageDepth.Recalc( *this );
  poreConcentration.Recalc( *this );
  poreArea.Recalc( *this );
  poreVolume.Recalc( *this );
  porePorosity.Recalc( *this );
}

static const Point<long> one_pnt(1,1,1);

static void InitSerpinskyFractalLow( BoolMatrix3D& matrix, ulong& count, const Point<long>& p1, const Point<long>& p2 )
{
   Point<long> v = (p2 - p1 + one_pnt) / 3, p, p11, p22;
   Point<long> beg[3], end[3];
   int i, j, k;

   if( v.x == 0 && v.y == 0 && v.z == 0 )
      return;

   beg[0] = p1;
   beg[1] = p1 + v;
   beg[2] = p2 - v + one_pnt;

   end[0] = p1 + v - one_pnt;
   end[1] = p2 - v;
   end[2] = p2;

   for( i = 0; i < 3; ++i )
     if( v[i] == 0 )
     {
       beg[2][i] = p1[i];
       end[0][i] = p2[i];
     }

   for( p.x = beg[1].x; p.x <= end[1].x; ++p.x )
      for( p.y = beg[1].y; p.y <= end[1].y; ++p.y )
         for( p.z = beg[1].z; p.z <= end[1].z; ++p.z )
            if( matrix.Value( p ) )
            {
               matrix.SetValue( p, false );
               --count;
            }

   for( i = v.x ? 0 : 2; i < 3; ++i )
      for( j = v.y ? 0 : 2; j < 3; ++j )
         for( k = v.z ? 0 : 2; k < 3; ++k )
         {
           if( i == 1 && j == 1 && k == 1 )
             continue;

           p11.x = beg[i].x;
           p11.y = beg[j].y;
           p11.z = beg[k].z;

           p22.x = end[i].x;
           p22.y = end[j].y;
           p22.z = end[k].z;

           InitSerpinskyFractalLow( matrix, count, p11, p22 );
         }
}

static void InitSerpinskyFractal( BoolMatrix3D& matrix, ulong& count, long max_deep )
{
   InitCube( matrix, count, max_deep );
   const Point<ulong>& sz = matrix.GetSize();
   Point<long> p1(0,0,0), p2(sz.x-1, sz.y-1, sz.z-1);

   InitSerpinskyFractalLow( matrix, count, p1, p2 );
}

void PorousModel::Init()
{
   matrix.Init(false);
   poreCount = 0;
   maxDeep = -1;
   ShiftSlitDeep( 0 );
   srand( 0 );

   //InitCube( matrx, poreCount, maxDeep );
   //InitSerpinskyFractal( matrix, poreCount, maxDeep );
}

bool PorousModel::InitDC( HDC winDC, POINT sz )
{
   if( winDC )   hDC.InitDC( winDC );
   if( !hDC )   return false;

   SIZE size = {0,0};
   if( hDC.Object() && GetBitmapDimensionEx( hDC.Object(), &size ) && size.cx == sz.x && size.cy == sz.y )
      return false;
   hDC.SetObject( CreateCompatibleBitmap( winDC, sz.x, sz.y ) );
   return true;
}

void PorousModel::SetWindow( HDC winDC )
{ 
   if( IsVertMode() || IsHorzMode() )
   {
      POINT size = { GetSize().y, IsVertMode() ? GetSize().z : GetSize().x };
      if( InitDC( winDC, size ) )
         ShiftSlitDeep( 0 );
   }
}

bool PorousModel::GetCellColor( const Point<long>& pnt, COLORREF* color ) const
{
   if( !matrix.Value(pnt) )
      return false;

   switch( ViewOpt().mode )
   {
     case vm3D_MODE:
     case vm3D_OPENGL_MODE:
       *color = ModelTheme.pore3dColor;
       break;
     default:
       *color = ModelTheme.poreColor;
       break;
   }
   return true;
}

void PorousModel::DrawSlit( long deep, ulong& dst_deep, ulong sz_deep, ulong sz1, ulong sz2, int arg_deep, int arg1, int arg2 )
{
   if( !hDC )   return;

   dst_deep = static_cast<long>( (deep + sz_deep) % sz_deep );
   const long transp_layers = 10;
   long transp_deep = dst_deep + transp_layers;
   long max_deep = min( sz_deep, transp_deep );

   BitBlt( hDC, 0, 0, sz1, sz2, 0, 0, 0, ModelTheme.dark ? BLACKNESS : WHITENESS );
   Point<long> p;
   COLORREF color;

   bool transp = (viewOpt.flags & vfTRANSPARENCY) != 0;
   long &x1 = p[arg1], &x2 = p[arg2], &x_deep = p[arg_deep];
   double color_koeff = 1.0/(max_deep-dst_deep), koeff = 1.0;
   long red, green, blue;
   for( x1 = sz1-1; x1 >= 0; --x1 )
      for( x2 = sz2-1; x2 >= 0; --x2 )
      {
         x_deep = dst_deep;
         if( GetCellColor( p, &color ) )
            SetPixel( hDC, x1, x2, color );
         else
            if( transp )
               for( ; x_deep < max_deep; ++x_deep )
                  if( GetCellColor( p, &color ) )
                  {
                     koeff = (sz_deep - x_deep)*color_koeff;

                     red   = GetRValue( color )*koeff;
                     green = GetGValue( color )*koeff;
                     blue  = GetBValue( color )*koeff;
                     color = RGB( red, green, blue );

                     SetPixel( hDC, x1, x2, color );
                     break;
                  }
      }
}

void PorousModel::SetVertSlitDeep( long deep )
{
   DrawSlit( deep, vertSlitDeep, GetSize().x, GetSize().y, GetSize().z, 0, 1, 2 );
}

void PorousModel::SetSlitDeep( long deep )
{
   if( IsVertMode() )
      DrawSlit( deep, vertSlitDeep, GetSize().x, GetSize().y, GetSize().z, 0, 1, 2 );
   else if( IsHorzMode() )
      DrawSlit( deep, horzSlitDeep, GetSize().z, GetSize().y, GetSize().x, 2, 1, 0 );
}

void PorousModel::SetViewOpt( ModelViewOptions view_opt )
{
   if( view_opt == viewOpt )   return;

   hDC.ClearObject();
   viewOpt = view_opt;
   SetWindow( 0 );
}

bool PorousModel::CreatePore( const Point<long>& pnt )
{
   if( matrix.Value( pnt ) )   return false;

   matrix.SetValue( pnt, true );
   ++poreCount;

   Maximize( maxDeep, pnt.z );

   UpdateProperties( pnt );

   // Display new pore
   if( hDC )
      if( IsVertMode() )
      {
         if( pnt.x == vertSlitDeep )
            SetPixel( hDC, pnt.y, pnt.z, ModelTheme.poreColor );
      }
      else if( IsHorzMode() && pnt.z == horzSlitDeep )
         SetPixel( hDC, pnt.y, pnt.x, ModelTheme.poreColor );

   return true;
}

void PorousModel::Save( File& f )
{
   matrix.Save( f );

   f << viewOpt << vertSlitDeep << horzSlitDeep << poreCount << maxDeep;
}

bool PorousModel::Load( File& f )
{
   Init();

   bool ok = matrix.Load( f );
   f >> viewOpt >> vertSlitDeep >> horzSlitDeep >> poreCount >> maxDeep;

   return true;
}

bool PorousModel::LoadBMP( HDC hDC, double bound, int depth )
{
   Init();

   const Point<ulong>& sz = GetSize();
   SimplePoint<long> p = { 0,0,0 };
   int red, green, blue;
   double brightness, bright_factor = 1.0/(3*0xFF);

   COLORREF color;
   for( p.y = 0; p.y < sz.y; ++p.y )
      for( p.z = 0; p.z < sz.z; ++p.z )
      {
         color = GetPixel( hDC, p.y, p.z + depth );

         if( color == CLR_INVALID )
            continue;

         red   = GetRValue(color);
         green = GetGValue(color);
         blue  = GetBValue(color);

         brightness = bright_factor*(red + green + blue);

         if( brightness > bound )
            continue;

         matrix.SetValue( p, true );
         Maximize( maxDeep, p.z );
         ++poreCount;
      }

   return true;
}

long* GetMatrixZDensity( const BoolMatrix3D& matrix, long* max_dens )
{
   const Point<ulong>& sz = matrix.GetSize();

   VP<long> dens = new long[sz.z];
   if( max_dens )
      *max_dens = 0;
   for( long z = 0; z < (long)sz.z; ++z )
   {
      dens[z] = matrix.CountValues( z );
      if( max_dens )
         Maximize( *max_dens, dens[z] );
   }

   return dens.GetPtr();
}

void PorousModel::PaintDistributions( HDC hDC, POINT size )
{
   long max_dens = 0;
   VP<long> dens = GetMatrixZDensity( matrix, &max_dens );

   PaintMatrixZDistribution( matrix, dens, max_dens, hDC, size, ModelTheme.poreColor );
}

void PorousModel::UpdateProperties( const Point<long>& pnt )
{
  fractalDim.Update( *this, pnt );
  correlationDim.Update( *this, pnt );
  massDim.Update( *this, pnt );
  averageDepth.Update( *this, pnt );
  poreConcentration.Update( *this, pnt );
  poreArea.Update( *this, pnt );
  poreVolume.Update( *this, pnt );
  porePorosity.Update( *this, pnt );
}