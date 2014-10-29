// Matrix of silicon model (box) with porous tree

#pragma once

#include <PorousModel.h>

// Тип функции-вероятности возникновения отверстия в клетке
// x - от 0 до 1
// результат так же от 0 до 1 (не вероятность, а тип функции вероятности)
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
                     // prob_deg - степень функции вероятности возниконовения поры (интерпретация напряжения)
                     // prob_range - интервал вероятности возникновения поры
                     // sideway_k - модификатор вероятности отклонения всторону (множитель)
                     // upward_k - модификатор вероятности отклонения вверх (множитель)
                     PorousModel1( Point<ulong> sz, ModelViewOptions opt, double prob_deg, DPoint2D prob_range, double sideways_k=.5, double upward_k=.05 );

                void Init( double prob_deg = 1 );

                     // Запустить волну травления из точки p по направлению dirDOWN
                     // (все изменения сохраняются в буфере, применить их можно функцией ApplyChanges)
                void WaveDrill( const Point<long>& p, const Model& surface );

                     // Применить изменения (по буферу)
                void ApplyChanges();

              double GetProbDeg() const { return probDeg; }
     const DPoint2D& GetProbRange() const { return probRange; }
             double  GetSidewayKoeff() const { return sidewayKoeff; }

                void Save( File& f );
                bool Load( File& f );
                         
   private:      
            double probDeg;   // степень функции вероятности возниконовения поры (интерпретация напряжения)
          //ProbFunc probFunc;  // см. описание ProbFunc
          DPoint2D probRange; // интервал вероятности возникновения поры
            double sidewayKoeff;      // Множитель для модификации вероятности возникновения дыры сбоку
            double upwardKoeff;       // Множитель для модификации вероятности возникновения дыры вверху
            double srfInfluence;      // Influence of surface on drill probability

                   // Вспомогательные вещи
      ArrPointLong buf;        // буфер клеток, которые нужно изменить (здесь изменение только на пустую)
      BoolMatrix3D visited;    // Посещённые клетки
 //queue<DrillPoint> wave;       // Волна из клеток, которые нужно "пробурить"
 Queue<DrillPoint> wave;   // Волна из клеток, которые нужно "пробурить"

              // Значение матрицы с учетом внешнего пространства
     inline   bool Value( const Point<long>& p ) const;

              // Кол-во непустых клеток вокруг точки p
              // dir - направление, перпендикулярное плоскости обхода
              // результат: 0-8 (считаем, что в точке p всегда есть отверстие)
              long CountFilled( const Point<long>& p, const BaseDir3d& dir );

              // Попробовать "пробурить" новое отверстие
              // dst_p - клетка, которую сверлим; считается, что эта клетка заполнена
              //    (так же предполагается, что эта клетка принадлежит matrix и она еще не была посещена - не содержится в visited)
              // dir - направление сверления
              // результат - удалось или нет
              bool DrillCell( const Point<long>& dst_p, const Dir3d& dir, char srf_state );

              void ClearBuffers();

     inline double ProbFunction( int& cnt, const int max_cnt );

              // Кол-во непустых клеток в окресности точки p с радиусом radius
              // в cells_num будет записано общее количество клеток в указанной окресности
              // результат не содержит саму точку p и находится в интервале [0,cells_num]
              int CountAroundFilledCells( const Point<long>& p, int radius, int* cells_num );

              // Get probability dependent on the cell around and direction
           double GetProb( const Point<long>& dst_p, const Dir3d& dir );

              // Get probability dependent on the cell around with the given radius and not dependent on the direction
           double GetProbSphere( const Point<long>& dst_p, int radius );
};
