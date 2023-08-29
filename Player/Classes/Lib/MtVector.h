#ifndef _MT_VECTOR_H_
#define _MT_VECTOR_H_
#include "Lib/Macros.h"
#include "Lib/NrArray.h"
#include "Manager/ThreadManager.h"

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
namespace agtk {
//マルチスレッドアクセスされたときに問題を起こしにくいVectorクラス。
template <typename T>
class MtVector {
public:
	MtVector(): m_array(nullptr), m_lastArray(nullptr), m_size(0), m_reserved(128){
		m_array = new T[m_reserved];
	}
	MtVector(MtVector &v) : m_array(nullptr), m_lastArray(nullptr), m_size(0), m_reserved(128) {
		if (!v.try_lock()) {
			v.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		if (m_reserved < v.m_size) {
			m_reserved = v.m_size;
		}
		m_array = new T[m_reserved];
		m_size = v.m_size;
		for (int i = 0; i < m_size; i++) {
			m_array[i] = v.m_array[i];
		}
		v.unlock();
	}
	virtual ~MtVector() {
		delete[] m_array;
		m_array = nullptr;
		delete[] m_lastArray;
		m_lastArray = nullptr;
		m_size = 0;
		m_reserved = 0;
	}
	void swap(int index1, int index2) {
		if (index1 >= 0 && index1 < (int)m_size && index2 >= 0 && index2 < (int)m_size) {
			T tmp = m_array[index1];
			m_array[index1] = m_array[index2];
			m_array[index2] = tmp;
		}
	}
	size_t size() const {
		return m_size;
	}
	size_t count() const {
		return m_size;
	}
	void clear() {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		auto size = m_size;
		for (int i = 0; i < (int)size; i++) {
			//delete m_array[i];
			m_array[i] = nullptr;
		}
		m_size = 0;
		m_mutex.unlock();
	}
	void removeAllObjects() {
		clear();
	}

	void push_back(const T &item) {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		for (int i = 0; i < (int)m_size; i++) {
			if (!m_array[i]) {
				m_array[i] = item;
				m_mutex.unlock();
				return;
			}
		}
		if (m_size >= m_reserved) {
			auto newReserved = m_reserved + 128;
			T *newArray = new T[newReserved];
			for (int i = 0; i < (int)m_size; i++) {
				newArray[i] = m_array[i];
			}
			if (m_lastArray) {
				delete[] m_lastArray;
				m_lastArray = nullptr;
			}
			m_lastArray = m_array;
			for (int i = 0; i < (int)m_size; i++) {
				m_lastArray[i] = nullptr;
			}
			m_array = newArray;
			m_reserved = newReserved;
		}
		m_array[m_size] = item;
		m_size++;
		m_mutex.unlock();
	}
	void addObject(const T &item) {
		push_back(item);
	}
	void append(MtVector &v) {
		if (this < &v) {
			if (!m_mutex.try_lock()) {
				m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
				ThreadManager::mtVectorBlockedCount++;
#endif
			}
			if (!v.m_mutex.try_lock()) {
				v.m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
				ThreadManager::mtVectorBlockedCount++;
#endif
			}
		}
		else {
			if (!v.m_mutex.try_lock()) {
				v.m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
				ThreadManager::mtVectorBlockedCount++;
#endif
			}
			if (!m_mutex.try_lock()) {
				m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
				ThreadManager::mtVectorBlockedCount++;
#endif
			}
		}
		int size = 0;
		for (int i = 0; i < (int)v.m_size; i++) {
			if (v.m_array[i]) {
				size++;
			}
		}
		if (m_size + size > m_reserved) {
			auto newReserved = m_size + size + 128;
			T *newArray = new T[newReserved];
			for (int i = 0; i < (int)m_size; i++) {
				newArray[i] = m_array[i];
			}
			if (m_lastArray) {
				delete[] m_lastArray;
				m_lastArray = nullptr;
			}
			m_lastArray = m_array;
			for (int i = 0; i < (int)m_size; i++) {
				m_lastArray[i] = nullptr;
			}
			m_array = newArray;
			m_reserved = newReserved;
		}
		int j = 0;
		for (int i = 0; i < (int)v.m_size; i++) {
			if (v.m_array[i]) {
				m_array[j + m_size] = v.m_array[i];
				j++;
			}
		}
		m_size += size;
		m_mutex.unlock();
		v.m_mutex.unlock();
	}
	void addObjectsFromArray(MtVector *v) {
		append(*v);
	}
	void addObjectsFromArray(cocos2d::__Array *array) {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		if (m_size + array->count() > m_reserved) {
			auto newReserved = m_size + array->count() + 128;
			T *newArray = new T[newReserved];
			for (int i = 0; i < (int)m_size; i++) {
				newArray[i] = m_array[i];
			}
			if (m_lastArray) {
				delete[] m_lastArray;
				m_lastArray = nullptr;
			}
			m_lastArray = m_array;
			for (int i = 0; i < (int)m_size; i++) {
				m_lastArray[i] = nullptr;
			}
			m_array = newArray;
			m_reserved = newReserved;
		}
		int i = 0;
		cocos2d::Ref* ref = nullptr;
		CCARRAY_FOREACH(array, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			m_array[i + m_size] = static_cast<T>(ref);
#else
			m_array[i + m_size] = dynamic_cast<T>(ref);
#endif
			i++;
		}
		m_size += array->count();
		m_mutex.unlock();
	}
	void addObjectsFromArray(agtk::NrArray *array) {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		if (m_size + array->count() > m_reserved) {
			auto newReserved = m_size + array->count() + 128;
			T *newArray = new T[newReserved];
			for (int i = 0; i < (int)m_size; i++) {
				newArray[i] = m_array[i];
			}
			if (m_lastArray) {
				delete[] m_lastArray;
				m_lastArray = nullptr;
			}
			m_lastArray = m_array;
			for (int i = 0; i < (int)m_size; i++) {
				m_lastArray[i] = nullptr;
			}
			m_array = newArray;
			m_reserved = newReserved;
		}
		int i = 0;
		//cocos2d::Ref* ref = nullptr;
		//CCARRAY_FOREACH(array, ref) {
		for(auto ref: *array){
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			m_array[i + m_size] = static_cast<T>(ref);
#else
			m_array[i + m_size] = dynamic_cast<T>(ref);
#endif
			i++;
		}
		m_size += array->count();
		m_mutex.unlock();
	}
	T &operator[](size_t pos) {
		if (pos >= m_size) {
			throw std::out_of_range("T &operator[]");
		}
		return m_array[pos];
	}
	T &getObjectAtIndex(size_t pos) {
		if (pos >= m_size) {
			throw std::out_of_range("T &getObjectAtIndex");
		}
		return m_array[pos];
	}
	void pop_back() {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		if (m_size > 0) {
			m_array[m_size - 1] = nullptr;
			m_size--;
		}
		m_mutex.unlock();
	}
	void removeObjectAtIndex(size_t pos) {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		m_array[pos] = nullptr;
		if (pos == m_size - 1) {
			m_size--;
		}
		m_mutex.unlock();
	}
	void removeObject(const T &item) {
		if (!m_mutex.try_lock()) {
			m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		auto size = m_size;
		for (int i = 0; i < (int)size; i++) {
			if (m_array[i] == item) {
				m_array[i] = nullptr;
				if (i == size - 1) {
					m_size = size - 1;
				}
				break;
			}
		}
		m_mutex.unlock();
	}
	int indexOf(const T &item) {
		auto size = m_size;
		for (int i = 0; i < (int)size; i++) {
			if (m_array[i] == item) {
				return i;
			}
		}
		return -1;
	}
	bool containsObject(const T &item) {
		auto size = m_size;
		for (int i = 0; i < (int)size; i++) {
			if (m_array[i] == item) {
				return true;
			}
		}
		return false;
	}
	MtVector<T> &operator=(const MtVector<T> &data) {
		if (!data.m_mutex.try_lock()) {
			data.m_mutex.lock();
#ifdef USE_MULTITHREAD_MEASURE
			ThreadManager::mtVectorBlockedCount++;
#endif
		}
		if (m_array) {
			delete[] m_array;
			m_array = nullptr;
		}
		if(m_lastArray){
			delete[] m_lastArray;
			m_lastArray = nullptr;
		}
		m_array = new T[data.m_size];
		m_size = data.m_size;
		m_reserved = m_size;
		for (int i = 0; i < m_size; i++) {
			m_array[i] = data.m_array[i];
		}
		data.m_mutex.unlock();
		return *this;
	}
protected:
	std::mutex m_mutex;
	T *m_array;
	T *m_lastArray;
	size_t m_size;
	size_t m_reserved;
};
}
#endif

#endif /* _MT_VECTOR_H_ **/
