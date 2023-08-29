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
#define LOG_TAG "AudioEngine-Win32"

#include "platform/CCPlatformConfig.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "audio/win32/AudioEngine-win32.h"

#ifdef OPENAL_PLAIN_INCLUDES
#include "alc.h"
#include "alext.h"
#else
#include "AL/alc.h"
#include "AL/alext.h"
#endif
#include "audio/include/AudioEngine.h"
#include "base/CCDirector.h"
#include "base/CCScheduler.h"
#include "platform/CCFileUtils.h"
#include "audio/win32/AudioDecoderManager.h"

#include <windows.h>

// log, CCLOG aren't threadsafe, since we uses sub threads for parsing pcm data, threadsafe log output
// is needed. Define the following macros (ALOGV, ALOGD, ALOGI, ALOGW, ALOGE) for threadsafe log output.

//FIXME: Move _winLog, winLog to a separated file
static void _winLog(const char *format, va_list args)
{
    static const int MAX_LOG_LENGTH = 16 * 1024;
    int bufferSize = MAX_LOG_LENGTH;
    char* buf = nullptr;

    do
    {
        buf = new (std::nothrow) char[bufferSize];
        if (buf == nullptr)
            return; // not enough memory

        int ret = vsnprintf(buf, bufferSize - 3, format, args);
        if (ret < 0)
        {
            bufferSize *= 2;

            delete[] buf;
        }
        else
            break;

    } while (true);

    strcat(buf, "\n");

    int pos = 0;
    int len = strlen(buf);
    char tempBuf[MAX_LOG_LENGTH + 1] = { 0 };
    WCHAR wszBuf[MAX_LOG_LENGTH + 1] = { 0 };

    do
    {
        std::copy(buf + pos, buf + pos + MAX_LOG_LENGTH, tempBuf);

        tempBuf[MAX_LOG_LENGTH] = 0;

        MultiByteToWideChar(CP_UTF8, 0, tempBuf, -1, wszBuf, sizeof(wszBuf));
        OutputDebugStringW(wszBuf);

        pos += MAX_LOG_LENGTH;

    } while (pos < len);

    delete[] buf;
}

void audioLog(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    _winLog(format, args);
    va_end(args);
}

using namespace cocos2d;
using namespace cocos2d::experimental;

static ALCdevice *s_ALDevice = nullptr;
static ALCcontext *s_ALContext = nullptr;

AudioEngineImpl::AudioEngineImpl()
: _lazyInitLoop(true)
, _currentAudioID(0)
, _scheduler(nullptr)
#ifdef USE_AGTK//sakihama-h, 2019.10.09
, _deviceCount(0)
#endif
{

}

AudioEngineImpl::~AudioEngineImpl()
{
    if (_scheduler != nullptr)
    {
        _scheduler->unschedule(CC_SCHEDULE_SELECTOR(AudioEngineImpl::update), this);
    }

#if USE_AGTK//sakihama-h, 2019.10.09
	finalize();
#else
    if (s_ALContext) {
        alDeleteSources(MAX_AUDIOINSTANCES, _alSources);

        _audioCaches.clear();

        alcMakeContextCurrent(nullptr);
        alcDestroyContext(s_ALContext);
        s_ALContext = nullptr;
    }

    if (s_ALDevice) {
        alcCloseDevice(s_ALDevice);
        s_ALDevice = nullptr;
    }

    AudioDecoderManager::destroy();
#endif
}

bool AudioEngineImpl::init()
{
	bool ret = false;
    do{
        s_ALDevice = alcOpenDevice(nullptr);

		if (s_ALDevice) {
            alGetError();
#ifdef USE_AGTK
#define USE_AGTK_STEREO_PAN
#endif
            s_ALContext = alcCreateContext(s_ALDevice, nullptr);
            alcMakeContextCurrent(s_ALContext);

            alGenSources(MAX_AUDIOINSTANCES, _alSources);
            auto alError = alGetError();
            if(alError != AL_NO_ERROR)
            {
                ALOGE("%s:generating sources failed! error = %x\n", __FUNCTION__, alError);
                break;
            }

            for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
                _alSourceUsed[_alSources[i]] = false;
            }

            _scheduler = Director::getInstance()->getScheduler();
			ret = AudioDecoderManager::init();

#if USE_AGTK//sakihama-h, 2019.10.09
			_deviceCount = this->getDeviceCount();
#endif
			ALOGI("OpenAL was initialized successfully!");
        }
    }while (false);

	return ret;
}

