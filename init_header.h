#include <fstream>
Quads QcollCenter[7];		//�������� �������� ����������� �������
Quads QcollMain[13];		//�������� ��������	������� �������
Quads QcollTunnelLeft[3];	//������� ������
Quads QcollTunnelRight[3];	//������� �������
Quads QcollTR[3];			//�������� �������� �������
Quads QcollTL[3];			//�������� �������� ������
Quads QcollTunnelRightTurn[3];
Quads QcollTunnelLeftTurn[3];

float CenterColl_x = 0.0f;
float CenterColl_z = 0.0f;
float MainColl_x = 0.0f;			//���������� ��������� ��������� �������� �� ��������
float MainColl_z = 0.0f;			//���������� ���������� ���������� �������� ��� �������� � ������ �������
float LeftTunnelTemp_x = 0.0f;
float LeftTunnelTemp_z = 0.0f;
float RightTunnelTemp_x = 0.0f;
float RightTunnelTemp_z = 0.0f;
float TRcoll_x = 0.0f;
float TRcoll_z = 0.0f;
float TLcoll_x = 0.0f;
float TLcoll_z = 0.0f;
float TunnelLeftTurn_x = 0.0f;
float TunnelLeftTurn_z = 0.0f;
float TunnelRightTurn_x = 0.0f;
float TunnelRightTurn_z = 0.0f;

Room Center;						//�������
Room MainRoom;
Room Tunnel;
Room TurnR;
Room TurnL;

CVector3* BufferVertex;				//������ ���� ������!
int Buffer_count;					//���-�� ������
int General_Quads_count;			//���-�� ����� ��������� ��� ������� � ����������� �������

class cScene		//����� ��������� ����� �� ������
{
public:
	GLvoid (*pFunc)(float, float, bool, int );		//��������� �� ����� (��� �����)
	float z;		//��������� �����
	float x;
	bool flag;
	int condition;
	cScene()
	{
		flag = false;
		z = 0;
		x = 0;
		condition = -3;
	}
};

class Monster
{
public:
	float x;	//������� ����������
	float z;
	char ch_direction;		//����������� ��������
	float position;			//���������� �� �����������
	float coord_stop;		//���������� ����������� ��������
	bool DeadTime;			//����, ��������������� � ������
	bool MonsterActivated;	//��� ���������� coord_activ ���������� ������� 
	bool Wait;				//������������� � ���������� ���� ��������������
	bool TurnFlag;			//������������� � ������������� ���������
	float param_speed;		//�������� ��������	
	float coord_activate;	//���������� ���������
	CVector3* vCam;
	bool DeadDistance();	//������� �������� ���������� �� ������� �� ������
	bool (*DrawMonster) (float x, float z, char ch_direction, bool TurnFlag);	//������� ��������� �������	
	bool MonsterFunction(int FPS);		//������� �������, ���������� �� ���������� ��������� � ������������� ���������
	Monster()
	{
		TurnFlag = false;
		Wait = false;
		MonsterActivated = false;
		z = 0.0f;
		x = 0.0f;
		coord_activate = 0.0f;
		vCam = NULL;
		DrawMonster = NULL;
		ch_direction = 'L';
		position = 0.0f;
		coord_stop = 0.0f;
		DeadTime = false;
		param_speed = 6.2f;
	}
};

