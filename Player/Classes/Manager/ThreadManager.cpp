#include <base/CCProfiling.h>
#include "ThreadManager.h"

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
ThreadManager* ThreadManager::_threadManager = NULL;
#ifdef USE_THREADMANAGER_PRINTF
std::mutex ThreadManager::sThreadMutex;
FILE *ThreadManager::sThreadFp = nullptr;
#endif
int ThreadManager::sUsedThreadCount = MAX(1, MIN(2, AGTK_THREAD_COUNT));
#ifdef USE_MULTITHREAD_MEASURE
int ThreadManager::mtVectorBlockedCount = 0;
int ThreadManager::objectBlockedCount = 0;
int ThreadManager::playerBlockedCount = 0;
#endif

ThreadManager::ThreadManager()
{
	// スレッド生成
	for (int i = 0; i < AGTK_THREAD_COUNT - 1; i++) {
		_threadInfoList[i].state = kThreadStateIdle;		// 状態
		_threadInfoList[i].i = i + 1;						// スレッド番号
		_threadInfoList[i].localMutex.lock();
		_threadInfoList[i].localMutex3.lock();
		_threadInfoList[i].thread = new std::thread(updateThread, &_threadInfoList[i]);	// スレッド生成
		_threadInfoList[i].func = nullptr;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
		auto result = SetThreadPriority(_threadInfoList[i].thread->native_handle(), THREAD_PRIORITY_ABOVE_NORMAL);	// 優先順位
		cocos2d::log("SetThreadPriority: %d", result);
#endif
	}
}

ThreadManager::~ThreadManager()
{
	// スレッド解放
	for (int i = 0; i < AGTK_THREAD_COUNT - 1; i++) {
		_threadInfoList[i].state = kThreadStateEnd;
		_threadInfoList[i].localMutex.unlock();
	}
	for (int i = 0; i < AGTK_THREAD_COUNT - 1; i++) {
		_threadInfoList[i].thread->join();
		_threadInfoList[i].localMutex3.unlock();
		delete _threadInfoList[i].thread;
		_threadInfoList[i].thread = nullptr;
	}
}

ThreadManager* ThreadManager::getInstance()
{
	if (!_threadManager) {
		_threadManager = new ThreadManager();
	}
	return _threadManager;
}

void ThreadManager::purge()
{
	if (!_threadManager) {
		return;
	}

	ThreadManager *tm = _threadManager;
	_threadManager = nullptr;
	tm->release();
}

ThreadManager::ThreadInfo *ThreadManager::getThreadInfo(int index)
{
	CC_ASSERT(index >= 0 && index < AGTK_THREAD_COUNT - 1);
	return &_threadInfoList[index];
}

#ifdef USE_MULTITHREAD_MEASURE
static std::chrono::time_point<std::chrono::steady_clock> getNow() {
	auto now = chrono::high_resolution_clock::now();
	return now;
}
#endif
void ThreadManager::updateThread(ThreadManager::ThreadInfo *threadInfo)
{
#if __NCC_TARGET_PLATFORM == CC_PLATFORM_NXX__
#endif
	threadInfo->localMutex2.lock();
	while (true) {
		threadInfo->localMutex.lock();
		// 状態がスレッド終了ならループを抜けて処理を終わらせる
		if (threadInfo->state == kThreadStateEnd) {
			threadInfo->localMutex.unlock();
			threadInfo->localMutex2.unlock();
			break;
		}
		// 状態が開始準備でない場合は10us待ち
		if (threadInfo->state != kThreadStateReady) {
			cocos2d::log("not ready: %d", threadInfo->state);
			threadInfo->localMutex.unlock();
			std::this_thread::sleep_for(10us);
			continue;
		}
		// 状態を作業中に変更し処理を行う
		threadInfo->state = kThreadStateWorking;
#ifdef USE_MULTITHREAD_MEASURE
		auto startTime = getNow();
		long maxDuration = 0;
#endif
		int index = -1;
		if (threadInfo->func) {
			threadInfo->func(threadInfo);
#ifdef USE_MULTITHREAD_MEASURE
			maxDuration = threadInfo->maxDuration;
#endif
		}
#ifdef USE_MULTITHREAD_MEASURE
		auto now = getNow();
		auto duration = static_cast<long>(chrono::duration_cast<chrono::microseconds>(now - startTime).count());
		sprintf(threadInfo->buf, "%d: max: %dus, total: %dus", threadInfo->count, threadInfo->maxDuration, duration);
#endif
		// 状態を終了に変更
		threadInfo->state = kThreadStateFinished;
		threadInfo->localMutex2.unlock();
		threadInfo->localMutex3.lock();
		threadInfo->localMutex.unlock();
		threadInfo->localMutex2.lock();
		threadInfo->localMutex3.unlock();
		//std::this_thread::sleep_for(1us);
	}
}

void ThreadManager::printf(const char *format, ...)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	sThreadMutex.lock();
	if (!sThreadFp) {
		auto fileUtils = FileUtils::getInstance();
		sThreadFp = fopen(fileUtils->getSuitableFOpen(fileUtils->getApplicationPath() + "/thread.log").c_str(), "wb");
	}
	if (sThreadFp) {
		va_list arglist;
		va_start(arglist, format);
		vfprintf(sThreadFp, format, arglist);
		va_end(arglist);
		if (format[strlen(format) - 1] != '\n') {
			fprintf(sThreadFp, "\r\n");
		}
		fflush(sThreadFp);
	}
	sThreadMutex.unlock();
#endif
}

#ifdef USE_MULTITHREAD_UPDOWN_THREADS
void ThreadManager::changeUsedThreadCount(int add)
{
	auto newUsedThreadCount = sUsedThreadCount + add;
	if (newUsedThreadCount < 1) {
		newUsedThreadCount = 1;
	}
	else if (newUsedThreadCount > AGTK_THREAD_COUNT) {
		newUsedThreadCount = AGTK_THREAD_COUNT;
	}
	if (sUsedThreadCount != newUsedThreadCount) {
		CCLOG("newUsedThreadCount: %d", newUsedThreadCount);
		sUsedThreadCount = newUsedThreadCount;
#ifdef USE_AGTK_PROFILER
		auto fp = Profiler::getInstance()->getFp();
		if (fp) {
			fprintf(fp, "#usedThreadCount: %d", sUsedThreadCount);
		}
#endif
		THREAD_PRINTF("#usedThreadCount: %d", sUsedThreadCount);
	}
}
#endif

void ThreadManager::setUsedThreadCount(int count)
{
	sUsedThreadCount = count;
}

int ThreadManager::getUsedThreadCount()
{
	return sUsedThreadCount;
}

#ifdef USE_MULTITHREAD_MEASURE
void ThreadManager::clearBlockedCount()
{
	mtVectorBlockedCount = 0;
	objectBlockedCount = 0;
	playerBlockedCount = 0;
}
#endif

#endif
