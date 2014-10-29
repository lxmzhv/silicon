// 3D directions

#include <Direction3d.h>

Dir3dTools dir3dTools;

//
// class Dir3dTools
//
Dir3dTools::Dir3dTools()
{
   for( int dir1 = dirUP; dir1 < dirTOTAL; ++dir1 )
      for( int dir2 = dirUP; dir2 < dirTOTAL; ++dir2 )
         for( int dir3 = dirUP; dir3 < dirTOTAL; ++dir3 )
            points[dir1][dir2][dir3] = Dirs[dir1] + Dirs[dir2] + Dirs[dir3];

   int cnt = 0;
   for( char dir1 = dirUP; dir1 <= dirLAST; ++dir1 )
   {
      dirs[cnt++].Set( (BaseDir3d)dir1 );
      for( char dir2 = dir1+1; dir2 < dirTOTAL; ++dir2 )
         if( dir2 != BackDirs[dir1] )
            for( char dir3 = dir2+1; dir3 < dirTOTAL; ++dir3 )
               if( dir3 != BackDirs[dir1] && dir3 != BackDirs[dir2] )
                  dirs[cnt++].Set( (BaseDir3d)dir1, (BaseDir3d)dir2, (BaseDir3d)dir3 );
   }
}

//
// class Dir3d
//
void Dir3d::Set( BaseDir3d d1, BaseDir3d d2, BaseDir3d d3 )
{
   dir1 = d1;
   dir2 = d2;
   dir3 = d3;
}

inline Dir3d Dir3d::Back() const
{
   Dir3d back_dir;
   back_dir.Set( BackDirs[dir1], BackDirs[dir2], BackDirs[dir3] );
   return back_dir;
}

inline const Point<long>& Dir3d::GetPoint() const
{
   return dir3dTools.GetPoint( (BaseDir3d)dir1, (BaseDir3d)dir2, (BaseDir3d)dir3 );
}
