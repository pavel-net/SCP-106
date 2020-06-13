#include <time.h>
#include "engine_header.h"		
#include "sounds.h"
#include "init_header.h"
#include <tchar.h>
#pragma comment(lib, "Opengl32.lib")
#pragma comment(lib, "GLu32.lib")
#pragma comment(lib, "glaux.lib")
#define rotate_speed  0.14f			//скорость вращения 
#define tex_count 19				//кол-во текстур

bool TextureDead;
CCamera  g_Camera;
int FPS = 1200;
int TimeGame;
float light_ambient[] = {0.2f,0.2f,0.2f,1.0f};	//рассеянный  и диффузный свет
float light_diffuse[] = {0.8f,0.8f,0.8f,1.0f};
float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
float specular[] = {0.8f, 0.8f, 0.8f, 1.0f};	//отражаемый от полигонов свет
float specref[] = {1.0f, 1.0f, 1.0f, 1.0f};

float radius = 3.0f;	//радиус освещённости
float att = 1.5f;		//главнавная величина, влияющая на скорость рассеивания
float kQ  = att / (2.5f* radius * radius);
float kL = att / (2.5f * radius);		//коэффициенты рассеивания света
float kC = att / 2.5f;

void CheckFPS()
{
	static float old_time =(float)GetTickCount();
	static int fps = 0;
	float new_time =(float)GetTickCount();
	fps++;
	if(new_time - old_time > 1000)
	{
		TimeGame++;
		FPS = fps;
		fps = 0;
		old_time = new_time;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static HGLRC hRC;              // Постоянный контекст рендеринга
static HDC hDC;                // Приватный контекст устройства GDI
BOOL   keys[256];              // Массив для процедуры обработки клавиатуры
bool TunnelToLeft;			   // специальный флаг состояния движения по туннелю

float g_FogDensity = 0.5f;       // Плотность дыма
GLfloat fogColor[4]= {0.2f, 0.2f, 0.2f, 1.0f}; // Цвет тумана

GLuint texture[tex_count];								//место для текстур
LPCWSTR stexture[19] = {L"textures/scp.bmp", L"textures/TunWall2.bmp", L"textures/TunPot2.bmp", L"textures/TunPol2.bmp",
						L"textures/MainWall3.bmp", L"textures/MainPot3.bmp", L"textures/MainPol.bmp",
						L"textures/CenterWall.bmp", L"textures/CenterPot.bmp", L"textures/CenterPol3.bmp", L"textures/scp-106.bmp",
						L"textures/scp-1.bmp", L"textures/scp-2.bmp", L"textures/scp-3.bmp", L"textures/dead.bmp", L"textures/Pandora.bmp",
						L"textures/scp-6.bmp", L"textures/scp-5.bmp", L"textures/nvidia.bmp"};	//массив имён файлов для текстур
const char* ArraySounds[] = {"sounds/fTrack1.ogg", "sounds/fTrack2.ogg", "sounds/fTrack3.ogg", "sounds/fTrack4.ogg", "sounds/fTrack5.ogg",
						"sounds/fTrack6.ogg", "sounds/fTrack7.ogg", "sounds/fTrack8.ogg", "sounds/fTrack9.ogg", "sounds/fTrack10.ogg",
						"sounds/fTrack11.ogg", "sounds/fTrack12.ogg", "sounds/fTrack13.ogg",
						"sounds/wTrack1.ogg", "sounds/wTrack2.ogg", "sounds/wTrack3.ogg", "sounds/wTrack4.ogg", "sounds/wTrack5.ogg",
						"sounds/wTrack6.ogg", "sounds/wTrack7.ogg", "sounds/wTrack8.ogg"}; 

// Загрузка картинки и конвертирование в текстуру
GLvoid LoadGLTextures()
{
	for(int i=0; i<tex_count; i++)
	{
		AUX_RGBImageRec* texture1=NULL;				//информация о текстуре
		texture1 = auxDIBImageLoad(stexture[i]);	//загружаем информацию
		if(!texture1)
			exit(1);
		glGenTextures(tex_count, &texture[i]);		//генерируем информацию о текстуре в необходимой форме (в i-ом элементе)
		glBindTexture(GL_TEXTURE_2D, texture[i]);	//начинаем описывать текстуру в программе
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texture1->sizeX, texture1->sizeY,
			GL_RGB, GL_UNSIGNED_BYTE, texture1->data); 
	}
}
bool Dead(remSnd* DEAD)
{
	static bool music_active = false;
	static bool flag = true;
	if(!music_active)
	{
		music_active = true;
		DEAD->Play();
	}
	if(flag) 
	{
		if(g_Camera.m_vPosition.y < 4.0 && g_Camera.m_vPosition.y > 3.0)
		{
			g_Camera.m_vPosition.y -= 0.5f/FPS;
			g_Camera.m_vView.y +=0.01f/FPS;
		}
		if(g_Camera.m_vPosition.y < 2.0f)
		{
			g_Camera.m_vPosition.y -= 1.5f/FPS;
			g_Camera.m_vView.y +=0.01f/FPS;
		}
		else 
		{
			g_Camera.m_vPosition.y -= 0.9f/FPS;
			g_Camera.m_vView.y +=0.01f/FPS;
		}
		g_Camera.RotateFunc(rotate_speed/3,true, true, FPS);
		g_Camera.m_vPosition.z -= 0.1f/FPS;
	}
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	light_diffuse[0] = 1.0f;
	light_diffuse[1] = 0.0f;
	light_diffuse[2] = 0.0f;
	light_ambient[0] = 1.0f;
	light_ambient[1] = 0.0f;
	light_ambient[2] = 0.0f;
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	if(g_Camera.m_vPosition.y < 1.5)
	{
		flag = false;
		return true;
	}
	return false;
}
bool TimeCheck(bool flag, remSnd* TimeDead)
{
	static bool play = false;
	static bool old_flag = true;
	static float old_time =(float)GetTickCount();
	float new_time = (float)GetTickCount();
	if(flag == old_flag)
	{
		if(new_time - old_time>15000)
		{
			if(!play)
			{
				TimeDead->mPos[0] = g_Camera.m_vPosition.x;
				TimeDead->mPos[1] = g_Camera.m_vPosition.y;
				TimeDead->mPos[2] = g_Camera.m_vPosition.z;
				TimeDead->Play();
			}
			TextureDead = true;
			play = true;
		}
		if(new_time - old_time>25000)
			return true;
	}
	else
	{
		play = false;
		TextureDead = false;
		old_flag = flag;
		old_time = new_time;
	}
	return false;
}

GLvoid InitGL(GLsizei Width, GLsizei Height)
{
	LoadGLTextures();			//Загрузка текстур
	glEnable(GL_TEXTURE_2D);	//Разрешение наложение текстуры
	g_Camera.PositionCamera(0.0, 4.0, -2.0,   0.0, 3.0f, -5.0f,   0.0, 1.0f, 0.0);
	float light_position[] = {g_Camera.m_vPosition.x, g_Camera.m_vPosition.y-3.2f, g_Camera.m_vPosition.z+1.5f, 1.0f};
	g_Camera.radius=1.0f;
	glClearDepth(1.0);                      // Разрешить очистку буфера глубины
	glDepthFunc(GL_LESS);                   // Тип теста глубины
	glEnable(GL_DEPTH_TEST);                // разрешить тест глубины
	glShadeModel(GL_SMOOTH);        // разрешить плавное цветовое сглаживание
	glMatrixMode(GL_PROJECTION);    // Выбор матрицы проекции
	glLoadIdentity();               // Сброс матрицы проекции
	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// Вычислить соотношение геометрических размеров для окна						 
	glMatrixMode(GL_MODELVIEW);     // Выбор матрицы просмотра модели	
	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};	//тёмный туман
	
	CVector3* Direct = g_Camera.m_vView.operator -(&g_Camera.m_vPosition);	//направление света
	Direct->Normalization();	
	Direct->x = Direct->x/8;
	Direct->y = Direct->y/8;
	Direct->z = Direct->z/8;
	glEnable(GL_LIGHT0);		//вкл наш главный источник освещения
	glEnable(GL_LIGHTING);		//разрешаем на сцене использовать освещение

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);	//настройки параметров источника света
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, kC);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, kL);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, kQ);


	glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, light_diffuse);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // Очистка экрана в цвет/////////////////////////////////////////////////////////////////////////
	glEnable(GL_FOG);                       // Включает туман (GL_FOG)
	glFogi(GL_FOG_MODE, GL_LINEAR);			// Выбираем тип тумана
	glFogfv(GL_FOG_COLOR, fogColor);        // Устанавливаем цвет тумана
	glFogf(GL_FOG_DENSITY, 1.0f);           // Насколько густым будет туман
	glHint(GL_FOG_HINT, GL_DONT_CARE);      // Вспомогательная установка тумана
	glFogf(GL_FOG_START, 12.0f);            // Глубина, с которой начинается туман
	glFogf(GL_FOG_END, 15.0f);				// Глубина, где туман заканчивается.
}

