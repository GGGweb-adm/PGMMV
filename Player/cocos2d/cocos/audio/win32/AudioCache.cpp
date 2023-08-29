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

#define LOG_TAG "AudioCache"

#include "platform/CCPlatformConfig.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#include "audio/win32/AudioCache.h"
#include <thread>
#include "base/CCDirector.h"
#include "base/CCScheduler.h"

#include "audio/win32/AudioDecoderManager.h"
#include "audio/win32/AudioDecoder.h"

#define VERY_VERY_VERBOSE_LOGGING
#ifdef VERY_VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(...) do{} while(false)
#endif

namespace {
unsigned int __idIndex = 0;
}

#define INVALID_AL_BUFFER_ID 0xFFFFFFFF
#define PCMDATA_CACHEMAXSIZE 1048576

using namespace cocos2d;
using namespace cocos2d::experimental;

AudioCache::AudioCache()
: _totalFrames(0)
, _framesRead(0)
, _format(-1)
, _duration(0.0f)
, _alBufferId(INVALID_AL_BUFFER_ID)
#ifdef USE_AGTK_STEREO_PAN
, _alBufferId2(INVALID_AL_BUFFER_ID)
#endif
, _pcmData(nullptr)
#ifdef USE_AGTK_STEREO_PAN
, _pcmData2(nullptr)
#endif
, _queBufferFrames(0)
, _state(State::INITIAL)
, _isDestroyed(std::make_shared<bool>(false))
, _id(++__idIndex)
, _isLoadingFinished(false)
, _isSkipReadDataTask(false)
#ifdef USE_AGTK_STEREO_PAN
, _isStereo(false)
#endif
#ifdef USE_AGTK//sakihama-h, 2018.10.03
, _loopStart(0)
, _loopLength(0)
#endif
{
    ALOGVV("AudioCache() %p, id=%u", this, _id);
    for (int i = 0; i < QUEUEBUFFER_NUM; ++i)
    {
        _queBuffers[i] = nullptr;
        _queBufferSize[i] = 0;
#ifdef USE_AGTK_STEREO_PAN
		_queBuffers2[i] = nullptr;
		_queBufferSize2[i] = 0;
#endif
    }
}

