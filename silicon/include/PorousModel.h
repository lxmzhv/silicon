// Matrix of silicon model (box) with porous tree

#pragma once

#include <Silicon.h>
#include <BoolMatrix3d.h>
#include <SiliconSrfModel.h>
#include <vector>
#include <queue>
#include <view.h>
#include <tmatrix.h>
#include <list.h>
#include <numbers.h>
#include <Direction3d.h>
#include <SiliconDim.h>
#include <SiliconProperty.h>

const double SILICON_WIDTH = 5e-4; // [meters] - width of silicon plate: usually 0.4-0.5 mm
const double CELL_SIZE     = 1e-6; // [meters]
const double CELL_VOLUME   = CELL_SIZE * CELL_SIZE * CELL_SIZE;

class PorousModel
{
   public:
                     PorousModel( Point<ulong> sz, ModelViewOptions vopt );
         virtual    ~PorousModel() {}

                void Init();

                     ulong GetPoreCount() const { return poreCount; } //  ол-во протравленных клеток 
inline const Point<ulong>& GetSize() const { return matrix.GetSize(); }
inline const BoolMatrix3D& GetMatrix() const { return matrix; }
                long GetMaxDeep() const { return maxDeep; }

       virtual  bool GetCellColor( const Point<long>& pnt, COLORREF* color ) const;
       virtual  void PaintDistributions( HDC hDC, POINT size );
                         
                void SetWindow( HDC winDC );

                void InvalidateProperties();
                void RecalcProperties();
          Dimension& GetFractalDim()      { return fractalDim; }
          Dimension& GetCorrelationDim()  { return correlationDim; }
          Dimension& GetMassDim()         { return massDim; }
           Property& GetAverageDepth()    { return averageDepth; }
           Property& GetPoreConcentration(){ return poreConcentration; }
           Property& GetPoreArea()        { return poreArea; }
           Property& GetPoreVolume()      { return poreVolume; }
           Property& GetPorePorosity()    { return porePorosity; }

                void SetSlitDeep( long deep );
                void SetVertSlitDeep( long deep );

                void ShiftSlitDeep( long add_deep ) { SetSlitDeep( SlitDeep() + add_deep ); }
                void ShiftVertSlitDeep( long add_deep ) { SetVertSlitDeep( VertSlitDeep() + add_deep ); }

                bool IsVertMode() const { return viewOpt.mode == vmVERT_SLIT; }
                bool IsHorzMode() const { return viewOpt.mode == vmHORZ_SLIT; }

               ulong SlitDeep() const { return IsHorzMode() ? horzSlitDeep : IsVertMode() ? vertSlitDeep : 0; }
               ulong VertSlitDeep() const { return vertSlitDeep; }

                 HDC GetDC() { return hDC; }
const ModelViewOptions& ViewOpt() const { return viewOpt; }
                void SetViewOpt( ModelViewOptions view_opt );
                     
                void Save( File& f );
                bool Load( File& f );
                bool LoadBMP( HDC hDC, double bound, int depth );

   protected:
        BoolMatrix3D matrix;     // true - hole exists; false - not exists
                long maxDeep;    // max z: matrix[*,*,z] == true
               ulong poreCount;  // number of pore cells

    ModelViewOptions viewOpt;            
               ulong vertSlitDeep;    // глубина вертикального разреза
               ulong horzSlitDeep;    // глубина горизонтального разреза

           Dimension fractalDim;
           Dimension correlationDim;
           Dimension massDim;
        AverageDepth averageDepth;
   PoreConcentration poreConcentration;
            PoreArea poreArea;
          PoreVolume poreVolume;
        PorePorosity porePorosity;

        virtual bool CreatePore( const Point<long>& pnt );

   private:
   DCObject<HBITMAP> hDC;             // ќтображение матрицы (какой-то разрез)

                   // ¬ернЄт true если нужно перерисовать hBmp
              bool InitDC( HDC winDC, POINT sz );
              void DrawSlit( long deep, ulong& dst_deep, ulong sz_deep, ulong sz1, ulong sz2, int arg_deep, int arg1, int arg2 );
              void UpdateProperties( const Point<long>& pnt );

              //bool CalcNextDimPoints( DPoint2D* points, long& num, Indicator* indic ) const;
              // ѕосчитать кол-во "дырок" в указанном масштабе (отображаем исходную матрицу в матрицу размером n*n*n и считаем кол-во дырок в полученной матрице)
              //long CalcHolesScale( int n, Indicator* indic=0, int cnt=0, int max_cnt=0 ) const;
};

void PaintMatrixZDistribution( const BoolMatrix3D& matrix, long* dens, long max_dens, HDC hDC, POINT displ_sz, COLORREF color );
long* GetMatrixZDensity( const BoolMatrix3D& matrix, long* max_dens );
