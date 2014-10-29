// 3D directions

#pragma once

#include <points.h>

enum BaseDir3d
{
   dirUP = 0,
   dirDOWN,
   dirLEFT,
   dirRIGHT,
   dirFORWARD,
   dirBACK,
   dirLAST = dirBACK,
   dirNULL,
   dirTOTAL,
};

const BaseDir3d BackDirs[dirTOTAL] = { dirDOWN, dirUP, dirRIGHT, dirLEFT, dirBACK, dirFORWARD, dirNULL };

const Point<long> PointUp( 0, 0,-1 );
const Point<long> PointDown( 0, 0, 1 );
const Point<long> PointLeft( -1, 0, 0 );
const Point<long> PointRight( 1, 0, 0 );
const Point<long> PointForward( 0,-1, 0 );
const Point<long> PointBack( 0, 1, 0 );
const Point<long> PointNull( 0, 0, 0 );

const Point<long> Dirs[dirTOTAL] = { PointUp, PointDown, PointLeft, PointRight, PointForward, PointBack, PointNull };

// All 3d directions
class DLL_OBJ Dir3d
{
   public:
      char dir1, dir2, dir3; // use type char instead of BaseDir3d to optimize memory use

                          void Set( BaseDir3d dir1=dirNULL, BaseDir3d dir2=dirNULL, BaseDir3d dir3=dirNULL );

                          bool operator == (const Dir3d& dir3d) {return dir1 == dir3d.dir1 && dir2 == dir3d.dir2 && dir3 == dir3d.dir3; }
                          bool operator != (const Dir3d& dir3d) {return dir1 != dir3d.dir1 || dir2 != dir3d.dir2 || dir3 != dir3d.dir3; }

   inline                Dir3d Back() const;
   inline   const Point<long>& GetPoint() const;

   inline                 bool IsDown() const { return GetPoint().z == 1; }
   inline                 bool IsUp() const { return GetPoint().z == -1; }
   inline                 bool IsHorizontal() const { return (GetPoint().x || GetPoint().y) && GetPoint().z == 0; }
   inline                 bool IsNullBaseDir() const { return dir1 == dirNULL || dir2 == dirNULL || dir3 == dirNULL; }
};

const int DIR_3D_CNT = 26;

// Utility class
class DLL_OBJ Dir3dTools
{
   public:
                              Dir3dTools();

                              // Fast access to Point value of 3D directions
   inline  const Point<long>& GetPoint( BaseDir3d dir1, BaseDir3d dir2, BaseDir3d dir3 ) const { return points[dir1][dir2][dir3]; }

                              // Access to directions by a number - from 0 to (DIR_3D_CNT-1)
   inline        const Dir3d& GetDir( int num ) const { return dirs[num]; }

   private:
   Point<long> points[dirTOTAL][dirTOTAL][dirTOTAL];
         Dir3d dirs[3*3*3-1];
};

extern DLL_OBJ Dir3dTools dir3dTools;
