/****************************************************************************
 Copyright (c) 2014-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "platform/CCPlatformConfig.h"

#include "audio/include/AudioEngine.h"
#include <condition_variable>
#include <queue>
#include "platform/CCFileUtils.h"
#include "base/ccUtils.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "audio/android/AudioEngine-inl.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC
#include "audio/apple/AudioEngine-inl.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "audio/win32/AudioEngine-win32.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WINRT
#include "audio/winrt/AudioEngine-winrt.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_LINUX
#include "audio/linux/AudioEngine-linux.h"
#elif CC_TARGET_PLATFORM == CC_PLATFORM_TIZEN
#include "audio/tizen/AudioEngine-tizen.h"
// #AGTK-NX
#elif CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif

#define TIME_DELAY_PRECISION 0.0001

#ifdef ERROR
#undef ERROR
#endif // ERROR

using namespace cocos2d;
using namespace cocos2d::experimental;

const int AudioEngine::INVALID_AUDIO_ID = -1;
const float AudioEngine::TIME_UNKNOWN = -1.0f;

//audio file path,audio IDs
std::unordered_map<std::string,std::list<int>> AudioEngine::_audioPathIDMap;
//profileName,ProfileHelper
std::unordered_map<std::string, AudioEngine::ProfileHelper> AudioEngine::_audioPathProfileHelperMap;
unsigned int AudioEngine::_maxInstances = MAX_AUDIOINSTANCES;
AudioEngine::ProfileHelper* AudioEngine::_defaultProfileHelper = nullptr;
std::unordered_map<int, AudioEngine::AudioInfo> AudioEngine::_audioIDInfoMap;
AudioEngineImpl* AudioEngine::_audioEngineImpl = nullptr;

AudioEngine::AudioEngineThreadPool* AudioEngine::s_threadPool = nullptr;
bool AudioEngine::_isEnabled = true;

AudioEngine::AudioInfo::AudioInfo()
: filePath(nullptr)
, profileHelper(nullptr)
, volume(1.0f)
#ifdef USE_AGTK//hidet-sa
, pitch(1.0f)
#endif
, loop(false)
, duration(TIME_UNKNOWN)
, state(AudioState::INITIALIZING)
{
#ifdef USE_AGTK//hidet-sa
	memset(pan, 0, sizeof(pan));
#endif
}

AudioEngine::AudioInfo::~AudioInfo()
{
}

class AudioEngine::AudioEngineThreadPool
{
public:
    AudioEngineThreadPool(int threads = 4)
        : _stop(false)
    {
        for (int index = 0; index < threads; ++index)
        {
            _workers.emplace_back(std::thread(std::bind(&AudioEngineThreadPool::threadFunc, this)));
        }
    }

    void addTask(const std::function<void()> &task){
        std::unique_lock<std::mutex> lk(_queueMutex);
        _taskQueue.emplace(task);
        _taskCondition.notify_one();
    }

    ~AudioEngineThreadPool()
    {
        {
            std::unique_lock<std::mutex> lk(_queueMutex);
            _stop = true;
            _taskCondition.notify_all();
        }

        for (auto&& worker : _workers) {
            worker.join();
        }
    }

private:
    void threadFunc()
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif
		while (true) {
            std::function<void()> task = nullptr;
            {
                std::unique_lock<std::mutex> lk(_queueMutex);
                if (_stop)
                {
                    break;
                }
                if (!_taskQueue.empty())
                {
                    task = std::move(_taskQueue.front());
                    _taskQueue.pop();
                }
                else
                {
                    _taskCondition.wait(lk);
                    continue;
                }
            }

            task();
        }
    }

    std::vector<std::thread>  _workers;
    std::queue< std::function<void()> > _taskQueue;

    std::mutex _queueMutex;
    std::condition_variable _taskCondition;
    bool _stop;
};

void AudioEngine::end()
{
    if (s_threadPool)
    {
        delete s_threadPool;
        s_threadPool = nullptr;
    }

    delete _audioEngineImpl;
    _audioEngineImpl = nullptr;

    delete _defaultProfileHelper;
    _defaultProfileHelper = nullptr;
}

bool AudioEngine::lazyInit()
{
    if (_audioEngineImpl == nullptr)
    {
        _audioEngineImpl = new (std::nothrow) AudioEngineImpl();
        if(!_audioEngineImpl ||  !_audioEngineImpl->init() ){
            delete _audioEngineImpl;
            _audioEngineImpl = nullptr;
           return false;
        }
    }

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
    if (_audioEngineImpl && s_threadPool == nullptr)
    {
        s_threadPool = new (std::nothrow) AudioEngineThreadPool();
    }
#endif

    return true;
}

#ifdef USE_AGTK//hidet-sa
int AudioEngine::play2d(const std::string& filePath, bool loop, float volume, float pitch, float *pan, float currentTime, const AudioProfile *profile)
#else
int AudioEngine::play2d(const std::string& filePath, bool loop, float volume, const AudioProfile *profile)
#endif
{
    int ret = AudioEngine::INVALID_AUDIO_ID;

    do {
        if (!isEnabled())
        {
            break;
        }
        
        if ( !lazyInit() ){
            break;
        }

        if ( !FileUtils::getInstance()->isFileExist(filePath)){
            break;
        }

        auto profileHelper = _defaultProfileHelper;
        if (profile && profile != &profileHelper->profile){
            CC_ASSERT(!profile->name.empty());
            profileHelper = &_audioPathProfileHelperMap[profile->name];
            profileHelper->profile = *profile;
        }
        
        if (_audioIDInfoMap.size() >= _maxInstances) {
            log("Fail to play %s cause by limited max instance of AudioEngine",filePath.c_str());
            break;
        }
        if (profileHelper)
        {
             if(profileHelper->profile.maxInstances != 0 && profileHelper->audioIDs.size() >= profileHelper->profile.maxInstances){
                 log("Fail to play %s cause by limited max instance of AudioProfile",filePath.c_str());
                 break;
             }
             if (profileHelper->profile.minDelay > TIME_DELAY_PRECISION) {
                 auto currTime = utils::gettime();
                 if (profileHelper->lastPlayTime > TIME_DELAY_PRECISION && currTime - profileHelper->lastPlayTime <= profileHelper->profile.minDelay) {
                     log("Fail to play %s cause by limited minimum delay",filePath.c_str());
                     break;
                 }
             }
        }
        
        if (volume < 0.0f) {
            volume = 0.0f;
        }
        else if (volume > 1.0f){
            volume = 1.0f;
        }
        
#ifdef USE_AGTK//hidet-sa
		ret = _audioEngineImpl->play2d(filePath, loop, volume, pitch, pan, currentTime);
#else
        ret = _audioEngineImpl->play2d(filePath, loop, volume);
#endif
        if (ret != INVALID_AUDIO_ID)
        {
            _audioPathIDMap[filePath].push_back(ret);
            auto it = _audioPathIDMap.find(filePath);
            
            auto& audioRef = _audioIDInfoMap[ret];
            audioRef.volume = volume;
#ifdef USE_AGTK//hidet-sa
			audioRef.pitch = pitch;
			memset(audioRef.pan, 0, sizeof(audioRef.pan));
			if (pan) {
				memcpy(audioRef.pan, pan, sizeof(audioRef.pan));
			}
#endif
            audioRef.loop = loop;
            audioRef.filePath = &it->first;

            if (profileHelper) {
                profileHelper->lastPlayTime = utils::gettime();
                profileHelper->audioIDs.push_back(ret);
            }
            audioRef.profileHelper = profileHelper;
        }
    } while (0);

    return ret;
}

#ifdef USE_AGTK_LOOP_INFO
#include "audio/win32/AudioDecoderManager.h"
#include "audio/win32/AudioDecoder.h"
#endif
#ifdef USE_AGTK_LOOP_INFO
void AudioEngine::getLoopInfo(const char *filename, uint32_t *pTotalFrames, uint32_t *pSampleRate, uint32_t *pLoopStart, uint32_t *pLoopLength)
{
	*pTotalFrames = 0;
	*pSampleRate = 0;
	*pLoopStart = 0xffffffff;
	*pLoopLength = 0xffffffff;
	auto decoder = AudioDecoderManager::createDecoder(filename);
	if (nullptr == decoder) {
		CC_ASSERT(0);
		return;
	}
	if (!decoder->open(filename)) {
		return;
	}
	*pTotalFrames = decoder->getTotalFrames();
	*pSampleRate = decoder->getSampleRate();
	*pLoopStart = decoder->getLoopStart();
	*pLoopLength = decoder->getLoopLength();
	AudioDecoderManager::destroyDecoder(decoder);
}
#endif
#ifdef USE_AGTK//sakihama-h, 2018.10.03
void AudioEngine::getLoopInfo(int audioID, uint32_t *pTotalFrames, uint32_t *pSampleRate, uint32_t *pLoopStart, uint32_t *pLoopLength)
{
	*pTotalFrames = 0;
	*pSampleRate = 0;
	*pLoopStart = 0xffffffff;
	*pLoopLength = 0xffffffff;
	AudioPlayer *audioPlayer = nullptr;
	AudioCache *cache = nullptr;
	if (_audioEngineImpl) {
		audioPlayer = _audioEngineImpl->getAudioPlayer(audioID);
	}
	if (audioPlayer) {
		bool bReadyAudioCache = false;
		cache = audioPlayer->getCache();
		if (cache) {
			bReadyAudioCache = (cache->getState() == AudioCache::State::READY);
		}
		if (bReadyAudioCache) {
			*pTotalFrames = cache->getTotalFrames();
			*pSampleRate = cache->getSampleRate();
			*pLoopStart = cache->getLoopStart();
			*pLoopLength = cache->getLoopLength();
		}
		else {
			auto filePath = audioPlayer->getFilePath().c_str();
			auto decoder = AudioDecoderManager::createDecoder(filePath);
			if (nullptr == decoder) {
				CC_ASSERT(0);
				return;
			}
			if (!decoder->open(filePath)) {
				return;
			}
			*pTotalFrames = decoder->getTotalFrames();
			*pSampleRate = decoder->getSampleRate();
			*pLoopStart = decoder->getLoopStart();
			*pLoopLength = decoder->getLoopLength();
			AudioDecoderManager::destroyDecoder(decoder);
		}
	}
}
#endif
#ifdef USE_AGTK//sakihama-h, 2018.11.19
bool AudioEngine::isAudioEngineImpl()
{
	return (_audioEngineImpl != nullptr) ? true : false;
}
#endif
void AudioEngine::setLoop(int audioID, bool loop)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end() && it->second.loop != loop){
        _audioEngineImpl->setLoop(audioID, loop);
        it->second.loop = loop;
    }
}

void AudioEngine::setVolume(int audioID, float volume)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end()){
        if (volume < 0.0f) {
            volume = 0.0f;
        }
        else if (volume > 1.0f){
            volume = 1.0f;
        }

        if (it->second.volume != volume){
            _audioEngineImpl->setVolume(audioID, volume);
            it->second.volume = volume;
        }
    }
}

#ifdef USE_AGTK//hidet-sa
void AudioEngine::setPitch(int audioID, float pitch)
{
	auto it = _audioIDInfoMap.find(audioID);
	if (it != _audioIDInfoMap.end()) {
		if (pitch < 0.0f) {
			pitch = 0.0f;
		}
		if (it->second.pitch != pitch) {
			_audioEngineImpl->setPitch(audioID, pitch);
			it->second.pitch = pitch;
		}
	}
}

void AudioEngine::setPan(int audioID, float *pan)
{
	auto it = _audioIDInfoMap.find(audioID);
	if (it != _audioIDInfoMap.end()) {
		float panX = cocos2d::clampf(pan[0], -1.0f, 1.0f);
		float panY = cocos2d::clampf(pan[1], -1.0f, 1.0f);
		float panZ = cocos2d::clampf(pan[2], -1.0f, 1.0f);
		if (it->second.pan[0] != panX || it->second.pan[1] != panY || it->second.pan[2] != panZ) {
			_audioEngineImpl->setPan(audioID, pan);
			it->second.pan[0] = panX;
			it->second.pan[1] = panY;
			it->second.pan[2] = panZ;
		}
	}
}

void AudioEngine::setPan(int audioID, float panX, float panY, float panZ)
{
	auto it = _audioIDInfoMap.find(audioID);
	if (it != _audioIDInfoMap.end()) {
		panX = cocos2d::clampf(panX, -1.0f, 1.0f);
		panY = cocos2d::clampf(panY, -1.0f, 1.0f);
		panZ = cocos2d::clampf(panZ, -1.0f, 1.0f);
		if (it->second.pan[0] != panX || it->second.pan[1] != panY || it->second.pan[2] != panZ) {
			_audioEngineImpl->setPan(audioID, panX, panY, panZ);
			it->second.pan[0] = panX;
			it->second.pan[1] = panY;
			it->second.pan[2] = panZ;
		}
	}
}
#endif
void AudioEngine::pause(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end() && it->second.state == AudioState::PLAYING){
        _audioEngineImpl->pause(audioID);
        it->second.state = AudioState::PAUSED;
    }
}

void AudioEngine::pauseAll()
{
    auto itEnd = _audioIDInfoMap.end();
    for (auto it = _audioIDInfoMap.begin(); it != itEnd; ++it)
    {
        if (it->second.state == AudioState::PLAYING)
        {
            _audioEngineImpl->pause(it->first);
            it->second.state = AudioState::PAUSED;
        }
    }
}

void AudioEngine::resume(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end() && it->second.state == AudioState::PAUSED){
        _audioEngineImpl->resume(audioID);
        it->second.state = AudioState::PLAYING;
    }
}

void AudioEngine::resumeAll()
{
    auto itEnd = _audioIDInfoMap.end();
    for (auto it = _audioIDInfoMap.begin(); it != itEnd; ++it)
    {
        if (it->second.state == AudioState::PAUSED)
        {
            _audioEngineImpl->resume(it->first);
            it->second.state = AudioState::PLAYING;
        }
    }
}

void AudioEngine::stop(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end()){
        _audioEngineImpl->stop(audioID);

        remove(audioID);
    }
}

void AudioEngine::remove(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end()){
        if (it->second.profileHelper) {
            it->second.profileHelper->audioIDs.remove(audioID);
        }
        _audioPathIDMap[*it->second.filePath].remove(audioID);
        _audioIDInfoMap.erase(audioID);
    }
}

void AudioEngine::stopAll()
{
    if(!_audioEngineImpl){
        return;
    }
    _audioEngineImpl->stopAll();
    auto itEnd = _audioIDInfoMap.end();
    for (auto it = _audioIDInfoMap.begin(); it != itEnd; ++it)
    {
        if (it->second.profileHelper){
            it->second.profileHelper->audioIDs.remove(it->first);
        }
    }
    _audioPathIDMap.clear();
    _audioIDInfoMap.clear();
}

void AudioEngine::uncache(const std::string &filePath)
{
    if(!_audioEngineImpl){
        return;
    }
    auto audioIDsIter = _audioPathIDMap.find(filePath);
    if (audioIDsIter != _audioPathIDMap.end())
    {
        //@Note: For safely iterating elements from the audioID list, we need to copy the list
        // since 'AudioEngine::remove' may be invoked in '_audioEngineImpl->stop' synchronously.
        // If this happens, it will break the iteration, and crash will appear on some devices.
        std::list<int> copiedIDs(audioIDsIter->second);
        
        for (int audioID : copiedIDs)
        {
            _audioEngineImpl->stop(audioID);
            
            auto itInfo = _audioIDInfoMap.find(audioID);
            if (itInfo != _audioIDInfoMap.end())
            {
                if (itInfo->second.profileHelper)
                {
                    itInfo->second.profileHelper->audioIDs.remove(audioID);
                }
                _audioIDInfoMap.erase(audioID);
            }
        }
        _audioPathIDMap.erase(filePath);
    }

    if (_audioEngineImpl)
    {
        _audioEngineImpl->uncache(filePath);
    }
}

void AudioEngine::uncacheAll()
{
    if(!_audioEngineImpl){
        return;
    }
    stopAll();
    _audioEngineImpl->uncacheAll();
}

float AudioEngine::getDuration(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end() && it->second.state != AudioState::INITIALIZING)
    {
        if (it->second.duration == TIME_UNKNOWN)
        {
            it->second.duration = _audioEngineImpl->getDuration(audioID);
        }
        return it->second.duration;
    }
    
    return TIME_UNKNOWN;
}

bool AudioEngine::setCurrentTime(int audioID, float time)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end() && it->second.state != AudioState::INITIALIZING) {
        return _audioEngineImpl->setCurrentTime(audioID, time);
    }

    return false;
}

float AudioEngine::getCurrentTime(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end() && it->second.state != AudioState::INITIALIZING) {
        return _audioEngineImpl->getCurrentTime(audioID);
    }
    return 0.0f;
}

void AudioEngine::setFinishCallback(int audioID, const std::function<void (int, const std::string &)> &callback)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end()){
        _audioEngineImpl->setFinishCallback(audioID, callback);
    }
}

bool AudioEngine::setMaxAudioInstance(int maxInstances)
{
    if (maxInstances > 0 && maxInstances <= MAX_AUDIOINSTANCES) {
        _maxInstances = maxInstances;
        return true;
    }

    return false;
}

bool AudioEngine::isLoop(int audioID)
{
    auto tmpIterator = _audioIDInfoMap.find(audioID);
    if (tmpIterator != _audioIDInfoMap.end())
    {
        return tmpIterator->second.loop;
    }
    
    log("AudioEngine::isLoop-->The audio instance %d is non-existent", audioID);
    return false;
}

float AudioEngine::getVolume(int audioID)
{
    auto tmpIterator = _audioIDInfoMap.find(audioID);
    if (tmpIterator != _audioIDInfoMap.end())
    {
        return tmpIterator->second.volume;
    }

    log("AudioEngine::getVolume-->The audio instance %d is non-existent", audioID);
    return 0.0f;
}

#ifdef USE_AGTK//hidet-sa
float AudioEngine::getPitch(int audioID)
{
	auto tmpIterator = _audioIDInfoMap.find(audioID);
	if (tmpIterator != _audioIDInfoMap.end()) {
		return tmpIterator->second.pitch;
	}

	log("AudioEngine::getPitch-->The audio instance %d is non->existent", audioID);
	return 1.0f;
}

float *AudioEngine::getPan(int audioID)
{
	auto tmpIterator = _audioIDInfoMap.find(audioID);
	if (tmpIterator != _audioIDInfoMap.end()) {
		return tmpIterator->second.pan;
	}

	log("AudioEngine::getPan-->The audio instance %d is non->existent", audioID);
	return nullptr;
}
#endif
AudioEngine::AudioState AudioEngine::getState(int audioID)
{
    auto tmpIterator = _audioIDInfoMap.find(audioID);
    if (tmpIterator != _audioIDInfoMap.end())
    {
        return tmpIterator->second.state;
    }
    
    return AudioState::ERROR;
}

AudioProfile* AudioEngine::getProfile(int audioID)
{
    auto it = _audioIDInfoMap.find(audioID);
    if (it != _audioIDInfoMap.end())
    {
        return &it->second.profileHelper->profile;
    }
    
    return nullptr;
}

AudioProfile* AudioEngine::getDefaultProfile()
{
    if (_defaultProfileHelper == nullptr)
    {
        _defaultProfileHelper = new (std::nothrow) ProfileHelper();
    }
    
    return &_defaultProfileHelper->profile;
}

AudioProfile* AudioEngine::getProfile(const std::string &name)
{
    auto it = _audioPathProfileHelperMap.find(name);
    if (it != _audioPathProfileHelperMap.end()) {
        return &it->second.profile;
    } else {
        return nullptr;
    }
}

void AudioEngine::preload(const std::string& filePath, std::function<void(bool isSuccess)> callback)
{
    if (!isEnabled())
    {
        callback(false);
        return;
    }
    
    lazyInit();

    if (_audioEngineImpl)
    {
        if (!FileUtils::getInstance()->isFileExist(filePath)){
            if (callback)
            {
                callback(false);
            }
            return;
        }

        _audioEngineImpl->preload(filePath, callback);
    }
}

void AudioEngine::addTask(const std::function<void()>& task)
{
    lazyInit();

    if (_audioEngineImpl && s_threadPool)
    {
        s_threadPool->addTask(task);
    }
}

int AudioEngine::getPlayingAudioCount()
{
    return static_cast<int>(_audioIDInfoMap.size());
}

void AudioEngine::setEnabled(bool isEnabled)
{
    if (_isEnabled != isEnabled)
    {
        _isEnabled = isEnabled;
        
        if (!_isEnabled)
        {
            stopAll();
        }
    }
}

bool AudioEngine::isEnabled()
{
    return _isEnabled;
}

#ifdef USE_AGTK//hidet-sa
unsigned int AudioEngine::getEmptyAlSource()
{
	return _audioEngineImpl->getEmptyAlSource();
}

void AudioEngine::useAlSource(unsigned int alSource)
{
	_audioEngineImpl->useAlSource(alSource);
}

void AudioEngine::unuseAlSource(unsigned int alSource)
{
	_audioEngineImpl->unuseAlSource(alSource);
}
#endif

#ifdef USE_AGTK//sakihama-h, 2019.10.09
bool AudioEngine::checkChangeDevice()
{
	if (!_audioEngineImpl) {
		return false;
	}
	return _audioEngineImpl->checkChangeDevice();
}

void AudioEngine::changeDevice()
{
	if (!_audioEngineImpl) {
		return;
	}
	_audioEngineImpl->finalize(false);
	_audioEngineImpl->init();
}

void AudioEngine::getDeviceNameList(std::vector<std::string>& deviceNameList)
{
	if (!_audioEngineImpl) {
		return;
	}
	_audioEngineImpl->getDeviceNameList(deviceNameList);
}

int AudioEngine::getDeviceCount()
{
	if (!_audioEngineImpl) {
		return -1;
	}
	return  _audioEngineImpl->getDeviceCount();
}
#endif
