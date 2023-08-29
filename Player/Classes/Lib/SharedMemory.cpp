// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#endif
#include "SharedMemory.h"

USING_NS_AGTK;

SharedMemory *SharedMemory::mInstance = NULL;

// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
using namespace boost::interprocess;
#endif

SharedMemory::SharedMemory()
: mSharedMemoryObject(nullptr)
, mRegion(nullptr)
, mKey()
, mSize(0)
//, mHead(0)
{
	mInstance = this;
}

SharedMemory::~SharedMemory()
{
    unlink();
    mInstance = NULL;
}

void SharedMemory::unlink()
{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	mSize = 0;
    if(mRegion){
        mapped_region *region = (mapped_region *)mRegion;
        mRegion = nullptr;
        delete region;
    }
    if(mSharedMemoryObject){
        //shared_memory_object::remove(mKey.c_str());
		delete (shared_memory_object *)mSharedMemoryObject;
        mSharedMemoryObject = nullptr;
    }
#endif
}

void SharedMemory::setKey(const std::string &key)
{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	unlink();
    mKey = key;

	//Create a shared memory object.
	shared_memory_object *shm = new shared_memory_object(open_only, mKey.c_str(), read_write);
    mSharedMemoryObject = (void *)shm;

	//Map the whole shared memory in this process
	mapped_region *region = new mapped_region(*shm, read_write);
    mRegion = (void *)region;
	mSize = region->get_size();
	CCLOG("SharedMemory::setKey: %s, %d", key.c_str(), mSize);
#endif
}

unsigned char *SharedMemory::getAreaHead(int offset, int size)
{
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
	if(mRegion == nullptr || offset < 0 || size < 0){
        return nullptr;
    }
	if (offset + size > mSize) {
		if (mRegion) {
			delete mRegion;
			mRegion = nullptr;
		}
		mapped_region *region = new mapped_region(*(shared_memory_object *)mSharedMemoryObject, read_write);
		mRegion = (void *)region;
		mSize = region->get_size();
		CCLOG("SharedMemory::getAreaHead: %s, %d", mKey.c_str(), mSize);
		if (offset + size > mSize) {
			return nullptr;
		}
	}
	mapped_region *region = (mapped_region *)mRegion;
	return (unsigned char *)region->get_address() + offset;
#else
	return nullptr;
#endif
}

