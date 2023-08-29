#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include "Lib/Macros.h"

NS_AGTK_BEGIN
/**
 * @brief 共有メモリーを管理するクラス。
 *		共有メモリーはプレビュー用プレイヤーでのみ使われ、パーティクルプレビュー時にプレイヤー画面をキャプチャーしてエディターに送るために使われる。
 */
class SharedMemory
{
public:
    SharedMemory();
    ~SharedMemory();
    void unlink();
    void setKey(const std::string &key);
	unsigned char *getAreaHead(int offset, int size);
	//int size(){ return mSize; }
    static SharedMemory *instance(){ return mInstance; }

protected:
    std::string mKey;
	void *mSharedMemoryObject;
	void *mRegion;
	int mSize;
    static SharedMemory *mInstance;
};
NS_AGTK_END

#endif // SHAREDMEMORY_H
