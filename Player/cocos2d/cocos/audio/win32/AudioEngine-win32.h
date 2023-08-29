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

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#ifndef __AUDIO_ENGINE_INL_H_
#define __AUDIO_ENGINE_INL_H_

#include <unordered_map>

#include "base/CCRef.h"
#include "audio/win32/AudioCache.h"
#include "audio/win32/AudioPlayer.h"

NS_CC_BEGIN

class Scheduler;

namespace experimental {
#define MAX_AUDIOINSTANCES 32

class CC_DLL AudioEngineImpl : public cocos2d::Ref
{
public:
    AudioEngineImpl();
    ~AudioEngineImpl();

    bool init();
#ifdef USE_AGTK//sakihama-h, 2019.10.09
	void finalize(bool cacheClear = true);
#endif
#ifdef USE_AGTK//hidet-sa
	int play2d(const std::string &fileFullPath, bool loop, float volume, float pitch, float *pan, float currentTime);
#else
    int play2d(const std::string &fileFullPath ,bool loop ,float volume);
#endif
    void setVolume(int audioID,float volume);
#ifdef USE_AGTK//hidet-sa
	void setPitch(int audioID, float pitch);
	void setPan(int audioID, float *pan);
	void setPan(int audioId, float panX, float panY, float panZ);
#endif
    void setLoop(int audioID, bool loop);
    bool pause(int audioID);
    bool resume(int audioID);
    void stop(int audioID);
    void stopAll();
    float getDuration(int audioID);
    float getCurrentTime(int audioID);
    bool setCurrentTime(int audioID, float time);
    void setFinishCallback(int audioID, const std::function<void (int, const std::string &)> &callback);

    void uncache(const std::string& filePath);
    void uncacheAll();
    AudioCache* preload(const std::string& filePath, std::function<void(bool)> callback);
    void update(float dt);
#ifdef USE_AGTK//hidet-sa
	ALuint getEmptyAlSource();
	void useAlSource(ALuint id);
	void unuseAlSource(ALuint id);
#endif
#ifdef USE_AGTK//sakihama-h, 2018.10.03
	AudioPlayer *getAudioPlayer(int audioID);
#endif
#ifdef USE_AGTK//sakihama-h, 2019.10.09
	bool checkChangeDevice();
	int getDeviceCount();
	bool getDeviceNameList(std::vector<std::string>& deviceNameList);
#endif
private:
#ifdef USE_AGTK
	void _play2d(AudioCache *cache, int audioID, float currentTime);
#else
    void _play2d(AudioCache *cache, int audioID);
#endif

    ALuint _alSources[MAX_AUDIOINSTANCES];

    //source,used
    std::unordered_map<ALuint, bool> _alSourceUsed;

    //filePath,bufferInfo
    std::unordered_map<std::string, AudioCache> _audioCaches;

    //audioID,AudioInfo
    std::unordered_map<int, AudioPlayer*>  _audioPlayers;
    std::mutex _threadMutex;

    bool _lazyInitLoop;
#ifdef USE_AGTK//sakihama-h, 2019.10.09
	int _deviceCount;
#endif
    int _currentAudioID;
    Scheduler* _scheduler;
};
}
NS_CC_END
#endif // __AUDIO_ENGINE_INL_H_
#endif

