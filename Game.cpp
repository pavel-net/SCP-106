#include <time.h>
#include "engine_header.h"		
#include "sounds.h"
#include "init_header.h"
#include <tchar.h>
#pragma comment(lib, "Opengl32.lib")
#pragma comment(lib, "GLu32.lib")
#pragma comment(lib, "glaux.lib")
#define rotate_speed  0.14f			//�������� �������� 
#define tex_count 19				//���-�� �������

bool TextureDead;
CCamera  g_Camera;
int FPS = 1200;
int TimeGame;
float light_ambient[] = {0.2f,0.2f,0.2f,1.0f};	//����������  � ��������� ����
float light_diffuse[] = {0.8f,0.8f,0.8f,1.0f};
float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
float specular[] = {0.8f, 0.8f, 0.8f, 1.0f};	//���������� �� ��������� ����
float specref[] = {1.0f, 1.0f, 1.0f, 1.0f};

float radius = 3.0f;	//������ ������������
float att = 1.5f;		//���������� ��������, �������� �� �������� �����������
float kQ  = att / (2.5f* radius * radius);
float kL = att / (2.5f * radius);		//������������ ����������� �����
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
static HGLRC hRC;              // ���������� �������� ����������
static HDC hDC;                // ��������� �������� ���������� GDI
BOOL   keys[256];              // ������ ��� ��������� ��������� ����������
bool TunnelToLeft;			   // ����������� ���� ��������� �������� �� �������

float g_FogDensity = 0.5f;       // ��������� ����
GLfloat fogColor[4]= {0.2f, 0.2f, 0.2f, 1.0f}; // ���� ������

GLuint texture[tex_count];								//����� ��� �������
LPCWSTR stexture[19] = {L"textures/scp.bmp", L"textures/TunWall2.bmp", L"textures/TunPot2.bmp", L"textures/TunPol2.bmp",
						L"textures/MainWall3.bmp", L"textures/MainPot3.bmp", L"textures/MainPol.bmp",
						L"textures/CenterWall.bmp", L"textures/CenterPot.bmp", L"textures/CenterPol3.bmp", L"textures/scp-106.bmp",
						L"textures/scp-1.bmp", L"textures/scp-2.bmp", L"textures/scp-3.bmp", L"textures/dead.bmp", L"textures/Pandora.bmp",
						L"textures/scp-6.bmp", L"textures/scp-5.bmp", L"textures/nvidia.bmp"};	//������ ��� ������ ��� �������
const char* ArraySounds[] = {"sounds/fTrack1.ogg", "sounds/fTrack2.ogg", "sounds/fTrack3.ogg", "sounds/fTrack4.ogg", "sounds/fTrack5.ogg",
						"sounds/fTrack6.ogg", "sounds/fTrack7.ogg", "sounds/fTrack8.ogg", "sounds/fTrack9.ogg", "sounds/fTrack10.ogg",
						"sounds/fTrack11.ogg", "sounds/fTrack12.ogg", "sounds/fTrack13.ogg",
						"sounds/wTrack1.ogg", "sounds/wTrack2.ogg", "sounds/wTrack3.ogg", "sounds/wTrack4.ogg", "sounds/wTrack5.ogg",
						"sounds/wTrack6.ogg", "sounds/wTrack7.ogg", "sounds/wTrack8.ogg"}; 

// �������� �������� � ��������������� � ��������
GLvoid LoadGLTextures()
{
	for(int i=0; i<tex_count; i++)
	{
		AUX_RGBImageRec* texture1=NULL;				//���������� � ��������
		texture1 = auxDIBImageLoad(stexture[i]);	//��������� ����������
		if(!texture1)
			exit(1);
		glGenTextures(tex_count, &texture[i]);		//���������� ���������� � �������� � ����������� ����� (� i-�� ��������)
		glBindTexture(GL_TEXTURE_2D, texture[i]);	//�������� ��������� �������� � ���������
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
	LoadGLTextures();			//�������� �������
	glEnable(GL_TEXTURE_2D);	//���������� ��������� ��������
	g_Camera.PositionCamera(0.0, 4.0, -2.0,   0.0, 3.0f, -5.0f,   0.0, 1.0f, 0.0);
	float light_position[] = {g_Camera.m_vPosition.x, g_Camera.m_vPosition.y-3.2f, g_Camera.m_vPosition.z+1.5f, 1.0f};
	g_Camera.radius=1.0f;
	glClearDepth(1.0);                      // ��������� ������� ������ �������
	glDepthFunc(GL_LESS);                   // ��� ����� �������
	glEnable(GL_DEPTH_TEST);                // ��������� ���� �������
	glShadeModel(GL_SMOOTH);        // ��������� ������� �������� �����������
	glMatrixMode(GL_PROJECTION);    // ����� ������� ��������
	glLoadIdentity();               // ����� ������� ��������
	gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f);	// ��������� ����������� �������������� �������� ��� ����						 
	glMatrixMode(GL_MODELVIEW);     // ����� ������� ��������� ������	
	GLfloat fogColor[4]= {0.0f, 0.0f, 0.0f, 1.0f};	//����� �����
	
	CVector3* Direct = g_Camera.m_vView.operator -(&g_Camera.m_vPosition);	//����������� �����
	Direct->Normalization();	
	Direct->x = Direct->x/8;
	Direct->y = Direct->y/8;
	Direct->z = Direct->z/8;
	glEnable(GL_LIGHT0);		//��� ��� ������� �������� ���������
	glEnable(GL_LIGHTING);		//��������� �� ����� ������������ ���������

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);	//��������� ���������� ��������� �����
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, kC);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, kL);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, kQ);


	glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, light_diffuse);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // ������� ������ � ����/////////////////////////////////////////////////////////////////////////
	glEnable(GL_FOG);                       // �������� ����� (GL_FOG)
	glFogi(GL_FOG_MODE, GL_LINEAR);			// �������� ��� ������
	glFogfv(GL_FOG_COLOR, fogColor);        // ������������� ���� ������
	glFogf(GL_FOG_DENSITY, 1.0f);           // ��������� ������ ����� �����
	glHint(GL_FOG_HINT, GL_DONT_CARE);      // ��������������� ��������� ������
	glFogf(GL_FOG_START, 12.0f);            // �������, � ������� ���������� �����
	glFogf(GL_FOG_END, 15.0f);				// �������, ��� ����� �������������.
}

