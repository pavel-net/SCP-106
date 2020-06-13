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
		char ch = pos->second->TypePlaneActive;				//определяет, какая плоскость будет активировать звук
		float Active_param=pos->second->Active_param;		//параметр активизации
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
		if(strcmp((pos->first), NameSound)==0)		//нашли нужный элемент
		{	//удаляем элемент и освобождаем память
			temp = pos->second;
			MapSounds.erase(pos);
			for( ; i<5; i++)		//отбрасываем звук в хвост сильнозагасающих звуков
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
			{	//удаляем старый далёкий звук ;)
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
   // Позиция слушателя.
  ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
  // Скорость слушателя.
  ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
  // Ориентация слушателя. (Первые 3 элемента – направление «на», последние 3 – «вверх»)
  ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
    // Открываем заданное по умолчанию устройство
   ALCdevice* pDevice = alcOpenDevice(NULL);
    // Проверка на ошибки
    // Создаем контекст рендеринга
   ALCcontext* pContext = alcCreateContext(pDevice, NULL);
////    if (!CheckALCError()) return false;  
//    // Делаем контекст текущим
    alcMakeContextCurrent(pContext);
    // Устанавливаем параметры слушателя
    // Позиция
    alListenerfv(AL_POSITION,    ListenerPos);
    // Скорость
    alListenerfv(AL_VELOCITY,    ListenerVel);
    // Ориентация
    alListenerfv(AL_ORIENTATION, ListenerOri);
    return true;
}

//Открывает файл с именем Filename, определяет его тип (wav),
//и подготавливает источник к проигрыванию данных из буфера, источник содержит только идентификатор буфера, в котором хранятся эти самые данные
bool remSnd::Open(string Filename, bool Looped, bool Streamed)
{
	// Проверяем файл на наличие
	ifstream a(Filename.c_str());
	if (!a.is_open()) return false;
	a.close();
	mLooped  = Looped;
	// Создаем источник соответствующий нашему звуку
	alGenSources(1, &mSourceID);
	//if (!CheckALError()) return false;
	//установка базовых параметров источника.
	//AL_PITCH – тон звука 
	//AL_GAIN – усиление звука. Влияет на то, как будет изменяться сила звука, по мере изменения расстояния от источника до слушателя. 
	//AL_POSITION – позиция источника в трёхмерных координатах. 
	//AL_VELOCITY – скорость движения звука.
	//AL_LOOPING – значение этого параметра определяет, будет ли ваш звук зациклен. 
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
	//param - параметр затухания
	CVector3* vec = Cam.operator -(&vPos);
	float mod = vec->ModuleVector();	//получили расстояние от источника до приёмника
	if(mod>140.0f)	return true;	//посылаем сигнал о удалении
	//вносим изменения
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
// параметры буфера со звуковыми данными.
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
  // Структура содержащая аудиопараметры
  SndInfo    buffer;
   // Формат данных в буфере
  ALenum    format;
   // Указатель на массив данных звука
  ALvoid    *data;
   // Размер этого массива
  ALsizei    size;
   // Частота звука в герцах
  ALsizei    freq;
   // Идентификатор циклического воспроизведения
  ALboolean  loop;
   // Идентификатор буфера
  ALuint    BufID = 0;

  // Если звук загружаем впервые
  if (!BufID)
  {
    // Создаём буфер
    alGenBuffers(1, &buffer.ID);
   // if (!CheckALError()) return false;
    // Загружаем данные из wav файла
    alutLoadWAVFile((ALbyte *)Filename.data(), &format, &data,
                     &size, &freq, &loop);
//   if (!CheckALError()) return false;
  
    buffer.Format    = format;
    buffer.Rate      = freq;
    // Заполняем буфер данными
    alBufferData(buffer.ID, format, data, size, freq);
//   if (!CheckALError()) return false;
    // Выгружаем файл за ненадобностью
    alutUnloadWAV(format, data, size, freq);
//   if (!CheckALError()) return false;

    // Добавляем этот буфер в массив
    Buffers[buffer.ID] = buffer;
  }
  else 
    buffer = Buffers[BufID];
  // Ассоциируем буфер с источником
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
  // Переменные
  char    eof = 0;
  int     current_section;
  long    TotalRet = 0, ret;
  // Буфер данных
  char    *PCM;

  if (Size < 1) return false;
  PCM = new char[Size];

  // Цикл чтения
  while (TotalRet < Size) 
  {
    ret = ov_read(mVF, PCM + TotalRet, Size - TotalRet, 0, 2, 1, 
             & current_section);

    // Если достигнут конец файла
    if (ret == 0) break;
    else if (ret < 0)     // Ошибка в потоке
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
	// Структура с функциями обратного вызова.
	// Cодержит указатели на 4 функции работы с источником данных:
	// чтение, поиск, закрытие, и сообщение о текущем месте положения читающего указателя.
	ov_callbacks  cb;
	// Заполняем структуру cb
	cb.close_func = CloseOgg;
	cb.read_func  = ReadOgg;
	cb.seek_func  = SeekOgg;
	cb.tell_func  = TellOgg;
	// Создаем структуру OggVorbis_File
	mVF = new OggVorbis_File;
	// Открываем OGG файл как бинарный
	OggFile.open(Filename.c_str(), ios_base::in | ios_base::binary);
	// Инициализируем файл средствами vorbisfile
	if (ov_open_callbacks(&OggFile, mVF, NULL, -1, cb) < 0)
	{
	// Если ошибка, то открываемый файл не является OGG
		return false;
	}
	// Начальные установки в зависимости от того потоковое ли проигрывание 
	// затребовано
	if (!Streamed)
	{
		// Размер блока – весь файл
		BlockSize = ov_pcm_total(mVF, -1) * 4;
	}
	else
	{
		//Размер блока задан
		BlockSize  = ov_pcm_total(mVF, -1)/64;
		BS = BlockSize;
		// Количество буферов в очереди задано
		DynBuffs  = 4;
		//alGenSources(1, &mSourceID);
		alSourcei(mSourceID, AL_LOOPING, AL_FALSE);
	} 
	// Получаем комментарии и информацию о файле
	mComment   = ov_comment(mVF, -1);
	mInfo      = ov_info(mVF, -1);
	// Заполняем SndInfo структуру данными
	buffer.Rate    = mInfo->rate;
	buffer.Filename  = Filename;
	buffer.Format = (mInfo->channels == 1) ? AL_FORMAT_MONO16 
			   : AL_FORMAT_STEREO16;
	// Если потоковое проигрывание, или буфер со звуком не найден то
	if (Streamed || !BufID)
	{
		for (i = 0; i < DynBuffs; i++)
		{
			// Создаем буфер
			alGenBuffers(1, &buffer.ID);
			//      if (!CheckALError()) return false;
			Buffers[buffer.ID] = buffer;
			// Считываем блок данных
			ReadOggBlock(buffer.ID, BlockSize);
			//    if (!CheckALError()) return false;
			if (Streamed) // Помещаем буфер в очередь.
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
  // Получаем количество отработанных буферов
  alGetSourcei(mSourceID, AL_BUFFERS_PROCESSED, &Processed);
  // Если таковые существуют то
  while (Processed--)
  {
    // Исключаем их из очереди
    alSourceUnqueueBuffers(mSourceID, 1, &BufID);
//    if (!CheckALError()) return;
    // Читаем очередную порцию данных и включаем буфер обратно в очередь
    if (ReadOggBlock(BufID, BS) != 0)
    {
      alSourceQueueBuffers(mSourceID, 1, &BufID);
//      if (!CheckALError()) return;
    }
    else // Если конец файла достигнут
    {
      // «перематываем» на начало
      ov_pcm_seek(mVF, 0);
      // Добавляем в очередь
      alSourceQueueBuffers(mSourceID, 1, &BufID);
//      if (!CheckALError()) return;      
      // Если не зацикленное проигрывание то стоп
      if (!mLooped) Stop();
    }
  }
}