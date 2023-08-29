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
#include "base/CCProfiling.h"
#ifdef USE_AGTK_PROFILER
#include "platform/CCFileUtils.h"
#endif

using namespace std;

NS_CC_BEGIN

// Profiling Categories
/* set to false the categories that you don't want to profile */
bool kProfilerCategorySprite = false;
bool kProfilerCategoryBatchSprite = false;
bool kProfilerCategoryParticles = false;


static Profiler* g_sSharedProfiler = nullptr;

Profiler* Profiler::getInstance()
{
    if (! g_sSharedProfiler)
    {
        g_sSharedProfiler = new (std::nothrow) Profiler();
        g_sSharedProfiler->init();
    }

    return g_sSharedProfiler;
}

// FIXME:: deprecated
Profiler* Profiler::sharedProfiler(void)
{
    return Profiler::getInstance();
}

ProfilingTimer* Profiler::createAndAddTimerWithName(const char* timerName)
{
    ProfilingTimer *t = new (std::nothrow) ProfilingTimer();
    t->initWithName(timerName);
    _activeTimers.insert(timerName, t);
    t->release();

    return t;
}

void Profiler::releaseTimer(const char* timerName)
{
    _activeTimers.erase(timerName);
}

void Profiler::releaseAllTimers()
{
    _activeTimers.clear();
#ifdef USE_AGTK_PROFILER
	_framePointList.clear();
	if (_fp) {
		fclose(_fp);
		_fp = nullptr;
	}
#endif
}

bool Profiler::init()
{
    return true;
}

#ifdef USE_AGTK_PROFILER
FILE *Profiler::getFp()
{
	if (_fpAllocated) {
		return _fp;
	}
	auto fileUtils = FileUtils::getInstance();
	char buf[256];

	time_t now = time(nullptr);
	struct tm *p = localtime(&now);
	snprintf(buf, sizeof(buf), "perf-%04d%02d%02d-%02d%02d%02d.log", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	auto filename = fileUtils->getApplicationPath() + buf;
	_fp = fopen(filename.c_str(), "wb");
	if (!_fp) {
		CCLOG("Failed to open %s", filename.c_str());
	}
	_fpAllocated = true;
	return _fp;
}

#endif

#ifdef USE_AGTK_PROFILER
Profiler::Profiler(void)
: _activeTimers()
, _fpAllocated(false)
, _fp(nullptr)
, _started(false)
, _changingState(kNotChanging)
, _lastFrameStartPoint()
, _frameStartPoint()
, _frameEnd(true)
, _framePointList()
{
}
#endif
Profiler::~Profiler(void)
{
}

#ifdef USE_AGTK_PROFILER
#else
void Profiler::displayTimers()
{
    for (auto& iter : _activeTimers)
    {
        ProfilingTimer* timer = iter.second;
        log("%s", timer->getDescription().c_str());
    }
}
#endif
#ifdef USE_AGTK_PROFILER
void Profiler::startProfiling()
{
	if (_started) {
		if (_changingState != kStopping) {
			CCLOG("Profiler::start(): already started");
			return;
		}
	}
	else {
		if (_changingState == kStarting) {
			CCLOG("Profiler::start(): already starting");
			return;
		}
	}
	if (_changingState == kStopping) {
		_changingState = kNotChanging;
		return;
	}
	_changingState = kStarting;
}

void Profiler::stopProfiling()
{
	if (!_started) {
		if (_changingState != kStarting) {
			CCLOG("Profiler::stop(): already stopped");
			return;
		}
	}
	else {
		if (_changingState == kStopping) {
			CCLOG("Profiler::stop(): already stopping");
			return;
		}
	}
	if (_changingState == kStarting) {
		_changingState = kNotChanging;
		return;
	}
	_changingState = kStopping;
}

bool Profiler::isProfilingStarted()
{
	if (_started && _changingState != kStopping) {
		return true;
	}
	if (_changingState == kStarting) {
		return true;
	}
	if (_started) {
		return true;
	}
	return false;
}

void Profiler::startFrame()
{
	if (!_frameEnd) {
		endFrame();
	}
	if (_started) {
		if (!getFp()) {
			return;
		}
		fprintf(_fp, " | %f ]\n",
			std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now() - _lastFrameStartPoint)).count() / 1000.0);
	}
	if (_changingState == kStarting) {
		if (!getFp()) {
			return;
		}
		_changingState = kNotChanging;
		_started = true;
		fprintf(_fp, "[Start]\n");
		fflush(_fp);
	}
	else if (_changingState == kStopping) {
		if (!getFp()) {
			return;
		}
		_changingState = kNotChanging;
		_started = false;
		fprintf(_fp, "[Stop]\n");
		fflush(_fp);
	}
	_frameStartPoint = std::chrono::high_resolution_clock::now();
	if (!_started) {
		return;
	}
	_frameEnd = false;
}

