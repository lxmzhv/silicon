// Matrix of silicon model (box) with porous tree

#pragma once

#include <PorousModel.h>

// ��� �������-����������� ������������� ��������� � ������
// x - �� 0 �� 1
// ��������� ��� �� �� 0 �� 1 (�� �����������, � ��� ������� �����������)
//typedef double (*ProbFunc)( double x );

//inline double LineFunc( double x ) { return x; }

typedef Point<long> PointLong;
typedef vector<PointLong> ArrPointLong;

struct DrillPoint
{
  SimplePoint<long> p;
              Dir3d dir;

  DrillPoint& Set( const Point<long>& pnt, BaseDir3d dir1, BaseDir3d dir2, BaseDir3d dir3 ) { p=pnt; dir.Set(dir1,dir2,dir3); return *this; }
  DrillPoint& Set( const Point<long>& pnt, const Dir3d& dir3d ) { p=pnt; dir=dir3d; return *this; }
};

class PorousModel1: public PorousModel
{
   public:
                     // prob_deg - ������� ������� ����������� �������������� ���� (������������� ����������)
                     // prob_range - �������� ����������� ������������� ����
                     // sideway_k - ����������� ����������� ���������� �������� (���������)
                     // upward_k - ����������� ����������� ���������� ����� (���������)
                     PorousModel1( Point<ulong> sz, ModelViewOptions opt, double prob_deg, DPoint2D prob_range, double sideways_k=.5, double upward_k=.05 );

                void Init( double prob_deg = 1 );

                     // ��������� ����� ��������� �� ����� p �� ����������� dirDOWN
                     // (��� ��������� ����������� � ������, ��������� �� ����� �������� ApplyChanges)
                void WaveDrill( const Point<long>& p, const Model& surface );

                     // ��������� ��������� (�� ������)
                void ApplyChanges();

              double GetProbDeg() const { return probDeg; }
     const DPoint2D& GetProbRange() const { return probRange; }
             double  GetSidewayKoeff() const { return sidewayKoeff; }

                void Save( File& f );
                bool Load( File& f );
                         
   private:      
            double probDeg;   // ������� ������� ����������� �������������� ���� (������������� ����������)
          //ProbFunc probFunc;  // ��. �������� ProbFunc
          DPoint2D probRange; // �������� ����������� ������������� ����
            double sidewayKoeff;      // ��������� ��� ����������� ����������� ������������� ���� �����
            double upwardKoeff;       // ��������� ��� ����������� ����������� ������������� ���� ������
            double srfInfluence;      // Influence of surface on drill probability

                   // ��������������� ����
      ArrPointLong buf;        // ����� ������, ������� ����� �������� (����� ��������� ������ �� ������)
      BoolMatrix3D visited;    // ���������� ������
 //queue<DrillPoint> wave;       // ����� �� ������, ������� ����� "���������"
 Queue<DrillPoint> wave;   // ����� �� ������, ������� ����� "���������"

              // �������� ������� � ������ �������� ������������
     inline   bool Value( const Point<long>& p ) const;

              // ���-�� �������� ������ ������ ����� p
              // dir - �����������, ���������������� ��������� ������
              // ���������: 0-8 (�������, ��� � ����� p ������ ���� ���������)
              long CountFilled( const Point<long>& p, const BaseDir3d& dir );

              // ����������� "���������" ����� ���������
              // dst_p - ������, ������� �������; ���������, ��� ��� ������ ���������
              //    (��� �� ��������������, ��� ��� ������ ����������� matrix � ��� ��� �� ���� �������� - �� ���������� � visited)
              // dir - ����������� ���������
              // ��������� - ������� ��� ���
              bool DrillCell( const Point<long>& dst_p, const Dir3d& dir, char srf_state );

              void ClearBuffers();

     inline double ProbFunction( int& cnt, const int max_cnt );

              // ���-�� �������� ������ � ���������� ����� p � �������� radius
              // � cells_num ����� �������� ����� ���������� ������ � ��������� ����������
              // ��������� �� �������� ���� ����� p � ��������� � ��������� [0,cells_num]
              int CountAroundFilledCells( const Point<long>& p, int radius, int* cells_num );

              // Get probability dependent on the cell around and direction
           double GetProb( const Point<long>& dst_p, const Dir3d& dir );

              // Get probability dependent on the cell around with the given radius and not dependent on the direction
           double GetProbSphere( const Point<long>& dst_p, int radius );
};
