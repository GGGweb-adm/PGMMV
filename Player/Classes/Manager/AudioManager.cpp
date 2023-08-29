#include "AudioManager.h"
#include "AudioEngine.h"
#include "Manager/GameManager.h"
#include "AppMacros.h"

USING_NS_CC;
using namespace experimental;

#if defined(USE_FADE)
//--------------------------------------------------------------------------------------------------------------------
// functions
//--------------------------------------------------------------------------------------------------------------------
static float calcPitch(float pitch)
{
	if (pitch >= 0.0f) return  AGTK_LINEAR_INTERPOLATE(1.0f, 4.0f, 50.0f, pitch);
	return AGTK_LINEAR_INTERPOLATE(1.0f, 0.25f, 50.0f, -pitch);
}
#endif

//--------------------------------------------------------------------------------------------------------------------
// シングルトンのインスタンス
//--------------------------------------------------------------------------------------------------------------------
AudioManager* AudioManager::_audioManager = NULL;

//--------------------------------------------------------------------------------------------------------------------
// コンストラクタ
//--------------------------------------------------------------------------------------------------------------------
AudioManager::AudioManager()
{
#if defined(USE_FADE)
	_bgmVolume = nullptr;
	_seVolume = nullptr;
	_voiceVolume = nullptr;
	_bgmList = nullptr;
	_seList = nullptr;
	_voiceList = nullptr;
#else
	_bgmVolume = 1.0f;
	_seVolume = 1.0f;
	_voiceVolume = 1.0f;
#endif
	_mute = false;
}

//--------------------------------------------------------------------------------------------------------------------
// デストラクタ
//--------------------------------------------------------------------------------------------------------------------
AudioManager::~AudioManager()
{
#if defined(USE_FADE)
	CC_SAFE_RELEASE_NULL(_bgmVolume);
	CC_SAFE_RELEASE_NULL(_seVolume);
	CC_SAFE_RELEASE_NULL(_voiceVolume);
	CC_SAFE_RELEASE_NULL(_bgmList);
	CC_SAFE_RELEASE_NULL(_seList);
	CC_SAFE_RELEASE_NULL(_voiceList);
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
}

//--------------------------------------------------------------------------------------------------------------------
// インスタンスを取得する。
//--------------------------------------------------------------------------------------------------------------------
AudioManager* AudioManager::getInstance()
{
	if(!_audioManager){
		_audioManager = new AudioManager();
#if defined(USE_FADE)
		Scheduler *s = Director::getInstance()->getScheduler();
		s->scheduleUpdate(_audioManager, kSchedulePriorityAudioManager, false);
#endif
	}
	return _audioManager;
}

//--------------------------------------------------------------------------------------------------------------------
// シングルトンを破棄する。
//--------------------------------------------------------------------------------------------------------------------
void AudioManager::purge()
{
	if(!_audioManager){
		return;
	}
#if defined(USE_FADE)
	// スケジューラーを停止する。
	Scheduler *s = Director::getInstance()->getScheduler();
	s->unscheduleUpdate(_audioManager);
#endif
	AudioManager *am = _audioManager;
	_audioManager = NULL;
	am->release();
}

bool AudioManager::init(agtk::data::SoundSettingData *soundSettingData)
{
	if (soundSettingData == nullptr) {
		return false;
	}

#if 1
	// ACT2-3658
	// 再生デバイスがなくAudioEngine::lazyInit()に失敗してもAudioManagerの初期化は済ませます。
	// またlazyInit()は再生時AudioEngine::play2d()で毎回行っているので問題ありません。
	AudioEngine::lazyInit();
#else
	if (AudioEngine::lazyInit() == false) {
		return false;
	}
#endif

#if defined(USE_FADE)
	_bgmVolume = agtk::ValueTimer<float>::create(soundSettingData->getBgmVolume() * 0.01f);
	CC_SAFE_RETAIN(_bgmVolume);
	_seVolume = agtk::ValueTimer<float>::create(soundSettingData->getSeVolume() * 0.01f);
	CC_SAFE_RETAIN(_seVolume);
	_voiceVolume = agtk::ValueTimer<float>::create(soundSettingData->getVoiceVolume() * 0.01f);
	CC_SAFE_RETAIN(_voiceVolume);
	this->setBgmList(cocos2d::__Dictionary::create());
	this->setSeList(cocos2d::__Dictionary::create());
	this->setVoiceList(cocos2d::__Dictionary::create());
#else
	this->setBgmVolume(soundSettingData->getBgmVolume() * 0.01f);
	this->setSeVolume(soundSettingData->getSeVolume() * 0.01f);
	this->setVoiceVolume(soundSettingData->getVoiceVolume() * 0.01f);
#endif
	return true;
}

#if defined(USE_FADE)
void AudioManager::update(float delta)
{
	//デバイス更新
	updateDevice();

	_sameTimePlaySeMap.clear();
	if(_bgmVolume) _bgmVolume->update(delta);
	if(_seVolume) _seVolume->update(delta);
	if(_voiceVolume) _voiceVolume->update(delta);

	//bgm
	auto bgmList = this->getBgmList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->update(delta);
			float nowVolume = AudioEngine::getVolume(p->getAudioId());
			float nextVolume = this->getBgmVolume() * this->getVariableVolume(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableBgmVolumeAdjust) * p->getVolume()->getValue();
			if (nowVolume != nextVolume) {
				AudioEngine::setVolume(p->getAudioId(), nextVolume);
			}
		}
	}
	//se
	auto seList = this->getSeList();
	CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->update(delta);
			float nowVolume = AudioEngine::getVolume(p->getAudioId());
			float nextVolume = this->getSeVolume() * this->getVariableVolume(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableSeVolumeAdjust) * p->getVolume()->getValue();
			if (nowVolume != nextVolume) {
				AudioEngine::setVolume(p->getAudioId(), nextVolume);
			}
		}
	}
	//voice
	auto voiceList = this->getVoiceList();
	CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->update(delta);
			float nowVolume = AudioEngine::getVolume(p->getAudioId());
			float nextVolume = this->getVoiceVolume() * this->getVariableVolume(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableVoiceVolumeAdjust) * p->getVolume()->getValue();
			if (nowVolume != nextVolume) {
				AudioEngine::setVolume(p->getAudioId(), nextVolume);
			}
		}
	}

	//stop
	this->updateStop();
}