AudioCache::~AudioCache()
{
    ALOGVV("~AudioCache() %p, id=%u, begin", this, _id);
    *_isDestroyed = true;
    while (!_isLoadingFinished)
    {
        if (_isSkipReadDataTask)
        {
            ALOGV("id=%u, Skip read data task, don't continue to wait!", _id);
            break;
        }
        ALOGVV("id=%u, waiting readData thread to finish ...", _id);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    //wait for the 'readDataTask' task to exit
    _readDataTaskMutex.lock();
    _readDataTaskMutex.unlock();

    if (_pcmData)
    {
        if (_state == State::READY)
        {
            if (_alBufferId != INVALID_AL_BUFFER_ID && alIsBuffer(_alBufferId))
            {
                ALOGV("~AudioCache(id=%u), delete buffer: %u", _id, _alBufferId);
                alDeleteBuffers(1, &_alBufferId);
                _alBufferId = INVALID_AL_BUFFER_ID;
            }
#ifdef USE_AGTK_STEREO_PAN
			if (_alBufferId2 != INVALID_AL_BUFFER_ID && alIsBuffer(_alBufferId2))
			{
				ALOGV("~AudioCache(id=%u), delete buffer2: %u", _id, _alBufferId2);
				alDeleteBuffers(1, &_alBufferId2);
				_alBufferId2 = INVALID_AL_BUFFER_ID;
			}
#endif
        }
        else
        {
            ALOGW("AudioCache (%p), id=%u, buffer isn't ready, state=%d", this, _id, _state);
        }

        free(_pcmData);
#ifdef USE_AGTK_STEREO_PAN
		if (_pcmData2) {
			free(_pcmData2);
		}
#endif
    }

    if (_queBufferFrames > 0)
    {
        for (int index = 0; index < QUEUEBUFFER_NUM; ++index)
        {
            free(_queBuffers[index]);
#ifdef USE_AGTK_STEREO_PAN
			if (_queBuffers2[index]) {
				free(_queBuffers2[index]);
			}
#endif
        }
    }
    ALOGVV("~AudioCache() %p, id=%u, end", this, _id);
}

#ifdef USE_AGTK_STEREO_PAN
static void copyData(char *pcmData, char *pcmData2, char *buf, int size)
{
	while (size > 0) {
		pcmData[0] = buf[0];
		pcmData[1] = buf[1];
		pcmData2[0] = buf[2];
		pcmData2[1] = buf[3];
		pcmData += 2;
		pcmData2 += 2;
		buf += 4;
		size -= 4;
	}
}
#endif
void AudioCache::readDataTask(unsigned int selfId)
{
    //Note: It's in sub thread
    ALOGVV("readDataTask begin, cache id=%u", selfId);

    _readDataTaskMutex.lock();
    _state = State::LOADING;

    AudioDecoder* decoder = AudioDecoderManager::createDecoder(_fileFullPath.c_str());
    do
    {
        if (decoder == nullptr || !decoder->open(_fileFullPath.c_str()))
            break;

        const uint32_t originalTotalFrames = decoder->getTotalFrames();
        const uint32_t bytesPerFrame = decoder->getBytesPerFrame();
        const uint32_t sampleRate = decoder->getSampleRate();
        const uint32_t channelCount = decoder->getChannelCount();

        uint32_t totalFrames = originalTotalFrames;
        uint32_t dataSize = totalFrames * bytesPerFrame;
        uint32_t remainingFrames = totalFrames;
        uint32_t adjustFrames = 0;

#ifdef USE_AGTK//sakihama-h, 2018.10.03
		_loopStart = decoder->getLoopStart();
		_loopLength = decoder->getLoopLength();
#endif
#ifdef USE_AGTK_STEREO_PAN
		_isStereo = channelCount > 1;
		_format = AL_FORMAT_MONO16;
#else
        _format = channelCount > 1 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
#endif
        _sampleRate = (ALsizei)sampleRate;
        _duration = 1.0f * totalFrames / sampleRate;
        _totalFrames = totalFrames;

        if (dataSize <= PCMDATA_CACHEMAXSIZE)
        {
            uint32_t framesRead = 0;
            const uint32_t framesToReadOnce = std::min(totalFrames, static_cast<uint32_t>(sampleRate * QUEUEBUFFER_TIME_STEP * QUEUEBUFFER_NUM));

            std::vector<char> adjustFrameBuf;

            if (decoder->seek(totalFrames))
            {
                char* tmpBuf = (char*)malloc(framesToReadOnce * bytesPerFrame);
                adjustFrameBuf.reserve(framesToReadOnce * bytesPerFrame);

                // Adjust total frames by setting position to the end of frames and try to read more data.
                // This is a workaround for https://github.com/cocos2d/cocos2d-x/issues/16938
                do
                {
                    framesRead = decoder->read(framesToReadOnce, tmpBuf);
                    if (framesRead > 0)
                    {
                        adjustFrames += framesRead;
                        adjustFrameBuf.insert(adjustFrameBuf.end(), tmpBuf, tmpBuf + framesRead * bytesPerFrame);
                    }

                } while (framesRead > 0);

                if (adjustFrames > 0)
                {
                    ALOGV("Orignal total frames: %u, adjust frames: %u, current total frames: %u", totalFrames, adjustFrames, totalFrames + adjustFrames);
                    totalFrames += adjustFrames;
                    _totalFrames = remainingFrames = totalFrames;
                }

                // Reset dataSize
                dataSize = totalFrames * bytesPerFrame;

                free(tmpBuf);
            }
            // Reset to frame 0
            BREAK_IF_ERR_LOG(!decoder->seek(0), "AudioDecoder::seek(0) failed!");

#ifdef USE_AGTK_STEREO_PAN
			if (_isStereo) {
				_pcmData = (char*)malloc(dataSize / 2);
				memset(_pcmData, 0x00, dataSize / 2);
				_pcmData2 = (char*)malloc(dataSize / 2);
				memset(_pcmData2, 0x00, dataSize / 2);
			} else {
				_pcmData = (char*)malloc(dataSize);
				memset(_pcmData, 0x00, dataSize);
			}
#else
            _pcmData = (char*)malloc(dataSize);
            memset(_pcmData, 0x00, dataSize);
#endif

            if (adjustFrames > 0)
            {
#ifdef USE_AGTK_STEREO_PAN
				if (_isStereo) {
					copyData(_pcmData + (dataSize - adjustFrameBuf.size()) / 2, _pcmData2 + (dataSize - adjustFrameBuf.size()) / 2, adjustFrameBuf.data(), adjustFrameBuf.size());
				} else {
					memcpy(_pcmData + (dataSize - adjustFrameBuf.size()), adjustFrameBuf.data(), adjustFrameBuf.size());
				}
#else
                memcpy(_pcmData + (dataSize - adjustFrameBuf.size()), adjustFrameBuf.data(), adjustFrameBuf.size());
#endif
            }

            alGenBuffers(1, &_alBufferId);
            auto alError = alGetError();
            if (alError != AL_NO_ERROR) {
                ALOGE("%s: attaching audio to buffer fail: %x", __FUNCTION__, alError);
                break;
            }
#ifdef USE_AGTK_STEREO_PAN
			if (_isStereo) {
				alGenBuffers(1, &_alBufferId2);
				alError = alGetError();
				if (alError != AL_NO_ERROR) {
					ALOGE("%s: attaching audio to buffer2 fail: %x", __FUNCTION__, alError);
					break;
				}
			}
#endif

            if (*_isDestroyed)
                break;

#ifdef USE_AGTK_STEREO_PAN
			if (_isStereo) {
				auto size = std::min(framesToReadOnce, remainingFrames);
				char* tmpBuf = (char*)malloc(size * bytesPerFrame);
				framesRead = decoder->readFixedFrames(size, tmpBuf);
				copyData(_pcmData + _framesRead * bytesPerFrame / 2, _pcmData2 + _framesRead * bytesPerFrame / 2, tmpBuf, size * bytesPerFrame);
				free(tmpBuf);
			} else {
				framesRead = decoder->readFixedFrames(std::min(framesToReadOnce, remainingFrames), _pcmData + _framesRead * bytesPerFrame);
			}
#else
            framesRead = decoder->readFixedFrames(std::min(framesToReadOnce, remainingFrames), _pcmData + _framesRead * bytesPerFrame);
#endif
            _framesRead += framesRead;
            remainingFrames -= framesRead;

            if (*_isDestroyed)
                break;

            uint32_t frames = 0;
            while (!*_isDestroyed && _framesRead < originalTotalFrames)
            {
                frames = std::min(framesToReadOnce, remainingFrames);
                if (_framesRead + frames > originalTotalFrames)
                {
                    frames = originalTotalFrames - _framesRead;
                }
#ifdef USE_AGTK_STEREO_PAN
				if (_isStereo) {
					char* tmpBuf = (char*)malloc(frames * bytesPerFrame);
					framesRead = decoder->read(frames, tmpBuf);
					copyData(_pcmData + _framesRead * bytesPerFrame / 2, _pcmData2 + _framesRead * bytesPerFrame / 2, tmpBuf, frames * bytesPerFrame);
					free(tmpBuf);
				} else {
					framesRead = decoder->read(frames, _pcmData + _framesRead * bytesPerFrame);
				}
#else
                framesRead = decoder->read(frames, _pcmData + _framesRead * bytesPerFrame);
#endif
                if (framesRead == 0)
                    break;
                _framesRead += framesRead;
                remainingFrames -= framesRead;
            }

            if (*_isDestroyed)
                break;

            if (_framesRead < originalTotalFrames)
            {
#ifdef USE_AGTK_STEREO_PAN
				if (_isStereo) {
					auto size = (totalFrames - _framesRead) * bytesPerFrame;
					memset(_pcmData + _framesRead * bytesPerFrame / 2, 0x00, size / 2);
					memset(_pcmData2 + _framesRead * bytesPerFrame / 2, 0x00, size / 2);
				}
				else {
					memset(_pcmData + _framesRead * bytesPerFrame, 0x00, (totalFrames - _framesRead) * bytesPerFrame);
				}
#else
                memset(_pcmData + _framesRead * bytesPerFrame, 0x00, (totalFrames - _framesRead) * bytesPerFrame);
#endif
            }
            ALOGV("pcm buffer was loaded successfully, total frames: %u, total read frames: %u, adjust frames: %u, remainingFrames: %u", totalFrames, _framesRead, adjustFrames, remainingFrames);

            _framesRead += adjustFrames;

#ifdef USE_AGTK_STEREO_PAN
			if (_isStereo) {
				alBufferData(_alBufferId, _format, _pcmData, (ALsizei)dataSize / 2, (ALsizei)sampleRate);
				alBufferData(_alBufferId2, _format, _pcmData2, (ALsizei)dataSize / 2, (ALsizei)sampleRate);
			} else {
				alBufferData(_alBufferId, _format, _pcmData, (ALsizei)dataSize, (ALsizei)sampleRate);
			}
#else
            alBufferData(_alBufferId, _format, _pcmData, (ALsizei)dataSize, (ALsizei)sampleRate);
#endif

            _state = State::READY;
        }
        else
        {
#ifdef USE_AGTK
            _queBufferFrames = sampleRate * QUEUEBUFFER_TIME_STEP * MARGIN_FOR_PITCH;
#else
            _queBufferFrames = sampleRate * QUEUEBUFFER_TIME_STEP;
#endif
            BREAK_IF_ERR_LOG(_queBufferFrames == 0, "_queBufferFrames == 0");

            const uint32_t queBufferBytes = _queBufferFrames * bytesPerFrame;

            for (int index = 0; index < QUEUEBUFFER_NUM; ++index)
            {
#ifdef USE_AGTK_STEREO_PAN
				if (_isStereo) {
					_queBuffers[index] = (char*)malloc(queBufferBytes / 2);
					_queBufferSize[index] = queBufferBytes / 2;
					_queBuffers2[index] = (char*)malloc(queBufferBytes / 2);
					_queBufferSize2[index] = queBufferBytes / 2;
					char* tmpBuf = (char*)malloc(queBufferBytes);
					decoder->readFixedFrames(_queBufferFrames, tmpBuf);
					copyData(_queBuffers[index], _queBuffers2[index], tmpBuf, queBufferBytes);
					free(tmpBuf);
				} else {
					_queBuffers[index] = (char*)malloc(queBufferBytes);
					_queBufferSize[index] = queBufferBytes;
					decoder->readFixedFrames(_queBufferFrames, _queBuffers[index]);
				}
#else
                _queBuffers[index] = (char*)malloc(queBufferBytes);
                _queBufferSize[index] = queBufferBytes;

                decoder->readFixedFrames(_queBufferFrames, _queBuffers[index]);
#endif
            }

            _state = State::READY;
        }

    } while (false);

    if (decoder != nullptr)
    {
        decoder->close();
    }

    AudioDecoderManager::destroyDecoder(decoder);

    if (_state != State::READY)
    {
        _state = State::FAILED;
        if (_alBufferId != INVALID_AL_BUFFER_ID && alIsBuffer(_alBufferId))
        {
            ALOGV("readDataTask failed, delete buffer: %u", _alBufferId);
            alDeleteBuffers(1, &_alBufferId);
            _alBufferId = INVALID_AL_BUFFER_ID;
        }
#ifdef USE_AGTK_STEREO_PAN
		if (_alBufferId2 != INVALID_AL_BUFFER_ID && alIsBuffer(_alBufferId2))
		{
			ALOGV("readDataTask failed, delete buffer2: %u", _alBufferId2);
			alDeleteBuffers(1, &_alBufferId2);
			_alBufferId2 = INVALID_AL_BUFFER_ID;
		}
#endif
    }

    //FIXME: Why to invoke play callback first? Should it be after 'load' callback?
    invokingPlayCallbacks();
    invokingLoadCallbacks();

    _isLoadingFinished = true;
    _readDataTaskMutex.unlock();
    ALOGVV("readDataTask end, cache id=%u", selfId);
}

void AudioCache::addPlayCallback(const std::function<void()>& callback)
{
    std::lock_guard<std::mutex> lk(_playCallbackMutex);
    switch (_state)
    {
        case State::INITIAL:
        case State::LOADING:
            _playCallbacks.push_back(callback);
            break;

        case State::READY:
        // If state is failure, we still need to invoke the callback
        // since the callback will set the 'AudioPlayer::_removeByAudioEngine' flag to true.
        case State::FAILED:
            callback();
            break;

        default:
            ALOGE("Invalid state: %d", _state);
            break;
    }
}

void AudioCache::invokingPlayCallbacks()
{
    std::lock_guard<std::mutex> lk(_playCallbackMutex);

    for (auto&& cb : _playCallbacks)
    {
        cb();
    }

    _playCallbacks.clear();
}

void AudioCache::addLoadCallback(const std::function<void(bool)>& callback)
{
    switch (_state)
    {
        case State::INITIAL:
        case State::LOADING:
            _loadCallbacks.push_back(callback);
            break;

        case State::READY:
            callback(true);
            break;
        case State::FAILED:
            callback(false);
            break;

        default:
            ALOGE("Invalid state: %d", _state);
            break;
    }
}

void AudioCache::invokingLoadCallbacks()
{
    if (*_isDestroyed)
    {
        ALOGV("AudioCache (%p) was destroyed, don't invoke preload callback ...", this);
        return;
    }

    auto isDestroyed = _isDestroyed;
    auto scheduler = Director::getInstance()->getScheduler();
    scheduler->performFunctionInCocosThread([&, isDestroyed](){
        if (*isDestroyed)
        {
            ALOGV("invokingLoadCallbacks perform in cocos thread, AudioCache (%p) was destroyed!", this);
            return;
        }

        for (auto&& cb : _loadCallbacks)
        {
            cb(_state == State::READY);
        }

        _loadCallbacks.clear();
    });
}

#endif