#ifdef USE_AGTK//sakihama-h, 2019.10.09
void AudioEngineImpl::finalize(bool cacheClear)
{
	
	if (s_ALContext) {
		alDeleteSources(MAX_AUDIOINSTANCES, _alSources);

		if (cacheClear) {
			_audioCaches.clear();
		}

		alcMakeContextCurrent(nullptr);
		alcDestroyContext(s_ALContext);
		s_ALContext = nullptr;
	}

	if (s_ALDevice) {
		alcCloseDevice(s_ALDevice);
		s_ALDevice = nullptr;
	}

	AudioDecoderManager::destroy();
}
#endif

AudioCache* AudioEngineImpl::preload(const std::string& filePath, std::function<void(bool)> callback)
{
    AudioCache* audioCache = nullptr;

    auto it = _audioCaches.find(filePath);
    if (it == _audioCaches.end()) {
        audioCache = &_audioCaches[filePath];
        audioCache->_fileFullPath = FileUtils::getInstance()->fullPathForFilename(filePath);
        unsigned int cacheId = audioCache->_id;
        auto isCacheDestroyed = audioCache->_isDestroyed;
        AudioEngine::addTask([audioCache, cacheId, isCacheDestroyed](){
            if (*isCacheDestroyed)
            {
                ALOGV("AudioCache (id=%u) was destroyed, no need to launch readDataTask.", cacheId);
                audioCache->setSkipReadDataTask(true);
                return;
            }
            audioCache->readDataTask(cacheId);
        });
    }
    else {
        audioCache = &it->second;
    }

    if (audioCache && callback)
    {
        audioCache->addLoadCallback(callback);
    }
    return audioCache;
}

#ifdef USE_AGTK//hidet-sa
int AudioEngineImpl::play2d(const std::string &filePath, bool loop, float volume, float pitch, float *pan, float currentTime)
#else
int AudioEngineImpl::play2d(const std::string &filePath ,bool loop ,float volume)
#endif
{
	if (s_ALDevice == nullptr) {
		return AudioEngine::INVALID_AUDIO_ID;
    }

    bool sourceFlag = false;
    ALuint alSource = 0;
    for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
        alSource = _alSources[i];

        if ( !_alSourceUsed[alSource]) {
            sourceFlag = true;
            break;
        }
    }
    if(!sourceFlag){
		return AudioEngine::INVALID_AUDIO_ID;
    }

#ifdef USE_AGTK_STEREO_PAN
	_alSourceUsed[alSource] = true;
	sourceFlag = false;
	ALuint alSource2 = 0;
	for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
		alSource2 = _alSources[i];
		if (!_alSourceUsed[alSource2]) {
			sourceFlag = true;
			break;
		}
	}
	_alSourceUsed[alSource] = false;
	if (!sourceFlag) {
		return AudioEngine::INVALID_AUDIO_ID;
	}
#endif
#ifdef USE_AGTK//sakihama-h, 2018.10.03
    auto player = new (std::nothrow) AudioPlayer(filePath);
#else
    auto player = new (std::nothrow) AudioPlayer;
#endif
    if (player == nullptr) {
		return AudioEngine::INVALID_AUDIO_ID;
    }

    player->_alSource = alSource;
#ifdef USE_AGTK_STEREO_PAN
	player->_alSource2 = alSource2;
#endif
    player->_loop = loop;
    player->_volume = volume;
#ifdef USE_AGTK//hidet-sa
	player->_pitch = pitch;
	if (pan) {
		player->_pan[0] = pan[0];
		player->_pan[1] = pan[1];
		player->_pan[2] = pan[2];
	}