void AudioManager::updateStop()
{
	//bgm
	bool bBgmRemoved;
	do {
		bBgmRemoved = false;
		auto bgmList = this->getBgmList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
			auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
			cocos2d::DictElement *el2;
			CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<AudioInfo *>(el2->getObject());
#else
				auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
				if (p->isStop()) {
					AudioEngine::stop(p->getAudioId());
					bgmAudioList->removeObjectForElememt(el2);
					bBgmRemoved = true;
					goto lSkipBgmRemoved;
				}
			}
		}
		lSkipBgmRemoved:;
	} while (bBgmRemoved);

	//se
	bool bSeRemoved;
	do {
		bSeRemoved = false;
		auto seList = this->getSeList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
			auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
			cocos2d::DictElement *el2;
			CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<AudioInfo *>(el2->getObject());
#else
				auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
				if (p->isStop()) {
					AudioEngine::stop(p->getAudioId());
					seAudioList->removeObjectForElememt(el2);
					bSeRemoved = true;
					goto lSkipSeRemoved;
				}
			}
		}
		lSkipSeRemoved:;
	} while (bSeRemoved);

	//voice
	bool bVoiceRemoved;
	do {
		bVoiceRemoved = false;
		auto voiceList = this->getVoiceList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
			auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
			cocos2d::DictElement *el2;
			CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto p = static_cast<AudioInfo *>(el2->getObject());
#else
				auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
				if (p->isStop()) {
					AudioEngine::stop(p->getAudioId());
					voiceAudioList->removeObjectForElememt(el2);
					bVoiceRemoved = true;
					goto lSkipVoiceRemoved;
				}
			}
		}
		lSkipVoiceRemoved:;
	} while (bVoiceRemoved);
}
#endif

void AudioManager::updateDevice()
{
	if (!AudioEngine::checkChangeDevice()) {
		return;
	}

	auto bgmAudioInfoList = cocos2d::__Array::create();
	auto seAudioInfoList = cocos2d::__Array::create();
	auto voiceAudioInfoList = cocos2d::__Array::create();

	//bgm
	auto bgmList = this->getBgmList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(bgmAudioList, el2) {
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
			if(p) bgmAudioInfoList->addObject(p);
		}
	}
	//se
	auto seList = this->getSeList();
	CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
			if(p) seAudioInfoList->addObject(p);
		}
	}
	//voice
	auto voiceList = this->getVoiceList();
	CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
			if(p) voiceAudioInfoList->addObject(p);
		}
	}

	//停止
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(bgmAudioInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto audioInfo = static_cast<AudioInfo *>(ref);
#else
		auto audioInfo = dynamic_cast<AudioInfo *>(ref);
#endif
		float currentTime = AudioEngine::getCurrentTime(audioInfo->getAudioId());
		AudioEngine::stop(audioInfo->getAudioId());
		audioInfo->setCurrentTime(currentTime);
	}
	CCARRAY_FOREACH(seAudioInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto audioInfo = static_cast<AudioInfo *>(ref);
#else
		auto audioInfo = dynamic_cast<AudioInfo *>(ref);
#endif
		float currentTime = AudioEngine::getCurrentTime(audioInfo->getAudioId());
		AudioEngine::stop(audioInfo->getAudioId());
		audioInfo->setCurrentTime(currentTime);
	}
	CCARRAY_FOREACH(voiceAudioInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto audioInfo = static_cast<AudioInfo *>(ref);
#else
		auto audioInfo = dynamic_cast<AudioInfo *>(ref);
#endif
		float currentTime = AudioEngine::getCurrentTime(audioInfo->getAudioId());
		AudioEngine::stop(audioInfo->getAudioId());
		audioInfo->setCurrentTime(currentTime);
	}

	bgmList->removeAllObjects();
	seList->removeAllObjects();
	voiceList->removeAllObjects();

	//デバイス変更
	AudioEngine::changeDevice();

	//再生
	CCARRAY_FOREACH(bgmAudioInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto audioInfo = static_cast<AudioInfo *>(ref);
#else
		auto audioInfo = dynamic_cast<AudioInfo *>(ref);
#endif
		int volume = (int)(audioInfo->getVolume()->getValue() * 100.0f);
		playBgm(audioInfo->getId(), audioInfo->getIsLoop(), volume, audioInfo->getPan(), audioInfo->getPitch(), 0.0f, audioInfo->getCurrentTime());
	}
	CCARRAY_FOREACH(seAudioInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto audioInfo = static_cast<AudioInfo *>(ref);
#else
		auto audioInfo = dynamic_cast<AudioInfo *>(ref);
#endif
		int volume = (int)(audioInfo->getVolume()->getValue() * 100.0f);
		playSe(audioInfo->getId(), audioInfo->getIsLoop(), volume, audioInfo->getPan(), audioInfo->getPitch(), 0.0f, audioInfo->getCurrentTime());
	}
	CCARRAY_FOREACH(voiceAudioInfoList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto audioInfo = static_cast<AudioInfo *>(ref);
#else
		auto audioInfo = dynamic_cast<AudioInfo *>(ref);
#endif
		int volume = (int)(audioInfo->getVolume()->getValue() * 100.0f);
		playVoice(audioInfo->getId(), audioInfo->getIsLoop(), volume, audioInfo->getPan(), audioInfo->getPitch(), 0.0f, audioInfo->getCurrentTime());
	}
}

