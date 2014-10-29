// Painting

#include <SiliconDisplay.h>
#include <points.h>
#include <numbers.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>

static int x = 0;
static int y = 0;
static HDC hDC;
static HGLRC hRC;

void DrawBounds( const PorousModel& model )
{
   SimplePoint<float> sz, center;

   sz = model.GetSize();
   scvec( 0.5f, sz, &center );

   glDisable(GL_LIGHTING);

   glTranslatef( -center.x, -center.z, -center.y );
	glColor3f( 0.5f, 0.5f, 0.5f );

	glBegin( GL_LINE_LOOP );

	glVertex3f( 0, 0, 0 );
	glVertex3f( 0, sz.z, 0 );
	glVertex3f( sz.x, sz.z, 0 );
	glVertex3f( sz.x, 0, 0 );
	glVertex3f( 0, 0, 0 );

	glVertex3f( 0, 0, sz.y );
	glVertex3f( 0, sz.z, sz.y );
	glVertex3f( sz.x, sz.z, sz.y );
	glVertex3f( sz.x, 0, sz.y );
	glVertex3f( 0, 0, sz.y );

	glEnd();

   glBegin( GL_LINES );

	glVertex3f( 0, sz.z, 0 );
	glVertex3f( 0, sz.z, sz.y );

   glVertex3f( sz.x, sz.z, 0 );
	glVertex3f( sz.x, sz.z, sz.y );

   glVertex3f( sz.x, 0, 0 );
	glVertex3f( sz.x, 0, sz.y );

   glEnd();

   glEnable(GL_LIGHTING);
   glTranslatef( center.x, center.z, center.y );
}

void DrawSysCoor()
{
	glColor3f( 1, 0, 0 );
	glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 1, 0, 0 );
	glEnd();

	glColor3f( 0, 1, 0 );
	glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 1, 0 );
	glEnd();

	glColor3f( 0, 0, 1 );
	glBegin( GL_LINES );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 0, 1 );
	glEnd();
}

void DrawModel( const PorousModel& model )
{
   const static FPoint radius( 0.5, 0.5, 0.5 );
   static Point<long> pnt;
   static Point<ulong> sz;
   static FPoint center, p;
   GLfloat material[3];

   COLORREF color;

   GLUquadric *q = gluNewQuadric();

   sz = model.GetSize();
   center.Init( 0.5f*sz.x, 0.5f*sz.y, 0.5f*sz.z );

   double volume = double(sz.x)*sz.y*sz.z;
   int quality;
   if( volume < 1e6 )
      quality = 10;
   else if ( volume < 1e9 )
      quality = 5;
   else
      quality = 3;

   for( pnt.x = 0; pnt.x < long(sz.x); ++pnt.x )
      for( pnt.y = 0; pnt.y < long(sz.y); ++pnt.y )
         for( pnt.z = 0; pnt.z < long(sz.z); ++pnt.z )
            if( model.GetCellColor( pnt, &color ) )
            {
               material[0] = float(GetRValue(color))/0xFF;
               material[1] = float(GetGValue(color))/0xFF;
               material[2] = float(GetBValue(color))/0xFF;

               glMaterialfv( GL_FRONT, GL_DIFFUSE, material );

               copy_vector( p, pnt );
               p += radius;
               p -= center;

               glTranslatef( p.x, -p.z, -p.y );
               gluSphere( q, 0.7, quality, quality );
               glTranslatef( -p.x, p.z, p.y );
            }

   //material[0] = material[1] = 1.0f;
   //material[2] = 0.0f;
   //glMaterialfv( GL_FRONT, GL_DIFFUSE, material );

   //glTranslatef( -sz.x/5, sz.z/5, 0 );
   //gluSphere( q, 1, 10, 10 );
   //glTranslatef( sz.x/5, -sz.z/5, 0 );

   //gluDeleteQuadric( q );
}

static void SetLighting( const PorousModel& model )
{
   float r = Length( model.GetSize() );

   GLfloat pos1[] = { -r, 0, r, 0 };
   GLfloat diffuse1[] = { 1, 1, 1, 1 };
   glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse1 );
   glLightfv( GL_LIGHT0, GL_POSITION, pos1 );

   GLfloat pos2[] = { r, 0, -r, 0 };
   GLfloat diffuse2[] = { 1, 1, 1, 1 };
   glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuse2 );
   glLightfv( GL_LIGHT1, GL_POSITION, pos2 );
}

