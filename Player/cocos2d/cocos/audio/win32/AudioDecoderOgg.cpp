/****************************************************************************
 Copyright (c) 2016 Chukong Technologies Inc.
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

#include "audio/win32/AudioDecoderOgg.h"
#include "audio/win32/AudioMacros.h"
#include "platform/CCFileUtils.h"

#define LOG_TAG "AudioDecoderOgg"

namespace cocos2d { namespace experimental {

#ifdef USE_AGTK//日本語ディレクトリの対応(※ファイルパスをutf8->sjisに変換） sakihama-h, 2018.05.07
#endif
    AudioDecoderOgg::AudioDecoderOgg()
    {
#ifdef USE_AGTK_MEMORY_OGG
		memset(&_memoryData, 0, sizeof(_memoryData));
#endif
    }

    AudioDecoderOgg::~AudioDecoderOgg()
    {
        close();
    }

#ifdef USE_AGTK_MEMORY_OGG
	size_t AudioDecoderOgg::memoryDataRead(void *buffer, size_t size, size_t count, void *stream)
	{
		MemoryData *md = (MemoryData *)stream;
		auto restBytes = md->_size - md->_head;
		auto useCount = restBytes / size;
		if (useCount > count) {
			useCount = count;
		}
		memcpy(buffer, md->_data + md->_head, useCount * size);
		md->_head += useCount * size;
		return useCount;
	}

	int AudioDecoderOgg::memoryDataSeek(void *stream, ogg_int64_t offset, int origin)
	{
		MemoryData *md = (MemoryData *)stream;
		ogg_int64_t newHead = 0;
		switch (origin) {
		case SEEK_CUR:
			newHead = md->_head + offset;
			break;
		case SEEK_END:
			newHead = md->_size + offset;
			break;
		case SEEK_SET:
			newHead = offset;
			break;
		default:
			return -1;
		}
		if (newHead > md->_size) {
			md->_head = md->_size;
			return -1;
		} else if(newHead < 0) {
			md->_head = 0;
			return -1;
		}
		md->_head = newHead;
		return 0;
	}

	int AudioDecoderOgg::memoryDataClose(void *stream)
	{
		return 0;
	}

	long AudioDecoderOgg::memoryDataTell(void *stream)
	{
		MemoryData *md = (MemoryData *)stream;
		return md->_head;
	}
#endif
    bool AudioDecoderOgg::open(const char* path)
    {
#ifdef USE_AGTK_MEMORY_OGG
		ssize_t nSize;
		auto loadData = cocos2d::FileUtils::getInstance()->getFileData(path, "rb", &nSize);
		_memoryData._data = loadData;
		_memoryData._size = nSize;
		_memoryData._head = 0;
		ov_callbacks callbacks = {
			memoryDataRead,
			memoryDataSeek,
			memoryDataClose,
			memoryDataTell
		};
		if (0 == ov_open_callbacks((void *)&_memoryData, &_vf, nullptr, 0, callbacks))
#else
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(path);
#ifdef USE_AGTK//日本語ディレクトリの対応(※ファイルパスをutf8->sjisに変換） sakihama-h, 2018.05.07
		fullPath = FileUtils::getInstance()->getSuitableFOpen(fullPath);
#endif
        if (0 == ov_fopen(FileUtils::getInstance()->getSuitableFOpen(fullPath).c_str(), &_vf))
#endif
        {
            // header
            vorbis_info* vi = ov_info(&_vf, -1);
            _sampleRate = static_cast<uint32_t>(vi->rate);
            _channelCount = vi->channels;
            _bytesPerFrame = vi->channels * sizeof(short);
            _totalFrames = static_cast<uint32_t>(ov_pcm_total(&_vf, -1));
#ifdef USE_AGTK_AUDIO_DECODER_LOOP_INFO
			char **ptr = ov_comment(&_vf, -1)->user_comments;
			while (*ptr) {
				if (memcmp(*ptr, "LOOPSTART=", 10) == 0) {
					_loopStart = std::atoi((const char *)*ptr + 10);
				}
				else if (memcmp(*ptr, "LOOPLENGTH=", 11) == 0) {
					_loopLength = std::atoi((const char *)*ptr + 11);
				}
				++ptr;
			}
			if (_loopLength < 44100) {
				// ACT2-5345 ループ長が短すぎると現状のバッファリング処理が扱えなくなるため無理やり大きくする。
				_loopLength = 44100;
			}
#endif
            _isOpened = true;
            return true;
        }
        return false;
    }

    void AudioDecoderOgg::close()
    {
        if (isOpened())
        {
            ov_clear(&_vf);
#ifdef USE_AGTK_MEMORY_OGG
			if (_memoryData._data) {
				free(_memoryData._data);
				memset(&_memoryData, 0, sizeof(_memoryData));
			}
#endif
            _isOpened = false;
        }
    }

    uint32_t AudioDecoderOgg::read(uint32_t framesToRead, char* pcmBuf)
    {
        int currentSection = 0;
        int bytesToRead = framesToRead * _bytesPerFrame;
        long bytesRead = ov_read(&_vf, pcmBuf, bytesToRead, 0, 2, 1, &currentSection);
        return static_cast<uint32_t>(bytesRead / _bytesPerFrame);
    }

    bool AudioDecoderOgg::seek(uint32_t frameOffset)
    {
        return 0 == ov_pcm_seek(&_vf, frameOffset);
    }

    uint32_t AudioDecoderOgg::tell() const
    {
        return static_cast<uint32_t>(ov_pcm_tell(const_cast<OggVorbis_File*>(&_vf)));
    }
#ifdef USE_AGTK_OGG_INFO
	void AudioDecoderOgg::getInfo(const char *filename, int *pSampleRate, int *pTotalSamples, int *pLoopStart, int *pLoopLength)
	{
		*pSampleRate = -1;
		*pTotalSamples = -1;
		*pLoopStart = -1;
		*pLoopLength = -1;
		if (!open(filename)) {
			return;
		}
		close();
	}
#endif

}} // namespace cocos2d { namespace experimental {