AudioManager::AudioInfo* AudioManager::playBgm(int id, bool loop)
{
	AudioManager::AudioInfo* ret = nullptr;

#if defined(USE_FADE)
	if (id < 0) return ret;
#else
	if (_nowBgmList[id]) {
		return ret;
	}
#endif
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return ret;
	auto p = project->getBgmData(id);
	if (p == nullptr) {
		CC_ASSERT(0);
		return ret;
	}
#if defined(USE_FADE)
	ret = this->playBgm(id, loop, p->getVolume(), p->getPan(), p->getPitch());
#else
	int audioId = AudioEngine::play2d(p->getFilename(), p->getLoop(), getBgmVolume() * p->getVolumeNormalize());// p->getPitch(), p->getPan());
	if (audioId < 0) {
		CC_ASSERT(0);
		return ret;
	}
	_nowBgmList[id] = audioId;
#endif
	return ret;
}

#if defined(USE_FADE)
AudioManager::AudioInfo* AudioManager::playBgm(int id, bool loop, int volume, int pan, int pitch, float seconds, float currentTime)
{
	AudioManager::AudioInfo* ret = nullptr;

	if (id < 0) return ret;
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return ret;
	auto p = project->getBgmData(id);
	if (p == nullptr) {
		CC_ASSERT(0);
		return ret;
	}

	float volume_ = (volume ? (float)volume / 100.0f : p->getVolumeNormalize());
	float initVolume_ = seconds > 0.0f ? 0.0f : volume_;
	cocos2d::Vec3 pan_ = cocos2d::Vec3((float)(pan ? pan : p->getPan()) / 50.0f, 0, 0);
	float pitch_ = calcPitch(pitch ? pitch : p->getPitch());

	int audioId = AudioEngine::play2d(p->getFilename(), loop, initVolume_ * this->getBgmVolume() * this->getVariableVolume(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableBgmVolumeAdjust), pitch_, (float *)&pan_, currentTime);
	if (audioId < 0) {
		CC_ASSERT(0);
		return ret;
	}
	//終了コールバッグを設定する。
	AudioEngine::setFinishCallback(audioId, [&](int audioID, std::string path) {
		auto bgmList = this->getBgmList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
			auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
			if (bgmAudioList->objectForKey(audioID)) {
				cocos2d::DictElement *el2;
				CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<AudioInfo *>(el2->getObject());
#else
					auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
					std::function<void(AudioManager::AudioInfo*)> _playEndCallBack = p->getPlayEndCallback();
					if (_playEndCallBack != nullptr) {
						_playEndCallBack(p);
					}
				}
			}
			bgmAudioList->removeObjectForKey(audioID);
		}
	});

	auto audioInfo = AudioInfo::create(id, audioId, initVolume_, loop, pan, pitch);
	ret = audioInfo;
#ifdef USE_OGG_LOOP
	audioInfo->setLoopInfo(audioId, p->getFilename());
#endif
	audioInfo->setFilename(p->getFilename());
	audioInfo->getVolume()->start(volume_, seconds);
	auto bgmList = this->getBgmList();
	if (bgmList->objectForKey(id)) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(bgmList->objectForKey(id));
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(bgmList->objectForKey(id));
#endif
		CC_ASSERT(bgmAudioList);
		bgmAudioList->setObject(audioInfo, audioId);
	}
	else {
		auto bgmAudioList = cocos2d::__Dictionary::create();
		bgmAudioList->setObject(audioInfo, audioId);
		bgmList->setObject(bgmAudioList, id);
	}

	return ret;
}
#endif

AudioManager::AudioInfo* AudioManager::playSe(int id)
{
	AudioManager::AudioInfo* ret = nullptr;

	if (id < 0) return ret;
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return ret;
	auto p = project->getSeData(id);
	if (p == nullptr) {
		CC_ASSERT(0);
		return ret;
	}
#if defined(USE_FADE)
	ret = this->playSe(id, p->getLoop(), p->getVolume(), p->getPan(), p->getPitch());
#else
	int audioId = AudioEngine::play2d(p->getFilename(), p->getLoop(), getSeVolume() * p->getVolumeNormalize());// p->getPitch(), p->getPan())
	if (audioId < 0) {
		return ret;
	}
	AudioEngine::setFinishCallback(audioId, CC_CALLBACK_2(AudioManager::finishCallback, this));
	_nowSeList[id] = audioId;
#endif

	return ret;
}

#if defined(USE_FADE)
AudioManager::AudioInfo* AudioManager::playSe(int id, bool loop, int volume, int pan, int pitch, float seconds, float currentTime)
{
	AudioManager::AudioInfo* ret = nullptr;

	if (id < 0) return ret;
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return ret;
	auto p = project->getSeData(id);
	if (p == nullptr) {
		CC_ASSERT(0);
		return ret;
	}

	//同じフレームで同じSEが10個を超えて再生されようとした場合は、11個目以降の再生を無視するようにする。
	if (_sameTimePlaySeMap.find(id) == _sameTimePlaySeMap.end()) {
		_sameTimePlaySeMap.insert(std::make_pair(id, 1));
	}
	else {
		auto count = _sameTimePlaySeMap[id];
		_sameTimePlaySeMap[id] = count + 1;
		if (count + 1 >= 11) {
			return ret;
		}
	}
	//同じSE再生しないように再生前に、再生中のSEを停止する。
	this->stopSe(id);

	//set value(volume, pan, pitch, ...)
	float _volume = (volume ? (float)volume / 100.0f : p->getVolumeNormalize());
	float _initVolume = seconds > 0.0f ? 0.0f : _volume;
	cocos2d::Vec3 _pan = cocos2d::Vec3((float)(pan ? pan : p->getPan()) / 50.0f, 0, 0);
	float _pitch = calcPitch(pitch ? pitch : p->getPitch());

	//再生
	int audioId = AudioEngine::play2d(p->getFilename(), loop, _initVolume * this->getSeVolume() * this->getVariableVolume(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableSeVolumeAdjust), _pitch, (float *)&_pan, currentTime);
	if (audioId < 0) {
		return ret;
	}

	//終了コールバッグを設定する。
	AudioEngine::setFinishCallback(audioId, [&](int audioID, std::string path) {
		auto seList = this->getSeList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
			auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
			if (seAudioList->objectForKey(audioID)) {
				cocos2d::DictElement *el2;
				CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<AudioInfo *>(el2->getObject());
#else
					auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
					std::function<void(AudioManager::AudioInfo*)> _playEndCallBack = p->getPlayEndCallback();
					if (_playEndCallBack != nullptr) {
						_playEndCallBack(p);
					}
				}
			}
			seAudioList->removeObjectForKey(audioID);
		}
	});
	auto audioInfo = AudioInfo::create(id, audioId, _initVolume, loop, pan, pitch );
	ret = audioInfo;
