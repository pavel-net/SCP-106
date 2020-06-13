#include "engine_header.h"

void Quads::InitQuads(float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3,
		float x4, float y4, float z4)
{
	Quad[0].InitVector(x1, y1, z1);
	Quad[1].InitVector(x2, y2, z2);
	Quad[2].InitVector(x3, y3, z3);
	Quad[3].InitVector(x4, y4, z4);
}

void Quads::InitQuads2(CVector3 vec1, CVector3 vec2, CVector3 vec3, CVector3 vec4)
{
	Quad[0] = vec1;
	Quad[1] = vec2;
	Quad[2] = vec3;
	Quad[3] = vec4;
}
	// i - обратная стена (ближняя)
	// d - дальняя стена 
	// l - левая стена
	// r - правая стена
	// t - потолок
	// b - пол
void QuadRoom::InitNormal(char ch)
{
	type_surface = ch;
	switch(type_surface)
	{
	case 'i':
		Normal.InitVector(0.0f, 0.0f, -1.0f);
		break;
	case 'd':
		Normal.InitVector(0.0f, 0.0f, 1.0f);
		break;
	case 'l':
		Normal.InitVector(1.0f, 0.0f, 0.0f);
		break;
	case 'r':
		Normal.InitVector(-1.0f, 0.0f, 0.0f);
		break;
	case 't':
		Normal.InitVector(0.0f, -1.0f, 0.0f);
		break;
	case 'b':
		Normal.InitVector(0.0f, 1.0f, 0.0f);
		break;
	default:
		break;
	}
}



