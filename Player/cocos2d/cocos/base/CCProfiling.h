/****************************************************************************
Copyright (c) 2010      Stuart Carnie
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#ifndef __SUPPORT_CCPROFILING_H__
#define __SUPPORT_CCPROFILING_H__
/// @cond DO_NOT_SHOW

#include <string>
#include <chrono>
#include "base/ccConfig.h"
#include "base/CCRef.h"
#include "base/CCMap.h"

#ifdef USE_AGTK
#define USE_AGTK_PROFILER
#endif
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
NS_CC_BEGIN

/**
 * @addtogroup global
 * @{
 */

class ProfilingTimer;

/** Profiler
 cocos2d builtin profiler.

 To use it, enable set the CC_ENABLE_PROFILERS=1 in the ccConfig.h file
 */

class CC_DLL Profiler : public Ref
{
public:
#ifdef USE_AGTK_PROFILER
	Profiler(void);
#endif
    /**
     * @js NA
     * @lua NA
     */
    ~Profiler(void);
#ifdef USE_AGTK_PROFILER
#else
    /** display the timers
     * @js NA
     * @lua NA
     */
    void displayTimers(void);
#endif
    /**
     * @js NA
     * @lua NA
     */
    bool init(void);

public:
    /** returns the singleton 
     * @js NA
     * @lua NA
     */
    static Profiler* getInstance(void);

    /**
     * @js NA
     * @lua NA
     */
    CC_DEPRECATED_ATTRIBUTE static Profiler* sharedProfiler(void);

    /** Creates and adds a new timer 
     * @js NA
     * @lua NA
     */
    ProfilingTimer* createAndAddTimerWithName(const char* timerName);
    /** releases a timer 
     * @js NA
     * @lua NA
     */
    void releaseTimer(const char* timerName);
    /** releases all timers 
     * @js NA
     * @lua NA
     */
    void releaseAllTimers();

#ifdef USE_AGTK_PROFILER
	void startProfiling();
	void stopProfiling();
	bool isProfilingStarted();
	void startFrame();
	void endFrame();
	void setFramePoint(const char *pointName);
	FILE *getFp();
#endif
    Map<std::string, ProfilingTimer*> _activeTimers;
#ifdef USE_AGTK_PROFILER
	bool _fpAllocated;
	FILE *_fp;
	bool _started;
	enum EnumChangingState {
		kNotChanging,
		kStopping,
		kStarting,
	};
	EnumChangingState _changingState;
	std::chrono::high_resolution_clock::time_point _lastFrameStartPoint;
	std::chrono::high_resolution_clock::time_point _frameStartPoint;
	bool _frameEnd;
	class FramePointData {
	public:
		FramePointData(const char *pointName, const std::chrono::nanoseconds &count) : _pointName(pointName), _count(count) {}
		~FramePointData() {}
		std::string _pointName;
		std::chrono::nanoseconds _count;
	};
	std::list<FramePointData> _framePointList;
#endif
};

class ProfilingTimer : public Ref
{
public:
    /**
     * @js NA
     * @lua NA
     */
    ProfilingTimer();
    /**
     * @js NA
     * @lua NA
     */
    ~ProfilingTimer(void);
    /**
     * @js NA
     * @lua NA
     */
    bool initWithName(const char* timerName);
    /**
     * @js NA
     * @lua NA
     */
    virtual std::string getDescription() const;
    /**
     * @js NA
     * @lua NA
     */
    const std::chrono::high_resolution_clock::time_point& getStartTime() { return _startTime; }

    /** resets the timer properties
     * @js NA
     * @lua NA
     */
    void reset();

    std::string _nameStr;
    std::chrono::high_resolution_clock::time_point _startTime;
    long _averageTime1;
    long _averageTime2;
    long minTime;
    long maxTime;
    long totalTime;
    long numberOfCalls;
#ifdef USE_AGTK_PROFILER
	bool _began;
	std::vector< std::chrono::high_resolution_clock::time_point> _startTimeStack;
#endif
};

extern void CC_DLL ProfilingBeginTimingBlock(const char *timerName);
extern void CC_DLL ProfilingEndTimingBlock(const char *timerName);
#ifdef USE_AGTK_PROFILER
#else
extern void CC_DLL ProfilingResetTimingBlock(const char *timerName);
#endif
#ifdef USE_AGTK
/**********************/
/** Profiling Macros **/
/**********************/
#if CC_ENABLE_PROFILERS

