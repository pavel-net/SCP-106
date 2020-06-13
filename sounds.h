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

//класс звука
class remSnd  
{
public:
	ALfloat mVel[3];		//положение и скорость источника в 3Д пространстве. Эти 2 переменные содержат x,y,z координаты
	ALfloat mPos[3];
	CVector3 vPos;
	// Потоковый ли наш звук?
	bool mStreamed;
	bool stop;
	bool SoundBePlayed;		//специальный флаг, указывающий на то был ли уже проигран звук или нет
	char TypePlaneActive;	//определяет, какая плоскость будет активировать звук
	float Active_param;		//параметр активизации
	bool mLooped;			//звук проигрывается в цикле  
	bool Open(string Filename, bool Looped, bool Streamed);
	bool IsStreamed();	//потоковое проигрывание
	void Play();
	void Close();
	void Update();		//предназначена для смены буферов для потокового звука
	void Move(float X, float Y, float Z);
	void Stop();
	bool GainSound(CVector3 Cam, float param);	//функция изменения громкости звука
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
  // Идентификатор источника
  ALuint mSourceID;
  // Переменные библиотеки vorbisfile
  // Главная структура описания файла
  OggVorbis_File  *mVF;
  // Структура комментариев к файлу
  vorbis_comment  *mComment;
  // Информация о файле
  vorbis_info    *mInfo;
  // Файловый поток содержащий наш ogg файл
  std::ifstream    OggFile;
  bool LoadWavFile (const std::string &Filename);
   // Функция чтения блока из файла в буфер
  bool ReadOggBlock(ALuint BufID, size_t Size);
  // Функция открытия и инициализации OGG файла
  bool LoadOggFile (const std::string &Filename, bool Streamed);
};
//класс всех звуков
class cSetSounds
{	
public:
	CVector3 vPosCam;				//положение камеры (приёмника)
	map<const char*, remSnd*> MapSounds;		//ассоциированный контейнер ключ/значение:  название звука/ссылка на звук
	map<const char*, remSnd*>::iterator pos;	//итератор множества
	void AddSound(const char*, remSnd*);
	void DeleteSound(const char*);	//удаление одного звука
	void ChangeGain();				//изменение громкости звука всех источников
	void PlaySound(const char*);	//запуск одного звука
	~cSetSounds();
	void ActivateSounds();
	void UpdateAll();
	remSnd* OldSounds[5];			//звуки, которые затихают быстрее и очищаются отдельно (звуки, отделившиеся от комнат)
	cSetSounds()
	{
		for(int i=0; i<5; i++)
			OldSounds[i] = NULL;
		vPosCam.InitVector(0,0,0);
		pos = 0;
	}
};