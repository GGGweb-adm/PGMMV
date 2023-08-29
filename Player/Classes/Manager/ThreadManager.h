#ifndef __THREAD_MANAGER_H__
#define	__THREAD_MANAGER_H__

#include "Lib/Macros.h"

// map関連で多数するのでwarningを抑制「C4503: 装飾された名前の長さが限界を超えました。名前は切り捨てられます。」
#pragma warning(disable:4503)

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#define USE_THREADMANAGER_PRINTF
class ThreadManager : public cocos2d::Ref
{
public:
private:
	ThreadManager();

public:
#ifdef USE_MULTITHREAD_UPDOWN_THREADS
#define AGTK_THREAD_COUNT	8
#else
#define AGTK_THREAD_COUNT	2
#endif

	enum ThreadState {
		kThreadStateIdle = 0,		// 待ち
		kThreadStateReady,			// 処理開始準備
		kThreadStateWorking,		// 処理中
		kThreadStateFinished,		// 処理終了
		kThreadStateEnd				// スレッド終了
	};
	struct ThreadInfo {
		ThreadState state;				// 状態
		int i;							// スレッド番号
		std::thread *thread;			// スレッド
		std::mutex localMutex;
		std::mutex localMutex2;
		std::mutex localMutex3;
#ifdef USE_MULTITHREAD_MEASURE
		int count;
		long maxDuration;
		char buf[1024];
#endif
		void (*func)(ThreadInfo *threadInfo);
		void *subInfo;
	};
	// デストラクタ
	virtual ~ThreadManager();

	// インスタンスを取得する。
	static ThreadManager* getInstance();

	static void updateThread(ThreadInfo *threadInfo);
	ThreadInfo *getThreadInfo(int index);
#ifdef USE_THREADMANAGER_PRINTF
	static void printf(const char *format, ...);
#endif
#ifdef USE_MULTITHREAD_UPDOWN_THREADS
	static void changeUsedThreadCount(int add);
#endif
	static void setUsedThreadCount(int count);
	static int getUsedThreadCount();
#ifdef USE_MULTITHREAD_MEASURE
	static void clearBlockedCount();
#endif

	static void purge();

public:
#ifdef USE_MULTITHREAD_MEASURE
	static int mtVectorBlockedCount;
	static int objectBlockedCount;
	static int playerBlockedCount;
#endif

protected:
	static ThreadManager *_threadManager;

	ThreadInfo _threadInfoList[AGTK_THREAD_COUNT - 1];	//メインスレッド分を除く。

#ifdef USE_THREADMANAGER_PRINTF
	static std::mutex sThreadMutex;
	static FILE *sThreadFp;
#endif
	static int sUsedThreadCount;

};
#endif
#ifdef USE_THREADMANAGER_PRINTF
#define THREAD_PRINTF(format, ...)	ThreadManager::printf(format, ##__VA_ARGS__)
#else
#define THREAD_PRINTF(...)
#endif


#endif	//__THREAD_MANAGER_H__