#ifdef USE_OGG_LOOP
	audioInfo->setLoopInfo(p->getFilename());
#endif
	audioInfo->setFilename(p->getFilename());
	audioInfo->getVolume()->start(_volume, seconds);
	auto seList = this->getSeList();
	cocos2d::__Dictionary *seAudioList = nullptr;
	if (seList->objectForKey(id)) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		seAudioList = static_cast<cocos2d::__Dictionary *>(seList->objectForKey(id));
#else
		seAudioList = dynamic_cast<cocos2d::__Dictionary *>(seList->objectForKey(id));
#endif
		CC_ASSERT(seAudioList);
	}
	else {
		seAudioList = cocos2d::__Dictionary::create();
		CC_ASSERT(seAudioList);
		seList->setObject(seAudioList, id);
	}
	seAudioList->setObject(audioInfo, audioId);

	return ret;
}
#endif

AudioManager::AudioInfo*  AudioManager::playVoice(int id)
{
	AudioManager::AudioInfo* ret = nullptr;

	if (id < 0) return ret;
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return ret;
	auto p = project->getVoiceData(id);
	if (p == nullptr) {
		CC_ASSERT(0);
		return ret;
	}
#if defined(USE_FADE)
	ret = this->playVoice(id, p->getLoop(), p->getVolume(), p->getPan(), p->getPitch());
#else
	int audioId = AudioEngine::play2d(p->getFilename(), p->getLoop(), getVoiceVolume() * p->getVolumeNormalize());// p->getPitch(), p->getPan());
	if (audioId < 0) {
		CC_ASSERT(0);
		return ret;
	}
	AudioEngine::setFinishCallback(audioId, CC_CALLBACK_2(AudioManager::finishCallback, this));
	_nowVoiceList[id] = audioId;
#endif
	return ret;
}

#if defined(USE_FADE)
AudioManager::AudioInfo*  AudioManager::playVoice(int id, bool loop, int volume, int pan, int pitch, float seconds, float currentTime)
{
	AudioManager::AudioInfo* ret = nullptr;

	if (id < 0) return ret;
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return ret;
	auto p = project->getVoiceData(id);
	if (p == nullptr) {
		CC_ASSERT(0);
		return ret;
	}

	float _volume = (volume ? (float)volume / 100.0f : p->getVolumeNormalize());
	float _initVolume = (seconds > 0.0f) ? 0.0f : _volume;
	cocos2d::Vec3 _pan = cocos2d::Vec3((float)(pan ? pan : p->getPan()) / 50.0f, 0, 0);
	float _pitch = calcPitch(pitch ? pitch : p->getPitch());

	int audioId = AudioEngine::play2d(p->getFilename(), loop, _initVolume * this->getVoiceVolume() * this->getVariableVolume(agtk::data::EnumProjectSystemVariable::kProjectSystemVariableVoiceVolumeAdjust), _pitch, (float *)&_pan, currentTime);
	if (audioId < 0) {
		CC_ASSERT(0);
		return ret;
	}

	//終了コールバッグを設定する。
	AudioEngine::setFinishCallback(audioId, [&](int audioID, std::string path) {
		auto voiceList = this->getVoiceList();
		cocos2d::DictElement *el;
		CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
			auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
			if (voiceAudioList->objectForKey(audioID)) {
				cocos2d::DictElement *el2;
				CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
					auto p = static_cast<AudioInfo *>(el2->getObject());
#else
					auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
					std::function<void(AudioManager::AudioInfo*)> _playEndCallBack = p->getPlayEndCallback();
					if (_playEndCallBack != nullptr) {
						_playEndCallBack(p);
					}
				}
			}
			voiceAudioList->removeObjectForKey(audioID);
		}
	});
	auto audioInfo = AudioInfo::create(id, audioId, _initVolume, loop, pan, pitch);
	ret = audioInfo;
#ifdef USE_OGG_LOOP
	audioInfo->setLoopInfo(p->getFilename());
#endif
	audioInfo->setFilename(p->getFilename());
	audioInfo->getVolume()->start(_volume, seconds);
	auto voiceList = this->getVoiceList();
	cocos2d::__Dictionary *voiceAudioList = nullptr;
	if (voiceList->objectForKey(id)) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		voiceAudioList = static_cast<cocos2d::__Dictionary *>(voiceList->objectForKey(id));
#else
		voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(voiceList->objectForKey(id));
#endif
		CC_ASSERT(voiceAudioList);
	}
	else {
		voiceAudioList = cocos2d::__Dictionary::create();
		CC_ASSERT(voiceAudioList);
		voiceList->setObject(voiceAudioList, id);
	}
	voiceAudioList->setObject(audioInfo, audioId);

	return ret;
}
#endif

