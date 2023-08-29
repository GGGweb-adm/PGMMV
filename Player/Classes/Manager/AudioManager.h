#ifndef __AUDIO_MANAGER_H__
#define	__AUDIO_MANAGER_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Data/ProjectData.h"
#include "Lib/Common.h"
#include "Data/PlayData.h"

USING_NS_CC;

#define USE_FADE
#define USE_OGG_LOOP

class FadeToBgmVolume;
class AGTKPLAYER_API AudioManager : public cocos2d::Ref
{
#if defined(USE_FADE)
public:
	class AudioInfo : public cocos2d::Ref {
	public:
		AudioInfo() {
			_id = -1;
			_audioId = -1;
			_volume = nullptr;
			_initVolume = 0.0f;
			_stopFlag = false;
			_isLoop = false;
			_pan = 0;
			_pitch = 0;
#ifdef USE_OGG_LOOP
			_loopStartTime = -1;
			_loopEndTime = -1;
			_looping = false;
#endif
			_playEndCallback = nullptr;
			_playEndCallbackObject = nullptr;
			_currentTime = 0.0f;
		}
		virtual ~AudioInfo() {
			CC_SAFE_RELEASE_NULL(_volume);
		}
	public:
		CREATE_FUNC_PARAM6(AudioInfo, int, id, int, audioId, float, volume, bool, isLoop, int ,pan, int ,pitch);
		virtual void update(float delta);
		void pause() { this->getVolume()->pause(); }
		void resume() { this->getVolume()->resume(); }
		void stop(float seconds) {
			this->getVolume()->start(0.0f, seconds);
			_stopFlag = true;
		}
		bool isStop() { return (_stopFlag == true) && (this->getVolume()->getValue() == 0.0f); }
#ifdef USE_OGG_LOOP
		void setLoopInfo(const char *filename);
		void setLoopInfo(int audioID, const char *filename);
#endif
	private:
		virtual bool init(int id, int audioId, float volume, bool isLoop, int pan, int pitch) {
			this->setId(id);
			this->setAudioId(audioId);
			this->setVolume(agtk::ValueTimer<float>::create(volume));
			this->getVolume()->onInterpolateFunc = [&](float s, float e, float range, float pos) {
				return AGTK_LINEAR_INTERPOLATE(s, e, range, pos);
			};
			this->setInitVolume(volume);
			this->setIsLoop(isLoop);
			this->setPan(pan);
			this->setPitch(pitch);
			return true;
		}
	private:
		CC_SYNTHESIZE(int, _id, Id);
		CC_SYNTHESIZE(int, _audioId, AudioId);
		CC_SYNTHESIZE_RETAIN(agtk::ValueTimer<float> *, _volume, Volume);
		bool _stopFlag;
		CC_SYNTHESIZE(bool, _isLoop, IsLoop);
		CC_SYNTHESIZE(int, _pan, Pan);
		CC_SYNTHESIZE(int, _pitch, Pitch);
#ifdef USE_OGG_LOOP
		CC_SYNTHESIZE(float, _loopStartTime, LoopStartTime);
		CC_SYNTHESIZE(float, _loopEndTime, LoopEndTime);
		CC_SYNTHESIZE(bool, _looping, Looping);
#endif
		CC_SYNTHESIZE(std::function<void(AudioManager::AudioInfo*)>, _playEndCallback, PlayEndCallback);
		CC_SYNTHESIZE(agtk::Object*, _playEndCallbackObject, PlayEndCallbackObject);
		CC_SYNTHESIZE(float, _initVolume, InitVolume);
		CC_SYNTHESIZE(std::string, _filename, Filename);
		CC_SYNTHESIZE(float, _currentTime, CurrentTime);
	};
#endif
private:
	AudioManager();
	static AudioManager *_audioManager;

public:
	virtual ~AudioManager();
	static AudioManager* getInstance();
	static void purge();
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#else
	bool AudioManager::init(agtk::data::SoundSettingData *soundSettingData);
#endif
public:
	AudioManager::AudioInfo* playBgm(int id, bool loop = true);
#if defined(USE_FADE)
	AudioManager::AudioInfo* playBgm(int id, bool loop, int volume, int pan, int pitch, float seconds = 0.0f, float currentTime = 0.0f);
#endif
	AudioManager::AudioInfo* playSe(int id);
#if defined(USE_FADE)
	AudioManager::AudioInfo* playSe(int id, bool loop, int volume, int pan, int pitch, float seconds = 0.0f, float currentTime = 0.0f);
#endif
	AudioManager::AudioInfo* playVoice(int id);
#if defined(USE_FADE)
	AudioManager::AudioInfo* playVoice(int id, bool loop, int volume, int pan, int pitch, float seconds = 0.0f, float currentTime = 0.0f);
#endif
#if defined(USE_FADE)
	void stopBgm(int id, float seconds = 0.0f, int audioId = -1);
	void stopAllBgm(float seconds = 0.0f);
	void stopBgmNonObject(int id, float seconds = 0.0f, int audioId = -1);
	void stopAllBgmNonObject(float seconds = 0.0f);
#else
	void stopBgm(int id);
	void stopAllBgm();
#endif
#if defined(USE_FADE)
	void stopSe(int id, float seconds = 0.0f, int audioId = -1);
	void stopAllSe(float seconds = 0.0f);
#else
	void stopSe(int id);
	void stopAllSe();
#endif
#if defined(USE_FADE)
	void stopVoice(int id, float seconds = 0.0f, int audioId = -1);
	void stopAllVoice(float seconds = 0.0f);
#else
	void stopVoice(int id);
	void stopAllVoice();
#endif
#if defined(USE_FADE)
	void stopAll(float seconds = 0.0f);
#else
	void stopAll();
#endif
	void pauseBgm(int id);
	void pauseSe(int id);
	void pauseVoice(int id);
	void pauseAll();
	void resumeBgm(int id);
	void resumeSe(int id);
	void resumeVoice(int id);
	void resumeAll();
	bool isPlayingBgm(int id);
	bool isPlayingSe(int id);
	bool isPlayingVoice(int id);
public:
	void uncacheAll();
public:
	void preloadSe(int id);
	void preloadSe(int id, std::function<void(bool isSuccess)> callback);
	void preloadBgm(int id, std::function<void(bool isSuccess)> callback);
	void preloadVoice(int id, std::function<void(bool isSuccess)> callback);
#if defined(USE_FADE)
	virtual void update(float delta);
	void updateStop();
#endif
	void updateDevice();
public:
#if defined(USE_FADE)
	void setVolume(float volume, float seconds = 0.0f);
#else
	void setVolume(float volume);
#endif
	float getBgmVolume();
	float getSeVolume();
	float getVoiceVolume();
	float getVariableVolume(agtk::data::EnumProjectSystemVariable kProjectSystemVariableId);
	float getBgmCurrentTime(int bgmId, int audioId = -1);
	float getSeCurrentTime(int seId, int audioId = -1);
	float getVoiceCurrentTime(int voiceId, int audioId = -1);
#if defined(USE_FADE)
	void setBgmVolume(float volume, float seconds = 0.0f);
	void setSeVolume(float volume, float seconds = 0.0f);
	void setVoiceVolume(float volume, float seconds = 0.0f);
#else
	void setBgmVolume(float volume);
	void setSeVolume(float volume);
	void setVoiceVolume(float volume);
#endif
	FadeToBgmVolume *fadeOutBgmVolume(float duration, float toVolume);
	bool isAudioEngine();

public:
	cocos2d::__Array* getBgmIdList();
	cocos2d::__Array* getSeIdList();
	cocos2d::__Array* getVoiceIdList();
#if defined(USE_FADE)
#else
private:
	void finishCallback(int audioID, std::string path);
#endif

private:
	CC_SYNTHESIZE(bool, _mute, Mute);
#if defined(USE_FADE)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _bgmList, BgmList);
#else
	std::map<int, int> _nowBgmList;
#endif
#if defined(USE_FADE)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _seList, SeList)
#else
	std::map<int, int> _nowSeList;
#endif
#if defined(USE_FADE)
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _voiceList, VoiceList);
#else
	std::map<int, int> _nowVoiceList;
#endif
#if defined(USE_FADE)
	agtk::ValueTimer<float> *_bgmVolume;
	agtk::ValueTimer<float> *_seVolume;
	agtk::ValueTimer<float> *_voiceVolume;
#else
	float _bgmVolume;
	float _seVolume;
	float _voiceVolume;
#endif
	std::map<int, int> _sameTimePlaySeMap;
};

class AGTKPLAYER_API FadeToBgmVolume : public cocos2d::ActionInterval
{
public:
	FadeToBgmVolume() {};
	virtual ~FadeToBgmVolume() {};

	static FadeToBgmVolume* create(float duration, float fromVolume, float toVolume);
	virtual void update(float delta) override;

protected:
	virtual bool initWithDurationVolume(float duration, float fromVolume, float toVolume);

private:
	float _toVolume;
	float _fromVolume;
};
#endif	//__AUDIO_MANAGER_H__
