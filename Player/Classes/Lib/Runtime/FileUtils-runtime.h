#ifndef __CC_FILEUTILS_RUNTIME_H__
#define __CC_FILEUTILS_RUNTIME_H__

#include "Lib/Macros.h"
#ifdef USE_RUNTIME
#include "platform/CCPlatformConfig.h"
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include "platform/win32/CCFileUtils-win32.h"
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX
#endif

#include "platform/CCFileUtils.h"
#include "platform/CCPlatformMacros.h"
#include "base/ccTypes.h"
#include <string>
#include <vector>

NS_CC_BEGIN

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
class FileUtilsRuntime : public FileUtilsWin32//FileUtils
// #AGTK-NX
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
{
    //friend class FileUtils;
protected:
    FileUtilsRuntime();
public:

	static void createAndSet();
	void setKey(const std::string &key);
	const char *key();
#if 0
	bool init();
	virtual std::string getWritablePath() const override;
    virtual bool isAbsolutePath(const std::string& strPath) const override;
    virtual std::string getSuitableFOpen(const std::string& filenameUtf8) const override;
    virtual long getFileSize(const std::string &filepath);
#ifdef USE_AGTK//sakihama-h, 2016.11.15
	virtual std::string getApplicationPath();
	virtual std::vector<std::string> getDirContents(std::string dirname);
#endif
#endif
protected:

#if 0
    virtual bool isFileExistInternal(const std::string& strFilePath) const override;

    virtual bool renameFile(const std::string &path, const std::string &oldname, const std::string &name) override;

    virtual bool renameFile(const std::string &oldfullpath, const std::string &newfullpath) override;

    virtual bool isDirectoryExistInternal(const std::string& dirPath) const override;

    virtual bool removeFile(const std::string &filepath) override;

    virtual bool createDirectory(const std::string& dirPath) override;

    virtual bool removeDirectory(const std::string& dirPath) override;
#endif

	virtual FileUtils::Status getContents(const std::string& filename, ResizableBuffer* buffer) const override;

#if 0
    virtual std::string getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const override;

    virtual std::string getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const override;
#endif
};

NS_CC_END

#endif // USE_RUNTIME

#endif    // __CC_FILEUTILS_RUNTIME_H__