static void SetOpenGLPerspective( HWND hWnd, const PorousModel& model )
{
   RECT screen;
	 GetClientRect(hWnd, &screen);

   int width = screen.right - screen.left;
   int height = screen.bottom - screen.top;
   GLfloat aspect = (height > NULL_DOUBLE ? ((float)width)/height : 1.0f);

	 glMatrixMode( GL_PROJECTION );
	 glLoadIdentity();
	 gluPerspective( 45.0f, aspect, 0.1f, 3*Length(model.GetSize()) );
	 glMatrixMode( GL_MODELVIEW );
}

void OpenGLDraw( HWND hWnd, const PorousModel& model, float zoom, float h_angle, float v_angle )
{
  //GLfloat trf[4][4];
  //
  //memset( trf, 0, sizeof(trf) );
  //
  //for( int i = 0; i < 3; ++i )
  //  trf[i][i] = 1.0f;
  //
  //for( int i = 0; i < 3; ++i )
  //{
  //   trf[0][i] = TrfMatrix[0][i];
  //   trf[1][i] = TrfMatrix[1][i];
  //   trf[2][i] = TrfMatrix[2][i];
  //   trf[i][3] = 0.0;
  //}
  //
  //// Coordinates of the center
  //trf[3][0] = 0.0;
  //trf[3][1] = 0.0;
  //trf[3][2] = 0.0;
  //trf[3][3] = 1.0;
  //
  //trf_x_pnt( trf, trf[3], trf[3] );
  //glMultMatrixf( (const GLfloat*)trf );

  SetOpenGLPerspective( hWnd, model );

  // как обычно указываем какие буферы будем чистить
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // обнуляем текущую матрицу
  glLoadIdentity();

  float r = Length( model.GetSize() );
  float cos_vert = cos(v_angle);

  //FPoint eye( r*sin(h_angle)*cos_vert, r*sin(v_angle), r*cos(h_angle)*cos_vert );
  //FPoint up( 0.0f, cos_vert > 0 ? 1.0f : -1.0f, 0.0f );
  FPoint eye( 0, 0, r );
  FPoint up( 0.0f, 1.0f, 0.0f );
  FPoint center( 0.0f, 0.0f, 0.0f );
  gluLookAt( eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z );

  glScalef( zoom, zoom, zoom );

  SetLighting( model );

  static const float koeff = (const float)(180.0f/M_PI);
  glRotatef( koeff*v_angle, 1.0f, 0.0f, 0.0f );
  glRotatef( -koeff*(M_PI/2+h_angle), 0.0f, 1.0f, 0.0f );

  glPushMatrix();

  DrawBounds( model );

  glPopMatrix();
  DrawModel( model );

  SwapBuffers(hDC);
}

void InitOpenGL_model( HWND hWnd, const PorousModel& model )
{
   SetOpenGLPerspective( hWnd, model );

   glEnable(GL_LIGHTING);
   glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, 0 );
   glEnable( GL_LIGHT0 );
   glEnable( GL_LIGHT1 );

   SetLighting( model );
}

static void InitOpenGL_low( HWND hWnd )
{   
  float light = ModelTheme.dark ? 0.0f : 1.0f;
	glClearColor(light, light, light, light); // устанавливаем цвет для очистки буфера цвета

	glClearDepth(1.0);		// устенавливаем параметр для очистки буфера глубины
	glDepthFunc(GL_LESS);	// настройка Z буфера
	glEnable(GL_DEPTH_TEST);	// и, наконец, включение

	glShadeModel(GL_SMOOTH);	// выбираете режим затенения ( flat или smooth )
}