#if defined(USE_FADE)
void AudioManager::stopBgm(int id, float seconds, int audioId)
{
	auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
	if (bgmAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	if (audioId != -1) {
		CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
			if (p->getAudioId() == audioId) {
				p->stop(seconds);
				break;
			}
		}
	}
	else {
		// audioId指定がなければ同じidのbgmを全て停止する
		CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
			p->stop(seconds);
		}
	}
}

void AudioManager::stopAllBgm(float seconds)
{
	auto bgmIdList = this->getBgmIdList();
	cocos2d::Ref* ref = nullptr;
	CCARRAY_FOREACH(bgmIdList, ref ){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		stopBgm(static_cast<Integer *>(ref)->getValue(), seconds);
#else
		stopBgm(dynamic_cast<Integer *>(ref)->getValue(), seconds);
#endif
	}
}

// オブジェクトが再生していないBGMのみ停止
void AudioManager::stopBgmNonObject(int id, float seconds, int audioId)
{
	auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
	if (bgmAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	if (audioId != -1) {
		CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
			if (p->getAudioId() == audioId) {
				if (p->getPlayEndCallback() == nullptr) {
					p->stop(seconds);
				}
				break;
			}
		}
	}
	else {
		// audioId指定がなければ同じidのbgmを全て停止する
		CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
			if (p->getPlayEndCallback() == nullptr) {
				p->stop(seconds);
			}
		}
	}
}

// オブジェクトが再生していないBGMをすべて停止
void AudioManager::stopAllBgmNonObject(float seconds)
{
	auto bgmIdList = this->getBgmIdList();
	cocos2d::Ref* ref = nullptr;
	CCARRAY_FOREACH(bgmIdList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		stopBgmNonObject(static_cast<Integer *>(ref)->getValue(), seconds);
#else
		stopBgmNonObject(dynamic_cast<Integer *>(ref)->getValue(), seconds);
#endif
	}
}
#else
void AudioManager::stopBgm(int id)
{
	auto itr = _nowBgmList.find(id);
	if(itr != _nowBgmList.end()){
		AudioEngine::stop(itr->second);
		_nowBgmList.erase(itr);
	}
}

void AudioManager::stopAllBgm()
{
	for (auto itr = _nowBgmList.begin(); itr != _nowBgmList.end(); ++itr) {
		AudioEngine::stop(itr->second);
	}
	_nowBgmList.clear();
}
#endif

#if defined(USE_FADE)
void AudioManager::stopSe(int id, float seconds, int audioId)
{
	auto seList = this->getSeList();
	if (seList == nullptr || seList->count() == 0) {
		return;
	}
	auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(seList->objectForKey(id));
	if (seAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	if (audioId != -1) {
		CCDICT_FOREACH(seAudioList, el) {
			auto audioInfo = dynamic_cast<AudioInfo *>(el->getObject());
			if (audioInfo && audioInfo->getAudioId() == audioId) {
				audioInfo->stop(seconds);
				break;
			}
		}
	}
	else {
		// audioId指定がなければ同じidのseを全て停止する
		CCDICT_FOREACH(seAudioList, el) {
			auto audioInfo = dynamic_cast<AudioInfo *>(el->getObject());
			if (audioInfo) {
				audioInfo->stop(seconds);
			}
		}
	}
}

void AudioManager::stopAllSe(float seconds)
{
	auto seList = this->getSeList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->stop(seconds);
		}
	}
}
#else
void AudioManager::stopSe(int id)
{
	auto itr = _nowSeList.find(id);
	if(itr != _nowSeList.end()){
		AudioEngine::stop(itr->second);
		_nowVoiceList.erase(itr);
	}
}
#endif

#if defined(USE_FADE)
void AudioManager::stopVoice(int id, float seconds, int audioId)
{
	auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(id));
	if (voiceAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	if (audioId != -1) {
		CCDICT_FOREACH(voiceAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
			if (p->getAudioId() == audioId) {
				p->stop(seconds);
				break;
			}
		}
	}
	else {
		// audioId指定がなければ同じidの音声を全て停止する
		CCDICT_FOREACH(voiceAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
			p->stop(seconds);
		}
	}
}

void AudioManager::stopAllVoice(float seconds)
{
	auto voiceList = this->getVoiceList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->stop(seconds);
		}
	}
}
#else
void AudioManager::stopVoice(int id)
{
	auto itr = _nowVoiceList.find(id);
	if(itr != _nowVoiceList.end()){
		AudioEngine::stop(itr->second);
		_nowVoiceList.erase(itr);
	}
}
#endif

#if defined(USE_FADE)
void AudioManager::stopAll(float seconds)
{
	this->stopAllBgm(seconds);
	this->stopAllSe(seconds);
	this->stopAllVoice(seconds);
}
#else
void AudioManager::stopAll()
{
	AudioEngine::stopAll();
	_nowBgmList.clear();
	_nowSeList.clear();
	_nowVoiceList.clear();
}
#endif

#if defined(USE_FADE)
void AudioManager::setVolume(float volume, float seconds)
{
	this->setBgmVolume(volume, seconds);
	this->setSeVolume(volume, seconds);
	this->setVoiceVolume(volume, seconds);
}
#else
void AudioManager::setVolume(float volume)
{
	this->setBgmVolume(volume);
	this->setSeVolume(volume);
	this->setVoiceVolume(volume);
}
#endif

float AudioManager::getBgmVolume()
{
#if defined(USE_FADE)
	return this->getMute() ? 0.0f : _bgmVolume ? _bgmVolume->getValue() : 0.0f;
#else
	return this->getMute() ? 0.0f : _bgmVolume;
#endif
}

float AudioManager::getSeVolume()
{
#if defined(USE_FADE)
	return this->getMute() ? 0.0f : _seVolume ? _seVolume->getValue() : 0.0f;
#else
	return this->getMute() ? 0.0f : _seVolume;
#endif
}