void Profiler::endFrame()
{
	if (!_started) {
		_lastFrameStartPoint = _frameStartPoint;
		return;
	}
	if (_frameEnd) {
		CCLOG("Profiler::endFrame(): frame already ended");
		return;
	}
	auto now = std::chrono::high_resolution_clock::now();
	//èoóÕÅB
	if (!getFp()) {
		return;
	}
	fprintf(_fp, "[ %f:",
		std::chrono::duration_cast<std::chrono::microseconds>((now - _frameStartPoint)).count() / 1000.0);
	long long lastCount = 0;
	for (auto &framePoint : _framePointList) {
		auto currentCount = std::chrono::duration_cast<std::chrono::microseconds>(framePoint._count).count();
		fprintf(_fp, " %s(%f) |%f|\n\t", framePoint._pointName.c_str(), (currentCount - lastCount) / 1000.0, currentCount / 1000.0);
		lastCount = currentCount;
	}
	fprintf(_fp, "\n");
	//	Map<std::string, ProfilingTimer*> _activeTimers;
#if 0
	std::string _nameStr;
	std::chrono::high_resolution_clock::time_point _startTime;
	long _averageTime1;
	long _averageTime2;
	long minTime;
	long maxTime;
	long totalTime;
	long numberOfCalls;
#endif
	for (auto &it : _activeTimers) {
		if (it.second->numberOfCalls == 1) {
			fprintf(_fp, "\t%s(%f)\n",
				it.first.c_str(), it.second->totalTime / 1000.0);
		}
		else {
			fprintf(_fp, "\t%s(#%d, avg:%f, min:%f, max:%f, total:%f)\n",
				it.first.c_str(),
				it.second->numberOfCalls,
				it.second->totalTime / (it.second->numberOfCalls * 1000.0),
				it.second->minTime / 1000.0,
				it.second->maxTime / 1000.0,
				it.second->totalTime / 1000.0);
		}
	}
	fflush(_fp);
	_lastFrameStartPoint = _frameStartPoint;
	_framePointList.clear();
	_activeTimers.clear();
	_frameEnd = true;
}

void Profiler::setFramePoint(const char *pointName)
{
	if (!_started) {
		return;
	}
	if (_frameEnd) {
		CCLOG("PerfProfiler::setFramePoint(): frame not started");
		return;
	}
	auto now = std::chrono::high_resolution_clock::now();
	_framePointList.push_back(FramePointData(pointName, now - _frameStartPoint));
}
#endif

// implementation of ProfilingTimer

ProfilingTimer::ProfilingTimer()
: _averageTime1(0)
, _averageTime2(0)
, minTime(100000000)
, maxTime(0)
, totalTime(0)
, numberOfCalls(0)
#ifdef USE_AGTK_PROFILER
, _began(false)
, _startTimeStack()
#endif
{
}

bool ProfilingTimer::initWithName(const char* timerName)
{
    _nameStr = timerName;
    return true;
}

ProfilingTimer::~ProfilingTimer(void)
{
    
}

std::string ProfilingTimer::getDescription() const
{
    static char s_description[512] = {0};

    sprintf(s_description, "%s ::\tavg1: %ldu,\tavg2: %ldu,\tmin: %ldu,\tmax: %ldu,\ttotal: %.2fs,\tnr calls: %ld", _nameStr.c_str(), _averageTime1, _averageTime2, minTime, maxTime, totalTime/1000000., numberOfCalls);
    return s_description;
}

void ProfilingTimer::reset()
{
    numberOfCalls = 0;
    _averageTime1 = 0;
    _averageTime2 = 0;
    totalTime = 0;
    minTime = 100000000;
    maxTime = 0;
    _startTime = chrono::high_resolution_clock::now();
}

void ProfilingBeginTimingBlock(const char *timerName)
{
    Profiler* p = Profiler::getInstance();
#ifdef USE_AGTK_PROFILER
	if (!p->_started) {
		return;
	}
#endif
    ProfilingTimer* timer = p->_activeTimers.at(timerName);
    if( ! timer )
    {
        timer = p->createAndAddTimerWithName(timerName);
    }

    timer->numberOfCalls++;
#ifdef USE_AGTK_PROFILER
	if (timer->_began) {
		timer->_startTimeStack.push_back(timer->_startTime);
	} else {
		timer->_began = true;
	}
#endif

    // should be the last instruction in order to be more reliable
    timer->_startTime = chrono::high_resolution_clock::now();
}

void ProfilingEndTimingBlock(const char *timerName)
{
    // should be the 1st instruction in order to be more reliable
    auto now = chrono::high_resolution_clock::now();

    Profiler* p = Profiler::getInstance();
#ifdef USE_AGTK_PROFILER
	if (!p->_started) {
		return;
	}
#endif
    ProfilingTimer* timer = p->_activeTimers.at(timerName);

    CCASSERT(timer, "CCProfilingTimer  not found");


    long duration = static_cast<long>(chrono::duration_cast<chrono::microseconds>(now - timer->_startTime).count());

    timer->totalTime += duration;
    timer->_averageTime1 = (timer->_averageTime1 + duration) / 2.0f;
    timer->_averageTime2 = timer->totalTime / timer->numberOfCalls;
    timer->maxTime = MAX( timer->maxTime, duration);
    timer->minTime = MIN( timer->minTime, duration);
#ifdef USE_AGTK_PROFILER
	if (timer->_startTimeStack.size() > 0) {
		timer->_startTime = timer->_startTimeStack.back();
		timer->_startTimeStack.pop_back();
	} else {
		timer->_began = false;
	}
#endif
}

#ifdef USE_AGTK_PROFILER
#else
void ProfilingResetTimingBlock(const char *timerName)
{
    Profiler* p = Profiler::getInstance();
    ProfilingTimer *timer = p->_activeTimers.at(timerName);

    CCASSERT(timer, "CCProfilingTimer not found");

    timer->reset();
}
#endif

NS_CC_END

