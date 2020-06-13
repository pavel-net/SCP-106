#include "engine_header.h"
#include "sounds.h"

#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "vorbisfile.lib")
#pragma comment(lib, "alut.lib")
#pragma comment(lib, "OpenAL32.lib")

int BS;

void cSetSounds::ActivateSounds()
{
	if(MapSounds.empty())
		return;
	for(pos=MapSounds.begin(); pos!=MapSounds.end(); ++pos)
	{
		if(pos->second->SoundBePlayed)
			continue;
		char ch = pos->second->TypePlaneActive;				//����������, ����� ��������� ����� ������������ ����
		float Active_param=pos->second->Active_param;		//�������� �����������
		if(ch == 'x')
		{
			if(abs(Active_param - vPosCam.x) < 2.0f)
			{
				pos->second->Play();
				pos->second->SoundBePlayed = true;
				return;
			}
		}
		if(ch == 'z')
		{
			if(abs(Active_param - vPosCam.z) < 2.0f)
			{
				pos->second->Play();
				pos->second->SoundBePlayed = true;
				return;
			}
		}
	}
}

cSetSounds::~cSetSounds()
{
	for(pos=MapSounds.begin(); pos!=MapSounds.end(); ++pos)
		pos->second->Close();
	MapSounds.clear();
}

void cSetSounds::DeleteSound(const char* NameSound)
{
	if(MapSounds.empty())	
		return;
	remSnd* temp  = NULL;	
	int i = 0;
	for(pos=MapSounds.begin(); pos!=MapSounds.end(); ++pos)
	{
		if(strcmp((pos->first), NameSound)==0)		//����� ������ �������
		{	//������� ������� � ����������� ������
			temp = pos->second;
			MapSounds.erase(pos);
			for( ; i<5; i++)		//����������� ���� � ����� ���������������� ������
			{
				if(OldSounds[i]==NULL)
				{
					OldSounds[i] = new remSnd();
					OldSounds[i] = temp;
					return;
				}
			}
			temp->Close();
			delete temp;
			return;
		}
	}
}
void cSetSounds::AddSound(const char* NameSound, remSnd* cSound)
{
	MapSounds.insert(pair<const char*,remSnd*>(NameSound, cSound));
}

void cSetSounds::ChangeGain()
{	
	for(int i=0; i<5; i++)
	{
		if(OldSounds[i])
		{
			if(OldSounds[i]->GainSound(vPosCam, 2.2f))
			{	//������� ������ ������ ���� ;)
				OldSounds[i]->Close();
				delete OldSounds[i];
				OldSounds[i] = NULL;
			}
		}
	}
	if(MapSounds.empty())
		return;
	for(pos=MapSounds.begin(); pos!=MapSounds.end(); ++pos)
		pos->second->GainSound(vPosCam, 1.0f);	
}
void cSetSounds::PlaySound(const char* SoundName)
{
	MapSounds[SoundName]->Play();
}

string ExtractFile(string Filename)
{
	const char* a = Filename.c_str();
	const char* aa = strrchr(a, '.')+1;
	string result = aa;
	return result;
}

bool InitializeOpenAL()
{
   // ������� ���������.
  ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
  // �������� ���������.
  ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
  // ���������� ���������. (������ 3 �������� � ����������� ���, ��������� 3 � �������)
  ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
    // ��������� �������� �� ��������� ����������
   ALCdevice* pDevice = alcOpenDevice(NULL);
    // �������� �� ������
    // ������� �������� ����������
   ALCcontext* pContext = alcCreateContext(pDevice, NULL);
////    if (!CheckALCError()) return false;  
//    // ������ �������� �������
    alcMakeContextCurrent(pContext);
    // ������������� ��������� ���������
    // �������
    alListenerfv(AL_POSITION,    ListenerPos);
    // ��������
    alListenerfv(AL_VELOCITY,    ListenerVel);
    // ����������
    alListenerfv(AL_ORIENTATION, ListenerOri);
    return true;
}

//��������� ���� � ������ Filename, ���������� ��� ��� (wav),
//� �������������� �������� � ������������ ������ �� ������, �������� �������� ������ ������������� ������, � ������� �������� ��� ����� ������
bool remSnd::Open(string Filename, bool Looped, bool Streamed)
{
	// ��������� ���� �� �������
	ifstream a(Filename.c_str());
	if (!a.is_open()) return false;
	a.close();
	mLooped  = Looped;
	// ������� �������� ��������������� ������ �����
	alGenSources(1, &mSourceID);
	//if (!CheckALError()) return false;
	//��������� ������� ���������� ���������.
	//AL_PITCH � ��� ����� 
	//AL_GAIN � �������� �����. ������ �� ��, ��� ����� ���������� ���� �����, �� ���� ��������� ���������� �� ��������� �� ���������. 
	//AL_POSITION � ������� ��������� � ��������� �����������. 
	//AL_VELOCITY � �������� �������� �����.
	//AL_LOOPING � �������� ����� ��������� ����������, ����� �� ��� ���� ��������. 
	alSourcef (mSourceID, AL_PITCH,    1.0f);
	alSourcef (mSourceID, AL_GAIN,     1.0f);
	alSourcefv(mSourceID, AL_POSITION,  mPos);
	alSourcefv(mSourceID, AL_VELOCITY,  mVel);
	alSourcei (mSourceID, AL_LOOPING,   mLooped);
	string Ext = ExtractFile(Filename);
	string wav = "wav";
	string ogg = "ogg";
	if (Ext.compare(wav)==0) return LoadWavFile(Filename);
	if (Ext.compare(ogg)==0) 
	{
		mStreamed = Streamed;
		return LoadOggFile(Filename, Streamed);
	} 
	return false;
}
void remSnd::Play()
{
	alSourcePlay(mSourceID);	
}