#endif

    auto audioCache = preload(filePath, nullptr);
    if (audioCache == nullptr) {
        delete player;
		return AudioEngine::INVALID_AUDIO_ID;
    }

    player->setCache(audioCache);
    _threadMutex.lock();
    _audioPlayers[_currentAudioID] = player;
    _threadMutex.unlock();

    _alSourceUsed[alSource] = true;
#ifdef USE_AGTK_STEREO_PAN
	_alSourceUsed[alSource2] = true;
#endif

#ifdef USE_AGTK
	audioCache->addPlayCallback(std::bind(&AudioEngineImpl::_play2d, this, audioCache, _currentAudioID, currentTime));
#else
    audioCache->addPlayCallback(std::bind(&AudioEngineImpl::_play2d,this,audioCache,_currentAudioID));
#endif

    if (_lazyInitLoop) {
        _lazyInitLoop = false;
        _scheduler->schedule(CC_SCHEDULE_SELECTOR(AudioEngineImpl::update), this, 0.05f, false);
    }

    return _currentAudioID++;
}

#ifdef USE_AGTK
void AudioEngineImpl::_play2d(AudioCache *cache, int audioID, float currentTime)
#else
void AudioEngineImpl::_play2d(AudioCache *cache, int audioID)
#endif
{
	//Note: It may bn in sub thread or main thread :(
    if (!*cache->_isDestroyed && cache->_state == AudioCache::State::READY)
    {
        _threadMutex.lock();
        auto playerIt = _audioPlayers.find(audioID);
#ifdef USE_AGTK
		// シーク再生
		if (currentTime > 0.0f) {
			playerIt->second->setTime(currentTime);
		}
#endif
        if (playerIt != _audioPlayers.end() && playerIt->second->play2d()) {
#ifdef USE_AGTK_STEREO_PAN
			auto player = playerIt->second;
			if (!player->_audioCache->_isStereo) {
				if (_alSourceUsed.find(player->_alSource2) != _alSourceUsed.end()) {
					_alSourceUsed[player->_alSource2] = false;
					player->_alSource2 = 0xffffffff;	//使っていない値のつもり。
				}
			}
#endif
            _scheduler->performFunctionInCocosThread([audioID](){

                if (AudioEngine::_audioIDInfoMap.find(audioID) != AudioEngine::_audioIDInfoMap.end()) {
                    AudioEngine::_audioIDInfoMap[audioID].state = AudioEngine::AudioState::PLAYING;
                }
            });
        }
        _threadMutex.unlock();
    }
    else
    {
        ALOGD("AudioEngineImpl::_play2d, cache was destroyed or not ready!");
        auto iter = _audioPlayers.find(audioID);
        if (iter != _audioPlayers.end())
        {
            iter->second->_removeByAudioEngine = true;
        }
    }
}

void AudioEngineImpl::setVolume(int audioID,float volume)
{
    auto player = _audioPlayers[audioID];
    player->_volume = volume;

    if (player->_ready) {
#ifdef USE_AGTK_STEREO_PAN
		float volume2[2];
		player->calcVolume2(player->_pan, volume2);
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource, AL_GAIN, volume2[0] * volume);
		}
		else {
			alSourcef(_audioPlayers[audioID]->_alSource, AL_GAIN, volume);
		}
#else
        alSourcef(_audioPlayers[audioID]->_alSource, AL_GAIN, volume);
#endif

        auto error = alGetError();
        if (error != AL_NO_ERROR) {
            ALOGE("%s: audio id = %d, error = %x", __FUNCTION__,audioID,error);
        }
#ifdef USE_AGTK_STEREO_PAN
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource2, AL_GAIN, volume2[1] * volume);
			error = alGetError();
			if (error != AL_NO_ERROR) {
				ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
			}
		}
#endif
    }
}