float AudioManager::getVoiceVolume()
{
#if defined(USE_FADE)
	return this->getMute() ? 0.0f : _voiceVolume ? _voiceVolume->getValue() : 0.0f;
#else
	return this->getMute() ? 0.0f : _voiceVolume;
#endif
}

// プロジェクト共通変数のボリュームを取得
float AudioManager::getVariableVolume(agtk::data::EnumProjectSystemVariable kProjectSystemVariableId)
{
	float variableVolume = 0;
	if (kProjectSystemVariableId < agtk::data::EnumProjectSystemVariable::kProjectSystemVariableBgmVolumeAdjust ||
		kProjectSystemVariableId > agtk::data::EnumProjectSystemVariable::kProjectSystemVariableVoiceVolumeAdjust) {
		return variableVolume;
	}

	auto gm = GameManager::getInstance();
	auto volumeVariableData = gm->getPlayData()->getVariableData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, kProjectSystemVariableId);
	variableVolume = volumeVariableData->getValue() / 100; // パーセント
	return variableVolume;
}

// BGMの再生位置を取得
float AudioManager::getBgmCurrentTime(int bgmId, int audioId) {
	auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(bgmId));
	if (bgmAudioList == nullptr) {
		return -1;
	}

	float _currentTime = -1;
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		int _audioId = p->getAudioId();
		if (audioId != -1) {
			if (_audioId == audioId) {
				_currentTime = AudioEngine::getCurrentTime(_audioId);
				break;
			}
		}
		else {
			// audioIdの指定がない場合は同じbgmIdでも最後に格納されたものを返す
			_currentTime = AudioEngine::getCurrentTime(_audioId);
		}

	}
	return _currentTime;
}

float AudioManager::getSeCurrentTime(int seId, int audioId) {
	auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getSeList()->objectForKey(seId));
	if (seAudioList == nullptr) {
		return -1;
	}
	float _currentTime = -1;
	cocos2d::DictElement *el;
	CCDICT_FOREACH(seAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		int _audioId = p->getAudioId();
		if (audioId != -1) {
			if (_audioId == audioId) {
				_currentTime = AudioEngine::getCurrentTime(_audioId);
				break;
			}
		}
		else {
			_currentTime = AudioEngine::getCurrentTime(_audioId);
		}
	}
	return _currentTime;
}

float AudioManager::getVoiceCurrentTime(int voiceId, int audioId) {
	auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(voiceId));
	if (voiceAudioList == nullptr) {
		return -1;
	}
	float _currentTime = -1;
	cocos2d::DictElement *el;
	CCDICT_FOREACH(voiceAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		int _audioId = p->getAudioId();
		if (audioId != -1) {
			if (_audioId == audioId) {
				_currentTime = AudioEngine::getCurrentTime(_audioId);
				break;
			}
		}
		else {
			_currentTime = AudioEngine::getCurrentTime(_audioId);
		}

	}
	return _currentTime;
}

#if defined(USE_FADE)
void AudioManager::setBgmVolume(float volume, float seconds)
{
	if (!_bgmVolume) return;
	_bgmVolume->start(volume, seconds);
}
#else
void AudioManager::setBgmVolume(float volume)
{
	_bgmVolume = volume;
	for(auto itr : _nowBgmList){
		AudioEngine::setVolume(itr.second, getBgmVolume());
	}
}
#endif

#if defined(USE_FADE)
void AudioManager::setSeVolume(float volume, float seconds)
{
	if (!_seVolume) return;
	_seVolume->start(volume, seconds);
}
#else
void AudioManager::setSeVolume(float volume)
{
	_seVolume = volume;
	for(auto itr : _nowSeList){
		AudioEngine::setVolume(itr.second, getSeVolume());
	}
}
#endif

#if defined(USE_FADE)
void AudioManager::setVoiceVolume(float volume, float seconds)
{
	if (!_voiceVolume) return;
	_voiceVolume->start(volume, seconds);
}
#else
void AudioManager::setVoiceVolume(float volume)
{
	_voiceVolume = volume;
	for(auto itr : _nowVoiceList){
		AudioEngine::setVolume(itr.second, getVoiceVolume());
	}
}
#endif

void AudioManager::pauseBgm(int id)
{
#if defined(USE_FADE)
	auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
	if (bgmAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		p->pause();
		AudioEngine::pause(p->getAudioId());
	}
#else
	auto itr = _nowBgmList.find(id);
	if(itr != _nowBgmList.end()){
		AudioEngine::pause(itr->second);
	}
#endif
}

void AudioManager::pauseSe(int id)
{
#if defined(USE_FADE)
	auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getSeList()->objectForKey(id));
	if (seAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	CCDICT_FOREACH(seAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		p->pause();
		AudioEngine::pause(p->getAudioId());
	}
#else
	auto itr = _nowSeList.find(id);
	if(itr != _nowSeList.end()){
		AudioEngine::pause(itr->second);
	}
#endif
}

void AudioManager::pauseVoice(int id)
{
#if defined(USE_FADE)
	auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(id));
	if (voiceAudioList == nullptr) {
		return;
	}
	cocos2d::DictElement *el;
	CCDICT_FOREACH(voiceAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		p->pause();
		AudioEngine::pause(p->getAudioId());
	}
#else
	auto itr = _nowVoiceList.find(id);
	if(itr != _nowVoiceList.end()){
		AudioEngine::pause(itr->second);
	}
#endif
}

void AudioManager::pauseAll()
{
	AudioEngine::pauseAll();
#if defined(USE_FADE)
	//bgm
	auto bgmList = this->getBgmList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->pause();
		}
	}
	//se
	auto seList = this->getSeList();
	CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->pause();
		}
	}
	//voice
	auto voiceList = this->getVoiceList();
	CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->pause();
		}
	}
