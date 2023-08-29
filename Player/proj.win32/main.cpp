//#include "C:/Program Files (x86)/Visual Leak Detector/include/vld.h"
#include "main.h"
#include "AppDelegate.h"
#include "cocos2d.h"
#ifdef USE_AGTK
#include <Windows.h>
#include <dbghelp.h>
#endif

#ifdef USE_AGTK
// WCHAR文字列をcharに変換する
static LPSTR wtoa(LPCWSTR src)
{
	LPSTR buf;
	int dst_size, rc;

	rc = WideCharToMultiByte(CP_UTF8, 0, src, -1, NULL, 0, NULL, NULL);
	if (rc == 0) {
		return NULL;
	}

	dst_size = rc + 1;
	buf = (LPSTR)malloc(dst_size);
	if (buf == NULL) {
		return NULL;
	}

	rc = WideCharToMultiByte(CP_UTF8, 0, src, -1, buf, dst_size, NULL, NULL);
	if (rc == 0) {
		free(buf);
		return NULL;
	}
	buf[rc] = '\0';

	return buf;
}

// char文字列をWCHARに変換する
static WCHAR *atow(const char *src)
{
	//
	WCHAR *buf;
	int dst_size, rc;

	rc = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
	if (rc == 0) {
		return nullptr;
	}

	dst_size = rc + 1;
	buf = new (std::nothrow)WCHAR[dst_size];
	if (buf == nullptr) {
		return nullptr;
	}

	rc = MultiByteToWideChar(CP_UTF8, 0, src, -1, buf, dst_size);
	if (rc == 0) {
		delete[] buf;
		return nullptr;
	}
	buf[rc] = 0;

	return buf;
}
#endif
#ifdef USE_AGTK
static int generateDump(EXCEPTION_POINTERS *expPtr)
{
	int logNumber = 0;
	int argc;
	auto argv_w = CommandLineToArgvW(GetCommandLine(), &argc);
#ifdef USE_RUNTIME
	do if (argc >= 2) {
		auto argv_c = wtoa(argv_w[1]);
		if (argv_c == NULL) {
			break;
		}
		CCLOG("%d: %s", 1, argv_c);
		logNumber = atoi(argv_c);
		free(argv_c);
	} while (0);
#else
	do if (argc >= 5) {
		auto argv_c = wtoa(argv_w[4]);
		if (argv_c == NULL) {
			break;
		}
		CCLOG("%d: %s", 4, argv_c);
		logNumber = atoi(argv_c);
		free(argv_c);
	} while (0);
#endif
	(void)LocalFree((HLOCAL)argv_w);

	WCHAR logNumberStr[8];
	memset(logNumberStr, 0, sizeof(logNumberStr));
	if (logNumber > 0) {
		_snwprintf(logNumberStr, 8, L"%d", logNumber);
	}

	WCHAR dumpPath[MAX_PATH];
	auto wAppPath = atow(FileUtils::getInstance()->getApplicationPath().c_str());
	_snwprintf(
		dumpPath, MAX_PATH, L"%s\\player%s.dmp",
		wAppPath, logNumberStr);
	delete[] wAppPath;

	HANDLE dumpFile =
		CreateFile(dumpPath, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	MINIDUMP_EXCEPTION_INFORMATION expInfo;
	expInfo.ThreadId = GetCurrentThreadId();
	expInfo.ExceptionPointers = expPtr;
	expInfo.ClientPointers = TRUE;

	BOOL miniDumpSuccessful =
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile,
			MiniDumpWithDataSegs, &expInfo, NULL, NULL);

	return EXCEPTION_EXECUTE_HANDLER;
}

#define STRUCTURED_EXCEPTION_EXIT_CODE	-999
static int doStructuredExceptionHandling()
{
	int ret = 0;
	__try {
		ret = Application::getInstance()->run();
	}
	__except (generateDump(GetExceptionInformation())) {
		ret = STRUCTURED_EXCEPTION_EXIT_CODE;
	}
	return ret;
}
#endif

USING_NS_CC;

int WINAPI _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // create the application instance
#if defined(USE_AGTK) && !defined(AGTK_DEBUG)
	//Releaseビルド/Runtimeビルドで、引数が指定されている時のみダンプファイルを作成するように。
	int argc;
	auto argv_w = CommandLineToArgvW(GetCommandLine(), &argc);
	(void)LocalFree((HLOCAL)argv_w);
	if (argc >= 2) {
		AppDelegate *app = new AppDelegate();
		int ret = doStructuredExceptionHandling();
		if (ret != STRUCTURED_EXCEPTION_EXIT_CODE) {
			delete app;
		}
		return ret;
}
	else {
		AppDelegate app;
		return Application::getInstance()->run();
	}
#else
    AppDelegate app;
    return Application::getInstance()->run();
#endif
}