#ifdef USE_AGTK//hidet-sa
void AudioEngineImpl::setPitch(int audioID, float pitch)
{
	auto player = _audioPlayers[audioID];
	player->_pitch = pitch;

	if (player->_ready) {
		alSourcef(_audioPlayers[audioID]->_alSource, AL_PITCH, pitch);

		auto error = alGetError();
		if (error != AL_NO_ERROR) {
			ALOGE("%s: audio id = %d, error = %x", __FUNCTION__, audioID, error);
		}
#ifdef USE_AGTK_STEREO_PAN
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource2, AL_PITCH, pitch);
			error = alGetError();
			if (error != AL_NO_ERROR) {
				ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
			}
		}
#endif
	}
}

void AudioEngineImpl::setPan(int audioID, float *pan)
{
	auto player = _audioPlayers[audioID];
	memcpy(player->_pan, pan, sizeof(player->_pan));

	if (player->_ready) {
#ifdef USE_AGTK_STEREO_PAN
		static float panList[2] = { -1.0f, 1.0f };
		float volume2[2];
		player->calcVolume2(pan, volume2);
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource, AL_GAIN, volume2[0] * player->_volume);
			alSource3f(_audioPlayers[audioID]->_alSource, AL_POSITION, panList[0], pan[1], pan[2]);
		} else {
			alSource3f(_audioPlayers[audioID]->_alSource, AL_POSITION, pan[0], pan[1], pan[2]);
		}
#else
		alSource3f(_audioPlayers[audioID]->_alSource, AL_POSITION, pan[0], pan[1], pan[2]);
#endif

		auto error = alGetError();
		if (error != AL_NO_ERROR) {
			ALOGE("%s: audio id = %d, error = %x", __FUNCTION__, audioID, error);
		}
#ifdef USE_AGTK_STEREO_PAN
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource2, AL_GAIN, volume2[1] * player->_volume);
			alSource3f(_audioPlayers[audioID]->_alSource2, AL_POSITION, panList[1], pan[1], pan[2]);
			error = alGetError();
			if (error != AL_NO_ERROR) {
				ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
			}
		}
		else {
			//alSource3f(_audioPlayers[audioID]->_alSource2, AL_POSITION, pan[0], pan[1], pan[2]);
		}
		//error = alGetError();
		//if (error != AL_NO_ERROR) {
			//ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
		//}
#endif
	}
}

void AudioEngineImpl::setPan(int audioID, float panX, float panY, float panZ)
{
	auto player = _audioPlayers[audioID];
	player->_pan[0] = panX;
	player->_pan[1] = panY;
	player->_pan[2] = panZ;

	if (player->_ready) {
#ifdef USE_AGTK_STEREO_PAN
		float volume2[2];
		static float panList[2] = { -1.0f, 1.0f };
		player->calcVolume2(player->_pan, volume2);
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource, AL_GAIN, volume2[0] * player->_volume);
			alSource3f(_audioPlayers[audioID]->_alSource, AL_POSITION, panList[0], panY, panZ);
		}
		else {
			alSource3f(_audioPlayers[audioID]->_alSource, AL_POSITION, panX, panY, panZ);
		}
#else
		alSource3f(_audioPlayers[audioID]->_alSource, AL_POSITION, panX, panY, panZ);
#endif

		auto error = alGetError();
		if (error != AL_NO_ERROR) {
			ALOGE("%s: audio id = %d, error = %x", __FUNCTION__, audioID, error);
		}
#ifdef USE_AGTK_STEREO_PAN
		if (player->_audioCache->_isStereo) {
			alSourcef(_audioPlayers[audioID]->_alSource2, AL_GAIN, volume2[1] * player->_volume);
			alSource3f(_audioPlayers[audioID]->_alSource2, AL_POSITION, panList[1], panY, panZ);
			error = alGetError();
			if (error != AL_NO_ERROR) {
				ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
			}
		}
		else {
			//alSource3f(_audioPlayers[audioID]->_alSource2, AL_POSITION, panX, panY, panZ);
		}
		//error = alGetError();
		//if (error != AL_NO_ERROR) {
			//ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
		//}
#endif
	}
}
#endif