#endif
}

void AudioManager::resumeBgm(int id)
{
#if defined(USE_FADE)
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
#else
	auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
#endif
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		p->resume();
		AudioEngine::resume(p->getAudioId());
	}
#else
	auto itr = _nowBgmList.find(id);
	if(itr != _nowBgmList.end()){
		AudioEngine::resume(itr->second);
	}
#endif
}

void AudioManager::resumeSe(int id)
{
#if defined(USE_FADE)
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto seAudioList = static_cast<cocos2d::__Dictionary *>(this->getSeList()->objectForKey(id));
#else
	auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getSeList()->objectForKey(id));
#endif
	cocos2d::DictElement *el;
	CCDICT_FOREACH(seAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		p->resume();
		AudioEngine::resume(p->getAudioId());
	}
#else
	auto itr = _nowSeList.find(id);
	if(itr != _nowSeList.end()){
		AudioEngine::resume(itr->second);
	}
#endif
}

void AudioManager::resumeVoice(int id)
{
#if defined(USE_FADE)
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(id));
#else
	auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(id));
#endif
	cocos2d::DictElement *el;
	CCDICT_FOREACH(voiceAudioList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AudioInfo *>(el->getObject());
#else
		auto p = dynamic_cast<AudioInfo *>(el->getObject());
#endif
		p->resume();
		AudioEngine::resume(p->getAudioId());
	}
#else
	auto itr = _nowVoiceList.find(id);
	if(itr != _nowVoiceList.end()){
		AudioEngine::resume(itr->second);
	}
#endif
}

void AudioManager::resumeAll()
{
	AudioEngine::resumeAll();
#if defined(USE_FADE)
	//bgm
	auto bgmList = this->getBgmList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(bgmAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->resume();
		}
	}
	//se
	auto seList = this->getSeList();
	CCDICT_FOREACH(seList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto seAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(seAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->resume();
		}
	}
	//voice
	auto voiceList = this->getVoiceList();
	CCDICT_FOREACH(voiceList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		cocos2d::DictElement *el2;
		CCDICT_FOREACH(voiceAudioList, el2) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto p = static_cast<AudioInfo *>(el2->getObject());
#else
			auto p = dynamic_cast<AudioInfo *>(el2->getObject());
#endif
			p->resume();
		}
	}
#endif
}

bool AudioManager::isPlayingBgm(int id)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
#else
	auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getBgmList()->objectForKey(id));
#endif
	CC_ASSERT(bgmAudioList);
	return (bgmAudioList->count() > 0) ? true : false;
}

bool AudioManager::isPlayingSe(int id)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto seAudioList = static_cast<cocos2d::__Dictionary *>(this->getSeList()->objectForKey(id));
#else
	auto seAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getSeList()->objectForKey(id));
#endif
	CC_ASSERT(seAudioList);
	return (seAudioList->count() > 0) ? true : false;
}

bool AudioManager::isPlayingVoice(int id)
{
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	auto voiceAudioList = static_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(id));
#else
	auto voiceAudioList = dynamic_cast<cocos2d::__Dictionary *>(this->getVoiceList()->objectForKey(id));
#endif
	CC_ASSERT(voiceAudioList);
	return (voiceAudioList->count() > 0) ? true : false;
}

void AudioManager::preloadSe(int id)
{
	this->preloadSe(id, nullptr);
}

void AudioManager::preloadSe(int id, std::function<void(bool isSuccess)> callback)
{
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return;
	auto p = project->getSeData(id);
	if (p) {
		AudioEngine::preload(p->getFilename(), callback);
	}
}

void AudioManager::preloadBgm(int id, std::function<void(bool isSuccess)> callback)
{
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return;
	auto p = project->getBgmData(id);
	if (p) {
		AudioEngine::preload(p->getFilename(), callback);
	}
}

void AudioManager::preloadVoice(int id, std::function<void(bool isSuccess)> callback)
{
	auto project = GameManager::getInstance()->getProjectData();
	if (project == nullptr) return;
	auto p = project->getVoiceData(id);
	if (p) {
		AudioEngine::preload(p->getFilename(), callback);
	}
}

void AudioManager::uncacheAll()
{
	AudioEngine::uncacheAll();
}

#if defined(USE_FADE)
#else
void AudioManager::finishCallback(int audioID, std::string path)
{
	//CCLOG("finishCallback:%d:%s", audioID, path.c_str());
	for(auto itr : _nowBgmList){
		if(itr.second == audioID){
			_nowBgmList.erase(itr.first);
			return;
		}
	}
	for(auto itr : _nowVoiceList){
		if(itr.second == audioID){
			_nowVoiceList.erase(itr.first);
			return;
		}
	}
	for (auto itr : _nowSeList) {
		if (itr.second == audioID) {
			_nowSeList.erase(itr.first);
			return;
		}
	}
/*	for(auto itr : _nowSeList){
		for(auto itr2 = itr.second.begin(); itr2 != itr.second.end(); itr2++){
			if(*itr2 == audioID){
				itr.second.erase(itr2);
				return;
			}
		}
	}*/
}
#endif

cocos2d::__Array* AudioManager::getBgmIdList()
{
#if defined(USE_FADE)
	auto bgmIdList = cocos2d::__Array::create();
	auto bgmList = this->getBgmList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(bgmList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto bgmAudioList = static_cast<cocos2d::__Dictionary *>(el->getObject());
#else
		auto bgmAudioList = dynamic_cast<cocos2d::__Dictionary *>(el->getObject());
#endif
		if (bgmAudioList->count() > 0) {
			bgmIdList->addObject(cocos2d::__Integer::create(el->getIntKey()));
		}
	}
	return bgmIdList;
#else
	auto arr = cocos2d::__Array::create();
	for (auto itr : _nowBgmList) {
		arr->addObject(cocos2d::Integer::create(itr.first));
	}
	return arr;
#endif
}