bool Monster::DeadDistance()
{
	if(ch_direction == 'L' || ch_direction == 'R')
	{
		if(abs(vCam->x - position)<2.5f)
			return true;
	}
	else 
	{
		if(abs(vCam->z - position)<3.0f)
			return true;
	}
	return false;
}
bool Monster::MonsterFunction(int FPS)
{	
	if(abs(position-coord_stop) < 0.1 && !DeadTime)
	{
		MonsterActivated = false;
		Wait = false;
		DrawMonster = NULL;
		return false;
	}	//��������� ������� ��� ���������� �������
	if(!TurnFlag)
	{
		if(ch_direction == 'L')
			x -= param_speed/FPS; 
		if(ch_direction == 'R')
			x += param_speed/FPS; 
		if(ch_direction == 'A')
			z += param_speed*10.0f/FPS;
		if(ch_direction == 'M' || ch_direction == 'A')
			position = z;	
		if(ch_direction == 'L' || ch_direction == 'R')
			position = x;
		if(ch_direction == 'M' && abs(vCam->z - position)<10.0f)
			ch_direction = 'A';	//� - ������ ��������
	}
	if(DrawMonster(x, z, ch_direction, TurnFlag))
	{
		TurnFlag = false;
		param_speed/=2.0f;
		return true;
	}	
	if(DeadTime)
		return true;
	if(DeadDistance() && ch_direction == 'A')
		return true;	//���� ����� ����������� �� z, �� �� �����
	if(DeadDistance())	//���� ����� ����������� �� �, �� ������� ������������� ������
		TurnFlag = true;
	return false;
}
//����� ��������� ���� � �����
class Game					
{
public:
	remSnd* cInfoSound[6];	//���������� � ������, ������� ������������� ��� ����� �������������
	cSetSounds cGameSounds;	//����� ������, ������� ������������� � ������ ������
	cScene Scenes[3];		//���� �������(�����)
	//Scenes[0] - ����� ������
	//Scenes[1] - ����� ����� �������
	//Scenes[2] - ����� �������
	Monster SCP_106;
	bool (*DrawMonster) (float x, float z, char ch_direction, bool TurnFlag);	//������� ��������� �������	
	bool (*DrawMonster2) (float x, float z, char ch_direction, bool TurnFlag);	//������� ��������� �������	
	int PandoraSost;	//��������� �������
	float pandora_x;
	float pandora_z;
	float PandoraTime;
	CVector3 vGamer;		//������� ������ (������)
	char SceneType[3];		//���� ���� �� ������
	// 'M' - ������� �������
	// 'C' - ����������� �������
	// 'T' - �������
	// 'R' - ������� � ��������� �������
	// 'L' - ������� � ��������� ������
	bool TunnelToLeft;		//����������� ���� ��������� �������� �� �������
	bool ChangeRoom;		//����������� ���� �������� � ������ �������
	GLvoid (*pointer_Scene[5])(float , float , bool, int );	//������ ������������ ����
	// 0 - ����������� �������
	// 1 - ������� �������
	// 2 - �������
	// 3 - ������� � ��������� �������
	// 4 - ������� � ��������� ������
	string sNamesSounds[6];	//������ ��� ��������������� � ���� ������,
	// ������ 2 ����� ������������� ������ ������ �������, ������ 2 ����� ������ �������, � ��������� 2 ������� �������
	float ArrayActivationSounds[3];	//������, ���������� ������� ���������� ���������� ���������, ������������ ����
	//0 ������� - ������� ���� ������� �������
	//1 ������� - ������ ���� ������� �������
	//2 ������� - ������� ���� ������� �������
	void ChangeSoundsOrientation();	//�������� �������������� ������ ��� �������� � ������ �������
	void InitialSoundsInRoom(int num, bool BackgroundSound, const char* NameSound);	//�������������� ���� � �������
	//num - ����� ������� (1, 2)!!!!!!!!, BackgroundSound - ������� ����, NameSound - ��� ��������� �����
	void  ControlAllSounds(float x, float y, float z);	//������� ��� ���������� � ��������� ������ � ����� ������
	Game()
	{
		PandoraTime = 0;
		pandora_x = 0;
		pandora_z = 0;
		PandoraSost = 0;
		DrawMonster = NULL;
		ChangeRoom = false;
		SceneType[0] = 'C';
		SceneType[1] = 'C';
		SceneType[2] = 'M';
		for(int i=0; i<6; i++)
		{
			cInfoSound[i] = NULL;
			sNamesSounds[i] = "NULL";
		}
		ArrayActivationSounds[0] = 0;
		ArrayActivationSounds[1] = 0;
		ArrayActivationSounds[2] = 0;
	}
};
void Game::ControlAllSounds(float x, float y, float z)
{
	vGamer.InitVector(x, y, z);
	SCP_106.vCam = &vGamer;
	cGameSounds.vPosCam.InitVector(vGamer.x, vGamer.y, vGamer.z);
	cGameSounds.ChangeGain();
}