void AudioEngineImpl::setLoop(int audioID, bool loop)
{
    auto player = _audioPlayers[audioID];

    if (player->_ready) {
        if (player->_streamingSource) {
            player->setLoop(loop);
        } else {
            if (loop) {
                alSourcei(player->_alSource, AL_LOOPING, AL_TRUE);
            } else {
                alSourcei(player->_alSource, AL_LOOPING, AL_FALSE);
            }

            auto error = alGetError();
            if (error != AL_NO_ERROR) {
                ALOGE("%s: audio id = %d, error = %x", __FUNCTION__,audioID,error);
            }
#ifdef USE_AGTK_STEREO_PAN
			if (player->_audioCache->_isStereo) {
				if (loop) {
					alSourcei(player->_alSource2, AL_LOOPING, AL_TRUE);
				}
				else {
					alSourcei(player->_alSource2, AL_LOOPING, AL_FALSE);
				}
				error = alGetError();
				if (error != AL_NO_ERROR) {
					ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
				}
			}
#endif
        }
    }
    else {
        player->_loop = loop;
    }
}

bool AudioEngineImpl::pause(int audioID)
{
    bool ret = true;
    alSourcePause(_audioPlayers[audioID]->_alSource);

    auto error = alGetError();
    if (error != AL_NO_ERROR) {
        ret = false;
        ALOGE("%s: audio id = %d, error = %x\n", __FUNCTION__,audioID,error);
    }
#ifdef USE_AGTK_STEREO_PAN
	if (_audioPlayers[audioID]->_audioCache->_isStereo) {
		alSourcePause(_audioPlayers[audioID]->_alSource2);
		error = alGetError();
		if (error != AL_NO_ERROR) {
			ret = false;
			ALOGE("%s: audio id = %d, error2 = %x\n", __FUNCTION__, audioID, error);
		}
	}
#endif

    return ret;
}

bool AudioEngineImpl::resume(int audioID)
{
    bool ret = true;
    alSourcePlay(_audioPlayers[audioID]->_alSource);

    auto error = alGetError();
    if (error != AL_NO_ERROR) {
        ret = false;
        ALOGE("%s: audio id = %d, error = %x\n", __FUNCTION__,audioID,error);
    }
#ifdef USE_AGTK_STEREO_PAN
	if (_audioPlayers[audioID]->_audioCache->_isStereo) {
		alSourcePlay(_audioPlayers[audioID]->_alSource2);
		error = alGetError();
		if (error != AL_NO_ERROR) {
			ret = false;
			ALOGE("%s: audio id = %d, error2 = %x\n", __FUNCTION__, audioID, error);
		}
	}
#endif

    return ret;
}

void AudioEngineImpl::stop(int audioID)
{
    auto player = _audioPlayers[audioID];
    player->destroy();

	//Note: Don't set the flag to false here, it should be set in 'update' function.
    // Otherwise, the state got from alSourceState may be wrong
//    _alSourceUsed[player->_alSource] = false;

    // Call 'update' method to cleanup immediately since the schedule may be cancelled without any notification.
    update(0.0f);
}

void AudioEngineImpl::stopAll()
{
	for(auto&& player : _audioPlayers)
    {
        player.second->destroy();
    }
    //Note: Don't set the flag to false here, it should be set in 'update' function.
    // Otherwise, the state got from alSourceState may be wrong
//    for(int index = 0; index < MAX_AUDIOINSTANCES; ++index)
//    {
//        _alSourceUsed[_alSources[index]] = false;
//    }

    // Call 'update' method to cleanup immediately since the schedule may be cancelled without any notification.
    update(0.0f);
}

float AudioEngineImpl::getDuration(int audioID)
{
    auto player = _audioPlayers[audioID];
    if(player->_ready){
        return player->_audioCache->_duration;
    } else {
        return AudioEngine::TIME_UNKNOWN;
    }
}