bool remSnd::GainSound(CVector3 Cam, float param)
{
	//param - �������� ���������
	CVector3* vec = Cam.operator -(&vPos);
	float mod = vec->ModuleVector();	//�������� ���������� �� ��������� �� ��������
	if(mod>140.0f)	return true;	//�������� ������ � ��������
	//������ ���������
	if(mod<3.0f)
		alSourcef (mSourceID, AL_GAIN, 1.0f);
	else alSourcef (mSourceID, AL_GAIN, 3.0f/(mod*param));
	delete vec;
	return false;
}

void remSnd::Close()
{
	alSourceStop(mSourceID);
	if (alIsSource(mSourceID)) alDeleteSources(1, &mSourceID);
	if (!mVF)
	{  
		ov_clear(mVF);
		delete mVF;
	}
}
void remSnd::Stop()
{
	stop = true;
	alSourceStop(mSourceID);
}
void remSnd::Move(float X, float Y, float Z)
{
  ALfloat Pos[3] = { X, Y, Z };
  alSourcefv(mSourceID, AL_POSITION, Pos);
}
// ��������� ������ �� ��������� �������.
typedef struct 
{
  unsigned int  ID;
  string    Filename;
  unsigned int  Rate;
  unsigned int  Format;
} SndInfo;
map<ALuint, SndInfo> Buffers;

bool remSnd::LoadWavFile(const string &Filename)
{
  // ��������� ���������� ��������������
  SndInfo    buffer;
   // ������ ������ � ������
  ALenum    format;
   // ��������� �� ������ ������ �����
  ALvoid    *data;
   // ������ ����� �������
  ALsizei    size;
   // ������� ����� � ������
  ALsizei    freq;
   // ������������� ������������ ���������������
  ALboolean  loop;
   // ������������� ������
  ALuint    BufID = 0;

  // ���� ���� ��������� �������
  if (!BufID)
  {
    // ������ �����
    alGenBuffers(1, &buffer.ID);
   // if (!CheckALError()) return false;
    // ��������� ������ �� wav �����
    alutLoadWAVFile((ALbyte *)Filename.data(), &format, &data,
                     &size, &freq, &loop);
//   if (!CheckALError()) return false;
  
    buffer.Format    = format;
    buffer.Rate      = freq;
    // ��������� ����� �������
    alBufferData(buffer.ID, format, data, size, freq);
//   if (!CheckALError()) return false;
    // ��������� ���� �� �������������
    alutUnloadWAV(format, data, size, freq);
//   if (!CheckALError()) return false;

    // ��������� ���� ����� � ������
    Buffers[buffer.ID] = buffer;
  }
  else 
    buffer = Buffers[BufID];
  // ����������� ����� � ����������
  alSourcei (mSourceID, AL_BUFFER, buffer.ID);
  return true;
}

size_t ReadOgg(void *ptr, size_t size, size_t nmemb, void *datasource)
{
  istream *File = reinterpret_cast<istream*>(datasource);
  File->read((char *)ptr, size * nmemb);
  return File->gcount();
}

int SeekOgg(void *datasource, ogg_int64_t offset, int whence)
{
  istream *File = reinterpret_cast<istream*>(datasource);
  ios_base::seekdir Dir;
  File->clear();
  switch (whence) 
  {
    case SEEK_SET: Dir = ios::beg;  break;
    case SEEK_CUR: Dir = ios::cur;  break;
    case SEEK_END: Dir = ios::end;  break;
    default: return -1;
  }
  File->seekg((streamoff)offset, Dir);
  return (File->fail() ? -1 : 0);
}

long TellOgg(void *datasource)
{
  istream *File = reinterpret_cast<istream*>(datasource);
  return File->tellg();
}

int CloseOgg(void *datasource)
{
  return 0;
}

bool remSnd::ReadOggBlock(ALuint BufID, size_t Size)
{
  // ����������
  char    eof = 0;
  int     current_section;
  long    TotalRet = 0, ret;
  // ����� ������
  char    *PCM;

  if (Size < 1) return false;
  PCM = new char[Size];

  // ���� ������
  while (TotalRet < Size) 
  {
    ret = ov_read(mVF, PCM + TotalRet, Size - TotalRet, 0, 2, 1, 
             & current_section);

    // ���� ��������� ����� �����
    if (ret == 0) break;
    else if (ret < 0)     // ������ � ������
    {
      return false;
    }
    else
    {
      TotalRet += ret;
    }
  }
  if (TotalRet > 0)
  {
    alBufferData(BufID, Buffers[BufID].Format, (void *)PCM, 
           TotalRet, Buffers[BufID].Rate);
//    CheckALError();
  }
  delete [] PCM;
  return (ret > 0);
}

