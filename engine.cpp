#include "engine_header.h"
using namespace std;

float CVector3::ModuleVector()	//метод подсчёта модуля вектора
{
	return (float)sqrt((double)(x*x+y*y+z*z));
}
CVector3* CVector3::operator-(CVector3* v)
{
	CVector3* res_v=new CVector3();
	res_v->InitVector(x - v->x, y - v->y, z - v->z);
	return res_v;
}
void CVector3::operator+(CVector3* v)
{
	x+=v->x;
	y+=v->y;
	z+=v->z;
}
void CVector3::Normalization()
{
	float b=ModuleVector();
	x=x/b;
	y=y/b;
	z=z/b;
}
void CVector3::InitVector(float m_x, float m_y, float m_z)
{
	x=m_x;
	y=m_y;
	z=m_z;
}
float GetCos(CVector3* v1, CVector3* v2)
{
	return ((v1->x*v2->x + v1->y*v2->y + v1->z*v2->z)/(v1->ModuleVector()*v2->ModuleVector()));
}
float GetAngle(CVector3* v1, CVector3* v2)
{
	float cos_v=GetCos(v1,v2);
	return acos(cos_v);
}
CCamera::CCamera()
{
    m_vPosition.InitVector(0.0, 0.0, 0.0);	// Вектор нашей позиции
	m_vView.InitVector(0.0, 1.0, 0.5);		// Иниц. вектор взгляда
	m_vUpVector.InitVector(0.0, 0.0, 1.0);	// Вертикальный вектор
	radius=0;
	MoveMode = 0;
}
//возвращает вектор взгляда
CVector3* CCamera::GetVector()
{
	CVector3* vvVector=new CVector3();
	vvVector->x = m_vView.x - m_vPosition.x;      //Направление по X
	vvVector->y = m_vView.y - m_vPosition.y;      //Направление по Y
	vvVector->z = m_vView.z - m_vPosition.z;      //Направление по Z
	return vvVector;
}
//векторное произведение
CVector3* VectorMultiply(CVector3* v1, CVector3* v2)
{
	CVector3* v_res=new CVector3();
	v_res->x=(v1->y*v2->z)-(v2->y*v1->z);
	v_res->y=(v1->z*v2->x)-(v1->x*v2->z);
	v_res->z=(v1->x*v2->y)-(v2->x*v1->y);
	return v_res;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//функция нахождения вектора нормали к плоскости
CVector3* GetNormal(CVector3* Point1, CVector3* Point2, CVector3* Point3)
{
	CVector3* v1 = 	Point1->operator-(Point2);
	CVector3* v2 = 	Point3->operator-(Point2);
	CVector3* result = VectorMultiply(v1, v2);
	result->Normalization();
	delete v1;
	delete v2;
	return result;
}
//определяет расстояние от центра отсчёта до плоскости
//Point - любая точка на плоскости
//Ax + By + Cz + D = 0  
//A,B,C-координаты нормального вектора
//x,y,z-координаты точки на плоскости
float PlaneDistance(CVector3* Normal, CVector3* Point)
{
	return -(Normal->x*Point->x + Normal->y*Point->y + Normal->z*Point->z);
}
//определяет пересекает ли прямая плоскость
//Ax + By + Cz + D = 0  
//x,y,z-координаты проверяемой точки
bool IntersectedLine(CVector3* vPolygon[], CVector3* vLine1, CVector3* vLine2, CVector3* Normal)
{
	float D = PlaneDistance(Normal, vPolygon[0]);
	float distance1, distance2;
	distance1 = Normal->x*vLine1->x + Normal->y*vLine1->y + Normal->z*vLine1->z + D;
	distance2 = Normal->x*vLine2->x + Normal->y*vLine2->y + Normal->z*vLine2->z + D;
	if(distance1*distance2>=0)	
		return false;
	else return true;	//точки лежат по разные стороны от плоскости
}
//проверяет лежит ли точка внутри полигона
bool InsidePolygon(CVector3* vInterPoint, CVector3 Polygon[], int VertexCount)
{
	float angle=0.0f;
	CVector3* A;
	CVector3* B;
	//суммируем углы между верторами вершин - точка, вершина - точка
	for(int i=0; i<VertexCount-1; i++)
	{
		A = vInterPoint->operator - (&Polygon[i]);
		B = vInterPoint->operator - (&Polygon[i+1]);
		angle+=GetAngle(A, B);
	}
	A = vInterPoint->operator - (&Polygon[0]);
	B = vInterPoint->operator - (&Polygon[VertexCount-1]);
	angle+=GetAngle(A,B);
	delete A;
	delete B;
	if((double)angle>=6.28)
		return true;
	else 
		return false;
	
}
//определяет положение сферы относительно полигона и возвращает расстояние от центра до плоскости
float PositionSphere(CVector3* vCenter, CVector3 Polygon[], CVector3* Normal)
{
	float D = PlaneDistance(Normal, &Polygon[0]);	//расстояние от начала отсчёта до полигона
	float distance = Normal->x*vCenter->x + Normal->y*vCenter->y + Normal->z*vCenter->z + D;
	//получили дистанцию от центра сферы до плоскости полигона
	return distance;
}
//проверяем пересечение сферы с гранями полигона
bool CheckBorder(CVector3* vCenter, CVector3 Polygon[], int VertexCount, float radius)
{
	float min_distance=radius;
	CVector3* A;
	float temp;
	//пробегаем по вершинам полигона, измеряя расстояние от центра сферы
	for(int i=0; i<VertexCount; i++)
	{
		A = vCenter->operator -(&Polygon[i]);
		temp = A->ModuleVector();
		if(temp<min_distance)
		{
			delete A;
			return true;
		}
	}
	delete A;
	return false;
}
//функция определяет смещенее сферы за плоскость
CVector3* GetCollisionDistance(CVector3* vCenter, CVector3* Normal, float distance, float radius)
{
	CVector3* OffsetDist = new CVector3();
	//определение местоположения относительно плоскости
	//distance - расстояние от центра сферы до плоскости
	if(distance>0)	
	{	//перед плоскостью
		//Найдем расстояние, на которое сфера углубилась в плоскость
        float distanceOver = radius - distance;
		OffsetDist->x = Normal->x * distanceOver;
		OffsetDist->y = Normal->y * distanceOver;
		OffsetDist->z = Normal->z * distanceOver;
	}
    else 
    {	// Если же сфера с задней стороны полигона
        float distanceOver = radius + distance;
		distanceOver=-distanceOver;
		OffsetDist->x = Normal->x * distanceOver;
		OffsetDist->y = Normal->y * distanceOver;
		OffsetDist->z = Normal->z * distanceOver;
    }
	return OffsetDist;
}
//проверяет пересечение сферы с полигоном
bool CheckIntersectedSphere(CVector3* vCenter, CVector3 Polygon[], int VertexCount, CVector3* Normal, float radius, float distance)
{
	//distance - расстояние от центра сферы до плоскости
	if(abs(distance)<radius)			
	{	//пересекается
		//тогда проецируем центр сферы на плоскость
		CVector3* ProectCenter = new CVector3();
		ProectCenter->x=vCenter->x - Normal->x*distance;
		ProectCenter->y=vCenter->y - Normal->y*distance;
		ProectCenter->z=vCenter->z - Normal->z*distance;
		//определяем принадлежность точки полигону
		if(InsidePolygon(ProectCenter, Polygon, VertexCount))
		{
			delete ProectCenter;
			return true;	//принадлежит полигону!
		}
		if(CheckBorder(vCenter, Polygon, VertexCount, radius))
			return true;	//центр не принадлежит полигону, но пересечение есть
	}
	return false;
}
//ГЛАВНАЯ функция нахождения коллизий камеры с полигонами 
void CCamera::CheckCameraCollision(Quads cQuads[], int VertexCount, int countPolygon)
{
	//пробегаем по всем полигонам и проверяем на коллизии
	for(int i=0; i<countPolygon; i++)
	{
		CVector3* Normal = GetNormal(&cQuads[i].Quad[0], &cQuads[i].Quad[1], &cQuads[i].Quad[2]);
		float distance = PositionSphere(&m_vPosition, cQuads[i].Quad, Normal);
		//distance - расстояние от сферы до плоскости
		if(abs(distance)<radius)
		{	//если есть пересечение с плоскостью
			if(CheckIntersectedSphere(&m_vPosition, cQuads[i].Quad, VertexCount, Normal, radius, distance))
			{	//если есть пересечение с полигоном
				CVector3* OffsetDistance = GetCollisionDistance(&m_vPosition,  Normal, distance, radius);		
				//получили вектор смещения!
				//применяем изменения
				m_vPosition.operator +(OffsetDistance);
				m_vView.operator +(OffsetDistance);
				delete OffsetDistance;
			}
		}
		delete Normal;
	}
}		
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//задаёт все необходимые координаты
void CCamera::PositionCamera(float positionX, float positionY, float positionZ,
        float viewX, float viewY, float viewZ, 
        float upVectorX, float upVectorY, float upVectorZ)
{
	m_vPosition.InitVector(positionX, positionY, positionZ);
	m_vView.InitVector(viewX, viewY, viewZ);
	m_vUpVector.InitVector(upVectorX, upVectorY, upVectorZ);
}


void CCamera::RotateView(float angle, float x, float y, float z)
{
    CVector3 vNewView;
    CVector3 vView;
    // Получим наш вектор взгляда (направление, куда мы смотрим)
    vView.x = m_vView.x - m_vPosition.x;    //направление по X
    vView.y = m_vView.y - m_vPosition.y;    //направление по Y
    vView.z = m_vView.z - m_vPosition.z;    //направление по Z
    // Рассчитаем 1 раз синус и косинус переданного угла
    float cosTheta = (float)cos(angle);
    float sinTheta = (float)sin(angle);
    // Найдем новую позицию X для вращаемой точки
    vNewView.x  = (cosTheta + (1 - cosTheta) * x * x)   * vView.x;
    vNewView.x += ((1 - cosTheta) * x * y - z * sinTheta)   * vView.y;
    vNewView.x += ((1 - cosTheta) * x * z + y * sinTheta)   * vView.z;
    // Найдем позицию Y
    vNewView.y  = ((1 - cosTheta) * x * y + z * sinTheta)   * vView.x;
    vNewView.y += (cosTheta + (1 - cosTheta) * y * y)   * vView.y;
    vNewView.y += ((1 - cosTheta) * y * z - x * sinTheta)   * vView.z;
    // И позицию Z
    vNewView.z  = ((1 - cosTheta) * x * z - y * sinTheta)   * vView.x;
    vNewView.z += ((1 - cosTheta) * y * z + x * sinTheta)   * vView.y;
    vNewView.z += (cosTheta + (1 - cosTheta) * z * z)   * vView.z;
    //добавим новый вектор вращения к нашей позиции, чтобы
    //установить новый взгляд камеры.
    m_vView.x = m_vPosition.x + vNewView.x;
    m_vView.y = m_vPosition.y + vNewView.y;
    m_vView.z = m_vPosition.z + vNewView.z;
}

void CCamera::MovementFunc(bool flag, bool direction, int FPS, float param_sp)
{
	static bool MutexMode = false;	//режим очерёдности выключен
	static bool step_up = true;		//голова поднимается
	float speed_step = 0.5f/FPS;
	//direction - прямо или в бок?
	//получаем вектор направления взгляда
	CVector3* vView=GetVector();
	CVector3* vProject=new CVector3();
	//получаем проекцию вектора
	vProject->x=vView->x;
	vProject->z=vView->z;
	vProject->y=0.0;
	//получаем длину проекции и длину вектора взгляда
	float LenghtProject=vProject->ModuleVector();
	float LenghtView=vView->ModuleVector();
	float speed_x=(LenghtView*vProject->x)/LenghtProject;
	float speed_z=(LenghtView*vProject->z)/LenghtProject;
	delete vView;
	delete vProject;
	//изменяем наши вектора
	if(direction)	//прямо?
	{
		if(flag)	//если двигаемся назад
		{
			speed_x=-speed_x;
			speed_z=-speed_z;
		}
		m_vPosition.x += param_sp*speed_x/FPS;
		m_vPosition.z += param_sp*speed_z/FPS;
		m_vView.x += param_sp*speed_x/FPS;
		m_vView.z += param_sp*speed_z/FPS;
	}
	else			//вбок
	{
		float temp=speed_x;
		speed_x=-speed_z;
		speed_z=temp;
		if(flag)	//если двигаемся влево
		{
			speed_x=-speed_x;
			speed_z=-speed_z;
		}
		m_vPosition.x += param_sp*speed_x/FPS;
		m_vPosition.z += param_sp*speed_z/FPS;
		m_vView.x += param_sp*speed_x/FPS;
		m_vView.z += param_sp*speed_z/FPS;
	}
	if(MoveMode == 2)	//одновременно нажаты клавиши, противоречивые друг другу
		return;
	if(MoveMode == 0)
		MutexMode = false;
	if(m_vPosition.y>4.10f && m_vPosition.y<4.15f)
		speed_step = speed_step/2.0f;
	if(step_up && !MutexMode)	
	{
		m_vPosition.y+=speed_step;
		m_vView.y+=speed_step;
		if(m_vPosition.y > 4.15f)
			step_up = false;
	}
	else if(!step_up && !MutexMode)
	{
		m_vPosition.y-=speed_step;
		m_vView.y-=speed_step;
		if(m_vPosition.y < 4.0f)
			step_up = true;
	}
	if(MoveMode == 1)	//одновременно нажаты клавиши, не противоречивые друг другу
		MutexMode = !MutexMode;
}

void CCamera::RotateFunc(float angle_rotate, bool view_up, bool rotate_y, int FPS)
{
	angle_rotate = angle_rotate*10.0f/FPS;
	//view_up - смотрим вверх?
	//rotate_y - вращение вокруг оси Y
	//получаем вектор направления взгляда и проекцию
	CVector3* vView=GetVector();
	CVector3* vProject=new CVector3();
	//получаем проекцию вектора
	vProject->x=vView->x;
	vProject->z=vView->z;
	vProject->y=0.0;
	//получаем ось вращения
	CVector3* AxisRotate=VectorMultiply(vView, &m_vUpVector);
	AxisRotate->Normalization();
	//получаем угол между взглядом и его проекцией
	float alfa = asin(vView->y/vView->ModuleVector());
	if(rotate_y)
	{
		RotateView(angle_rotate, 0.0, 1.0, 0.0);
		return;
	}
	if(view_up)	
	{
		if(alfa>3.1415/3.0)
			return;
		RotateView(angle_rotate, AxisRotate->x, AxisRotate->y, AxisRotate->z);
	}
	else
	{
		if(alfa<-3.1415/3.0)
			return;
		RotateView(angle_rotate, AxisRotate->x, AxisRotate->y, AxisRotate->z);
	}	
}
void CCamera::SetViewByMouse()
{
	POINT mousePos;			  // Это структура, хранящяя X и Y позиции мыши
	int middleX = 800 >> 1;   // Вычисляем половину ширины
    int middleY = 600 >> 1;   // И половину высоты экрана
	float angleX = 0.0f;      // Направление взгляда вверх/вниз
    float angleY = 0.0f;      // Значение, необходимое для вращения влево-вправо (по оси Y)
	//Получаем текущие коорд. мыши
    GetCursorPos(&mousePos);
    // Если курсор остался в том же положении, мы не вращаем камеру
    if( (mousePos.x == middleX) && (mousePos.y == middleY) ) return;
	// Теперь, получив координаты курсора, возвращаем его обратно в середину.
    SetCursorPos(middleX, middleY);
	angleY = (float)((middleX - mousePos.x))/1000.0f;
    angleX = (float)((middleY - mousePos.y))/1000.0f;
	CVector3* view = GetVector();
	CVector3* vAxis = VectorMultiply(view, &m_vUpVector);
	float temp = vAxis->ModuleVector();
	vAxis->x=vAxis->x/temp;
	vAxis->y=vAxis->y/temp;
	vAxis->z=vAxis->z/temp;
	float alfa = asin(view->y/view->ModuleVector());
	if(angleX>0)
	{
		if(alfa>3.1415/3.0)
		{
			RotateView( angleY, 0, 1, 0);
			return;
		}
		RotateView( angleX, vAxis->x, vAxis->y, vAxis->z);
	}
	else 
	{
		if(alfa<-3.1415/3.0)
		{
			RotateView( angleY, 0, 1, 0);
			return;
		}
		RotateView( angleX, vAxis->x, vAxis->y, vAxis->z);
	}
	RotateView( angleY, 0, 1, 0);	
}