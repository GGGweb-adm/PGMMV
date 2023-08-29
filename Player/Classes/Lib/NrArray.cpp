/**
 * @brief NrArray
 */
#include "NrArray.h"

NS_AGTK_BEGIN

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
//  ----------------------------------------------------------------------------------
// std::vector implementation
//  ----------------------------------------------------------------------------------

NrArray::NrArray()
	: data()
{
	init();
}

NrArray* NrArray::create()
{
	NrArray* array = new (std::nothrow) NrArray();

	if (array && array->initWithCapacity(7))
	{
		array->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(array);
	}

	return array;
}

NrArray* NrArray::createWithObject(Ref* object)
{
	NrArray* array = new (std::nothrow) NrArray();

	if (array && array->initWithObject(object))
	{
		array->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(array);
	}

	return array;
}

NrArray* NrArray::create(Ref* object, ...)
{
	va_list args;
	va_start(args, object);

	NrArray* array = create();
	if (array && object)
	{
		array->addObject(object);
		Ref* i = va_arg(args, Ref*);
		while (i)
		{
			array->addObject(i);
			i = va_arg(args, Ref*);
		}
	}
	else
	{
		CC_SAFE_DELETE(array);
	}

	va_end(args);

	return array;
}

NrArray* NrArray::createWithArray(NrArray* otherArray)
{
	return otherArray->clone();
}

NrArray* NrArray::createWithCapacity(ssize_t capacity)
{
	CCASSERT(capacity >= 0, "Invalid capacity");

	NrArray* array = new (std::nothrow) NrArray();

	if (array && array->initWithCapacity(capacity))
	{
		array->autorelease();
	}
	else
	{
		CC_SAFE_DELETE(array);
	}

	return array;
}

#if 0
NrArray* NrArray::createWithContentsOfFile(const std::string& fileName)
{
	NrArray* ret = NrArray::createWithContentsOfFileThreadSafe(fileName);
	if (ret != nullptr)
	{
		ret->autorelease();
	}
	return ret;
}

NrArray* NrArray::createWithContentsOfFileThreadSafe(const std::string& fileName)
{
	return FileUtils::getInstance()->createArrayWithContentsOfFile(fileName);
}
#endif

bool NrArray::init()
{
	return initWithCapacity(7);
}

bool NrArray::initWithObject(Ref* object)
{
	bool ret = initWithCapacity(7);
	if (ret)
	{
		addObject(object);
	}
	return ret;
}

/** Initializes an array with some objects */
bool NrArray::initWithObjects(Ref* object, ...)
{
	bool ret = false;
	do
	{
		CC_BREAK_IF(object == nullptr);

		va_list args;
		va_start(args, object);

		if (object)
		{
			this->addObject(object);
			Ref* i = va_arg(args, Ref*);
			while (i)
			{
				this->addObject(i);
				i = va_arg(args, Ref*);
			}
			ret = true;
		}
		va_end(args);

	} while (false);

	return ret;
}

bool NrArray::initWithCapacity(ssize_t capacity)
{
	CCASSERT(capacity >= 0, "Invalid capacity");

	data.reserve(capacity);
	return true;
}

bool NrArray::initWithArray(NrArray* otherArray)
{
	data = otherArray->data;
	return true;
}

ssize_t NrArray::getIndexOfObject(Ref* object) const
{
	auto it = data.begin();

	for (ssize_t i = 0; it != data.end(); ++it, ++i)
	{
		if (*it == object)
		{
			return i;
		}
	}

	return -1;
}

Ref*  NrArray::getRandomObject()
{
	if (data.size() == 0)
	{
		return nullptr;
	}

	float r = CCRANDOM_0_1();

	if (r == 1) // to prevent from accessing data-arr[data->num], out of range.
	{
		r = 0;
	}

	r *= data.size();

	return data[r];
}