bool remSnd::LoadOggFile(const string &Filename, bool Streamed)
{
	int i, DynBuffs = 1, BlockSize;
	// OAL specific
	SndInfo     buffer;
	ALuint      BufID = 0; 
	// ��������� � ��������� ��������� ������.
	// C������� ��������� �� 4 ������� ������ � ���������� ������:
	// ������, �����, ��������, � ��������� � ������� ����� ��������� ��������� ���������.
	ov_callbacks  cb;
	// ��������� ��������� cb
	cb.close_func = CloseOgg;
	cb.read_func  = ReadOgg;
	cb.seek_func  = SeekOgg;
	cb.tell_func  = TellOgg;
	// ������� ��������� OggVorbis_File
	mVF = new OggVorbis_File;
	// ��������� OGG ���� ��� ��������
	OggFile.open(Filename.c_str(), ios_base::in | ios_base::binary);
	// �������������� ���� ���������� vorbisfile
	if (ov_open_callbacks(&OggFile, mVF, NULL, -1, cb) < 0)
	{
	// ���� ������, �� ����������� ���� �� �������� OGG
		return false;
	}
	// ��������� ��������� � ����������� �� ���� ��������� �� ������������ 
	// �����������
	if (!Streamed)
	{
		// ������ ����� � ���� ����
		BlockSize = ov_pcm_total(mVF, -1) * 4;
	}
	else
	{
		//������ ����� �����
		BlockSize  = ov_pcm_total(mVF, -1)/64;
		BS = BlockSize;
		// ���������� ������� � ������� ������
		DynBuffs  = 4;
		//alGenSources(1, &mSourceID);
		alSourcei(mSourceID, AL_LOOPING, AL_FALSE);
	} 
	// �������� ����������� � ���������� � �����
	mComment   = ov_comment(mVF, -1);
	mInfo      = ov_info(mVF, -1);
	// ��������� SndInfo ��������� �������
	buffer.Rate    = mInfo->rate;
	buffer.Filename  = Filename;
	buffer.Format = (mInfo->channels == 1) ? AL_FORMAT_MONO16 
			   : AL_FORMAT_STEREO16;
	// ���� ��������� ������������, ��� ����� �� ������ �� ������ ��
	if (Streamed || !BufID)
	{
		for (i = 0; i < DynBuffs; i++)
		{
			// ������� �����
			alGenBuffers(1, &buffer.ID);
			//      if (!CheckALError()) return false;
			Buffers[buffer.ID] = buffer;
			// ��������� ���� ������
			ReadOggBlock(buffer.ID, BlockSize);
			//    if (!CheckALError()) return false;
			if (Streamed) // �������� ����� � �������.
			{				
				alSourceQueueBuffers(mSourceID, 1, &buffer.ID);
			//	alGenSources(1, &mSourceID);
			//        if (!CheckALError()) return false;
			}
			else 
				alSourcei(mSourceID, AL_BUFFER, buffer.ID);
		}
	}
	else
	{
		alSourcei(mSourceID, AL_BUFFER, Buffers[BufID].ID);
	}
	return true;
}
void cSetSounds::UpdateAll()
{	
	static float time1 = (float)GetTickCount();
	float time2 = (float)GetTickCount();
	if(time2 - time1 > 10)
	{
		for(int i=0; i<5; i++)
		{
			if(OldSounds[i]!=NULL)
				OldSounds[i]->Update();
		}
		for(pos=MapSounds.begin(); pos!=MapSounds.end(); ++pos)
		{
			if(pos->second->mStreamed)
				pos->second->Update();
		}
	}
	time1 = time2;
}
void remSnd::Update()
{
  if (!mStreamed || stop ) return;  
  int      Processed = 0;
  ALuint      BufID;
  // �������� ���������� ������������ �������
  alGetSourcei(mSourceID, AL_BUFFERS_PROCESSED, &Processed);
  // ���� ������� ���������� ��
  while (Processed--)
  {
    // ��������� �� �� �������
    alSourceUnqueueBuffers(mSourceID, 1, &BufID);
//    if (!CheckALError()) return;
    // ������ ��������� ������ ������ � �������� ����� ������� � �������
    if (ReadOggBlock(BufID, BS) != 0)
    {
      alSourceQueueBuffers(mSourceID, 1, &BufID);
//      if (!CheckALError()) return;
    }
    else // ���� ����� ����� ���������
    {
      // ������������� �� ������
      ov_pcm_seek(mVF, 0);
      // ��������� � �������
      alSourceQueueBuffers(mSourceID, 1, &BufID);
//      if (!CheckALError()) return;      
      // ���� �� ����������� ������������ �� ����
      if (!mLooped) Stop();
    }
  }
}