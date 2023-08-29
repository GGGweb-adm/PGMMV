#ifndef __NR_ARRAY_H__
#define	__NR_ARRAY_H__

#include "Lib/Macros.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
//RetainしないArray
class NrArray : public Ref, public Clonable
{
public:

	/** Creates an empty array. Default capacity is 10
	* @js NA
	* @lua NA
	*/
	static NrArray* create();
	/** Create an array with objects
	* @js NA
	*/
	static NrArray* create(Ref* object, ...) CC_REQUIRES_NULL_TERMINATION;
	/** Create an array with one object
	* @js NA
	*/
	static NrArray* createWithObject(Ref* object);
	/** Create an array with a default capacity
	* @js NA
	*/
	static NrArray* createWithCapacity(ssize_t capacity);
	/** Create an array with from an existing array
	* @js NA
	*/
	static NrArray* createWithArray(NrArray* otherArray);
	/**
	@brief   Generate a NrArray pointer by file
	@param   pFileName  The file name of *.plist file
	@return  The NrArray pointer generated from the file
	* @js NA
	*/
	//static NrArray* createWithContentsOfFile(const std::string& pFileName);

	/*
	@brief The same meaning as arrayWithContentsOfFile(), but it doesn't call autorelease, so the
	invoker should call release().
	* @js NA
	* @lua NA
	*/
	//static NrArray* createWithContentsOfFileThreadSafe(const std::string& pFileName);
	/**
	* @js NA
	* @lua NA
	*/
	~NrArray();

	/** Initializes an array
	* @js NA
	* @lua NA
	*/
	bool init();
	/** Initializes an array with one object
	* @js NA
	* @lua NA
	*/
	bool initWithObject(Ref* object);
	/** Initializes an array with some objects
	* @js NA
	* @lua NA
	*/
	bool initWithObjects(Ref* object, ...) CC_REQUIRES_NULL_TERMINATION;
	/** Initializes an array with capacity
	* @js NA
	* @lua NA
	*/
	bool initWithCapacity(ssize_t capacity);
	/** Initializes an array with an existing array
	* @js NA
	* @lua NA
	*/
	bool initWithArray(NrArray* otherArray);

	// Querying an NrArray

	/** Returns element count of the array
	* @js NA
	*/
	ssize_t count() const
	{
		return data.size();
	}
	/** Returns capacity of the array
	* @js NA
	*/
	ssize_t capacity() const
	{
		return data.capacity();
	}
	/** Returns index of a certain object, return UINT_MAX if doesn't contain the object
	* @js NA
	* @lua NA
	*/
	ssize_t getIndexOfObject(Ref* object) const;
	/**
	* @js NA
	*/
	CC_DEPRECATED_ATTRIBUTE ssize_t indexOfObject(Ref* object) const { return getIndexOfObject(object); }

	/** Returns an element with a certain index
	* @js NA
	* @lua NA
	*/
	Ref* getObjectAtIndex(ssize_t index)const
	{
		CCASSERT(index >= 0 && index < count(), "index out of range in getObjectAtIndex()");
		return data[index];
	}
	CC_DEPRECATED_ATTRIBUTE Ref* objectAtIndex(ssize_t index) { return getObjectAtIndex(index); }
	/** Returns the last element of the array
	* @js NA
	*/
	Ref* getLastObject()
	{
		return data.back();
	}
	/**
	* @js NA
	*/
	CC_DEPRECATED_ATTRIBUTE Ref* lastObject() { return getLastObject(); }
	/** Returns a random element
	* @js NA
	* @lua NA
	*/
	Ref* getRandomObject();
	/**
	* @js NA
	*/
	CC_DEPRECATED_ATTRIBUTE Ref* randomObject() { return getRandomObject(); }
	/** Returns a Boolean value that indicates whether object is present in array.
	* @js NA
	*/
	bool containsObject(Ref* object) const;
	/** @since 1.1
	* @js NA
	*/
	bool isEqualToArray(const NrArray* otherArray)const;
	// Adding Objects

	/** Add a certain object
	* @js NA
	*/
	void addObject(Ref* object);
	/**
	* @js NA
	*/
	/** Add all elements of an existing array
	* @js NA
	*/
	void addObjectsFromArray(NrArray* otherArray);
	void addObjectsFromArray(cocos2d::Array* otherArray);
	/** Insert a certain object at a certain index
	* @js NA
	*/
	void insertObject(Ref* object, ssize_t index);
	/** sets a certain object at a certain index
	* @js NA
	* @lua NA
	*/
	void setObject(Ref* object, ssize_t index);
	/** sets a certain object at a certain index without retaining. Use it with caution
	* @js NA
	* @lua NA
	*/
	void fastSetObject(Ref* object, ssize_t index)
	{
		setObject(object, index);
	}
	/**
	* @js NA
	* @lua NA
	*/
	void swap(ssize_t indexOne, ssize_t indexTwo)
	{
		CCASSERT(indexOne >= 0 && indexOne < count() && indexTwo >= 0 && indexTwo < count(), "Invalid indices");
		std::swap(data[indexOne], data[indexTwo]);
	}

	// Removing Objects

	/** Remove last object
	* @js NA
	*/
	void removeLastObject(bool releaseObj = true);
	/** Remove a certain object
	* @js NA
	*/
	void removeObject(Ref* object, bool releaseObj = true);
	/** Remove an element with a certain index
	* @js NA
	*/
	void removeObjectAtIndex(ssize_t index, bool releaseObj = true);
	/** Remove all elements
	* @js NA
	*/
	void removeObjectsInArray(NrArray* otherArray);
	/** Remove all objects
	* @js NA
	*/
	void removeAllObjects();
	/** Fast way to remove a certain object
	* @js NA
	*/
	void fastRemoveObject(Ref* object);
	/** Fast way to remove an element with a certain index
	* @js NA
	*/
	void fastRemoveObjectAtIndex(ssize_t index);

	// Rearranging Content

	/** Swap two elements
	* @js NA
	*/
	void exchangeObject(Ref* object1, Ref* object2);
	/** Swap two elements with certain indexes
	* @js NA
	*/
	void exchangeObjectAtIndex(ssize_t index1, ssize_t index2);

	/** Replace object at index with another object.
	* @js NA
	*/
	void replaceObjectAtIndex(ssize_t index, Ref* object, bool releaseObject = true);

	/** Revers the array
	* @js NA
	*/
	void reverseObjects();
	/* Shrinks the array so the memory footprint corresponds with the number of items
	* @js NA
	*/
	void reduceMemoryFootprint();

	/* override functions
	* @js NA
	*/
	//virtual void acceptVisitor(DataVisitor &visitor);
	/**
	* @js NA
	* @lua NA
	*/
	virtual NrArray* clone() const override;

	// ------------------------------------------
	// Iterators
	// ------------------------------------------
	typedef std::vector<cocos2d::Ref *>::iterator iterator;
	typedef std::vector<cocos2d::Ref *>::const_iterator const_iterator;
	/**
	* @js NA
	* @lua NA
	*/
	iterator begin() { return data.begin(); }
	/**
	* @js NA
	* @lua NA
	*/
	iterator end() { return data.end(); }
	const_iterator cbegin() { return data.cbegin(); }
	/**
	* @js NA
	* @lua NA
	*/
	const_iterator cend() { return data.cend(); }

	std::vector<cocos2d::Ref *> data;


	//protected:
	/**
	* @js NA
	* @lua NA
	*/
	NrArray();
};
#endif

NS_AGTK_END

#endif	//__NR_ARRAY_H__