//������� ����������������, ���������� ����� �������� ������ ����

GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)	
{
     if (Height==0)          // �������������� ������� �� ����, ���� ���� ������� ����
             Height=1;
     glViewport(0, 0, Width, Height);		 // ����� ������� ������� ������ � ������������� ��������������            
     glMatrixMode(GL_PROJECTION);            // ����� ������� ��������
     glLoadIdentity();                       // ����� ������� ��������
     gluPerspective(45.0f,(GLfloat)Width/(GLfloat)Height,0.1f,100.0f); // ���������� ����������� �������������� �������� ��� ����           
     glMatrixMode(GL_MODELVIEW);			// ����� ������� ��������� ������
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
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z+1.0f);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z-1.0f);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z-1.0f);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z+1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z-1.0f);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z+1.0f);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z+1.0f);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z-1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z+1.0f);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z+1.0f);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z+1.0f);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z+1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x+1.0f, 1.0f, z-1.0f);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x-1.0f, 1.0f, z-1.0f);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z-1.0f);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z-1.0f);
	glEnd();
	glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 3.0f, z+1.0f);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 3.0f, z+1.0f);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 3.0f, z-1.0f);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 3.0f, z-1.0f);
	glEnd();
}

bool DrawSCP(float x, float z, char ch_direction, bool TurnFlag)	
{
	static bool MoveUP = true;
	static float save_y = 0.0f;
	float step_y = 0.5f/FPS;
	float step_z = 2.0f;
	if(ch_direction == 'L')		//������ ��������� �����
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
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x, 0.5f + save_y, z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x, 0.5f + save_y, z-step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x, 4.7f + save_y, z-step_z);	// ���� �����
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
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-1.0f, 3.5f, z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+1.0f, 3.5f, z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+1.0f, 4.7f, z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(x-1.0f, 4.7f, z);
	glEnd();
	count++;
	if(count>FPS/2)
	{
		glBindTexture(GL_TEXTURE_2D, texture[12]);
		glBegin(GL_QUADS);
		glNormal3f(0.0, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(x-2.0f, 0.0f, z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(x+2.0f, 0.0f, z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(x+2.0f, 4.0f, z);	// ���� �����
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
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(-2.5f, 3.0f, -10);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(+2.5f, 3.0f, -10);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(+2.5f, 6.0f, -10);	// ���� �����
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
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(scp_x3+step_old, 3.0f, scp_z3+1.5f);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(scp_x3+step_old, 3.0f, scp_z3-1.5f);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(scp_x3+step_old, 4.5f, scp_z3-1.5f);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(scp_x3+step_old, 4.5f, scp_z3+1.5f);
	glEnd();
	if(abs(scp_x3+step_old - g_Camera.m_vPosition.x)<2.0f)
	{
		step_old = 0.0f;
		return true;
	}
	return false;
}
// step_x - �������� �� x, step_z - �������� �� z
// flag - ���� ������� ������, condition - ������������� ������ �������, -3 - ������������� �� ���������, ������� ������ ��������
GLvoid DrawGLSceneCenter(float step_x, float step_z, bool flag, int condition=-3)				//����������� �������
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
		if((Center.QR[i].type_surface == 'd') || (Center.QR[i].type_surface == 'i') || (Center.QR[i].type_surface == 'l') || (Center.QR[i].type_surface == 'r'))	glBindTexture(GL_TEXTURE_2D, texture[7]);	//���������� ��������
		else if(Center.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[8]);
		else glBindTexture(GL_TEXTURE_2D, texture[9]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Center.QR[i].Normal.x, Center.QR[i].Normal.y, Center.QR[i].Normal.z);
		//glMaterialfv(GL_FRONT, GL_DIFFUSE, front_color);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(BufferVertex[Center.QR[i].v1].x+step_x, BufferVertex[Center.QR[i].v1].y, BufferVertex[Center.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(BufferVertex[Center.QR[i].v2].x+step_x, BufferVertex[Center.QR[i].v2].y, BufferVertex[Center.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(BufferVertex[Center.QR[i].v3].x+step_x, BufferVertex[Center.QR[i].v3].y, BufferVertex[Center.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(BufferVertex[Center.QR[i].v4].x+step_x, BufferVertex[Center.QR[i].v4].y, BufferVertex[Center.QR[i].v4].z+step_z);	// ���� ����	
	glEnd();
	}
}

GLvoid DrawGLSceneTunnel(float step_x, float step_z, bool flag, int condition=-3)				//������� 
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
		if((Tunnel.QR[i].type_surface == 'd') || (Tunnel.QR[i].type_surface == 'i'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//���������� ��������
		else if(Tunnel.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Tunnel.QR[i].Normal.x, Tunnel.QR[i].Normal.y, Tunnel.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v1].x+step_x, BufferVertex[Tunnel.QR[i].v1].y, BufferVertex[Tunnel.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v2].x+step_x, BufferVertex[Tunnel.QR[i].v2].y, BufferVertex[Tunnel.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v3].x+step_x, BufferVertex[Tunnel.QR[i].v3].y, BufferVertex[Tunnel.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v4].x+step_x, BufferVertex[Tunnel.QR[i].v4].y, BufferVertex[Tunnel.QR[i].v4].z+step_z);	// ���� ����
		glEnd();
	}
}
GLvoid DrawGLSceneMain(float step_x, float step_z, bool flag, int condition=-3)				//������� �������
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
		if(MainRoom.QR[i].type_surface == 'l' || MainRoom.QR[i].type_surface == 'r' || MainRoom.QR[i].type_surface == 'd' || MainRoom.QR[i].type_surface == 'i')	glBindTexture(GL_TEXTURE_2D, texture[4]);	//���������� �������� 
		else if(MainRoom.QR[i].type_surface == 't')	glBindTexture(GL_TEXTURE_2D, texture[5]);
		else glBindTexture(GL_TEXTURE_2D, texture[6]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glBegin(GL_QUADS);
		glNormal3f(MainRoom.QR[i].Normal.x, MainRoom.QR[i].Normal.y, MainRoom.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v1].x+step_x, BufferVertex[MainRoom.QR[i].v1].y, BufferVertex[MainRoom.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v2].x+step_x, BufferVertex[MainRoom.QR[i].v2].y, BufferVertex[MainRoom.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v3].x+step_x, BufferVertex[MainRoom.QR[i].v3].y, BufferVertex[MainRoom.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f);  glVertex3f(BufferVertex[MainRoom.QR[i].v4].x+step_x, BufferVertex[MainRoom.QR[i].v4].y, BufferVertex[MainRoom.QR[i].v4].z+step_z);	// ���� ����
	glEnd();
	}
}
GLvoid DrawGLSceneTurnR(float step_x, float step_z, bool flag, int condition=-3)				//������� �������
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);	
	for(int i=0; i<TurnR.count_polygon; i++)
	{
		if((TurnR.QR[i].type_surface == 'd') || (TurnR.QR[i].type_surface == 'i') || (TurnR.QR[i].type_surface == 'l') || (TurnR.QR[i].type_surface == 'r'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//���������� ��������
		else if(TurnR.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
	glBegin(GL_QUADS);
		glNormal3f(TurnR.QR[i].Normal.x, TurnR.QR[i].Normal.y, TurnR.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[TurnR.QR[i].v1].x+step_x, BufferVertex[TurnR.QR[i].v1].y, BufferVertex[TurnR.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[TurnR.QR[i].v2].x+step_x, BufferVertex[TurnR.QR[i].v2].y, BufferVertex[TurnR.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[TurnR.QR[i].v3].x+step_x, BufferVertex[TurnR.QR[i].v3].y, BufferVertex[TurnR.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[TurnR.QR[i].v4].x+step_x, BufferVertex[TurnR.QR[i].v4].y, BufferVertex[TurnR.QR[i].v4].z+step_z);	// ���� ����
	glEnd();
	}
}
GLvoid DrawGLSceneTurnL(float step_x, float step_z, bool flag, int condition=-3)				//������� ������
{
	if(flag)	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z,
          g_Camera.m_vView.x,     g_Camera.m_vView.y,     g_Camera.m_vView.z,
          g_Camera.m_vUpVector.x, g_Camera.m_vUpVector.y,g_Camera.m_vUpVector.z);		 	
	for(int i=0; i<TurnL.count_polygon; i++)
	{
		if((TurnL.QR[i].type_surface == 'd') || (TurnL.QR[i].type_surface == 'i') || (TurnL.QR[i].type_surface == 'l') || (TurnL.QR[i].type_surface == 'r'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//���������� ��������
		else if(TurnL.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(TurnL.QR[i].Normal.x, TurnL.QR[i].Normal.y, TurnL.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[TurnL.QR[i].v1].x+step_x, BufferVertex[TurnL.QR[i].v1].y, BufferVertex[TurnL.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[TurnL.QR[i].v2].x+step_x, BufferVertex[TurnL.QR[i].v2].y, BufferVertex[TurnL.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[TurnL.QR[i].v3].x+step_x, BufferVertex[TurnL.QR[i].v3].y, BufferVertex[TurnL.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[TurnL.QR[i].v4].x+step_x, BufferVertex[TurnL.QR[i].v4].y, BufferVertex[TurnL.QR[i].v4].z+step_z);	// ���� ����
		glEnd();
	}
}
GLvoid DrawGLSceneTunnelRight(float step_x, float step_z, bool flag, int condition=-3)			//������� � ��������� �������
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
		if((Tunnel.QR[i].type_surface == 'd') || (Tunnel.QR[i].type_surface == 'i'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//���������� ��������
		else if(Tunnel.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Tunnel.QR[i].Normal.x, Tunnel.QR[i].Normal.y, Tunnel.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v1].x+step_x, BufferVertex[Tunnel.QR[i].v1].y, BufferVertex[Tunnel.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v2].x+step_x, BufferVertex[Tunnel.QR[i].v2].y, BufferVertex[Tunnel.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v3].x+step_x, BufferVertex[Tunnel.QR[i].v3].y, BufferVertex[Tunnel.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v4].x+step_x, BufferVertex[Tunnel.QR[i].v4].y, BufferVertex[Tunnel.QR[i].v4].z+step_z);	// ���� ����
		glEnd();
	}
}
GLvoid DrawGLSceneTunnelLeft(float step_x, float step_z, bool flag, int condition=-3)					//������� � ��������� ������
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
		if((Tunnel.QR[i].type_surface == 'd') || (Tunnel.QR[i].type_surface == 'i'))	glBindTexture(GL_TEXTURE_2D, texture[1]);	//���������� ��������
		else if(Tunnel.QR[i].type_surface == 't')		glBindTexture(GL_TEXTURE_2D, texture[2]);
		else glBindTexture(GL_TEXTURE_2D, texture[3]);
		if(TextureDead)	glBindTexture(GL_TEXTURE_2D, texture[14]);
		glBegin(GL_QUADS);
		glNormal3f(Tunnel.QR[i].Normal.x, Tunnel.QR[i].Normal.y, Tunnel.QR[i].Normal.z);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v1].x+step_x, BufferVertex[Tunnel.QR[i].v1].y, BufferVertex[Tunnel.QR[i].v1].z+step_z);	// ��� ����
		glTexCoord2f(1.0f, 0.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v2].x+step_x, BufferVertex[Tunnel.QR[i].v2].y, BufferVertex[Tunnel.QR[i].v2].z+step_z);	// ��� �����
		glTexCoord2f(1.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v3].x+step_x, BufferVertex[Tunnel.QR[i].v3].y, BufferVertex[Tunnel.QR[i].v3].z+step_z);	// ���� �����
		glTexCoord2f(0.0f, 1.0f); glVertex3f(BufferVertex[Tunnel.QR[i].v4].x+step_x, BufferVertex[Tunnel.QR[i].v4].y, BufferVertex[Tunnel.QR[i].v4].z+step_z);	// ���� ����	
		glEnd();
	}
}

LRESULT CALLBACK WndProc(  HWND    hWnd,
                           UINT    message,
                           WPARAM  wParam,
                           LPARAM  lParam)
{
     RECT    Screen;         // ������������ ������� ��� �������� ����
     GLuint  PixelFormat;
     static  PIXELFORMATDESCRIPTOR pfd=
		{
		 sizeof(PIXELFORMATDESCRIPTOR),  // ������ ���� ���������
		 1,                              // ����� ������ (?)
		 PFD_DRAW_TO_WINDOW |            // ������ ��� ����
		 PFD_SUPPORT_OPENGL |            // ������ ��� OpenGL
		 PFD_DOUBLEBUFFER,               // ������ ��� �������� ������
		 PFD_TYPE_RGBA,                  // ��������� RGBA ������
		 16,                             // ����� 16 ��� ������� �����
		 0, 0, 0, 0, 0, 0,                       // ������������� �������� ����� (?)
		 0,                              // ��� ������ ������������
		 0,                              // ��������� ��� ������������ (?)
		 0,                              // ��� ������ �����������
		 0, 0, 0, 0,                             // ���� ����������� ������������ (?)
		 16,                             // 16 ������ Z-����� (����� �������) 
		 0,                              // ��� ������ ����������
		 0,                              // ��� ��������������� ������� (?)
		 PFD_MAIN_PLANE,                 // ������� ���� ���������
		 0,                              // ������ (?)
		 0, 0, 0                         // ����� ���� ������������ (?)
		};

	switch (message)        // ��� ���������
	{
	case WM_CREATE:
		hDC = GetDC(hWnd);      // �������� �������� ���������� ��� ����
		PixelFormat = ChoosePixelFormat(hDC, &pfd);	// ����� ��������� ���������� ��� ������ ������� ��������       
		if (!PixelFormat)
		{
			MessageBox(0,L"Can't Find A Suitable PixelFormat"
			, L"Error" ,MB_OK|MB_ICONERROR);
			PostQuitMessage(0);	// ��� ��������� �������, ��� ��������� ������ ����������
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
//������ �� ����� �������� ����������, � ��� ���������� ������� ��� ��������, ��� ���� ����� OpenGL ��� �������� � ����
		if(!wglMakeCurrent(hDC, hRC))
        {
			MessageBox(0,L"Can't activate GLRC.",(LPCWSTR)"Error",MB_OK|MB_ICONERROR);
			PostQuitMessage(0);
			break;
        }
		GetClientRect(hWnd, &Screen);			//���������� ������ � ������ ����
        InitGL(Screen.right, Screen.bottom);	//�������������� ������� ��� ���������!!!! �������-�� ����
        break;
	case WM_DESTROY:
    case WM_CLOSE:
        ChangeDisplaySettings(NULL, 0);
        wglMakeCurrent(hDC,NULL);
        wglDeleteContext(hRC);
        ReleaseDC(hWnd,hDC);
        PostQuitMessage(0);
        break;
	case WM_KEYDOWN:		//���������� ������ ��� ��� ������� �������
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
	MSG             msg;    // ��������� ��������� Windows
	WNDCLASS        wc;	    // ��������� ������ Windows ��� ��������� ���� ����
	HWND            hWnd;   // ���������� ����������� ����
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
         L"SCP-106", // ��������� ������ ����
         WS_POPUP |
         WS_CLIPCHILDREN |
         WS_CLIPSIBLINGS,
         0, 0,                   // ������� ���� �� ������
         Width, Height,          // ������ � ������ ����
         NULL,
         NULL,
         hInstance,
         NULL);
	if(!hWnd)
	{
		MessageBox(0, L"Window Creation Error.",(LPCWSTR)"Error",MB_OK|MB_ICONERROR); 
		return FALSE;
	}	//640 480
	DEVMODE dmScreenSettings;								// ����� ������
	memset(&dmScreenSettings, 0, sizeof(DEVMODE));          // ������� ��� �������� ���������
	dmScreenSettings.dmSize = sizeof(DEVMODE);              // ������ ��������� Devmode
	dmScreenSettings.dmPelsWidth    = Width;                  // ������ ������
	dmScreenSettings.dmPelsHeight   = Height;                  // ������ ������
	dmScreenSettings.dmFields       = DM_PELSWIDTH | DM_PELSHEIGHT; // ����� �������
	ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
	// ������������ � ������ �����!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	ShowCursor(FALSE);
	//������ ����� ��������� ���� � �������������� ����� �������� �����, ��������� � �������� ������!!!
	TextureDead = false;
	Game SCP;
	SCP.pointer_Scene[0] = &DrawGLSceneCenter;
	SCP.pointer_Scene[1] = &DrawGLSceneMain;
	SCP.pointer_Scene[2] = &DrawGLSceneTunnel;
	SCP.pointer_Scene[3] = &DrawGLSceneTunnelRight;
	SCP.pointer_Scene[4] = &DrawGLSceneTunnelLeft;
	//�������� ��������� ������������� ���� �� ������!!!
	SCP.Scenes[0].pFunc = SCP.pointer_Scene[0];
	SCP.Scenes[1].pFunc = SCP.pointer_Scene[0];		//�� �����!!!
	SCP.Scenes[2].pFunc = SCP.pointer_Scene[1];
	SCP.Scenes[1].flag	= true;		//���������� ��������� ��������� � ����� �����
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
	const char step_array[] = {'A', 'D', 'W', 'S'};	//������ ��������
	float param_speed = 1.2f;
	bool param_sp_flag = false;		//���� ��������� ��������
	int count_step=0;		// ������� �����
	int rand_num;			// ��������� �������� ;)
	GLvoid (*pointer_temp)(float , float , bool, int);
	float temp_x;		//��������� �������� ��� ��������� ��������� ������
	float temp_z;
	float scp_x3 = 0;	//��������, ������������ ��� ���3
	float scp_z3 = 0;
	char char_temp;		//������������ ��� ��������� ��������� ������
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
		// ��������� ���� ���������
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
		// ��������� ��������� ���� �� ������!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		switch(SCP.SceneType[1])	//��������� ���� ��������������
		{
		case 'C':					//�� ��������� � ����������� �������
			//SCP.ChangeRoom = false;
			//�������� ������ ���� ������� ����������� �������� �������,
			//��������� �� ����������� ������� �����
			if((SCP.Scenes[1].z - 36.0f) > g_Camera.m_vPosition.z)
			{	//���� ������� ����������, �� ������ �����
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
				SCP.SceneType[1] = SCP.SceneType[2];
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].z -= 35.0f;				//�������� ������� �������
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			break;
		case 'M':				//�� ��������� � ������� �������!!!
			//�������� ��� �������� �������� �������
			if((SCP.Scenes[1].z - 36.0f) > g_Camera.m_vPosition.z)
			{	//�� ������ �����, ������ �����
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
				SCP.SceneType[1] = SCP.SceneType[2];
				//��������� ��� ���������� � ����� �����			
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].z -= 35.0f;				//�������� ������� �������
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			else if((SCP.Scenes[1].x - 14.0f) > g_Camera.m_vPosition.x)
			{	//�� ������ � ����� �������, ������ �����
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//��������� ��� ���������� � ����� �����
				SCP.Scenes[1].pFunc = SCP.pointer_Scene[2];
				SCP.Scenes[1].x -= 13.0f;
				SCP.Scenes[1].z -= 9.0f;
				SCP.SceneType[1] = 'T';
				//���������� ������� ����� ��� ������� 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
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
			{	//�� ������ � ������ �������, ������ �����
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//��������� ��� ���������� � ����� �����
				SCP.Scenes[1].pFunc = SCP.pointer_Scene[2];
				SCP.Scenes[1].x += 43.0f;
				SCP.Scenes[1].z -= 16.0f;
				SCP.SceneType[1] = 'T';
				//���������� ������� ����� ��� ������� 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
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
		case 'T':			//�� ��������� � �������!!!!
			if(SCP.TunnelToLeft && (SCP.Scenes[1].x - 31.0f > g_Camera.m_vPosition.x) && SCP.SceneType[2]=='T')
			{	//�� ��������� � ����� ������� � ��������� �������
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//��������� ��� ���������� � ����� �����
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
				SCP.SceneType[1] = SCP.SceneType[2];
				//���������� ������� ����� ��� ������� 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
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
			{	//�� ��������� � ������ ������� � ��������� �������
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//��������� ��� ���������� � ����� �����
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
				SCP.SceneType[1] = SCP.SceneType[2];
				//���������� ������� ����� ��� ������� 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
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
			{	//�� ��������� � ������ ������� � ��������� ������� (����� 2 ��� ���������)
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//��������� ��� ���������� � ����� �����
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
				SCP.SceneType[1] = SCP.SceneType[2];
				//���������� ������� ����� ��� ������� � ��������� 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].x += 2.0f;
				SCP.Scenes[2].z -= 15.0f;				//�������� ������� �������
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			else if(SCP.TunnelToLeft && (SCP.Scenes[1].x - 31.0f > g_Camera.m_vPosition.x) && SCP.SceneType[2]=='R')
			{	//�� ��������� � ����� ������� � ��������� �������(����� 2 ��� ���������)
				SCP.ChangeRoom = true;
				rand_num = rand()%2;
				char_temp = SCP.SceneType[1];
				pointer_temp = SCP.Scenes[1].pFunc;
				temp_x = SCP.Scenes[1].x;
				temp_z = SCP.Scenes[1].z;
				//��������� ��� ���������� � ����� �����
				SCP.Scenes[1].pFunc = SCP.Scenes[2].pFunc;	
				SCP.Scenes[1].x = SCP.Scenes[2].x;
				SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
				SCP.SceneType[1] = SCP.SceneType[2];
				//���������� ������� ����� ��� ������� 
				SCP.SceneType[0] = char_temp;
				SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
				SCP.Scenes[0].x = temp_x;
				SCP.Scenes[0].z = temp_z;
				SCP.Scenes[0].condition=-3;
				SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
				SCP.Scenes[2].x -= 32.0f;
				SCP.Scenes[2].z -= 15.0f;				//�������� ������� �������
				if(rand_num == 0) SCP.SceneType[2] = 'C';
				else SCP.SceneType[2] = 'M';
			}
			break;
		case 'R':		//�� ��������� � ����� ������� � ��������� �������!!!
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
					SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
					SCP.SceneType[1] = SCP.SceneType[2];
					//��������� ��� ���������� � ����� �����			
					SCP.SceneType[0] = char_temp;
					SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
					SCP.Scenes[0].x = temp_x;
					SCP.Scenes[0].z = temp_z;
					SCP.Scenes[0].condition=-3;
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
					SCP.Scenes[2].z -= 35.0f;				//�������� ������� �������
					if(rand_num == 0) SCP.SceneType[2] = 'C';
					else SCP.SceneType[2] = 'M';
				}
				break;
		case 'L':		//�� ��������� � ������� � ��������� ������
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
					SCP.Scenes[1].z = SCP.Scenes[2].z;		//�������� ���� ������� ���������
					SCP.SceneType[1] = SCP.SceneType[2];
					//��������� ��� ���������� � ����� �����			
					SCP.SceneType[0] = char_temp;
					SCP.Scenes[0].pFunc = pointer_temp;		//�������� ��������� �������
					SCP.Scenes[0].x = temp_x;
					SCP.Scenes[0].z = temp_z;
					SCP.Scenes[0].condition=-3;
					SCP.Scenes[2].pFunc = SCP.pointer_Scene[rand_num];
					SCP.Scenes[2].z -= 35.0f;				//�������� ������� �������
					if(rand_num == 0) SCP.SceneType[2] = 'C';
					else SCP.SceneType[2] = 'M';
				}
		default:
			break;
		}		
		if(SCP.ChangeRoom == true)		//���� �� ������� �������, ��
		{
			TimeFlag = !TimeFlag;
			SCP.ChangeRoom = false;
			switch(SCP.SceneType[1])
			{
			case 'C':	//������� � ����������� �������, �������� ���������� ��������� �������� ��������
				for(int i=0; i<7; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollCenter[i].Quad[j].x += SCP.Scenes[1].x - CenterColl_x;		//� ������������ ���������� ���������� ���������� 
						QcollCenter[i].Quad[j].z += SCP.Scenes[1].z - CenterColl_z;		//������������ ��������� � �������
					}
				}
				CenterColl_x = SCP.Scenes[1].x;
				CenterColl_z = SCP.Scenes[1].z;		//���������� ������ ���������
				break;
			case 'M':
				for(int i=0; i<13; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollMain[i].Quad[j].x += SCP.Scenes[1].x - MainColl_x;		//� ������������ ���������� ���������� ���������� 
						QcollMain[i].Quad[j].z += SCP.Scenes[1].z - MainColl_z;		//������������ ��������� � �������
					}
				}
				MainColl_x = SCP.Scenes[1].x;
				MainColl_z = SCP.Scenes[1].z;		//���������� ������ ���������
				break;
			case 'T':
				if(SCP.TunnelToLeft)	//��������� ����� �� �������
				{
					TunnelToLeft = true;
					for(int i=0; i<3; i++)
					{
						for(int j=0; j<4; j++)
						{
							QcollTunnelLeft[i].Quad[j].x += SCP.Scenes[1].x - LeftTunnelTemp_x;		//� ������������ ���������� ���������� ���������� 
							QcollTunnelLeft[i].Quad[j].z += SCP.Scenes[1].z - LeftTunnelTemp_z;		//������������ ��������� � �������
						}
					}
					LeftTunnelTemp_x = SCP.Scenes[1].x;
					LeftTunnelTemp_z = SCP.Scenes[1].z;		//���������� ������ ���������
				}
				else
				{
					TunnelToLeft = false;
					for(int i=0; i<3; i++)
					{
						for(int j=0; j<4; j++)
						{
							QcollTunnelRight[i].Quad[j].x += SCP.Scenes[1].x - RightTunnelTemp_x;		//� ������������ ���������� ���������� ���������� 
							QcollTunnelRight[i].Quad[j].z += SCP.Scenes[1].z - RightTunnelTemp_z;		//������������ ��������� � �������
						}
					}
					RightTunnelTemp_x = SCP.Scenes[1].x;
					RightTunnelTemp_z = SCP.Scenes[1].z;		//���������� ������ ���������
				}
				break;
			case 'R':
				TunnelToLeft = true;
				for(int i=0; i<3; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollTunnelLeftTurn[i].Quad[j].x += SCP.Scenes[1].x - TunnelLeftTurn_x;		//� ������������ ���������� ���������� ���������� 
						QcollTunnelLeftTurn[i].Quad[j].z += SCP.Scenes[1].z - TunnelLeftTurn_z;		//������������ ��������� � �������
						QcollTR[i].Quad[j].x += SCP.Scenes[1].x - TRcoll_x;
						QcollTR[i].Quad[j].z += SCP.Scenes[1].z - TRcoll_z;
					}
				}
				TunnelLeftTurn_x = SCP.Scenes[1].x;
				TunnelLeftTurn_z = SCP.Scenes[1].z;	
				//���������� ������ ���������
				TRcoll_x = SCP.Scenes[1].x;
				TRcoll_z = SCP.Scenes[1].z;	
				break;
			case 'L':
				TunnelToLeft = false;
				for(int i=0; i<3; i++)
				{
					for(int j=0; j<4; j++)
					{
						QcollTunnelRightTurn[i].Quad[j].x += SCP.Scenes[1].x - TunnelRightTurn_x;		//� ������������ ���������� ���������� ���������� 
						QcollTunnelRightTurn[i].Quad[j].z += SCP.Scenes[1].z - TunnelRightTurn_z;		//������������ ��������� � �������
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
			///////////////////////////////////////////////  ����  ///////////////////////////////////////////////////////
			//���������� ��������� ����!
			rand_num = rand()%3;
			SCP.ChangeSoundsOrientation();	//�������� ������ �����
			if(rand_num==0)	//��������� ����� ������� ���� � ���� �������
			{
				rand_num = rand()%13;
				SCP.InitialSoundsInRoom(1, true, ArraySounds[rand_num]);
			}
			else if(rand_num==1)//��������� ����� ������� ���� � ������� �������
			{
				rand_num = rand()%13;
				SCP.InitialSoundsInRoom(2, true, ArraySounds[rand_num]);
			}
			else 	//��������� ����� ������ ���� � ���� �������
			{
				rand_num = rand()%21;
				if(rand_num>13 && rand_num<21)
					SCP.InitialSoundsInRoom(1, false, ArraySounds[rand_num]);
				else if(rand_num<6)
					SCP.InitialSoundsInRoom(1, false, ArraySounds[rand_num+14]);
			}
			//������� ����� ����!
			if(SCP.SceneType[1] == 'T')
			{
				rand_num = rand()%4;
				if(rand_num == 3)	//���������� ���3
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
			//��������� ����������� ������� �������� �������
			if(SCP.PandoraSost == 0 && SCP.SceneType[1] == 'C')
			{
				int temp = rand()%4;
				if(temp == 0)
				{
					SCP.PandoraSost = -1;	//������� � ��������� ����������!
					SCP.pandora_x = BufferVertex[20].x + SCP.Scenes[1].x+2.0f;
					SCP.pandora_z = BufferVertex[20].z + SCP.Scenes[1].z;
				}
			}				
		}
		SCP.cGameSounds.UpdateAll();
		SCP.ControlAllSounds(g_Camera.m_vPosition.x, g_Camera.m_vPosition.y, g_Camera.m_vPosition.z);
		//������������ ��������� ���������
		SCP.cGameSounds.ActivateSounds();		//���������� �����
		SCP.Scenes[1].pFunc(SCP.Scenes[1].x, SCP.Scenes[1].z, SCP.Scenes[1].flag, SCP.Scenes[1].condition);
		SCP.Scenes[0].pFunc(SCP.Scenes[0].x, SCP.Scenes[0].z, SCP.Scenes[0].flag, SCP.Scenes[0].condition);
		SCP.Scenes[2].pFunc(SCP.Scenes[2].x, SCP.Scenes[2].z, SCP.Scenes[2].flag, SCP.Scenes[2].condition);
		if(SCP3_flag)
		{	//������ ���3		
			if(DrawSCP3(scp_x3, scp_z3, SCP3_left))
			{
				ScarySCP->mPos[0] = g_Camera.m_vPosition.x;
				ScarySCP->mPos[1] = g_Camera.m_vPosition.y;
				ScarySCP->mPos[2] = g_Camera.m_vPosition.z;
				SCP3_flag = false;
				ScarySCP->Play();
			}
		}
		if(SCP.PandoraSost == -1)	//������ � ��������
		{
			DrawPandora(SCP.pandora_x, SCP.pandora_z);
			//��������� �� �����������
			if(abs(SCP.pandora_z - g_Camera.m_vPosition.z)<1.0f)
				SCP.PandoraSost = 1;	//���������� �������!
		}
		else if(SCP.PandoraSost == 1)
		{
			int temp = rand()%3 + 2;
			SCP.PandoraSost = temp;
			SCP.PandoraTime = (float)GetTickCount();
		}
		else if(SCP.PandoraSost == 2)
		{	//����������� ��������!
			SCP.PandoraSost = 5;
			light_diffuse[0] = 0.0f;
			light_diffuse[1] = 1.0f;
			light_diffuse[2] = 0.0f;
			glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
			param_speed = param_speed*2;
		}
		else if(SCP.PandoraSost == 3)
		{	//��������� ��������!
			light_diffuse[0] = 0.0f;
			light_diffuse[1] = 0.0f;
			light_diffuse[2] = 1.0f;
			glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
			param_speed = param_speed/2;
			SCP.PandoraSost = 6;
		}
		else if(SCP.PandoraSost == 4)
		{	//������!
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
				_stprintf(message,TEXT("�� ���������, ���� ����� ��������� %d.\n������ %d.\nYou died. Your survival time is %d. The best time is %d"),TimeGame, TimeGame, Record, Record);
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
        //����������� ����� ������
		if (keys[VK_ESCAPE]) SendMessage(hWnd,WM_CLOSE,0,0);    // ���� ESC - �����
		if(SCP.SCP_106.Wait)
		{
			if(SCP.SCP_106.ch_direction=='M' && abs(g_Camera.m_vPosition.z - SCP.SCP_106.coord_activate) < 1.0f)
				SCP.SCP_106.MonsterActivated = true;
			else if(SCP.SCP_106.ch_direction!='M' && abs(g_Camera.m_vPosition.x - SCP.SCP_106.coord_activate) < 1.0f)
				SCP.SCP_106.MonsterActivated = true;
		}
		if(SCP.SCP_106.MonsterActivated)	//���� ������ �����������, ��������� �� �����������
		{			
			if(SCP.SCP_106.MonsterFunction(FPS))	//���� true, �� ��, �����!
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
					_stprintf(message,TEXT("�� ���������, ���� ����� ��������� %d.\n������ %d.\nYou died. Your survival time is %d. The best time is %d"),TimeGame, TimeGame, Record, Record);
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
				_stprintf(message,TEXT("�� ���������, ���� ����� ��������� %d.\n������ %d.\nYou died. Your survival time is %d. The best time is %d"),TimeGame, TimeGame, Record, Record);
				MessageBox(0, message,	L"The End",MB_OK|MB_SERVICE_NOTIFICATION);
				SendMessage(hWnd,WM_CLOSE,0,0); 
			}
			continue;
		}
		
		g_Camera.SetViewByMouse();
		for(int i=0; i<4; i++)
			if(keys[step_array[i]])  count_step++;
		if(count_step == 1)			//������ ������ ���� ������� ��������
			g_Camera.MoveMode = 0;
		else if(keys['W'] && keys['S'] || keys['A'] && keys['D'])
			g_Camera.MoveMode = 2;	//������ ��� �������������� �������, ����� �� �����
		else	g_Camera.MoveMode = 1;
		if(g_Camera.m_vPosition.y<4.0 && count_step>0)
			STEP->Play();
		count_step = 0;		
		if(keys['A']) 
			g_Camera.MovementFunc(true, false, FPS, param_speed);
		if(keys['D'])
			g_Camera.MovementFunc(false, false, FPS, param_speed);
		if(keys['W'])	// ���� ������ "�����"
			g_Camera.MovementFunc(false, true, FPS, param_speed);							
		if(keys['S'])	// ���� "�����"
			g_Camera.MovementFunc(true, true, FPS, param_speed);		
		if(keys[VK_LEFT])	// ���� ������ "�����"
			g_Camera.RotateFunc(rotate_speed,true, true, FPS);
		if(keys[VK_RIGHT])   // ���� "������"
			g_Camera.RotateFunc(-rotate_speed,true, true, FPS);
		if(keys[VK_UP]) 						
			g_Camera.RotateFunc(rotate_speed,true, false, FPS);	
		if(keys[VK_DOWN]) 	
			g_Camera.RotateFunc(-rotate_speed,false, false, FPS);
     
		 }
		 catch(int )
		 {
			 MessageBox(0, L"������ �������� ��������� �����\nCant open audio file.",
				L"Error",MB_OK|MB_ICONERROR);
			 SendMessage(hWnd,WM_CLOSE,0,0); 
		 }
	 }
	 }
catch(int )
{
	 MessageBox(0, L"����������� ������ ��� ���������� ����� Room.txt\n�� ������� ���������� � ������������!\nCant open or reade file Room.txt",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
catch(char )
{
	 MessageBox(0, L"����������� ������ ��� ���������� ����� Options.txt\n��������� ������������ ����������� �����!\nCant open or reade file Options.txt",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
catch(bool )
{
	 MessageBox(0, L"������ �������� ��������� �����\nCant open audio file.",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
catch(...)
{
	 MessageBox(0, L"����������� ������!\n�� ������� ���������� � ������������!\nUnknown error!",
		L"Error",MB_OK|MB_ICONERROR);
	 exit(1);
}
}