cocos2d::__Array* AudioManager::getSeIdList()
{
#if defined(USE_FADE)
	return this->getSeList()->allKeys();
#else
	auto arr = cocos2d::__Array::create();
	for (auto itr : _nowSeList) {
		arr->addObject(cocos2d::Integer::create(itr.first));
	}
	return arr;
#endif
}

cocos2d::__Array* AudioManager::getVoiceIdList()
{
#if defined(USE_FADE)
	return this->getVoiceList()->allKeys();
#else
	auto arr = cocos2d::__Array::create();
	for (auto itr : _nowVoiceList) {
		arr->addObject(cocos2d::Integer::create(itr.first));
	}
	return arr;
#endif
}

/**
 * BGMのボリュームをフェードアウトする
 * @param	duration	フェード時間
 * @param	toVolume	最終ボリューム
 * @return				FadeToBgmVolume インスタンス
 */
FadeToBgmVolume *AudioManager::fadeOutBgmVolume(float duration, float toVolume)
{
	return FadeToBgmVolume::create(duration, getBgmVolume(), toVolume);
}

// オーディオエンジンのインスタンスがあるかチェックする。
bool AudioManager::isAudioEngine()
{
	return AudioEngine::isAudioEngineImpl();
}

//--------------------------------------------------------------------------------------------------------------------
// AudioManager::AudioInfo クラス
//--------------------------------------------------------------------------------------------------------------------
void AudioManager::AudioInfo::update(float delta)
{
	this->getVolume()->update(delta);
#ifdef USE_OGG_LOOP
	if (_isLoop && _loopStartTime >= 0) {
		//ループ情報あり。
		//OGGにLOOPSTART, LOOPLENGTHが設定されていたらその区間をループさせる。
		auto currentTime = AudioEngine::getCurrentTime(_audioId);
		if (!_looping && currentTime >= _loopStartTime) {
			setLooping(true);
		}
		if (_looping && (currentTime < _loopStartTime || currentTime >= _loopEndTime)) {
			//log("Audio LOOP: %f --> %f", currentTime, _loopStartTime);
			//AudioEngine::pause(_audioId);
			if (currentTime < _loopStartTime) {
				AudioEngine::setCurrentTime(_audioId, _loopStartTime + currentTime);
			}
			else {
				AudioEngine::setCurrentTime(_audioId, _loopStartTime + (currentTime - _loopEndTime));
			}
			//AudioEngine::resume(_audioId);
		}
		else {
			//log("Audio: %f", currentTime);
		}
	}
#endif
}

#ifdef USE_OGG_LOOP
void AudioManager::AudioInfo::setLoopInfo(const char *filename)
{
	uint32_t totalFrames;
	uint32_t sampleRate;
	uint32_t loopStart;
	uint32_t loopLength;
	AudioEngine::getLoopInfo(filename, &totalFrames, &sampleRate, &loopStart, &loopLength);
	if (loopStart != 0xffffffff && loopLength != 0xffffffff) {
		this->setLoopStartTime((float)loopStart / sampleRate);
		this->setLoopEndTime((float)(loopStart + loopLength) / sampleRate);
		CCLOG("OGG LOOP INFO: [%f, %f): %d, %d, %d, %d", this->getLoopStartTime(), this->getLoopEndTime(), loopStart, loopLength, totalFrames, sampleRate);
	}
}

void AudioManager::AudioInfo::setLoopInfo(int audioID, const char *filename)
{
	uint32_t totalFrames;
	uint32_t sampleRate;
	uint32_t loopStart;
	uint32_t loopLength;
	if (AudioEngine::isAudioEngineImpl()) {
		AudioEngine::getLoopInfo(audioID, &totalFrames, &sampleRate, &loopStart, &loopLength);
	}
	else {
		AudioEngine::getLoopInfo(filename, &totalFrames, &sampleRate, &loopStart, &loopLength);
	}
	if (loopStart != 0xffffffff && loopLength != 0xffffffff) {
		this->setLoopStartTime((float)loopStart / sampleRate);
		this->setLoopEndTime((float)(loopStart + loopLength) / sampleRate);
		CCLOG("OGG LOOP INFO: [%f, %f): %d, %d, %d, %d", this->getLoopStartTime(), this->getLoopEndTime(), loopStart, loopLength, totalFrames, sampleRate);
	}
}
#endif


//--------------------------------------------------------------------------------------------------------------------
// FadeToBgmVolume クラス
//--------------------------------------------------------------------------------------------------------------------
/**
* 生成
* @param	duration	フェード時間
* @param	toVolume	開始ボリューム
* @param	toVolume	最終ボリューム
* @return				FadeToBgmVolume インスタンス
*/
FadeToBgmVolume *FadeToBgmVolume::create(float duration, float fromVolume, float toVolume)
{
	auto ret = new FadeToBgmVolume();
	if (ret && ret->initWithDurationVolume(duration, fromVolume, toVolume)) {
		ret->autorelease();
	}
	else {
		CC_SAFE_DELETE(ret);
	}
	return ret;
}

/**
* 初期化
* @param	duration	フェード時間
* @param	toVolume	開始ボリューム
* @param	toVolume	最終ボリューム
* @return				初期化の成否
*/
bool FadeToBgmVolume::initWithDurationVolume(float duration, float fromVolume, float toVolume)
{
	if (!ActionInterval::initWithDuration(duration)) {
		return false;
	}

	_fromVolume = fromVolume;
	_toVolume = toVolume;

	return true;
}

/**
* 更新
* @param	delta	前フレームからの経過時間
*/
void FadeToBgmVolume::update(float delta)
{
	auto newVol = (_fromVolume + (_toVolume - _fromVolume) * delta);
	AudioManager::getInstance()->setBgmVolume(newVol);
}