void InitOpenGL( HWND hWnd )
{
   GLuint PixelFormat;	// формат пикселя

   static PIXELFORMATDESCRIPTOR pfd =
   {
	   sizeof(PIXELFORMATDESCRIPTOR),// размер структуры
	   1,			// версия ?
	   PFD_DRAW_TO_WINDOW|	// format must support Window
	   PFD_SUPPORT_OPENGL|	// format must support OpenGL
	   PFD_DOUBLEBUFFER,		// must support double buffer
	   PFD_TYPE_RGBA,		// требуется RGBA формат
	   16,			// 16Bit color depth
	   0, 0, 0, 0, 0, 0,		// Color bits ignored ?
	   0,			// No Alpha buffer
	   0,			// shift bit ignored
	   0,			// no accumulation buffer
	   0, 0, 0, 0,		// accumulation buffer bits ignored
	   16,			// 16bit z-buffer (depth buffer)
	   0,			// no stencil buffer
	   0,			// no auxiliary buffer
	   PFD_MAIN_PLANE,		// main drawing layer
	   0,			// reserved
	   0, 0, 0			// layer mask ignored
   };

	hDC = GetDC(hWnd);		// получаем контекст windows

	// Следующие несколько строк настраивают формат пикселей
	PixelFormat = ChoosePixelFormat(hDC, &pfd);

	if( !PixelFormat )
		Error( "Can't find suitable PixelFormat" );

	if( !SetPixelFormat( hDC, PixelFormat, &pfd ) )
		Error( "Can't set The PixelFormat" );

	hRC = wglCreateContext( hDC );	// создаем контекст рендеринга
	if( !hRC )
		Error( "Can't Create Render Device Context" );

	if( !wglMakeCurrent(hDC, hRC) ) // устанавливаем его текущим
		Error( "Can't set current Render Device Context" );

	InitOpenGL_low( hWnd );

   //DEVMODE dmScreenSettings;
	//memset(&dmScreenSettings, 0, sizeof(DEVMODE));
	//dmScreenSettings.dmSize			= sizeof(DEVMODE);
	//dmScreenSettings.dmPelsWidth	= 800;
	//dmScreenSettings.dmPelsHeight	= 600;
	//dmScreenSettings.dmFields		= DM_PELSWIDTH | DM_PELSHEIGHT;
	//ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	//ShowWindow(hWnd, SW_SHOW);
	//UpdateWindow(hWnd);
	//SetFocus(hWnd);
}

//GLvoid ResizeOpenGLWindow(GLsizei Width, GLsizei Height)
//{
//	if(Height==0)
//		Height = 1;
//	glViewport(0, 0, Width, Height);
//
//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//
//	gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f);
//
//	glMatrixMode(GL_MODELVIEW);
//}

void ReleaseOpenGL( HWND hWnd )
{
   ChangeDisplaySettings(NULL, 0);	// восстанавливаем установки

   wglMakeCurrent(hDC, NULL);		// устанавливаем обычный device context
   wglDeleteContext(hRC);		// удаляем контекст рендеринга
   ReleaseDC(hWnd, hDC);		// освобождаем device context
}

//void Paint3D_OpenGL( const PorousHoleModel& model, HDC h_dc, POINT size, Trf_3d trf, double zoom, double view_pos )
//{
//   OpenGL_InitWindow();
//}
//
//void CALLBACK display (void)
//{
//	glClear( GL_COLOR_BUFFER_BIT );
//
//	glMatrixMode( GL_PROJECTION );
//	glLoadIdentity();
//	gluPerspective (130, 1, 50, 0);
//
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//
//   gluLookAt (x, y, 300, 150, 150, 0, 0, 1, 0);
//	glColor3f(0.0, 0.0, 1);
//	glBegin(GL_POLYGON);
//		glVertex3f(100,100,100);
//		glVertex3f(200,100,100);
//		glVertex3f(200,100,200);
//		glVertex3f(100,100,200);
//	glEnd();    
//	glColor3f(0.0, 1, 0);
//	glBegin(GL_POLYGON);
//		glVertex3f(100,100,100);
//		glVertex3f(100,100,200);
//		glVertex3f(100,200,200);
//		glVertex3f(100,200,100);
//	glEnd(); 
//	glColor3f(1, 0, 0);
//	glBegin(GL_POLYGON);
//		glVertex3f(100,100,200);
//		glVertex3f(100,200,200);
//		glVertex3f(200,200,200);
//		glVertex3f(200,100,200);
//	glEnd(); 
//	glColor3f(1, 0, 1);
//	glBegin(GL_POLYGON);
//		glVertex3f(100,100,100);
//		glVertex3f(200,100,100);
//		glVertex3f(200,200,100);
//		glVertex3f(100,200,100);
//	glEnd(); 
//
//	glFlush();
//}
//
//void InitOpenGL( HWND hWnd )
//{
//   auxInitDisplayMode( AUX_SINGLE | AUX_RGB );
//   auxInitPosition( 0, 0, 800, 600 );
//   auxInitWindow( "OpenGL Window");
//
//   InitWin();
//	x = 300;
//   y = 300;
//
//	auxKeyFunc(AUX_UP, ProcUp);
//	auxKeyFunc(AUX_DOWN, ProcDown);
//	auxKeyFunc(AUX_LEFT, RrocLeft);
//	auxKeyFunc(AUX_RIGHT, ProcRight);
//   auxMainLoop( display );
//}
