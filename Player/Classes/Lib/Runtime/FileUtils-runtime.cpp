#include "Lib/Macros.h"
#ifdef USE_RUNTIME
#include "platform/CCPlatformConfig.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "platform/win32/CCFileUtils-win32.h"
#endif
#include "platform/CCCommon.h"
#include "FileUtils-runtime.h"

using namespace std;

NS_CC_BEGIN

#define CC_MAX_PATH  512

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
//static FileUtilsWin32 *s_platformFileUtils = nullptr;
#endif
static char s_key[16 + 1];

// The root path of resources, the character encoding is UTF-8.
// UTF-8 is the only encoding supported by cocos2d-x API.
static std::string s_resourcePath = "";

#if 0
FileUtils* FileUtils::getInstance()
{
    if (s_sharedFileUtils == nullptr)
    {
        s_sharedFileUtils = new FileUtilsWin32();
        if(!s_sharedFileUtils->init())
        {
          delete s_sharedFileUtils;
          s_sharedFileUtils = nullptr;
          CCLOG("ERROR: Could not init CCFileUtilsWin32");
        }
    }
    return s_sharedFileUtils;
}
#endif

FileUtilsRuntime::FileUtilsRuntime()
{
}

void FileUtilsRuntime::createAndSet()
{
	auto fileUtil = new FileUtilsRuntime();
	fileUtil->init();
	FileUtils::setDelegate(fileUtil);
}

void FileUtilsRuntime::setKey(const std::string &key)
{
	memcpy(s_key, key.c_str(), key.size() + 1);
}

const char *FileUtilsRuntime::key()
{
	return s_key;
}

#if 0
bool FileUtilsRuntime::init()
{
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	//s_platformFileUtils = dynamic_cast<FileUtilsWin32 *>(FileUtils::getInstance());
#endif
    s_sharedFileUtils = this;
}

std::string FileUtilsRuntime::getWritablePath() const
{
    s_platformFileUtils->getWritablePath();
}

bool FileUtilsRuntime::isAbsolutePath(const std::string& strPath) const
{
    return s_platformFileUtils->isAbsolutePath(strPath);
}

std::string FileUtilsRuntime::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return s_platformFileUtils->getSuitableFOpen(filenameUtf8);
}

long FileUtilsRuntime::getFileSize(const std::string &filepath)
{
    return s_platformFileUtils->getFileSize(filepath);
}

#ifdef USE_AGTK//sakihama-h, 2016.11.15
std::string FileUtilsRuntime::getApplicationPath()
{
    return s_platformFileUtils->getApplicationPath();
}

std::vector<std::string> FileUtilsRuntime::getDirContents(std::string dirname)
{
    return s_platformFileUtils->getDirContents(dirname);
}

#endif


bool FileUtilsRuntime::isFileExistInternal(const std::string& strFilePath) const
{
    return s_platformFileUtils->isFileExistInternal(strFilePath);
}

bool FileUtilsRuntime::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    return s_platformFileUtils->renameFile(path, oldname, name);
}

bool FileUtilsRuntime::renameFile(const std::string &oldfullpath, const std::string &newfullpath)
{
    return s_platformFileUtils->renameFile(oldfullpath, newfullpath);
}

bool FileUtilsRuntime::isDirectoryExistInternal(const std::string& dirPath) const
{
    return s_platformFileUtils->isDirectoryExistInternal(dirPath);
}

bool FileUtilsRuntime::removeFile(const std::string &filepath)
{
    return s_platformFileUtils->removeFile(filepath);
}

bool FileUtilsRuntime::createDirectory(const std::string& dirPath)
{
    return s_platformFileUtils->createDirectory(dirPath);
}

bool FileUtilsRuntime::removeDirectory(const std::string& dirPath)
{
    return s_platformFileUtils->removeDirectory(dirPath);
}
#endif

FileUtils::Status FileUtilsRuntime::getContents(const std::string& filename, ResizableBuffer* buffer) const
{
	if (strlen(s_key) == 0) {
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
		return FileUtilsWin32::getContents(filename, buffer);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
	}
	Data d;
	ResizableBufferAdapter<Data> buf(&d);
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
	auto status = FileUtilsWin32::getContents(filename, &buf);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
    if(status != FileUtils::Status::OK){
		buffer->resize(d.getSize());
		memcpy(buffer->buffer(), d.getBytes(), d.getSize());
		d.clear();
        return status;
    }
	buffer->resize(d.getSize());
	memcpy(buffer->buffer(), d.getBytes(), d.getSize());
	d.clear();
	return status;
}

#if 0
std::string FileUtilsRuntime::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    return s_platformFileUtils->getPathForFilename(filename, resolutionDirectory, searchPath);
}

std::string FileUtilsRuntime::getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const
{
    return s_platformFileUtils->getFullPathForDirectoryAndFilename(directory, filename);
}
#endif

NS_CC_END

#endif