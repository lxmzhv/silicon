// ������ ����������� �������

#pragma once

#include <points.h>
#include <pointers.h>
#include <ClusterModel.h>

// Not change the values of this constants! 
enum SiliconSrfState
{
  sssNO_MOLECULE          = 0, // ���� ��� ��������������� ��������
  sssNOT_CHARGED_MOLECULE = 1, // ���� � ��������������� ������������ ���������
  sssCHARGED_MOLECULE     = 2, // ���� � ��������������� ���������� ���������
};

// ���������: 
class SiliconSrfModel: public Model
{
   public:
                       // timeX - ��. �������� waitTime
                       SiliconSrfModel( ulong n=1, ulong m=1, ulong time0=0, ulong time1=0, ulong time2=0 );
        virtual   void Init( ulong n=1, ulong m=1, const char* fld=0 );
        virtual   void InitRandom( char colors=3 );
                  void InitWaitings( bool random = false );
        virtual   char NewState( long x, long y );
        virtual   void Save( File& f );
        virtual   bool Load( File& f );
   const Point<ulong>& GetWaitTimes() const { return waitTime; }

   protected:
      
     Point<ulong> waitTime;  // ������� "����������" ������ � ���������� 0,1,2 - ���-�� �����, ������� �� ����� �������, ������ ��� ��� ������ �������� ��� ���������
 	     VP<ulong> waitings;     // ����� ����� �� ���������� ��������� ������

                 ulong GetWaiting( long x, long y ) const { return waitings[y*size.x+x]; }
                  void SetWaiting( long x, long y, ulong new_waiting ) { waitings[y*size.x+x] = new_waiting; }

        virtual   void OnChange( ulong x, ulong y, char new_state );

        void CountArroundStates( long x, long y, char* cnt ) const;
};

// ������ ��������� �����������
// �� ������ ���� ������ ���������� � ������������ prob
// ������� ���������� ��������� �� �� ���������� ������ �� �����
class OxidModel: public Model
{
   public:
                      // oxid_prob - ����������� ��������� ������ 
                      OxidModel( ulong n, ulong m, double oxid_prob ): Model(n,m), prob(oxid_prob) { srand((ulong)GetTickCount()); }
         virtual char NewState( long x, long y );
     virtual     void Save( File& f );
     virtual     bool Load( File& f );
               double GetProb() const { return prob; }

   private:
      double prob; // ����������� ���������
};