float AudioEngineImpl::getCurrentTime(int audioID)
{
    float ret = 0.0f;
    auto player = _audioPlayers[audioID];
    if(player->_ready){
        if (player->_streamingSource) {
            ret = player->getTime();
        } else {
            alGetSourcef(player->_alSource, AL_SEC_OFFSET, &ret);

            auto error = alGetError();
            if (error != AL_NO_ERROR) {
                ALOGE("%s, audio id:%d,error code:%x", __FUNCTION__,audioID,error);
            }
#ifdef USE_AGTK_STEREO_PAN
			if (_audioPlayers[audioID]->_audioCache->_isStereo) {
				alGetSourcef(player->_alSource2, AL_SEC_OFFSET, &ret);
				error = alGetError();
				if (error != AL_NO_ERROR) {
					ALOGE("%s, audio id:%d,error2 code:%x", __FUNCTION__, audioID, error);
				}
			}
#endif
        }
    }

    return ret;
}

bool AudioEngineImpl::setCurrentTime(int audioID, float time)
{
    bool ret = false;
    auto player = _audioPlayers[audioID];

    do {
        if (!player->_ready) {
            break;
        }

        if (player->_streamingSource) {
            ret = player->setTime(time);
            break;
        }
        else {
            if (player->_audioCache->_framesRead != player->_audioCache->_totalFrames &&
                (time * player->_audioCache->_sampleRate) > player->_audioCache->_framesRead) {
                ALOGE("%s: audio id = %d", __FUNCTION__,audioID);
                break;
            }

            alSourcef(player->_alSource, AL_SEC_OFFSET, time);

            auto error = alGetError();
            if (error != AL_NO_ERROR) {
                ALOGE("%s: audio id = %d, error = %x", __FUNCTION__,audioID,error);
            }
#ifdef USE_AGTK_STEREO_PAN
			if (player->_audioCache->_isStereo) {
				alSourcef(player->_alSource2, AL_SEC_OFFSET, time);
				error = alGetError();
				if (error != AL_NO_ERROR) {
					ALOGE("%s: audio id = %d, error2 = %x", __FUNCTION__, audioID, error);
				}
			}
#endif
            ret = true;
        }
    } while (0);

    return ret;
}

void AudioEngineImpl::setFinishCallback(int audioID, const std::function<void (int, const std::string &)> &callback)
{
    _audioPlayers[audioID]->_finishCallbak = callback;
}

void AudioEngineImpl::update(float dt)
{
	ALint sourceState;
    int audioID;
    AudioPlayer* player;
    ALuint alSource;

//    ALOGV("AudioPlayer count: %d", (int)_audioPlayers.size());

    for (auto it = _audioPlayers.begin(); it != _audioPlayers.end(); ) {
        audioID = it->first;
        player = it->second;
        alSource = player->_alSource;
#ifdef USE_AGTK_STEREO_PAN
		auto alSource2 = player->_alSource2;
#endif
        alGetSourcei(alSource, AL_SOURCE_STATE, &sourceState);

        if (player->_removeByAudioEngine)
        {
			AudioEngine::remove(audioID);
            _threadMutex.lock();
            it = _audioPlayers.erase(it);
            _threadMutex.unlock();
            delete player;
            _alSourceUsed[alSource] = false;
#ifdef USE_AGTK_STEREO_PAN
			if (_alSourceUsed.find(alSource2) != _alSourceUsed.end()) {
				_alSourceUsed[alSource2] = false;
			}
#endif
		}
        else if (player->_ready && sourceState == AL_STOPPED) {
#ifdef USE_AGTK
			// ソースにバインドされたままの可能性があるのでプレイヤーを開放する前にアンバインドを行う
			alSourcei(alSource, AL_BUFFER, NULL);
#ifdef USE_AGTK_STEREO_PAN
			if (alSource2 != 0xffffffff) {
				alSourcei(alSource2, AL_BUFFER, NULL);
			}
#endif
#endif

			std::string filePath;
            if (player->_finishCallbak) {
                auto& audioInfo = AudioEngine::_audioIDInfoMap[audioID];
                filePath = *audioInfo.filePath;
                player->setCache(nullptr); // it's safe for player didn't free audio cache
            }

            AudioEngine::remove(audioID);
            
            _threadMutex.lock();
            it = _audioPlayers.erase(it);
            _threadMutex.unlock();

			if (player->_finishCallbak) {
                player->_finishCallbak(audioID, filePath); //FIXME: callback will delay 50ms
            }
            delete player;
            _alSourceUsed[alSource] = false;
#ifdef USE_AGTK_STEREO_PAN
			_alSourceUsed[alSource2] = false;
#endif
        }
        else{
            ++it;
        }
    }

    if(_audioPlayers.empty()){
        _lazyInitLoop = true;
        _scheduler->unschedule(CC_SCHEDULE_SELECTOR(AudioEngineImpl::update), this);
    }
}