//функция масштабируемости, вызывается когда меняется размер окна

GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)	
{
     if (Height==0)          // Предотвращение деления на ноль, если окно слишком мало
             Height=1;
     glViewport(0, 0, Width, Height);		 // Сброс текущей области вывода и перспективных преобразований            
     glMatrixMode(GL_PROJECTION);            // Выбор матрицы проекций
     glLoadIdentity();                       // Сброс матрицы проекции
     gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f); // Вычисление соотношения геометрических размеров для окна           
     glMatrixMode(GL_MODELVIEW);			// Выбор матрицы просмотра модели
}
void DrawPandora(float x, float z)	
{
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	glBindTexture(GL_TEXTURE_2D, texture[15]);
	glBegin(GL_QUADS);
		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z+1.0f);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z-1.0f);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z-1.0f);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z+1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z-1.0f);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z+1.0f);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z+1.0f);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z-1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z+1.0f);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z+1.0f);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z+1.0f);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z+1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z-1.0f);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z-1.0f);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z-1.0f);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z-1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 3.0f, z+1.0f);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 3.0f, z+1.0f);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z-1.0f);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z-1.0f);
	glEnd();
}

bool DrawSCP(float x, float z, char ch_direction, bool TurnFlag)	
{
	static bool MoveUP = true;
	static float save_y = 0.0f;
	float step_y = 0.5f/FPS;
	float step_z = 2.0f;
	if(ch_direction == 'L')		//монстр двигается влево
		step_z = -step_z;
	if(save_y > 0.2f)
		MoveUP = false;
	if(save_y<-0.2)
		MoveUP = true;
	if(!MoveUP)
		step_y = -step_y;
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	glBindTexture(GL_TEXTURE_2D, texture[11]);
	glBegin(GL_QUADS);
		glNormal3f(step_z, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x, 0.5f + save_y, z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x, 0.5f + save_y, z-step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x, 4.7f + save_y, z-step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x, 4.7f + save_y, z+step_z);
	glEnd();
	if(TurnFlag)
	{
		if(abs(g_Camera.m_vView.z - g_Camera.m_vPosition.z)<0.5f)
		{
			if(ch_direction == 'L' && (g_Camera.m_vView.x - g_Camera.m_vPosition.x) > 0)
				return true;
			if(ch_direction == 'R' && (g_Camera.m_vView.x - g_Camera.m_vPosition.x) < 0)
				return true;
		}
		if(ch_direction == 'L')
		{
			if(g_Camera.m_vView.z > g_Camera.m_vPosition.z)
				g_Camera.RotateFunc(3.5f*rotate_speed,true, true, FPS);
			else	g_Camera.RotateFunc(-3.5f*rotate_speed,true, true, FPS);
		}
		else 
		{
			if(g_Camera.m_vView.z < g_Camera.m_vPosition.z)
				g_Camera.RotateFunc(3.5f*rotate_speed,true, true, FPS);
			else	g_Camera.RotateFunc(-3.5f*rotate_speed,true, true, FPS);
		}
		if(g_Camera.m_vView.y < 4.0f && abs(g_Camera.m_vView.y - 4.0f)>0.5f)
			g_Camera.RotateFunc(rotate_speed,true, false, FPS);	
		else if(g_Camera.m_vView.y > 4.0f && abs(g_Camera.m_vView.y - 4.0f)>0.5f)
			g_Camera.RotateFunc(-rotate_speed,true, false, FPS);	
		return false;
	}
	save_y += step_y;
	return false;
}
bool DrawSCP2(float x, float z, char ch_direction, bool TurnFlag)	
{
	static int count = 0;
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 3.5f, z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 3.5f, z);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 4.7f, z);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 4.7f, z);
	glEnd();
	count++;
	if(count>FPS/2)
	{
		glBindTexture(GL_TEXTURE_2D, texture[12]);
		glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-2.0f, 0.0f, z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+2.0f, 0.0f, z);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+2.0f, 4.0f, z);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-2.0f, 4.0f, z);
		glEnd();
		if(count>FPS)
			count = 0;
	}	
	return false;
}
void DrawTitle(bool flag)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	if(flag)	glBindTexture(GL_TEXTURE_2D, texture[0]);
	else		glBindTexture(GL_TEXTURE_2D, texture[18]);
	glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(-2.5f, 3.0f, -10);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(+2.5f, 3.0f, -10);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(+2.5f, 6.0f, -10);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(-2.5f, 6.0f, -10);
	glEnd();
}
bool DrawSCP3(float scp_x3, float scp_z3, bool SCP3_left)
{
	float step_new = 20.0f/FPS;
	if(!SCP3_left)	step_new = -1.0f*step_new;
	static float step_old = 0.0f;
	static int count = 0;
	if(abs(g_Camera.m_vPosition.x - scp_x3) < 15.0f)
		step_old+=step_new;
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	if(count<FPS/2)	glBindTexture(GL_TEXTURE_2D, texture[16]);
	else	glBindTexture(GL_TEXTURE_2D, texture[17]);
	if(count>FPS)	count = 0;
	count++;
	glBegin(GL_QUADS);
		if(SCP3_left)	glNormal3f(1.0, 0.0f, 0.0f);
		else	glNormal3f(-1.0, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(scp_x3+step_old, 3.0f, scp_z3+1.5f);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(scp_x3+step_old, 3.0f, scp_z3-1.5f);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(scp_x3+step_old, 4.5f, scp_z3-1.5f);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(scp_x3+step_old, 4.5f, scp_z3+1.5f);
	glEnd();
	if(abs(scp_x3+step_old - g_Camera.m_vPosition.x)<2.0f)
	{
		step_old = 0.0f;
		return true;
	}
	return false;
}
// step_x - смещение по x, step_z - смещение по z
// flag - флаг очистки экрана, condition - идентификатор особых событий, -3 - идентификатор по умолчанию, никаких особых указаний
GLvoid DrawGLSceneCenter(float step_x, float step_z, bool flag, int condition=-3)				//центральная комната
{	
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if(flag)	g_Camera.CheckCameraCollision(QcollCenter, 4, 7);
	//glLightfv( GL_LIGHT0, GL_POSITION, g_LightPosition );
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
		
	for(int i=0; i<Center.count_polygon; i++)
	{		
		if((Center.QR[i].type_surface == 'd') || (Center.QR[i].type_surface == 'i') || (Center.QR[i].type_surface == 'l') || (Center.QR[i].type_surface == 'r'))	glBindTexture(GL_TEXTURE_2D, texture[7]);	//связывание текстуры
		else if(Center.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[8]);
		else glBindTexture(GL_TEXTURE_2D, texture[9]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Center.QR[i].Normal.x, Center.QR[i].Normal.y, Center.QR[i].Normal.z);
		//glMaterialfv(GL_FRONT, GL_DIFFUSE, front_color);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(BufferVertex[Center.QR[i].v1].x+step_x, BufferVertex[Center.QR[i].v1].y, BufferVertex[Center.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(BufferVertex[Center.QR[i].v2].x+step_x, BufferVertex[Center.QR[i].v2].y, BufferVertex[Center.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(BufferVertex[Center.QR[i].v3].x+step_x, BufferVertex[Center.QR[i].v3].y, BufferVertex[Center.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(BufferVertex[Center.QR[i].v4].x+step_x, BufferVertex[Center.QR[i].v4].y, BufferVertex[Center.QR[i].v4].z+step_z);	// Верх лево	
	glEnd();
	}
}

GLvoid DrawGLSceneTunnel(float step_x, float step_z, bool flag, int condition=-3)				//туннель 
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	if(flag && TunnelToLeft) g_Camera.CheckCameraCollision(QcollTunnelLeft, 4, 3);
	if(flag && !TunnelToLeft) g_Camera.CheckCameraCollision(QcollTunnelRight, 4, 3);
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);	
	for(int i=0; i<Tunnel.count_polygon; i++)
	{
		if((Tunnel.QR[i].type_surface == 'd') || (Tunnel.QR[i].type_surface == 'i'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//связывание текстуры
		else if(Tunnel.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Tunnel.QR[i].Normal.x, Tunnel.QR[i].Normal.y, Tunnel.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v1].x+step_x, BufferVertex[Tunnel.QR[i].v1].y, BufferVertex[Tunnel.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v2].x+step_x, BufferVertex[Tunnel.QR[i].v2].y, BufferVertex[Tunnel.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v3].x+step_x, BufferVertex[Tunnel.QR[i].v3].y, BufferVertex[Tunnel.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v4].x+step_x, BufferVertex[Tunnel.QR[i].v4].y, BufferVertex[Tunnel.QR[i].v4].z+step_z);	// Верх лево
		glEnd();
	}
}
GLvoid DrawGLSceneMain(float step_x, float step_z, bool flag, int condition=-3)				//главная комната
{
	
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(condition == 1)DrawGLSceneTunnel(step_x-13.0f, step_z-9.0f, false);
	else if(condition == -1) DrawGLSceneTunnel(step_x+43.0f, step_z-16.0f, false);
	else 
	{
		DrawGLSceneTunnel(step_x-13.0f, step_z-9.0f, false);
		DrawGLSceneTunnel(step_x+43.0f, step_z-16.0f, false);
	}
	glLoadIdentity();
	if(flag)	g_Camera.CheckCameraCollision(QcollMain, 4, 13);
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);	
	for(int i=0; i<MainRoom.count_polygon; i++)
	{
		if(MainRoom.QR[i].type_surface == 'l' || MainRoom.QR[i].type_surface == 'r' || MainRoom.QR[i].type_surface == 'd' || MainRoom.QR[i].type_surface == 'i')	glBindTexture(GL_TEXTURE_2D, texture[4]);	//связывание текстуры 
		else if(MainRoom.QR[i].type_surface == 't')	glBindTexture(GL_TEXTURE_2D, texture[5]);
		else glBindTexture(GL_TEXTURE_2D, texture[6]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glBegin(GL_QUADS);
		glNormal3f(MainRoom.QR[i].Normal.x, MainRoom.QR[i].Normal.y, MainRoom.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v1].x+step_x, BufferVertex[MainRoom.QR[i].v1].y, BufferVertex[MainRoom.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v2].x+step_x, BufferVertex[MainRoom.QR[i].v2].y, BufferVertex[MainRoom.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v3].x+step_x, BufferVertex[MainRoom.QR[i].v3].y, BufferVertex[MainRoom.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v4].x+step_x, BufferVertex[MainRoom.QR[i].v4].y, BufferVertex[MainRoom.QR[i].v4].z+step_z);	// Верх лево
	glEnd();
	}
}
GLvoid DrawGLSceneTurnR(float step_x, float step_z, bool flag, int condition=-3)				//поворот направо
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);	
	for(int i=0; i<TurnR.count_polygon; i++)
	{
		if((TurnR.QR[i].type_surface == 'd') || (TurnR.QR[i].type_surface == 'i') || (TurnR.QR[i].type_surface == 'l') || (TurnR.QR[i].type_surface == 'r'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//связывание текстуры
		else if(TurnR.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glBegin(GL_QUADS);
		glNormal3f(TurnR.QR[i].Normal.x, TurnR.QR[i].Normal.y, TurnR.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[TurnR.QR[i].v1].x+step_x, BufferVertex[TurnR.QR[i].v1].y, BufferVertex[TurnR.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[TurnR.QR[i].v2].x+step_x, BufferVertex[TurnR.QR[i].v2].y, BufferVertex[TurnR.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[TurnR.QR[i].v3].x+step_x, BufferVertex[TurnR.QR[i].v3].y, BufferVertex[TurnR.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[TurnR.QR[i].v4].x+step_x, BufferVertex[TurnR.QR[i].v4].y, BufferVertex[TurnR.QR[i].v4].z+step_z);	// Верх лево
	glEnd();
	}
}
GLvoid DrawGLSceneTurnL(float step_x, float step_z, bool flag, int condition=-3)				//поворот налево
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);		 	
	for(int i=0; i<TurnL.count_polygon; i++)
	{
		if((TurnL.QR[i].type_surface == 'd') || (TurnL.QR[i].type_surface == 'i') || (TurnL.QR[i].type_surface == 'l') || (TurnL.QR[i].type_surface == 'r'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//связывание текстуры
		else if(TurnL.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(TurnL.QR[i].Normal.x, TurnL.QR[i].Normal.y, TurnL.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[TurnL.QR[i].v1].x+step_x, BufferVertex[TurnL.QR[i].v1].y, BufferVertex[TurnL.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[TurnL.QR[i].v2].x+step_x, BufferVertex[TurnL.QR[i].v2].y, BufferVertex[TurnL.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[TurnL.QR[i].v3].x+step_x, BufferVertex[TurnL.QR[i].v3].y, BufferVertex[TurnL.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[TurnL.QR[i].v4].x+step_x, BufferVertex[TurnL.QR[i].v4].y, BufferVertex[TurnL.QR[i].v4].z+step_z);	// Верх лево
		glEnd();
	}
}
GLvoid DrawGLSceneTunnelRight(float step_x, float step_z, bool flag, int condition=-3)			//туннель с поворотом направо
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawGLSceneTurnR(step_x, step_z, false);
	glLoadIdentity();
	if(flag)
	{
		g_Camera.CheckCameraCollision(QcollTunnelLeftTurn, 4, 3);
		g_Camera.CheckCameraCollision(QcollTR, 4, 3);
	}
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);
	for(int i=0; i<Tunnel.count_polygon; i++)
	{
		if((Tunnel.QR[i].type_surface == 'd') || (Tunnel.QR[i].type_surface == 'i'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//связывание текстуры
		else if(Tunnel.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Tunnel.QR[i].Normal.x, Tunnel.QR[i].Normal.y, Tunnel.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v1].x+step_x, BufferVertex[Tunnel.QR[i].v1].y, BufferVertex[Tunnel.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v2].x+step_x, BufferVertex[Tunnel.QR[i].v2].y, BufferVertex[Tunnel.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v3].x+step_x, BufferVertex[Tunnel.QR[i].v3].y, BufferVertex[Tunnel.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v4].x+step_x, BufferVertex[Tunnel.QR[i].v4].y, BufferVertex[Tunnel.QR[i].v4].z+step_z);	// Верх лево
		glEnd();
	}
}
GLvoid DrawGLSceneTunnelLeft(float step_x, float step_z, bool flag, int condition=-3)					//туннель с поворотом налево
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawGLSceneTurnL(step_x, step_z, false);
	glLoadIdentity();
	if(flag)
	{
		g_Camera.CheckCameraCollision(QcollTunnelRightTurn, 4, 3);
		g_Camera.CheckCameraCollision(QcollTL, 4, 3);
	}
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);	
	for(int i=0; i<Tunnel.count_polygon; i++)
	{
		if((Tunnel.QR[i].type_surface == 'd') || (Tunnel.QR[i].type_surface == 'i'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//связывание текстуры
		else if(Tunnel.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Tunnel.QR[i].Normal.x, Tunnel.QR[i].Normal.y, Tunnel.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v1].x+step_x, BufferVertex[Tunnel.QR[i].v1].y, BufferVertex[Tunnel.QR[i].v1].z+step_z);	// Низ лево
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v2].x+step_x, BufferVertex[Tunnel.QR[i].v2].y, BufferVertex[Tunnel.QR[i].v2].z+step_z);	// Низ право
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v3].x+step_x, BufferVertex[Tunnel.QR[i].v3].y, BufferVertex[Tunnel.QR[i].v3].z+step_z);	// Верх право
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v4].x+step_x, BufferVertex[Tunnel.QR[i].v4].y, BufferVertex[Tunnel.QR[i].v4].z+step_z);	// Верх лево	
		glEnd();
	}
}

LRESULT CALLBACK WndProc(  HWND    hWnd,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam)
{
     RECT    Screen;         // используется позднее для размеров окна
     GLuint  PixelFormat;
     static  PIXELFORMATDESCRIPTOR pfd=
		{
		 sizeof(PIXELFORMATDESCRIPTOR),  // Размер этой структуры
		 1,                              // Номер версии (?)
		 PFD_DRAW_TO_WINDOW |            // Формат для Окна
		 PFD_SUPPORT_OPENGL |            // Формат для OpenGL
		 PFD_DOUBLEBUFFER,               // Формат для двойного буфера
		 PFD_TYPE_RGBA,                  // Требуется RGBA формат
		 16,                             // Выбор 16 бит глубины цвета
		 0, 0, 0, 0, 0, 0,                       // Игнорирование цветовых битов (?)
		 0,                              // нет буфера прозрачности
		 0,                              // Сдвиговый бит игнорируется (?)
		 0,                              // Нет буфера аккумуляции
		 0, 0, 0, 0,                             // Биты аккумуляции игнорируются (?)
		 16,                             // 16 битный Z-буфер (буфер глубины) 
		 0,                              // Нет буфера траффарета
		 0,                              // Нет вспомогательных буферов (?)
		 PFD_MAIN_PLANE,                 // Главный слой рисования
		 0,                              // Резерв (?)
		 0, 0, 0                         // Маски слоя игнорируются (?)
		};

	switch (message)        // Тип сообщения
	{
	case WM_CREATE:
		hDC = GetDC(hWnd);      // Получить контекст устройства для окна
		PixelFormat = ChoosePixelFormat(hDC, &pfd);	// Найти ближайшее совпадение для нашего формата пикселов       
		if (!PixelFormat)
		{
			MessageBox(0,L"Can't Find A Suitable PixelFormat"
			, L"Error" ,MB_OK|MB_ICONERROR);
			PostQuitMessage(0);	// Это сообщение говорит, что программа должна завершится
			break; 
		}
		if(!SetPixelFormat(hDC,PixelFormat,&pfd))
		{
			 MessageBox(0,L"Can't Set The  PixelFormat"
			 ,(LPCWSTR)"Error",MB_OK|MB_ICONERROR);
			 PostQuitMessage(0);
			 break;
		}
		hRC = wglCreateContext(hDC);
		if(!hRC)
		{
			MessageBox(0,L"Can't Create A GL Rendering Context"
			,(LPCWSTR)"Error",MB_OK|MB_ICONERROR);
			PostQuitMessage(0);
			break;
		}
//Сейчас мы имеем Контекст Рендеринга, и нам необходимо сделать его активным, для того чтобы OpenGL мог рисовать в окно
		if(!wglMakeCurrent(hDC, hRC))
        {
			MessageBox(0,L"Can't activate GLRC.",(LPCWSTR)"Error",MB_OK|MB_ICONERROR);
			PostQuitMessage(0);
			break;
        }
		GetClientRect(hWnd, &Screen);			//возвращает ширину и высоту окна
        InitGL(Screen.right, Screen.bottom);	//инициализируем область для рисования!!!! НАКОНЕЦ-ТО БЛИН
        break;
	case WM_DESTROY:
    case WM_CLOSE:
        ChangeDisplaySettings(NULL, 0);
        wglMakeCurrent(hDC,NULL);
        wglDeleteContext(hRC);
        ReleaseDC(hWnd,hDC);
        PostQuitMessage(0);
        break;
	case WM_KEYDOWN:		//вызывается всякий раз при нажатии клавиши
        keys[wParam] = TRUE;
        break;
	case WM_KEYUP:
	    keys[wParam] = FALSE;
        break;
	case WM_SIZE:
	    ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
        break;
	default:
		return (DefWindowProc(hWnd, message, wParam, lParam));
	}
return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, 
                 LPSTR lpCmdLine,int nCmdShow)
{
try
{
	int Width = 800;
	int Height = 600;
	if(!ReadParametrs(Width, Height))
		throw 'a';
	if(Width<640 || Height<480)
	{
		Width = 800;
		Height = 600;
	}
	char name[10]="Room.txt";
	ifstream ifs("Room.txt");
	if(!ifs.is_open())
		throw 5;
	if(!RoomScript(ifs, name))
		throw 5;
	ifs.close();
	InitializeOpenAL();
	MSG             msg;    // Структура сообщения Windows
	WNDCLASS        wc;	    // Структура класса Windows для установки типа окна
	HWND            hWnd;   // Сохранение дискриптора окна
	wc.style        = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc  = (WNDPROC) WndProc;
	wc.cbClsExtra   = 0;
	wc.cbWndExtra   = 0;
	wc.hInstance    = hInstance;
	wc.hIcon        = NULL;
	wc.hCursor      = NULL;
	wc.hbrBackground= NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName= L"SCP-106";
	if(!RegisterClass(&wc))
	{
		MessageBox(0, L"Failed To Register The Window Class"
		,(LPCWSTR)"Error",MB_OK|MB_ICONERROR);
		return FALSE;
	}
	 hWnd = CreateWindow(
         L"SCP-106",
         L"SCP-106", // Заголовок вверху окна
         WS_POPUP |
         WS_CLIPCHILDREN |
         WS_CLIPSIBLINGS,
         0, 0,                   // Позиция окна на экране
         Width, Height,          // Ширина и высота окна
         NULL,
         NULL,
         hInstance,
         NULL);
	if(!hWnd)
	{
		MessageBox(0, L"Window Creation Error.",(LPCWSTR)"Error",MB_OK|MB_ICONERROR); 
		return FALSE;
	}	//640 480
	DEVMODE dmScreenSettings;								// Режим работы
	memset(&dmScreenSettings, 0, sizeof(DEVMODE));          // Очистка для хранения установок
	dmScreenSettings.dmSize = sizeof(DEVMODE);              // Размер структуры Devmode
	dmScreenSettings.dmPelsWidth    = Width;                  // Ширина экрана
	dmScreenSettings.dmPelsHeight   = Height;                  // Высота экрана
	dmScreenSettings.dmFields       = DM_PELSWIDTH | DM_PELSHEIGHT; // Режим Пиксела
	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	// Переключение в полный экран!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	ShowCursor(FALSE);
	//создаём класс состояния игры и инициализируем сцены согласно схеме, указанной в описании класса!!!
	TextureDead = false;
	Game SCP;
	SCP.pointer_Scene[0] = &DrawGLSceneCenter;
	SCP.pointer_Scene[1] = &DrawGLSceneMain;
	SCP.pointer_Scene[2] = &DrawGLSceneTunnel;
	SCP.pointer_Scene[3] = &DrawGLSceneTunnelRight;
	SCP.pointer_Scene[4] = &DrawGLSceneTunnelLeft;
	//проводим начальную инициализацию сцен на экране!!!
	SCP.Scenes[0].pFunc = SCP.pointer_Scene[0];
	SCP.Scenes[1].pFunc = SCP.pointer_Scene[0];		//МЫ ЗДЕСЬ!!!
	SCP.Scenes[2].pFunc = SCP.pointer_Scene[1];
	SCP.Scenes[1].flag	= true;		//обозначаем начальную отрисовку в нашей сцене
	SCP.Scenes[0].z = 35.0f;
	SCP.Scenes[2].z = -35.0f;
	SCP.DrawMonster = DrawSCP;
	SCP.DrawMonster2 = DrawSCP2;
	remSnd* DEAD = new remSnd();
	remSnd* DeadTime = new remSnd();
	remSnd* ScarySCP = new remSnd();
	ifstream ifss("record.txt");
	if(!ifss.is_open())
		throw;
	int Record;
	ifss>>Record;
	ifss.close();
	if(!DEAD->Open("sounds/DEAD.ogg", false, false))
		throw false;
	if(!DeadTime->Open("sounds/TimeDead.ogg", false, false))
		throw false;
	if(!ScarySCP->Open("sounds/ScarySCP.ogg", false, false))
		throw false;
	const char step_array[] = {'A', 'D', 'W', 'S'};	//массив движения
	float param_speed = 1.2f;
	bool param_sp_flag = false;		//флаг изменения скорости
	int count_step=0;		// счётчик шагов
	int rand_num;			// случайная величина ;)
	GLvoid (*pointer_temp)(float , float , bool, int);
	float temp_x;		//временные величины для изменения координат комнат
	float temp_z;
	float scp_x3 = 0;	//величина, используемая для сцп3
	float scp_z3 = 0;
	char char_temp;		//используется для изменения положения комнат
	float scp_z = -30.0f;
	srand(time(NULL));
	bool TimeFlag = true;
	bool SCP3_flag = false;
	bool SCP3_left = false;
	remSnd* STEP = new remSnd();
	if(!STEP->Open("sounds/step.wav", false, false))
		throw false;
	int time1 = GetTickCount();
	int time2 = GetTickCount();
	g_Camera.m_vView.y +=1.0f;
	glDisable(GL_LIGHTING);
	while(time2-time1 < 6000)
	{
		CheckFPS();
		time2 = GetTickCount();
		SwapBuffers(hDC); 
		if(time2-time1 < 3000)	DrawTitle(true);
		else	DrawTitle(false);
	}
	g_Camera.m_vView.y -=1.0f;
	glEnable(GL_LIGHTING);
	while (1)
	{
		try
		{
		STEP->vPos.InitVector(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z);
		CheckFPS();
		// Обработка всех сообщений
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				return TRUE;
			}
		}
		SCP.ChangeRoom = false;
		// Обработка состояния сцен на ЭКРАНЕ!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		switch(SCP.SceneType[1])	//проверяем наше местоположение
		{
		case 'C':					//мы находимся в центральной комнате
			//SCP.ChangeRoom = false;
			//возможен только один вариант дальнейшего развития событий,
			//проверяем на пересечение дальней двери
			if((SCP.Scenes[1].z - 36.0f) > g_Camera.m_vPosition.z)
			{	//если граница пересечена, то меняем сцены
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
				SCP.SceneType[1] = SCP.SceneType[2];
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].z -= 35.0f;				//изменили комнату впереди
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			break;
		case 'M':				//мы находимся в главной комнате!!!
			//возможны три варианта развития событий
			if((SCP.Scenes[1].z - 36.0f) > g_Camera.m_vPosition.z)
			{	//мы прошли вперёд, меняем сцены
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
				SCP.SceneType[1] = SCP.SceneType[2];
				//запомнили всю информацию о нашей сцене			
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].z -= 35.0f;				//изменили комнату впереди
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			else if((SCP.Scenes[1].x - 14.0f) > g_Camera.m_vPosition.x)
			{	//мы прошли в левый туннель, меняем сцены
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//запомнили всю информацию о нашей сцене
				SCP.Scenes[1].pFunc = SCP.pointer_Scene[2];
				SCP.Scenes[1].x -= 13.0f;
				SCP.Scenes[1].z -= 9.0f;
				SCP.SceneType[1] = 'T';
				//установили текущую сцену как туннель 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-1;
				if(rand_num == 0)
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[2];
					SCP.SceneType[2] = 'T';
				}			
				else
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[3];
					SCP.SceneType[2] = 'R';
				}
				SCP.Scenes[2].x = SCP.Scenes[1].x - 30.0f;
				SCP.Scenes[2].z = SCP.Scenes[1].z;
				SCP.TunnelToLeft = true;
			}
			else if((SCP.Scenes[1].x + 14.0f) < g_Camera.m_vPosition.x)
			{	//мы прошли в правый туннель, меняем сцены
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//запомнили всю информацию о нашей сцене
				SCP.Scenes[1].pFunc = SCP.pointer_Scene[2];
				SCP.Scenes[1].x += 43.0f;
				SCP.Scenes[1].z -= 16.0f;
				SCP.SceneType[1] = 'T';
				//установили текущую сцену как туннель 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=1;
				if(rand_num == 0)
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[2];
					SCP.SceneType[2] = 'T';
				}
				else
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[4];
					SCP.SceneType[2] = 'L';
				}
				SCP.Scenes[2].x = SCP.Scenes[1].x + 30.0f;
				SCP.Scenes[2].z = SCP.Scenes[1].z;
				SCP.TunnelToLeft = false;
			}
			break;
		case 'T':			//мы находимся в туннеле!!!!
			if(SCP.TunnelToLeft && (SCP.Scenes[1].x - 31.0f > g_Camera.m_vPosition.x) && SCP.SceneType[2]=='T')
			{	//мы находимся в левом туннеле и пересекли границу
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//запомнили всю информацию о нашей сцене
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
				SCP.SceneType[1] = SCP.SceneType[2];
				//установили текущую сцену как туннель 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				if(rand_num == 0)
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[2];
					SCP.SceneType[2] = 'T';
				}
				else
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[3];
					SCP.SceneType[2] = 'R';
				}
				SCP.Scenes[2].x = SCP.Scenes[1].x - 30.0f;
				SCP.Scenes[2].z = SCP.Scenes[1].z;
			}
			else if(!SCP.TunnelToLeft && (SCP.Scenes[1].x+1.0f  < g_Camera.m_vPosition.x) && SCP.SceneType[2]=='T')
			{	//мы находимся в правом туннеле и пересекли границу
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//запомнили всю информацию о нашей сцене
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
				SCP.SceneType[1] = SCP.SceneType[2];
				//установили текущую сцену как туннель 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				if(rand_num == 0)
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[2];
					SCP.SceneType[2] = 'T';
				}
				else
				{
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[4];
					SCP.SceneType[2] = 'L';
				}
				SCP.Scenes[2].x = SCP.Scenes[1].x + 30.0f;
				SCP.Scenes[2].z = SCP.Scenes[1].z;
			}
			else if(!SCP.TunnelToLeft && (SCP.Scenes[1].x+1.0f  < g_Camera.m_vPosition.x) && SCP.SceneType[2]=='L')
			{	//мы находимся в правом туннеле и пересекли границу (сцена 2 явл поворотом)
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//запомнили всю информацию о нашей сцене
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
				SCP.SceneType[1] = SCP.SceneType[2];
				//установили текущую сцену как туннель с поворотом 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].x += 2.0f;
				SCP.Scenes[2].z -= 15.0f;				//изменили комнату впереди
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			else if(SCP.TunnelToLeft && (SCP.Scenes[1].x - 31.0f > g_Camera.m_vPosition.x) && SCP.SceneType[2]=='R')
			{	//мы находимся в левом туннеле и пересекли границу(сцена 2 явл поворотом)
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//запомнили всю информацию о нашей сцене
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
				SCP.SceneType[1] = SCP.SceneType[2];
				//установили текущую сцену как туннель 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].x -= 32.0f;
				SCP.Scenes[2].z -= 15.0f;				//изменили комнату впереди
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			break;
		case 'R':		//мы находимся в левом туннеле с поворотом направо!!!
				if((SCP.Scenes[1].z - 16.0f > g_Camera.m_vPosition.z))
				{
					SCP.ChangeRoom = true;
					rand_num = rand()%2;
					char_temp = SCP.SceneType[1];
					pointer_temp = SCP.Scenes[1].pFunc;
					temp_x = SCP.Scenes[1].x;
					temp_z = SCP.Scenes[1].z;
					SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
					SCP.Scenes[1].x = SCP.Scenes[2].x;
					SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
					SCP.SceneType[1] = SCP.SceneType[2];
					//запомнили всю информацию о нашей сцене			
					SCP.SceneType[0] = char_temp;
					SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
					SCP.Scenes[0].x = temp_x;
					SCP.Scenes[0].z = temp_z;
					SCP.Scenes[0].condition=-3;
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
					SCP.Scenes[2].z -= 35.0f;				//изменили комнату впереди
					if(rand_num == 0) SCP.SceneType[2] = 'C';
					else SCP.SceneType[2] = 'M';
				}
				break;
		case 'L':		//мы находимся в туннеле с поворотом налево
				if((SCP.Scenes[1].z - 16.0f > g_Camera.m_vPosition.z))
				{
					SCP.ChangeRoom = true;
					rand_num = rand()%2;
					char_temp = SCP.SceneType[1];
					pointer_temp = SCP.Scenes[1].pFunc;
					temp_x = SCP.Scenes[1].x;
					temp_z = SCP.Scenes[1].z;
					SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
					SCP.Scenes[1].x = SCP.Scenes[2].x;
					SCP.Scenes[1].z = SCP.Scenes[2].z;		//изменили наше текущее положение
					SCP.SceneType[1] = SCP.SceneType[2];
					//запомнили всю информацию о нашей сцене			
					SCP.SceneType[0] = char_temp;
					SCP.Scenes[0].pFunc = pointer_temp;		//изменили последнюю комнату
					SCP.Scenes[0].x = temp_x;
					SCP.Scenes[0].z = temp_z;
					SCP.Scenes[0].condition=-3;
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
					SCP.Scenes[2].z -= 35.0f;				//изменили комнату впереди
					if(rand_num == 0) SCP.SceneType[2] = 'C';
					else SCP.SceneType[2] = 'M';
				}
		default:
			break;
		}		
		if(SCP.ChangeRoom == true)		//если мы сменили комнату, то
		{
			TimeFlag = !TimeFlag;
			SCP.ChangeRoom = false;
			switch(SCP.SceneType[1])
			{
			case 'C':	//перешли в центральную комнату, изменяем координаты полигонов проверки коллизии
				for(int i=0; i<7; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollCenter[i].Quad[j].x += SCP.Scenes[1].x - CenterColl_x;		//к существующим параметрам прибавляем приращение 
						QcollCenter[i].Quad[j].z += SCP.Scenes[1].z - CenterColl_z;		//необходимого параметра и старого
					}
				}
				CenterColl_x = SCP.Scenes[1].x;
				CenterColl_z = SCP.Scenes[1].z;		//запоминаем старые параметры
				break;
			case 'M':
				for(int i=0; i<13; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollMain[i].Quad[j].x += SCP.Scenes[1].x - MainColl_x;		//к существующим параметрам прибавляем приращение 
						QcollMain[i].Quad[j].z += SCP.Scenes[1].z - MainColl_z;		//необходимого параметра и старого
					}
				}
				MainColl_x = SCP.Scenes[1].x;
				MainColl_z = SCP.Scenes[1].z;		//запоминаем старые параметры
				break;
			case 'T':
				if(SCP.TunnelToLeft)	//двигаемся влево по туннелю
				{
					TunnelToLeft = true;
					for(int i=0; i<3; i++)
					{
						for(int j=0; j<4; j++)
						{
							QcollTunnelLeft[i].Quad[j].x += SCP.Scenes[1].x - LeftTunnelTemp_x;		//к существующим параметрам прибавляем приращение 
							QcollTunnelLeft[i].Quad[j].z += SCP.Scenes[1].z - LeftTunnelTemp_z;		//необходимого параметра и старого
						}
					}
					LeftTunnelTemp_x = SCP.Scenes[1].x;
					LeftTunnelTemp_z = SCP.Scenes[1].z;		//запоминаем старые параметры
				}
				else
				{
					TunnelToLeft = false;
					for(int i=0; i<3; i++)
					{
						for(int j=0; j<4; j++)
						{
							QcollTunnelRight[i].Quad[j].x += SCP.Scenes[1].x - RightTunnelTemp_x;		//к существующим параметрам прибавляем приращение 
							QcollTunnelRight[i].Quad[j].z += SCP.Scenes[1].z - RightTunnelTemp_z;		//необходимого параметра и старого
						}
					}
					RightTunnelTemp_x = SCP.Scenes[1].x;
					RightTunnelTemp_z = SCP.Scenes[1].z;		//запоминаем старые параметры
				}
				break;
			case 'R':
				TunnelToLeft = true;
				for(int i=0; i<3; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollTunnelLeftTurn[i].Quad[j].x += SCP.Scenes[1].x - TunnelLeftTurn_x;		//к существующим параметрам прибавляем приращение 
						QcollTunnelLeftTurn[i].Quad[j].z += SCP.Scenes[1].z - TunnelLeftTurn_z;		//необходимого параметра и старого
						QcollTR[i].Quad[j].x += SCP.Scenes[1].x - TRcoll_x;
						QcollTR[i].Quad[j].z += SCP.Scenes[1].z - TRcoll_z;
					}
				}
				TunnelLeftTurn_x = SCP.Scenes[1].x;
				TunnelLeftTurn_z = SCP.Scenes[1].z;	
				//запоминаем старые параметры
				TRcoll_x = SCP.Scenes[1].x;
				TRcoll_z = SCP.Scenes[1].z;	
				break;
			case 'L':
				TunnelToLeft = false;
				for(int i=0; i<3; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollTunnelRightTurn[i].Quad[j].x += SCP.Scenes[1].x - TunnelRightTurn_x;		//к существующим параметрам прибавляем приращение 
						QcollTunnelRightTurn[i].Quad[j].z += SCP.Scenes[1].z - TunnelRightTurn_z;		//необходимого параметра и старого
						QcollTL[i].Quad[j].x += SCP.Scenes[1].x - TLcoll_x;
						QcollTL[i].Quad[j].z += SCP.Scenes[1].z - TLcoll_z;
					}
				}
				TunnelRightTurn_x = SCP.Scenes[1].x;
				TunnelRightTurn_z = SCP.Scenes[1].z;
				TLcoll_x = SCP.Scenes[1].x;
				TLcoll_z = SCP.Scenes[1].z;
				break;
			default:
				break;
			}
			///////////////////////////////////////////////  ЗВУК  ///////////////////////////////////////////////////////
			//генерируем случайный звук!
			rand_num = rand()%3;
			SCP.ChangeSoundsOrientation();	//очистили старые звуки
			if(rand_num==0)	//добавляем новый фоновый звук в нашу комнату
			{
				rand_num = rand()%13;
				SCP.InitialSoundsInRoom(1, true, ArraySounds[rand_num]);
			}
			else if(rand_num==1)//добавляем новый фоновый звук в дальнюю комнату
			{
				rand_num = rand()%13;
				SCP.InitialSoundsInRoom(2, true, ArraySounds[rand_num]);
			}
			else 	//добавляем новый резкий звук в нашу комнату
			{
				rand_num = rand()%21;
				if(rand_num>13 && rand_num<21)
					SCP.InitialSoundsInRoom(1, false, ArraySounds[rand_num]);
				else if(rand_num<6)
					SCP.InitialSoundsInRoom(1, false, ArraySounds[rand_num+14]);
			}
			//создали новый звук!
			if(SCP.SceneType[1] == 'T')
			{
				rand_num = rand()%4;
				if(rand_num == 3)	//генерируем сцп3
				{
					if(SCP.TunnelToLeft)
					{
						SCP3_left = true;
						scp_x3 = g_Camera.m_vPosition.x-25.0f;
					}
					else{
						SCP3_left = false;
						scp_x3 = g_Camera.m_vPosition.x+25.0f;
					}
					scp_z3 = g_Camera.m_vPosition.z;
					SCP3_flag = true;				
				}
			}
			//проверяем возможность создать шкатулку пандоры
			if(SCP.PandoraSost == 0 && SCP.SceneType[1] == 'C')
			{
				int temp = rand()%4;
				if(temp == 0)
				{
					SCP.PandoraSost = -1;	//пандора в состоянии готовности!
					SCP.pandora_x = BufferVertex[20].x + SCP.Scenes[1].x+2.0f;
					SCP.pandora_z = BufferVertex[20].z + SCP.Scenes[1].z;
				}
			}				
		}
		SCP.cGameSounds.UpdateAll();
		SCP.ControlAllSounds(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z);
		//контролируем изменение координат
		SCP.cGameSounds.ActivateSounds();		//активируем звуки
		SCP.Scenes[1].pFunc(SCP.Scenes[1].x, SCP.Scenes[1].z, SCP.Scenes[1].flag, SCP.Scenes[1].condition);
		SCP.Scenes[0].pFunc(SCP.Scenes[0].x, SCP.Scenes[0].z, SCP.Scenes[0].flag, SCP.Scenes[0].condition);
		SCP.Scenes[2].pFunc(SCP.Scenes[2].x, SCP.Scenes[2].z, SCP.Scenes[2].flag, SCP.Scenes[2].condition);
		if(SCP3_flag)
		{	//рисуем сцп3		
			if(DrawSCP3(scp_x3, scp_z3, SCP3_left))
			{
				ScarySCP->mPos[0] = g_Camera.m_vPosition.x;
				ScarySCP->mPos[1] = g_Camera.m_vPosition.y;
				ScarySCP->mPos[2] = g_Camera.m_vPosition.z;
				SCP3_flag = false;
				ScarySCP->Play();
			}
		}
		if(SCP.PandoraSost == -1)	//готовы к действию
		{
			DrawPandora(SCP.pandora_x, SCP.pandora_z);
			//проверяем на приближение
			if(abs(SCP.pandora_z - g_Camera.m_vPosition.z)<1.0f)
				SCP.PandoraSost = 1;	//активируем пандору!
		}
		else if(SCP.PandoraSost == 1)
		{
			int temp = rand()%3 + 2;
			SCP.PandoraSost = temp;
			SCP.PandoraTime = (float)GetTickCount();
		}
		else if(SCP.PandoraSost == 2)
		{	//увеличиваем скорость!
			SCP.PandoraSost = 5;
			light_diffuse[0] = 0.0f;
			light_diffuse[1] = 1.0f;
			light_diffuse[2] = 0.0f;
			glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
			param_speed = param_speed*2;
		}
		else if(SCP.PandoraSost == 3)
		{	//уменьшаем скорость!
			light_diffuse[0] = 0.0f;
			light_diffuse[1] = 0.0f;
			light_diffuse[2] = 1.0f;
			glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
			param_speed = param_speed/2;
			SCP.PandoraSost = 6;
		}
		else if(SCP.PandoraSost == 4)
		{	//смерть!
			SCP.pandora_z = SCP.pandora_z+0.00004f*FPS;
			DrawSCP2(SCP.pandora_x, SCP.pandora_z-4.0f, 'A', false);	
			SCP.SCP_106.DeadTime = true;
			DEAD->mPos[0]=g_Camera.m_vPosition.x;
			DEAD->mPos[1]=g_Camera.m_vPosition.y;
			DEAD->mPos[2]=g_Camera.m_vPosition.z;
			if(Dead(DEAD))
			{
				_TCHAR message[512]={0};
				if(TimeGame > Record)
				{
					ofstream ofs("record.txt");
					Record = TimeGame;
					ofs << Record;
				}
				_stprintf(message,TEXT("Вы проиграли, ваше время выживания %d.\nРекорд %d.\nYou died. Your survival time is %d. The best time is %d"),TimeGame, TimeGame, Record, Record);
				MessageBox(0, message,	L"End",MB_OK|MB_SERVICE_NOTIFICATION);
				SendMessage(hWnd,WM_CLOSE,0,0); 
			}
		}
		else if(SCP.PandoraSost == 5)
		{
			float temp =(float)GetTickCount();
			if(abs(temp - SCP.PandoraTime)>10000.0f)
			{
				light_diffuse[0] = 0.7f;
				light_diffuse[1] = 0.7f;
				light_diffuse[2] = 0.7f;
				glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
				param_speed = param_speed/2;
				SCP.PandoraSost = 0;
				SCP.PandoraTime = 0;
			}
		}
		else if(SCP.PandoraSost == 6)
		{
			float temp = (float)GetTickCount();
			if(abs(temp - SCP.PandoraTime)>10000.0f)
			{
				light_diffuse[0] = 0.7f;
				light_diffuse[1] = 0.7f;
				light_diffuse[2] = 0.7f;
				glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
				param_speed = param_speed*2;
				SCP.PandoraSost = 0;
				SCP.PandoraTime = 0;
			}
		}
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //Переключить буфер экрана
		if (keys[VK_ESCAPE]) SendMessage(hWnd,WM_CLOSE,0,0);    // Если ESC - выйти
		if(SCP.SCP_106.Wait)
		{
			if(SCP.SCP_106.ch_direction=='M' && abs(g_Camera.m_vPosition.z - SCP.SCP_106.coord_activate) < 1.0f)
				SCP.SCP_106.MonsterActivated = true;
			else if(SCP.SCP_106.ch_direction!='M' && abs(g_Camera.m_vPosition.x - SCP.SCP_106.coord_activate) < 1.0f)
				SCP.SCP_106.MonsterActivated = true;
		}
		if(SCP.SCP_106.MonsterActivated)	//если монстр активирован, проверяем на приближение
		{			
			if(SCP.SCP_106.MonsterFunction(FPS))	//если true, то всё, капец!
			{
				SCP.SCP_106.DeadTime = true;
				int Record;
				ifs>>Record;
				DEAD->mPos[0]=g_Camera.m_vPosition.x;
				DEAD->mPos[1]=g_Camera.m_vPosition.y;
				DEAD->mPos[2]=g_Camera.m_vPosition.z;
				if(Dead(DEAD))
				{
					_TCHAR message[512]={0};
					if(TimeGame > Record)
					{
						ofstream ofs("record.txt");
						Record = TimeGame;
						ofs << Record;
					}
					_stprintf(message,TEXT("Вы проиграли, ваше время выживания %d.\nРекорд %d.\nYou died. Your survival time is %d. The best time is %d"),TimeGame, TimeGame, Record, Record);
					MessageBox(0, message,	L"The End",MB_OK|MB_SERVICE_NOTIFICATION);
					SendMessage(hWnd,WM_CLOSE,0,0); 
				}
			}
		}
		SwapBuffers(hDC); 
		if(SCP.SCP_106.TurnFlag)
			continue;
		if(SCP.SCP_106.DeadTime)
			continue;
		if(TimeCheck(TimeFlag, DeadTime))
		{
			DEAD->mPos[0]=g_Camera.m_vPosition.x;
			DEAD->mPos[1]=g_Camera.m_vPosition.y;
			DEAD->mPos[2]=g_Camera.m_vPosition.z;
			DEAD->vPos.InitVector(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z);
			if(Dead(DEAD))
			{
				int Record;
				ifs>>Record;
				_TCHAR message[512]={0};
				if(TimeGame > Record)
				{
					ofstream ofs("record.txt");
					Record = TimeGame;
					ofs << Record;
				}
				_stprintf(message,TEXT("Вы проиграли, ваше время выживания %d.\nРекорд %d.\nYou died. Your survival time is %d. The best time is %d"),TimeGame, TimeGame, Record, Record);
				MessageBox(0, message,	L"The End",MB_OK|MB_SERVICE_NOTIFICATION);
				SendMessage(hWnd,WM_CLOSE,0,0); 
			}
			continue;
		}
		
		g_Camera.SetViewByMouse();
		for(int i=0; i<4; i++)
			if(keys[step_array[i]])  count_step++;
		if(count_step == 1)			//нажата только одна клавиша движения
			g_Camera.MoveMode = 0;
		else if(keys['W'] && keys['S'] || keys['A'] && keys['D'])
			g_Camera.MoveMode = 2;	//нажаты две противоречивые клавиши, стоим на месте
		else	g_Camera.MoveMode = 1;
		if(g_Camera.m_vPosition.y<4.0 && count_step>0)
			STEP->Play();
		count_step = 0;		
		if(keys['A']) 
			g_Camera.MovementFunc(true, false, FPS, param_speed);
		if(keys['D'])
			g_Camera.MovementFunc(false, false, FPS, param_speed);
		if(keys['W'])	// Если нажали "вперёд"
			g_Camera.MovementFunc(false, true, FPS, param_speed);							
		if(keys['S'])	// Если "назад"
			g_Camera.MovementFunc(true, true, FPS, param_speed);		
		if(keys[VK_LEFT])	// Если нажали "влево"
			g_Camera.RotateFunc(rotate_speed,true, true, FPS);
		if(keys[VK_RIGHT])   // Если "вправо"
			g_Camera.RotateFunc(-rotate_speed,true, true, FPS);
		if(keys[VK_UP]) 						
			g_Camera.RotateFunc(rotate_speed,true, false, FPS);	
		if(keys[VK_DOWN]) 	
			g_Camera.RotateFunc(-rotate_speed,false, false, FPS);
     
		 }
		 catch(int )
		 {
			 MessageBox(0, L"Ошибка открытия звукового файла\nCant open audio file.",
				L"Error",MB_OK|MB_ICONERROR);
			 SendMessage(hWnd,WM_CLOSE,0,0); 
		 }
	 }
	 }
catch(int )
{
	 MessageBox(0, L"Неизвестная ошибка при считывании файла Room.txt\nЗа помощью обратитесь к разработчику!\nCant open or reade file Room.txt",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
catch(char )
{
	 MessageBox(0, L"Неизвестная ошибка при считывании файла Options.txt\nПроверьте корректность содержимого файла!\nCant open or reade file Options.txt",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
catch(bool )
{
	 MessageBox(0, L"Ошибка открытия звукового файла\nCant open audio file.",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
catch(...)
{
	 MessageBox(0, L"Неизвестная ошибка!\nЗа помощью обратитесь к разработчику!\nUnknown error!",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
}
