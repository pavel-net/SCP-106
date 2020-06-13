#include "al.h"
#include "alc.h"
#include "alu.h"
#include "alut.h"

#include "ogg.h"
#include "os_types.h"
#include "os_types.h"
#include "vorbisfile.h"
#include <map>
using namespace std;

bool InitializeOpenAL();

//����� �����
class remSnd  
{
public:
	ALfloat mVel[3];		//��������� � �������� ��������� � 3� ������������. ��� 2 ���������� �������� x,y,z ����������
	ALfloat mPos[3];
	CVector3 vPos;
	// ��������� �� ��� ����?
	bool mStreamed;
	bool stop;
	bool SoundBePlayed;		//����������� ����, ����������� �� �� ��� �� ��� �������� ���� ��� ���
	char TypePlaneActive;	//����������, ����� ��������� ����� ������������ ����
	float Active_param;		//�������� �����������
	bool mLooped;			//���� ������������� � �����  
	bool Open(string Filename, bool Looped, bool Streamed);
	bool IsStreamed();	//��������� ������������
	void Play();
	void Close();
	void Update();		//������������� ��� ����� ������� ��� ���������� �����
	void Move(float X, float Y, float Z);
	void Stop();
	bool GainSound(CVector3 Cam, float param);	//������� ��������� ��������� �����
	remSnd()
	{
		stop = false;
		mVel[0] = 0.0f;
		mVel[1] = 0.0f;
		mVel[2] = 0.0f;
		mPos[0] = 0.0f;
		mPos[1] = 0.0f;
		mPos[2] = 0.0f;
		vPos.InitVector(0,0,0);
		mLooped = false;
		mStreamed = false;
		SoundBePlayed = false;
		TypePlaneActive = 'z';
		Active_param = 20;
	}
private:
  // ������������� ���������
  ALuint mSourceID;
  // ���������� ���������� vorbisfile
  // ������� ��������� �������� �����
  OggVorbis_File  *mVF;
  // ��������� ������������ � �����
  vorbis_comment  *mComment;
  // ���������� � �����
  vorbis_info    *mInfo;
  // �������� ����� ���������� ��� ogg ����
  std::ifstream    OggFile;
  bool LoadWavFile (const std::string &Filename);
   // ������� ������ ����� �� ����� � �����
  bool ReadOggBlock(ALuint BufID, size_t Size);
  // ������� �������� � ������������� OGG �����
  bool LoadOggFile (const std::string &Filename, bool Streamed);
};
//����� ���� ������
class cSetSounds
{	
public:
	CVector3 vPosCam;				//��������� ������ (��������)
	map<const char*, remSnd*> MapSounds;		//��������������� ��������� ����/��������:  �������� �����/������ �� ����
	map<const char*, remSnd*>::iterator pos;	//�������� ���������
	void AddSound(const char*, remSnd*);
	void DeleteSound(const char*);	//�������� ������ �����
	void ChangeGain();				//��������� ��������� ����� ���� ����������
	void PlaySound(const char*);	//������ ������ �����
	~cSetSounds();
	void ActivateSounds();
	void UpdateAll();
	remSnd* OldSounds[5];			//�����, ������� �������� ������� � ��������� �������� (�����, ������������ �� ������)
	cSetSounds()
	{
		for(int i=0; i<5; i++)
			OldSounds[i] = NULL;
		vPosCam.InitVector(0,0,0);
		pos = 0;
	}
};