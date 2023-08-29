#ifndef __POOL_H__
#define	__POOL_H__

// #AGTK-NX
#if CC_TARGET_PLATFORM == CC_PLATFORM_NX

#include <new>
#include <vector>
#include <mutex>
#include "base/ccRef.h"
#include <iostream>

template<typename T, int NUM>
class PoolList
{
public:
	static PoolList<T,NUM>* getInstance()
	{
		if (_instance == nullptr)
		{
			_instance = new PoolList<T,NUM>();
		}
		return _instance;
	}

	PoolList<T,NUM>()
	{
		_poolList.reserve(NUM);
	}
	~PoolList<T,NUM>() {
	}

	void add(T* ptr)
	{
		lock();
		_poolList.push_back(ptr);
		unlock();
	}

	T* get()
	{
		lock();
		if (_poolList.empty())
		{
			unlock();
			return nullptr;
		}

		T* ptr = _poolList.back();
		_poolList.pop_back();
		unlock();

		return ptr;
	}

	void clear()
	{
		auto it = _poolList.begin();
		for (; it != _poolList.end(); it++)
		{
			delete (cocos2d::Ref*)*it;
		}
		_poolList.clear();
	}

private:
	void lock()
	{
		if (!_mutex.try_lock())
		{
			_mutex.lock();
		}
	}

	void unlock()
	{
		_mutex.unlock();
	}

	static PoolList<T,NUM>* _instance;
	std::vector<T*> _poolList;
	std::mutex _mutex;
};

template<typename T, int NUM>
PoolList<T, NUM>* PoolList<T, NUM>::_instance = nullptr;

#define POOL_OBJECT_METHOD(_T,_NUM) \
public: \
	static void* operator new(std::size_t sz, const std::nothrow_t& nothrow) \
	{ \
		auto list = PoolList<_T,_NUM>::getInstance(); \
		void* p = list->get(); \
		if (!p) \
		{ \
			p = ::new(std::nothrow) _T; \
			if (!p) \
				return nullptr; \
		} else { \
			p = ::new(p) _T; \
		} \
		return p; \
	} \
	\
	static void operator delete(void* ptr) \
	{ \
		_T* p = static_cast<_T*>(ptr); \
		auto list = PoolList<_T, _NUM>::getInstance(); \
		list->add(p); \
	}

#define POOL_OBJECT_METHOD_WITH_THROW(_T,_NUM) \
public: \
	static void* operator new(std::size_t sz) \
	{ \
		auto list = PoolList<_T,_NUM>::getInstance(); \
		void* p = list->get(); \
		if (!p) \
		{ \
			p = ::new(std::nothrow) _T; \
			if (!p) \
				return nullptr; \
		} else { \
			p = ::new(p) _T; \
		} \
		return p; \
	} \
	\
	static void operator delete(void* ptr) \
	{ \
		_T* p = static_cast<_T*>(ptr); \
		auto list = PoolList<_T, _NUM>::getInstance(); \
		list->add(p); \
	}

#define POOL_OBJECT_METHOD_PARTICLE(_T,_NUM) \
public: \
	static void* operator new(std::size_t sz, const std::nothrow_t& nothrow, int seed) \
	{ \
		auto list = PoolList<_T,_NUM>::getInstance(); \
		void* p = list->get(); \
		if (!p) \
		{ \
			p = ::new(std::nothrow) _T(seed); \
			if (!p) \
				return nullptr; \
		} else { \
			p = ::new(p) _T(seed); \
		} \
		return p; \
	} \
	\
	static void operator delete(void* ptr) \
	{ \
		_T* p = static_cast<_T*>(ptr); \
		auto list = PoolList<_T, _NUM>::getInstance(); \
		list->add(p); \
	}

#define POOL_DEFAULT_NUM_ObjectWallIntersect		50
#define POOL_DEFAULT_NUM_ObjectWallIntersectTemp	50
#define POOL_DEFAULT_NUM_WallHitInfoGroup			50
#define POOL_DEFAULT_NUM_ObjectMovementValue		50
#define POOL_DEFAULT_NUM_ObjectMovementForceMove	50
#define POOL_DEFAULT_NUM_ObjectMovementTimerFloat	50
#define POOL_DEFAULT_NUM_Player						50
#define POOL_DEFAULT_NUM_AreaData					50
#define POOL_DEFAULT_NUM_ParticleGroup				50
#define POOL_DEFAULT_NUM_Particle					100
#define POOL_DEFAULT_NUM_Bullet						50
#define POOL_DEFAULT_NUM_CollisionNode				50
#define POOL_DEFAULT_NUM_CollisionComponent			50

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_NX

#endif // __POOL_H__