#ifdef USE_AGTK_PROFILER
#define CC_PROFILER_START_PROFILING() NS_CC::Profiler::getInstance()->startProfiling()
#define CC_PROFILER_STOP_PROFILING() NS_CC::Profiler::getInstance()->stopProfiling()
#define CC_PROFILER_IS_PROFILING_STARTED() NS_CC::Profiler::getInstance()->isProfilingStarted()
#define CC_PROFILER_START_FRAME() NS_CC::Profiler::getInstance()->startFrame()
#define CC_PROFILER_END_FRAME() NS_CC::Profiler::getInstance()->endFrame()
#define CC_PROFILER_SET_FRAME_POINT(pointName) NS_CC::Profiler::getInstance()->setFramePoint(pointName)
#else
#define CC_PROFILER_DISPLAY_TIMERS() NS_CC::Profiler::getInstance()->displayTimers()
#endif
#define CC_PROFILER_PURGE_ALL() NS_CC::Profiler::getInstance()->releaseAllTimers()

#define CC_PROFILER_START(__name__) NS_CC::ProfilingBeginTimingBlock(__name__)
#define CC_PROFILER_STOP(__name__) NS_CC::ProfilingEndTimingBlock(__name__)
#ifdef USE_AGTK_PROFILER
#else
#define CC_PROFILER_RESET(__name__) NS_CC::ProfilingResetTimingBlock(__name__)
#endif

#define CC_PROFILER_START_CATEGORY(__cat__, __name__) do{ if(__cat__) NS_CC::ProfilingBeginTimingBlock(__name__); } while(0)
#define CC_PROFILER_STOP_CATEGORY(__cat__, __name__) do{ if(__cat__) NS_CC::ProfilingEndTimingBlock(__name__); } while(0)
#ifdef USE_AGTK_PROFILER
#else
#define CC_PROFILER_RESET_CATEGORY(__cat__, __name__) do{ if(__cat__) NS_CC::ProfilingResetTimingBlock(__name__); } while(0)
#endif

#define CC_PROFILER_START_INSTANCE(__id__, __name__) do{ NS_CC::ProfilingBeginTimingBlock( NS_CC::String::createWithFormat("%08X - %s", __id__, __name__)->getCString() ); } while(0)
#define CC_PROFILER_STOP_INSTANCE(__id__, __name__) do{ NS_CC::ProfilingEndTimingBlock(    NS_CC::String::createWithFormat("%08X - %s", __id__, __name__)->getCString() ); } while(0)
#ifdef USE_AGTK_PROFILER
#else
#define CC_PROFILER_RESET_INSTANCE(__id__, __name__) do{ NS_CC::ProfilingResetTimingBlock( NS_CC::String::createWithFormat("%08X - %s", __id__, __name__)->getCString() ); } while(0)
#endif

//! @brief コンストラクタからデストラクタまでの時間計測
class ScopeProfiler {
public:
	ScopeProfiler(const char* name) : _name(name) {
		CC_PROFILER_START(_name);
	}
	~ScopeProfiler() {
		CC_PROFILER_STOP(_name);
	}
	const char* _name;
};
#define PROFILING(blockName,varName) ScopeProfiler	_local##varName(blockName)

#else

#ifdef USE_AGTK_PROFILER
#define CC_PROFILER_START_PROFILING()
#define CC_PROFILER_STOP_PROFILING()
#define CC_PROFILER_IS_PROFILING_STARTED() false
#define CC_PROFILER_START_FRAME()
#define CC_PROFILER_END_FRAME()
#define CC_PROFILER_SET_FRAME_POINT(pointName)
#else
#define CC_PROFILER_DISPLAY_TIMERS() do {} while (0)
#endif
#define CC_PROFILER_PURGE_ALL() do {} while (0)

#define CC_PROFILER_START(__name__)  do {} while (0)
#define CC_PROFILER_STOP(__name__) do {} while (0)
#define CC_PROFILER_RESET(__name__) do {} while (0)

#define CC_PROFILER_START_CATEGORY(__cat__, __name__) do {} while(0)
#define CC_PROFILER_STOP_CATEGORY(__cat__, __name__) do {} while(0)
#define CC_PROFILER_RESET_CATEGORY(__cat__, __name__) do {} while(0)

#define CC_PROFILER_START_INSTANCE(__id__, __name__) do {} while(0)
#define CC_PROFILER_STOP_INSTANCE(__id__, __name__) do {} while(0)
#define CC_PROFILER_RESET_INSTANCE(__id__, __name__) do {} while(0)

#define PROFILING(blockName,varName) do {} while(0)

#endif
#endif

/*
 * cocos2d profiling categories
 * used to enable / disable profilers with granularity
 */

extern bool kProfilerCategorySprite;
extern bool kProfilerCategoryBatchSprite;
extern bool kProfilerCategoryParticles;

// end of global group
/// @}

NS_CC_END

/// @endcond
#endif // __SUPPORT_CCPROFILING_H__