bool NrArray::containsObject(Ref* object) const
{
	ssize_t i = this->getIndexOfObject(object);
	return (i >= 0);
}

bool NrArray::isEqualToArray(const NrArray* otherArray) const
{
	for (ssize_t i = 0; i < this->count(); ++i)
	{
		if (this->getObjectAtIndex(i) != otherArray->getObjectAtIndex(i))
		{
			return false;
		}
	}
	return true;
}

void NrArray::addObject(Ref* object)
{
	data.push_back(object);
}

void NrArray::addObjectsFromArray(NrArray* otherArray)
{
	data.insert(data.end(), otherArray->data.begin(), otherArray->data.end());
}
void NrArray::addObjectsFromArray(cocos2d::Array* otherArray)
{
	data.insert(data.end(), otherArray->begin(), otherArray->end());
}

void NrArray::insertObject(Ref* object, ssize_t index)
{
	data.insert(std::begin(data) + index, object);
}

void NrArray::setObject(Ref* object, ssize_t index)
{
	data[index] = object;
}

void NrArray::removeLastObject(bool releaseObj)
{
	CCASSERT(data.size(), "no objects added");
	data.pop_back();
}

void NrArray::removeObject(Ref* object, bool releaseObj /* ignored */)
{
	auto it = std::remove(data.begin(), data.end(), object);
	if (it != data.end()) {
		data.erase(it);
	}
}

void NrArray::removeObjectAtIndex(ssize_t index, bool releaseObj /* ignored */)
{
	auto obj = data[index];
	data.erase(data.begin() + index);
}

void NrArray::removeObjectsInArray(NrArray* otherArray)
{
	CCASSERT(false, "not implemented");
}

void NrArray::removeAllObjects()
{
	data.erase(std::begin(data), std::end(data));
}

void NrArray::fastRemoveObjectAtIndex(ssize_t index)
{
	removeObjectAtIndex(index);
}

void NrArray::fastRemoveObject(Ref* object)
{
	removeObject(object);
}

void NrArray::exchangeObject(Ref* object1, Ref* object2)
{
	ssize_t idx1 = getIndexOfObject(object1);
	ssize_t idx2 = getIndexOfObject(object2);

	CCASSERT(idx1 >= 0 && idx2 >= 2, "invalid object index");

	std::swap(data[idx1], data[idx2]);
}

void NrArray::exchangeObjectAtIndex(ssize_t index1, ssize_t index2)
{
	std::swap(data[index1], data[index2]);
}

void NrArray::replaceObjectAtIndex(ssize_t index, Ref* object, bool releaseObject /* ignored */)
{
	data[index] = object;
}

void NrArray::reverseObjects()
{
	std::reverse(std::begin(data), std::end(data));
}

void NrArray::reduceMemoryFootprint()
{
	// N/A
}

NrArray::~NrArray()
{
	CCLOGINFO("deallocing NrArray: %p - len: %d", this, count());
}

NrArray* NrArray::clone() const
{
	NrArray* ret = new (std::nothrow) NrArray();
	ret->autorelease();
	ret->initWithCapacity(this->data.size() > 0 ? this->data.size() : 1);

	Ref* tmpObj = nullptr;
	Clonable* clonable = nullptr;
	//const NrArray* self(this);
	for (auto obj : this->data) {
		clonable = dynamic_cast<Clonable*>(obj);
		if (clonable)
		{
			tmpObj = dynamic_cast<Ref*>(clonable->clone());
			if (tmpObj)
			{
				ret->addObject(tmpObj);
			}
		}
		else
		{
			CCLOGWARN("%s isn't clonable.", typeid(*obj).name());
		}
	}
	return ret;
}

#if 0
void NrArray::acceptVisitor(DataVisitor &visitor)
{
	visitor.visit(this);
}
#endif
// ----------------------------------------------------------------------------------
// ccArray implementation
// ----------------------------------------------------------------------------------
#endif

NS_AGTK_END
