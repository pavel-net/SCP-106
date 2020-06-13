#include <windows.h>                      // ������������ ���� ��� Windows
#include <gl\gl.h>                        // ������������ ���� ��� OpenGL32 ����������
#include <gl\glu.h>                       // ������������ ���� ��� GLu32 ����������
#include "glaux.h"						  // ������������ ���� ��� GLaux ����������

#include <iostream>
#include <fstream>
#include <math.h>
#include <queue>
using namespace std;

class CVector3{
public:
	float x,y,z;
	CVector3()
	{
		x=0;
		y=0;
		z=0;
	}
	float ModuleVector();
	void InitVector(float m_x, float m_y, float m_z);
	CVector3* operator-(CVector3* v);
	void operator+(CVector3* v);
	void Normalization();
};

class Quads{
public:
	CVector3 Quad[4];
	Quads(){ };	
	void InitQuads(float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float x4, float y4, float z4);
	void InitQuads2(CVector3 vec1, CVector3 vec2, CVector3 vec3, CVector3 vec4);
};

class CCamera {
public:
    CVector3 m_vPosition;   // ������� ������.
    CVector3 m_vView;		// ����������� ������.
    CVector3 m_vUpVector;   // ������������ ������.
	float radius;			// ������ �����
	int MoveMode;			// ������ ������� ���������� ������	
	//MoveMode = 1	�������� ���������� 
	//MoveMode = 2	��������� ����
	//MoveMode = 0	������ �� ���������
	CCamera();
	void PositionCamera(float positionX, float positionY, float positionZ,
                     float viewX,     float viewY,     float viewZ,
                 float upVectorX, float upVectorY, float upVectorZ);
    void RotateView(float angle, float x, float y, float z);
	//�������� ����������� �������
	CVector3* GetVector();
	void RotateFunc(float angle_rotate, bool view_up, bool rotate_y, int FPS);
	void MovementFunc(bool flag, bool direction, int FPS, float param_sp);
	void SetViewByMouse();
	void CheckCameraCollision(Quads cQuads[], int VertexCount, int countPolygon);
};

void DownloadPolygonsInArray(FILE* file, Quads QuadsArray[], int QuadsCount);

class QuadRoom{
public:
	unsigned short v1;
	unsigned short v2;
	unsigned short v3;
	unsigned short v4;
	char type_surface;		//��� �����������
	CVector3 Normal;		//������ �������
	// i - �������� ����� (�������)
	// d - ������� ����� 
	// l - ����� �����
	// r - ������ �����
	// t - �������
	// b - ���
	void InitNormal(char ch);
	void QuadInit(unsigned short v1, unsigned short v2, unsigned short v3, unsigned short v4)
	{
		this->v1=v1;
		this->v2=v2;
		this->v3=v3;
		this->v4=v4;
	}
	QuadRoom()
	{
		type_surface = 'X';
		v1=0;
		v2=0;
		v3=0;
		v4=0;
	}
};

class Room
{
public:
	QuadRoom* QR;					//������ ��������� �������
	unsigned short count_polygon;	//���������� ���������
	Room()
	{
		count_polygon = 0;
		QR = NULL;
	}
};
	