void Game::InitialSoundsInRoom(int num, bool BackgroundSound, const char* NameSound)
{
	if(num<1)	return;	
	for(int i=0; i<6; ++i)
	{
		if(strcmp(sNamesSounds[i].c_str(), NameSound)==0)
			return;
	}
	string str_temp = "NULL";
	float rand_num = (float)(rand()%30);	//������� �������������� ���������!
	remSnd* Snd_temp;
	if(BackgroundSound)
	{
		if(num == 1 )	//������� ���� ��� �������, ��� �� ���������!
		{
			cInfoSound[2] = new remSnd();
			if(!cInfoSound[2]->Open(NameSound, false, true))
				throw  2;
			ArrayActivationSounds[0] = rand_num;
			cGameSounds.AddSound(NameSound, cInfoSound[2]);
			sNamesSounds[2] = NameSound;
			Snd_temp = cInfoSound[2];
			//���������� ��������� ���������
		}
		else if(num == 2)	//������� ���� ��� ������� �����!
		{
			cInfoSound[4] = new remSnd();
			if(!cInfoSound[4]->Open(NameSound, false, true))
				throw  4;
			sNamesSounds[4] = NameSound;
			cGameSounds.AddSound(NameSound, cInfoSound[4]);
			ArrayActivationSounds[2] = rand_num;
			Snd_temp = cInfoSound[4];
		}
	}
	else if(num == 1)	//������ ���� ��� ����� �������
	{
			cInfoSound[3] = new remSnd();
			if(!cInfoSound[3]->Open(NameSound, false, true))
				throw 3;
			sNamesSounds[3] = NameSound;
			cGameSounds.AddSound(NameSound, cInfoSound[3]);
			ArrayActivationSounds[1] = rand_num;
			Snd_temp = cInfoSound[3];
	}
	//������� ��������� ���������� � ����� ������ � ������� ����� ��������� ������!
	float sig = -1;
	Snd_temp->SoundBePlayed = false;
	switch(SceneType[1])
	{
		case 'M':
		case 'C':	//��������� � ������� ��� ����������� �������
			Snd_temp->TypePlaneActive = 'z';
			if(num == 1 && BackgroundSound)		//����� ������� ���� � ����� �������
			{
				Snd_temp->vPos.InitVector(vGamer.x, vGamer.y, vGamer.z);
				Snd_temp->Active_param = vGamer.z - ArrayActivationSounds[0];	//���������� ��������� ���������
			}
			else if(num == 1 && !BackgroundSound)	//������ ���� � ����� �������
			{
				Snd_temp->vPos.InitVector(vGamer.x, vGamer.y, vGamer.z - ArrayActivationSounds[1]);
				Snd_temp->Active_param = vGamer.z - ArrayActivationSounds[1];	//���������� ��������� ���������
			}
			else if(num == 2)						//������� ���� � ������� �����
			{
				Snd_temp->vPos.InitVector(vGamer.x, vGamer.y, vGamer.z - 35.0f - ArrayActivationSounds[2]);
				Snd_temp->Active_param = vGamer.z - ArrayActivationSounds[2];	//���������� ��������� ���������
			}
			
			break;
		case 'T':
			Snd_temp->TypePlaneActive = 'x';
			if(TunnelToLeft)	//���� ��������� �� ������� �����
				sig = 1;			
			if(num == 1 && BackgroundSound)			//����� ������� ���� � ����� �������
			{
				Snd_temp->vPos.InitVector(vGamer.x, vGamer.y, vGamer.z);
				Snd_temp->Active_param = vGamer.x - sig*ArrayActivationSounds[0];	//���������� ��������� ���������
			}
			else if(num == 1 && !BackgroundSound)	//������ ���� � ����� �������
			{
				Snd_temp->vPos.InitVector(vGamer.x - sig*ArrayActivationSounds[1], vGamer.y, vGamer.z);
				Snd_temp->Active_param = vGamer.x - sig*ArrayActivationSounds[1];	//���������� ��������� ���������
			}
			else if(num == 2)
			{
				Snd_temp->vPos.InitVector(vGamer.x - sig*35.0f  - sig*ArrayActivationSounds[2], vGamer.y, vGamer.z );
				Snd_temp->Active_param = vGamer.x - sig*ArrayActivationSounds[2];	//���������� ��������� ���������
			}
			break;
		case'R':
		case'L':
			sig = -1;
			if(TunnelToLeft)	//���� ��������� �� ������� �����
				sig = 1;		
			if(num == 1 && BackgroundSound)		//����� ������� ���� � ����� �������
			{
				Snd_temp->TypePlaneActive = 'x';
				Snd_temp->vPos.InitVector(vGamer.x, vGamer.y, vGamer.z);
				Snd_temp->Active_param = vGamer.x - sig*ArrayActivationSounds[0];	//���������� ��������� ���������
			}
			else if(num == 1 && !BackgroundSound)	//������ ���� � ����� �������
			{
				Snd_temp->TypePlaneActive = 'x';
				Snd_temp->vPos.InitVector(vGamer.x - sig*ArrayActivationSounds[1], vGamer.y, vGamer.z);
				Snd_temp->Active_param = vGamer.x - sig*ArrayActivationSounds[1];	//���������� ��������� ���������
			}
			else if(num == 2)
			{
				Snd_temp->TypePlaneActive = 'z';
				Snd_temp->vPos.InitVector(vGamer.x , vGamer.y, vGamer.z - sig*35.0f - sig*ArrayActivationSounds[2] );
				Snd_temp->Active_param = vGamer.z - ArrayActivationSounds[2];	//���������� ��������� ���������
			}
		default:
			break;		
	}
	if(!BackgroundSound)	//���� ����� ������ ����, �� ������������� ����������� ��������� � ���� SCP
	{
		SCP_106.MonsterActivated = false;
		int ran_num = rand()%2;
		if(ran_num == 1)
		{
			switch(SceneType[1])
			{	//�������� ���������� ���� ��������������
			case 'M':
			{
				SCP_106.ch_direction='M';
				SCP_106.x = BufferVertex[20].x + Scenes[1].x+1.5f;
				SCP_106.z = BufferVertex[20].z + Scenes[1].z;
				SCP_106.position = SCP_106.z;
				SCP_106.coord_stop = SCP_106.position+10.0f;
				SCP_106.coord_activate = vGamer.z-10.0f;
				SCP_106.DrawMonster = DrawMonster2;
				SCP_106.Wait = true;
				break;
			}
			case 'R':
			case 'L':
			case 'T':
				if(TunnelToLeft)
				{
					SCP_106.x = BufferVertex[64].x + Scenes[1].x;
					SCP_106.z = BufferVertex[64].z + Scenes[1].z - 2.5f;
					SCP_106.ch_direction = 'L';
					SCP_106.position = SCP_106.x;
					SCP_106.coord_stop = SCP_106.position-30.0f;
					SCP_106.coord_activate = vGamer.x-ArrayActivationSounds[1];
				}
				else
				{
					SCP_106.x = BufferVertex[69].x + Scenes[1].x;
					SCP_106.z = BufferVertex[69].z + Scenes[1].z - 2.5f;
					SCP_106.ch_direction = 'R';
					SCP_106.position = SCP_106.x;
					SCP_106.coord_stop = SCP_106.position+30.0f;
					SCP_106.coord_activate = vGamer.x+ArrayActivationSounds[1];
				}
				SCP_106.DrawMonster = DrawMonster;
				SCP_106.Wait = true;
				break;
			default:
				break;
			}
			//����� ��������������������� ����� ������� ���!
		}
	}
	//������� ��� ����������� ���������� � ��������������� � ��������� �����!!!
}
void Game::ChangeSoundsOrientation()
{
	string str_temp = "NULL";
	//������� ������ �����
	if(sNamesSounds[0].compare(str_temp)!=0)
		cGameSounds.DeleteSound(sNamesSounds[0].c_str());
	if(sNamesSounds[1].compare(str_temp)!=0)
		cGameSounds.DeleteSound(sNamesSounds[1].c_str());
	//�������� ���������� ������ � ������� ��� � ������� ���������� � ������!
	for(int i=2; i<6; i++)
	{
		cInfoSound[i-2] = cInfoSound[i];
		sNamesSounds[i-2] = sNamesSounds[i];
	}
	cInfoSound[4] = NULL;
	cInfoSound[5] = NULL;
	sNamesSounds[4] = "NULL";
	sNamesSounds[5] = "NULL";
	for(int i=0; i<4; i++)
	{
		if(cInfoSound[i])
			cInfoSound[i]->SoundBePlayed = false;
	}
	//��������� ��� ����� ������ ��� ���������� ������ �������!!!
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//������� �������� ���������� � ���������
bool RoomScript(ifstream& ifs, char* name)
{
	char str[20];
	unsigned short temp1=0;
	unsigned short temp2=0;
	unsigned short temp3=0;
	do{
		ifs >> str;
		temp1++;
	}while(str[0]!='C');
	Buffer_count = (temp1-1)/3;		//�������� ����� ������!
	do{
		ifs >> str;
	}while(str[0]!='f');			//��������� ��������
	do{
		ifs >> str;
		temp2++;
		temp3++;
	}while(str[0]!='R');
	General_Quads_count = (temp2-1)/5;			//������� ���-�� ����� ��������� � ������ �������
	temp2--;
	temp3--;
	do{
		ifs >> str;
		temp2++;
	}while(str[0]!='M');
	temp2--;
	do{
		ifs >> str;
		temp3++;
	}while(str[0]!='T');
	do{
		ifs >> str;
		Tunnel.count_polygon++;
	}while(str[0]!='T');
	do{
		ifs >> str;
		TurnR.count_polygon++;
	}while(str[0]!='T');
	do{
		ifs >> str;
		TurnL.count_polygon++;
	}while(!ifs.eof());
	temp3--;
	Center.count_polygon = temp2/5;
	MainRoom.count_polygon = temp3/5;
	Tunnel.count_polygon = (Tunnel.count_polygon-1)/5;
	TurnR.count_polygon = (TurnR.count_polygon-1)/5;
	TurnL.count_polygon = (TurnL.count_polygon-1)/5;
	TurnL.count_polygon++;	
	BufferVertex = new CVector3[Buffer_count];			//�������� ������ ��� ������ ������
	Center.QR = new QuadRoom[Center.count_polygon];		//�������� ������ ��� ������ ���������
	MainRoom.QR = new QuadRoom[MainRoom.count_polygon];	//�������� ������ ��� ������ ���������
	Tunnel.QR = new QuadRoom[Tunnel.count_polygon];
	TurnR.QR = new QuadRoom[TurnR.count_polygon];
	TurnL.QR = new QuadRoom[TurnL.count_polygon];	
	ifs.close();
	ifstream ifss(name);				//!!!!!!!!!!!!!!!!
	for(int j=0; j<Buffer_count; j++)
	{
		ifss >> BufferVertex[j].x;
		ifss >> BufferVertex[j].y;
		ifss >> BufferVertex[j].z;
	}
	ifss >> str;
	if(str[0]!='C')
		return false;
	unsigned short b,c,d,e;
	char str_type;
	for(int i=0; i<7; i++)			//��������� �������� ������
	{	
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		QcollCenter[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	}
	QcollCenter[4].Quad[2].z -= 2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollCenter[4].Quad[3].z -= 2.0f;
	QcollCenter[5].Quad[2].z -= 2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollCenter[5].Quad[3].z -= 2.0f;
	ifss >> str;
	if(str[0]!='C')
		return false;	
	for(int i=0; i<13; i++)			//��������� ������� ��������
	{	
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		QcollMain[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	}
	QcollMain[11].Quad[2].z -=2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollMain[11].Quad[3].z -=2.0f;
	QcollMain[12].Quad[2].z -=2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollMain[12].Quad[3].z -=2.0f;

	QcollMain[10].Quad[1].x -=2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollMain[10].Quad[2].x -=2.0f;
	QcollMain[9].Quad[1].x -=2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollMain[9].Quad[2].x -=2.0f;

	QcollMain[8].Quad[0].x +=2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollMain[8].Quad[3].x +=2.0f;
	QcollMain[7].Quad[0].x +=2.0f;	//�������� ���������� ��� ���������� ������ �� �������� ����!!!
	QcollMain[7].Quad[3].x +=2.0f;
	ifss >> str;
	if(str[0]!='C')
		return false;
	for(int i=0; i<2; i++)			//���������  �������� �������
	{	
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		QcollTunnelLeft[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
		QcollTunnelRight[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
		QcollTunnelRightTurn[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
		QcollTunnelLeftTurn[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	}	
	QcollTunnelLeft[0].Quad[1].x -= 2.5f;
	QcollTunnelLeft[0].Quad[2].x -= 2.5f;
	QcollTunnelRight[1].Quad[1].x -= 2.5f;
	QcollTunnelRight[1].Quad[2].x -= 2.5f;

	QcollTunnelLeft[0].Quad[0].x += 2.5f;
	QcollTunnelLeft[0].Quad[3].x += 2.5f;
	QcollTunnelRight[1].Quad[0].x += 2.5f;
	QcollTunnelRight[1].Quad[3].x += 2.5f;
	//��������� ��� ��������� ���������� ��������� ������� ;)
	ifss >> b;  
	ifss >> c;
	ifss >> d;
	ifss >> e;
	QcollTunnelLeft[2].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	QcollTunnelLeftTurn[2].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	ifss >> b;  
	ifss >> c;
	ifss >> d;
	ifss >> e;
	QcollTunnelRight[2].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	QcollTunnelRightTurn[2].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	////////////////////////////////////////////////////////
	ifss >> str;
	if(str[0]!='C')
		return false;
	for(int i=0; i<3; i++)			//��������� �������� �������� �������
	{	
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		QcollTR[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	}
	QcollTR[1].Quad[1].z -=1.0f;
	QcollTR[1].Quad[2].z -=1.0f;
	QcollTR[2].Quad[1].z -=1.0f;
	QcollTR[2].Quad[2].z -=1.0f;
	ifss >> str;
	if(str[0]!='C')
		return false;
	for(int i=0; i<3; i++)			//��������� �������� �������� ������
	{	
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		QcollTL[i].InitQuads2(BufferVertex[b-1],BufferVertex[c-1], BufferVertex[d-1], BufferVertex[e-1]);
	}
	QcollTL[1].Quad[1].z -=1.0f;
	QcollTL[1].Quad[2].z -=1.0f;
	QcollTL[2].Quad[1].z -=1.0f;
	QcollTL[2].Quad[2].z -=1.0f;
	ifss >> str;
	if(str[0]!='f')
		return false;
	for(int i=0; i<General_Quads_count; i++)
	{
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		ifss >> str_type;
		Center.QR[i].v1=b-1;
		Center.QR[i].v2=c-1;
		Center.QR[i].v3=d-1;
		Center.QR[i].v4=e-1;
		Center.QR[i].InitNormal(str_type);
		MainRoom.QR[i].v1=b-1;
		MainRoom.QR[i].v2=c-1;
		MainRoom.QR[i].v3=d-1;
		MainRoom.QR[i].v4=e-1;
		MainRoom.QR[i].InitNormal(str_type);
	}
	ifss >> str;
	if(str[0]!='R')
		return false;	
	for(int i=General_Quads_count; i<Center.count_polygon; i++)
	{
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		ifss >> str_type;
		Center.QR[i].v1=b-1;
		Center.QR[i].v2=c-1;
		Center.QR[i].v3=d-1;
		Center.QR[i].v4=e-1;
		Center.QR[i].InitNormal(str_type);
	}
	ifss >> str;
	if(str[0]!='M')
		return false;
	for(int i=General_Quads_count; i<MainRoom.count_polygon; i++)
	{
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		ifss >> str_type;
		MainRoom.QR[i].v1=b-1;
		MainRoom.QR[i].v2=c-1;
		MainRoom.QR[i].v3=d-1;
		MainRoom.QR[i].v4=e-1;
		MainRoom.QR[i].InitNormal(str_type);
	}
	ifss >> str;
	if(str[0]!='T')
		return false;
	for(int i=0; i<Tunnel.count_polygon; i++)
	{
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		ifss >> str_type;
		Tunnel.QR[i].v1=b-1;
		Tunnel.QR[i].v2=c-1;
		Tunnel.QR[i].v3=d-1;
		Tunnel.QR[i].v4=e-1;
		Tunnel.QR[i].InitNormal(str_type);
	}
	ifss >> str;
	if(str[0]!='T')
		return false;
	for(int i=0; i<TurnR.count_polygon; i++)
	{
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		ifss >> str_type;
		TurnR.QR[i].v1=b-1;
		TurnR.QR[i].v2=c-1;
		TurnR.QR[i].v3=d-1;
		TurnR.QR[i].v4=e-1;
		TurnR.QR[i].InitNormal(str_type);
	}
	ifss >> str;
	if(str[0]!='T')
		return false;
	for(int i=0; i<TurnL.count_polygon; i++)
	{
		ifss >> b;  
		ifss >> c;
		ifss >> d;
		ifss >> e;
		ifss >> str_type;
		TurnL.QR[i].v1=b-1;
		TurnL.QR[i].v2=c-1;
		TurnL.QR[i].v3=d-1;
		TurnL.QR[i].v4=e-1;
		TurnL.QR[i].InitNormal(str_type);
	}
	return true;
}

bool ReadParametrs(int& Width, int& Height)
{
	ifstream ifs("Options.txt");
	if(!ifs.is_open())
		return false;
	char str[20];
	int len = 0;
	ifs >> str;
	len = strlen(str);
	if(str[len-1] == '=')
	{
		ifs >> str;
		Width = atoi(str);
	}
	else
	{
		ifs>>str;
		if(str[0]!='=')
			return false;
		ifs>>str;
		Width = atoi(str);
	}
	ifs >> str;
	len = strlen(str);
	if(str[len-1] == '=')
	{
		ifs >> str;
		Height = atoi(str);
	}
	else
	{
		ifs>>str;
		if(str[0]!='=')
			return false;
		ifs>>str;
		Height = atoi(str);
	}
	return true;
}