void AudioEngineImpl::uncache(const std::string &filePath)
{
    _audioCaches.erase(filePath);
}

void AudioEngineImpl::uncacheAll()
{
    _audioCaches.clear();
    for(auto&& player : _audioPlayers)
    {
        // prevent player hold invalid AudioCache* pointer, since all audio caches purged
        player.second->setCache(nullptr);
    }
}
#ifdef USE_AGTK//hidet-sa
ALuint AudioEngineImpl::getEmptyAlSource()
{
	bool sourceFlag = false;
	ALuint alSource = 0;
	for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
		alSource = _alSources[i];
		if (!_alSourceUsed[alSource]) {
			sourceFlag = true;
			break;
		}
	}
	if (!sourceFlag) {
		return AudioEngine::INVALID_AUDIO_ID;
	}
	return alSource;
}

void AudioEngineImpl::useAlSource(ALuint alSource)
{
	for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
		if (_alSources[i] == alSource) {
			_alSourceUsed[alSource] = true;
			break;
		}
	}
}

void AudioEngineImpl::unuseAlSource(ALuint alSource)
{
	for (int i = 0; i < MAX_AUDIOINSTANCES; ++i) {
		if (_alSources[i] == alSource) {
			_alSourceUsed[alSource] = false;
			break;
		}
	}
}
#endif
#ifdef USE_AGTK//sakihama-h, 2018.10.03
AudioPlayer *AudioEngineImpl::getAudioPlayer(int audioID)
{
	return _audioPlayers[audioID];
}
#endif

#ifdef USE_AGTK//sakihama-h, 2019.10.09
bool AudioEngineImpl::checkChangeDevice()
{
	int deviceCount = getDeviceCount();
	bool ret = (deviceCount != _deviceCount);
	_deviceCount = deviceCount;

	if (_deviceCount == 0) {
		if (s_ALDevice != nullptr) {
			//起動時にサウンドデバイス有り、プレイ中にデバイスが無くなった場合は、
			//デバイスを再設定せずに保持するようにする。
			ret = false;
		}
	}

	return ret;
}

int AudioEngineImpl::getDeviceCount()
{
	return ::waveOutGetNumDevs();
}

bool AudioEngineImpl::getDeviceNameList(std::vector<std::string> &deviceNameList)
{
	std::function<bool(const ALCchar *, std::vector<std::string>&)> compare = [](const ALCchar *str, std::vector<std::string>& strList) {
		//同じ文字列がある場合はTRUE,無い場合はFLASE
		for (auto p : strList) {
			if (p.compare(str) == 0) {
				return true;
			}
		}
		return false;
	};
	std::function<void(const ALCchar*, std::vector<std::string>&)> getStringList = [compare](const ALCchar *devices, std::vector<std::string>& list) {
		const ALCchar *ptr, *nptr;
		nptr = ptr = devices;
		if (devices != nullptr) {
			while (*(nptr += strlen(ptr) + 1) != 0) {
				if (compare(ptr, list) == false) {
					list.push_back(ptr);
				}
				ptr = nptr;
			}
			if (compare(ptr, list) == false) {
				list.push_back(ptr);
			}
		}
	};

	if (alcIsExtensionPresent(NULL, "ALC_enumeration_EXT") == AL_TRUE) {
		if (alcIsExtensionPresent(NULL, "ALC_enumerate_all_EXT") == AL_FALSE) {
			getStringList(alcGetString(NULL, ALC_DEVICE_SPECIFIER), deviceNameList);
		} else {
			getStringList(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER), deviceNameList);
		}
		return true;
	}
	return false;
}
#endif

#endif
