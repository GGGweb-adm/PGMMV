#include "AnimationData.h"

USING_NS_CC;

NS_AGTK_BEGIN
NS_DATA_BEGIN

//-------------------------------------------------------------------------------------------------------------------

/**
 * @class FrameData
 */
FrameData::FrameData()
{
	_id = -1;
	_playSeId = -1;
	_playVoiceId = -1;
}

FrameData::~FrameData()
{
}

bool FrameData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("frameCount300"));
	this->setFrameCount300(json["frameCount300"].GetInt());
	CC_ASSERT(json.HasMember("interpType"));
	this->setInterpType((FrameData::Interpolate)json["interpType"].GetInt());
	CC_ASSERT(json.HasMember("playSe"));
	this->setPlaySe(json["playSe"].GetBool());
	CC_ASSERT(json.HasMember("playSeId"));
	this->setPlaySeId(json["playSeId"].GetInt());
	CC_ASSERT(json.HasMember("playVoice"));
	this->setPlayVoice(json["playVoice"].GetBool());
	CC_ASSERT(json.HasMember("playVoiceId"));
	this->setPlayVoiceId(json["playVoiceId"].GetInt());
	CC_ASSERT(json.HasMember("centerOrigin"));
	this->setCenterOrigin(json["centerOrigin"].GetBool());
	CC_ASSERT(json.HasMember("originX"));
	this->setOriginX(json["originX"].GetDouble());
	CC_ASSERT(json.HasMember("originY"));
	this->setOriginY(json["originY"].GetDouble());
	CC_ASSERT(json.HasMember("offsetX"));
	this->setOffsetX(json["offsetX"].GetDouble());
	CC_ASSERT(json.HasMember("offsetY"));
	this->setOffsetY(json["offsetY"].GetDouble());
	CC_ASSERT(json.HasMember("centerX"));
	this->setCenterX(json["centerX"].GetDouble());
	CC_ASSERT(json.HasMember("centerY"));
	this->setCenterY(json["centerY"].GetDouble());
	CC_ASSERT(json.HasMember("scalingX"));
	this->setScalingX(json["scalingX"].GetDouble());
	CC_ASSERT(json.HasMember("scalingY"));
	this->setScalingY(json["scalingY"].GetDouble());
	CC_ASSERT(json.HasMember("flipX"));
	this->setFlipX(json["flipX"].GetBool());
	CC_ASSERT(json.HasMember("flipY"));
	this->setFlipY(json["flipY"].GetBool());
	CC_ASSERT(json.HasMember("rotation"));
	this->setRotation(json["rotation"].GetDouble());
	CC_ASSERT(json.HasMember("alpha"));
	this->setAlpha(json["alpha"].GetInt());
	CC_ASSERT(json.HasMember("r"));
	this->setR(json["r"].GetInt());
	CC_ASSERT(json.HasMember("g"));
	this->setG(json["g"].GetInt());
	CC_ASSERT(json.HasMember("b"));
	this->setB(json["b"].GetInt());
	CC_ASSERT(json.HasMember("imageTileX"));
	this->setImageTileX(json["imageTileX"].GetInt());
	CC_ASSERT(json.HasMember("imageTileY"));
	this->setImageTileY(json["imageTileY"].GetInt());
	return true;
}

cocos2d::Color3B FrameData::getColor()
{
	return cocos2d::Color3B(this->getR(), this->getG(), this->getB());
}

cocos2d::Vec2 FrameData::getScale()
{
	return cocos2d::Vec2(this->getScalingX() * 0.01f, this->getScalingY() * 0.01f);
}

cocos2d::Vec2 FrameData::getOffset()
{
	return cocos2d::Vec2(this->getOffsetX(), this->getOffsetY());
}

cocos2d::Vec2 FrameData::getCenter()
{
	return cocos2d::Vec2(this->getCenterX(), this->getCenterY());
}

#if defined(AGTK_DEBUG)
void FrameData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("frameCount300:%d", this->getFrameCount300());
	CCLOG("interpType:%d", this->getInterpType());
	CCLOG("playSe:%d", this->getPlaySe());
	CCLOG("playSeId:%d", this->getPlaySeId());
	CCLOG("playVoice:%d", this->getPlayVoice());
	CCLOG("playVoiceId:%d", this->getPlayVoiceId());
	CCLOG("centerOrigin:%d", this->getCenterOrigin());
	CCLOG("originX:%f", this->getOriginX());
	CCLOG("originY:%f", this->getOriginY());
	CCLOG("offsetX:%f", this->getOffsetX());
	CCLOG("offsetY:%f", this->getOffsetY());
	CCLOG("centerX:%f", this->getCenterX());
	CCLOG("centerY:%f", this->getCenterY());
	CCLOG("scalingX:%f", this->getScalingX());
	CCLOG("scalingY:%f", this->getScalingY());
	CCLOG("flipX:%d", this->getFlipX());
	CCLOG("flipY:%d", this->getFlipY());
	CCLOG("rotation:%f", this->getRotation());
	CCLOG("alpha:%d", this->getAlpha());
	CCLOG("r:%d", this->getR());
	CCLOG("g:%d", this->getG());
	CCLOG("b:%d", this->getB());
	CCLOG("imageTileX:%d", this->getImageTileX());
	CCLOG("imageTileY:%d", this->getImageTileY());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
TimelineInfoAreaData::TimelineInfoAreaData()
{
}

TimelineInfoAreaData::~TimelineInfoAreaData()
{
}

bool TimelineInfoAreaData::init(const rapidjson::Value& json)
{
	this->setFrame(json["frame"].GetInt());
	this->setX(json["x"].GetDouble());
	this->setY(json["y"].GetDouble());
	this->setWidth(json["width"].GetDouble());
	this->setHeight(json["height"].GetDouble());
	this->setBackside(json["backside"].GetBool());
	this->setValid(json["valid"].GetBool());
	this->setInterpType((EnumInterpType)json["interpType"].GetInt());
	return true;
}

#if defined(AGTK_DEBUG)
void TimelineInfoAreaData::dump()
{
	CCLOG("frame:%d", this->getFrame());
	CCLOG("x:%f", this->getX());
	CCLOG("y:%f", this->getY());
	CCLOG("width:%f", this->getWidth());
	CCLOG("height:%f", this->getHeight());
	CCLOG("backside:%d", this->getBackside());
	CCLOG("valid:%d", this->getValid());
	CCLOG("interpType:%d", this->getInterpType());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
TimelineInfoData::TimelineInfoData()
{
	_name = nullptr;
	_areaList = nullptr;
}
TimelineInfoData::~TimelineInfoData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_areaList);
}

bool TimelineInfoData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setTimelineType((EnumTimelineType)json["timelineType"].GetInt());
	auto areaList = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["areaList"].Size(); i++) {
		auto areaData = TimelineInfoAreaData::create(json["areaList"][i]);
		if (areaData == nullptr) {
			CC_ASSERT(0);
			return false;
		}
		areaList->addObject(areaData);
	}
	this->setAreaList(areaList);
	return true;
}

const char *TimelineInfoData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

agtk::data::TimelineInfoAreaData *TimelineInfoData::getAreaData(int id)
{
	if (id < 0 || id >= this->getAreaList()->count()) {
		return nullptr;
	}
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
	return static_cast<agtk::data::TimelineInfoAreaData *>(this->getAreaList()->getObjectAtIndex(id));
#else
	return dynamic_cast<agtk::data::TimelineInfoAreaData *>(this->getAreaList()->getObjectAtIndex(id));
#endif
}

#if defined(AGTK_DEBUG)
void TimelineInfoData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("timelineType:%d", this->getTimelineType());
	auto areaList = this->getAreaList();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(areaList, ref) {
		auto areaData = dynamic_cast<TimelineInfoAreaData *>(ref);
		areaData->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * class DirectionData
 */
DirectionData::DirectionData()
{
	_name = nullptr;
	_animationName = nullptr;
	_timelineInfoList = nullptr;
	_frameList = nullptr;
	_imageWidth = 0;
	_imageHeight = 0;
}

DirectionData::~DirectionData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_animationName);
	CC_SAFE_RELEASE_NULL(_timelineInfoList);
	CC_SAFE_RELEASE_NULL(_frameList);
}

bool DirectionData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("directionBit"));
	this->setDirectionBit(json["directionBit"].GetInt());
	CC_ASSERT(json.HasMember("autoGeneration"));
	this->setAutoGeneration(json["autoGeneration"].GetBool());
	CC_ASSERT(json.HasMember("yFlip"));
	this->setYFlip(json["yFlip"].GetBool());
	CC_ASSERT(json.HasMember("resourceInfoId"));
	this->setResourceInfoId(json["resourceInfoId"].GetInt());
	CC_ASSERT(json.HasMember("animationName"));
	this->setAnimationName(cocos2d::__String::create(json["animationName"].GetString()));
	CC_ASSERT(json.HasMember("timelineInfoList"));
	auto timelineInfoList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["timelineInfoList"].Size(); i++) {
		auto timelineInfoData = TimelineInfoData::create(json["timelineInfoList"][i]);
		if (timelineInfoData == nullptr) {
			CC_ASSERT(0);
			return false;
		}
#if defined(AGTK_DEBUG)
//		CC_ASSERT(timelineInfoList->objectForKey(timelineInfoData->getId()) == nullptr);
#endif
		timelineInfoList->setObject(timelineInfoData, timelineInfoData->getId());
	}
	this->setTimelineInfoList(timelineInfoList);
	CC_ASSERT(json.HasMember("frameList"));
	auto frameList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["frameList"].Size(); i++) {
		auto frameData = FrameData::create(json["frameList"][i]);
		frameList->setObject(frameData, frameData->getId());
	}
	this->setFrameList(frameList);
	CC_ASSERT(json.HasMember("imageWidth"));
	this->setImageWidth(json["imageWidth"].GetInt());
	CC_ASSERT(json.HasMember("imageHeight"));
	this->setImageHeight(json["imageHeight"].GetInt());
	return true;
}

bool DirectionData::init(AnimationData * animationData)
{
	// IDは1で固定
	this->setId(1);

	auto frameList = cocos2d::__Dictionary::create();

	cocos2d::DictElement *el = nullptr;
	auto animationFrameList = animationData->getFrameList();
	CC_ASSERT(animationFrameList);

	CCDICT_FOREACH(animationFrameList, el) {
#if defined(STATIC_DOWN_CAST)
		auto frameData = static_cast<FrameData *>(el->getObject());
#else
		auto frameData = dynamic_cast<FrameData *>(el->getObject());
#endif
		frameList->setObject(frameData, frameData->getId());
	}
	this->setFrameList(frameList);

	// リソースIDを設定
	auto resourseInfoData = animationData->getResourceInfoData();
	CC_ASSERT(resourseInfoData);

	this->setResourceInfoId(resourseInfoData->getId());

	return true;
}

const char *DirectionData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *DirectionData::getAnimationName()
{
	CC_ASSERT(_animationName);
	return _animationName->getCString();
}

TimelineInfoData *DirectionData::getTimelineInfoData(int id)
{
	CC_ASSERT(_timelineInfoList);
	return dynamic_cast<TimelineInfoData *>(this->getTimelineInfoList()->objectForKey(id));
}

#if defined(AGTK_DEBUG)
void DirectionData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("directionBit:%d", this->getDirectionBit());
	CCLOG("autoGeneration:%d", this->getAutoGeneration());
	CCLOG("yFlip:%d", this->getYFlip());
	CCLOG("resourceInfoId", this->getResourceInfoId());
	CCLOG("animationName:%s", this->getAnimationName());
	cocos2d::DictElement *el = nullptr;
	auto timelineInfoList = this->getTimelineInfoList();
	CCDICT_FOREACH(timelineInfoList, el) {
		auto timelineInfoData = dynamic_cast<TimelineInfoData *>(el->getObject());
		timelineInfoData->dump();
	}
	el = nullptr;
	auto frameList = this->getFrameList();
	CCDICT_FOREACH(frameList, el) {
		auto frameData = dynamic_cast<FrameData *>(el->getObject());
		frameData->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------

/**
 * @clas MotionData
 */
MotionData::MotionData()
{
	_name = nullptr;
	_directionList = nullptr;
}

MotionData::~MotionData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_directionList);
}

bool MotionData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("dispFrameCount300"));
	this->setDispFrameCount300(json["dispFrameCount300"].GetInt());
	CC_ASSERT(json.HasMember("loopCount"));
	this->setLoopCount(json["loopCount"].GetInt());
	CC_ASSERT(json.HasMember("infiniteLoop"));
	this->setInfiniteLoop(json["infiniteLoop"].GetBool());
	CC_ASSERT(json.HasMember("reversePlay"));
	this->setReversePlay(json["reversePlay"].GetBool());
	CC_ASSERT(json.HasMember("directionList"));
	auto directionList = cocos2d::__Dictionary::create();
	for (rapidjson::SizeType i = 0; i < json["directionList"].Size(); i++) {
		auto directionData = DirectionData::create(json["directionList"][i]);
#if defined(AGTK_DEBUG)
		CC_ASSERT(directionList->objectForKey(directionData->getId()) == nullptr);
#endif
		directionList->setObject(directionData, directionData->getId());
	}
	this->setDirectionList(directionList);
	return true;
}

bool MotionData::init(AnimationData * animationData)
{
	this->setId(1);
	this->setName(cocos2d::__String::create(animationData->getName()));
	this->setDispFrameCount300(animationData->getFrameCount300());
	this->setLoopCount(animationData->getLoopCount());
	this->setInfiniteLoop(animationData->getInfiniteLoop());
	this->setReversePlay(animationData->getReversePlay());

	// 方向データを偽装する
	auto directionList = cocos2d::__Dictionary::create();
	auto directionData = DirectionData::create(animationData);
	directionList->setObject(directionData, directionData->getId());
	this->setDirectionList(directionList);

	return true;
}

const char *MotionData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

agtk::data::DirectionData *MotionData::getDirectionData(int id)
{
	CC_ASSERT(_directionList);
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<DirectionData *>(this->getDirectionList()->objectForKey(id));
#else
	return dynamic_cast<DirectionData *>(this->getDirectionList()->objectForKey(id));
#endif
}

agtk::data::DirectionData *MotionData::getDirectionDataByDirectionBit(int directionBit, int directionBitOld)
{
	agtk::data::DirectionData *directDataFirst = nullptr;
	cocos2d::DictElement *el = nullptr;
	auto directionList = this->getDirectionList();
	//表示方向指定がある
	{
		CCDICT_FOREACH(directionList, el) {
// ATGK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
			auto data = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
#endif
			if (data->getAutoGeneration()) {//自動生成。
				continue;
			}
			if (data->getDirectionBit() & directionBit) {
				return data;
			}
			if (directDataFirst == nullptr) {
				directDataFirst = data;
			}
		}
	}
	//自動生成
	{
		CCDICT_FOREACH(directionList, el) {
// ATGK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
			auto data = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
#endif
			if (data->getAutoGeneration()) {
				return data;
			}
		}
	}

	//近い方向へ
	std::function<agtk::data::DirectionData *(int)> getNearDirection = [&](int bit)->agtk::data::DirectionData* {
		cocos2d::DictElement *el;
		CCDICT_FOREACH(directionList, el) {
// ATGK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
			auto data = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
#endif
			if (data->getAutoGeneration()) {//自動生成。
				continue;
			}
			if (data->getDirectionBit() & bit) {
				return data;
			}
		}
		return nullptr;
	};

	if (directionBit & (1 << 1)) {//左下
		if (directionBitOld & ((1 << 4) | (1 << 2))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//左(4)
		data = getNearDirection(1 << 4);
		if (data) return data;
		//下(2)
		data = getNearDirection(1 << 2);
		if (data) return data;

	}
	if (directionBit & (1 << 2)) {//下
		if (directionBitOld & ((1 << 1) | (1 << 3))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//左下(1)
		data = getNearDirection(1 << 1);
		if (data) return data;
		//右下(3)
		data = getNearDirection(1 << 3);
		if (data) return data;
	}
	if (directionBit & (1 << 3)) {//右下
		if (directionBitOld & ((1 << 2) | (1 << 6))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//下(2)
		data = getNearDirection(1 << 2);
		if (data) return data;
		//右(6)
		data = getNearDirection(1 << 6);
		if (data) return data;
	}
	if (directionBit & (1 << 4)) {//左
		if (directionBitOld & ((1 << 7) | (1 << 1))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//左上(7)
		data = getNearDirection(1 << 7);
		if (data) return data;
		//左下(1)
		data = getNearDirection(1 << 1);
		if (data) return data;
	}
	if (directionBit & (1 << 6)) {//右
		if (directionBitOld & ((1 << 3) | (1 << 9))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//右下(3)
		data = getNearDirection(1 << 3);
		if (data) return data;
		//右上(9)
		data = getNearDirection(1 << 9);
		if (data) return data;
	}
	if (directionBit & (1 << 7)) {//左上
		if (directionBitOld & ((1 << 8) | (1 << 4))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//上(8)
		data = getNearDirection(1 << 8);
		if (data) return data;
		//左(4)
		data = getNearDirection(1 << 4);
		if (data) return data;
	}
	if (directionBit & (1 << 8)) {//上
		if (directionBitOld & ((1 << 7) | (1 << 9))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//左上(7)
		data = getNearDirection(1 << 7);
		if (data) return data;
		//右上(9)
		data = getNearDirection(1 << 9);
		if (data) return data;
	}
	if (directionBit & (1 << 9)) {//右上
		if (directionBitOld & ((1 << 6) | (1 << 8))) {
			goto lSkip;
		}
		agtk::data::DirectionData *data = nullptr;
		//右(6)
		data = getNearDirection(1 << 6);
		if (data) return data;
		//上(8)
		data = getNearDirection(1 << 8);
		if (data) return data;
	}

lSkip:;
	//前回方向
	if (directionBitOld > 0) {
		auto directionList = this->getDirectionList();
		CCDICT_FOREACH(directionList, el) {
// ATGK-NX
#ifdef STATIC_DOWN_CAST
			auto data = static_cast<agtk::data::DirectionData *>(el->getObject());
#else
			auto data = dynamic_cast<agtk::data::DirectionData *>(el->getObject());
#endif
			if (data->getDirectionBit() & directionBitOld) {
				return data;
			}
		}
	}
	//※なければ始めのDirectDataにする。
	return directDataFirst;
}

#if defined(AGTK_DEBUG)
void MotionData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("dispFrameCount300:%d", this->getDispFrameCount300());
	CCLOG("loopCount:%d", this->getLoopCount());
	CCLOG("infiniteLoop:%d", this->getInfiniteLoop());
	CCLOG("reversePlay:%d", this->getReversePlay());
	auto keys = this->getDirectionList()->allKeys();
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(keys, ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		auto directionData = dynamic_cast<DirectionData *>(this->getDirectionList()->objectForKey(id->getValue()));
		directionData->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------

/**
 * @class ResourceInfo
 */
bool ResourceInfoData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("image"));
	this->setImage(json["image"].GetBool());
	CC_ASSERT(json.HasMember("imageId"));
	this->setImageId(json["imageId"].GetInt());
	_imageIdList.clear();
	_resourceSetIdList.clear();
	if (json.HasMember("resourceSetIdImageIdList")) {
		const auto &list = json["resourceSetIdImageIdList"];
		int itemCount = list.Size() / 2;
		for (int i = 0; i < itemCount; i++) {
			_resourceSetIdList.push_back(list[i * 2 + 0].GetInt());
			_imageIdList.push_back(list[i * 2 + 1].GetInt());
		}
	}
	else {
		_imageIdList.push_back(_imageId);
		_resourceSetIdList.push_back(1);
	}
	CC_ASSERT(json.HasMember("hdivCount"));
	this->setHDivCount(json["hdivCount"].GetInt());
	CC_ASSERT(json.HasMember("vdivCount"));
	this->setVDivCount(json["vdivCount"].GetInt());
	return true;
}

bool ResourceInfoData::compareImage(ResourceInfoData *data)
{
	//戻り値： falseは選択するリソースが同じ、trueは選択するリソースが違う
	if (data == nullptr) {
		return true;
	}
	if (this->getImage() != data->getImage()) {
		return true;
	}
	if (this->_imageIdList != data->_imageIdList) {
		return true;
	}
	if (this->_resourceSetIdList != data->_resourceSetIdList) {
		return true;
	}
	if (this->getImage() && (this->getImage() == data->getImage())) {
		return false;
	}
	if (this->getImageId() == data->getImageId()) {
		return false;
	}
	return true;
}

#if defined(AGTK_DEBUG)
void ResourceInfoData::dump()
{
	CCLOG("ResourceInfo ----");
	CCLOG("id:%d", this->getId());
	CCLOG("image:%d", this->getImage());
	CCLOG("imageId:%d", this->getImageId());
	if (_image && (_resourceSetIdList.size() != 1 || _resourceSetIdList[0] != 1)) {
		std::string s;
		char buf[16];
		for (int i = 0; i < (int)_imageIdList.size(); i++) {
			if (i > 0) {
				s += ",";
			}
			snprintf(buf, sizeof(buf), "%d", _imageIdList[i]);
			s += buf;
		}
		CCLOG("imageIdList:%s", s.c_str());
		s.clear();
		for (int i = 0; i < (int)_resourceSetIdList.size(); i++) {
			if (i > 0) {
				s += ",";
			}
			snprintf(buf, sizeof(buf), "%d", _resourceSetIdList[i]);
			s += buf;
		}
		CCLOG("resourceSetIdList:%s", s.c_str());
	}
	CCLOG("hdivCount:%d", this->getHDivCount());
	CCLOG("vdivCount:%d", this->getVDivCount());
}
#endif

int ResourceInfoData::getImageIdByResourceSetId(int resourceSetId)
{
	int itemCount = _resourceSetIdList.size();
	for (int i = 0; i < itemCount; i++) {
		if (_resourceSetIdList[i] == resourceSetId) {
			return _imageIdList[i];
		}
	}
	return kInvalidImageId;
}

//-------------------------------------------------------------------------------------------------------------------

/**
 * @class AnimeParticleData
 */
AnimeParticleData::AnimeParticleData()
{
	_description = nullptr;
}

AnimeParticleData::~AnimeParticleData()
{
	CC_SAFE_RELEASE_NULL(_description);
}

bool AnimeParticleData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("description"));
	this->setDescription(cocos2d::__String::create(json["description"].GetString()));
	CC_ASSERT(json.HasMember("playSe"));
	this->setPlaySe(json["playSe"].GetBool());
	CC_ASSERT(json.HasMember("playSeId"));
	this->setPlaySeId(json["playSeId"].GetInt());
	CC_ASSERT(json.HasMember("playVoice"));
	this->setPlayVoice(json["playVoice"].GetBool());
	CC_ASSERT(json.HasMember("playVoiceId"));
	this->setPlayVoiceId(json["playVoiceId"].GetInt());
	CC_ASSERT(json.HasMember("duration300"));
	this->setDuration300(json["duration300"].GetInt());
	CC_ASSERT(json.HasMember("loop"));
	this->setLoop(json["loop"].GetBool());
	CC_ASSERT(json.HasMember("emitVolume"));
	this->setEmitVolume(json["emitVolume"].GetDouble());
	CC_ASSERT(json.HasMember("instantEmit"));
	this->setInstantEmit(json["instantEmit"].GetBool());
	CC_ASSERT(json.HasMember("previousEmit"));
	this->setPreviousEmit(json["previousEmit"].GetBool());
	CC_ASSERT(json.HasMember("lifetime300"));
	this->setLifetime300(json["lifetime300"].GetInt());
	CC_ASSERT(json.HasMember("lifetimeDispersion"));
	this->setLifetimeDispersion(json["lifetimeDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("direction"));
	this->setDirection(json["direction"].GetDouble());
	CC_ASSERT(json.HasMember("directionDispersion"));
	this->setDirectionDispersion(json["directionDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("disableAntiAlias"));
	this->setDisableAntiAlias(json["disableAntiAlias"].GetBool());
	CC_ASSERT(json.HasMember("speed"));
	this->setSpeed(json["speed"].GetDouble());
	CC_ASSERT(json.HasMember("speedDispersion"));
	this->setSpeedDispersion(json["speedDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("startSize"));
	this->setStartSize(json["startSize"].GetDouble());
	CC_ASSERT(json.HasMember("startSizeDispersion"));
	this->setStartSizeDispersion(json["startSizeDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("endSize"));
	this->setEndSize(json["endSize"].GetDouble());
	CC_ASSERT(json.HasMember("endSizeDispersion"));
	this->setEndSizeDispersion(json["endSizeDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("startRotation"));
	this->setStartRotation(json["startRotation"].GetDouble());
	CC_ASSERT(json.HasMember("startRotationDispersion"));
	this->setStartRotationDispersion(json["startRotationDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("endRotation"));
	this->setEndRotation(json["endRotation"].GetDouble());
	CC_ASSERT(json.HasMember("endRotationDispersion"));
	this->setEndRotationDispersion(json["endRotationDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("endRotationIsRelative"));
	this->setEndRotationIsRelative(json["endRotationIsRelative"].GetBool());
	CC_ASSERT(json.HasMember("addMode"));
	this->setAddMode(json["addMode"].GetBool());
	CC_ASSERT(json.HasMember("emitterMode"));
	this->setEmitterMode(json["emitterMode"].GetInt());
	CC_ASSERT(json.HasMember("rotations"));
	this->setRotations(json["rotations"].GetDouble());
	CC_ASSERT(json.HasMember("rotationsDispersion"));
	this->setRotationsDispersion(json["rotationsDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("startRotationWidth"));
	this->setStartRotationWidth(json["startRotationWidth"].GetDouble());
	CC_ASSERT(json.HasMember("startRotationWidthDispersion"));
	this->setStartRotationWidthDispersion(json["startRotationWidthDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("endRotationWidth"));
	this->setEndRotationWidth(json["endRotationWidth"].GetDouble());
	CC_ASSERT(json.HasMember("endRotationWidthDispersion"));
	this->setEndRotationWidthDispersion(json["endRotationWidthDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("normalAccel"));
	this->setNormalAccel(json["normalAccel"].GetDouble());
	CC_ASSERT(json.HasMember("normalAccelDispersion"));
	this->setNormalAccelDispersion(json["normalAccelDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("tangentAccel"));
	this->setTangentAccel(json["tangentAccel"].GetDouble());
	CC_ASSERT(json.HasMember("tangentAccelDispersion"));
	this->setTangentAccelDispersion(json["tangentAccelDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("gravityDirection"));
	this->setGravityDirection(json["gravityDirection"].GetDouble());
	CC_ASSERT(json.HasMember("gravity"));
	this->setGravity(json["gravity"].GetDouble());
	CC_ASSERT(json.HasMember("x"));
	this->setX(json["x"].GetDouble());
	CC_ASSERT(json.HasMember("xDispersion"));
	this->setXDispersion(json["xDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("y"));
	this->setY(json["y"].GetDouble());
	CC_ASSERT(json.HasMember("yDispersion"));
	this->setYDispersion(json["yDispersion"].GetDouble());
	CC_ASSERT(json.HasMember("startAlpha"));
	this->setStartAlpha(json["startAlpha"].GetInt());
	CC_ASSERT(json.HasMember("startAlphaDispersion"));
	this->setStartAlphaDispersion(json["startAlphaDispersion"].GetInt());
	CC_ASSERT(json.HasMember("startR"));
	this->setStartR(json["startR"].GetInt());
	CC_ASSERT(json.HasMember("startRDispersion"));
	this->setStartRDispersion(json["startRDispersion"].GetInt());
	CC_ASSERT(json.HasMember("startG"));
	this->setStartG(json["startG"].GetInt());
	CC_ASSERT(json.HasMember("startGDispersion"));
	this->setStartGDispersion(json["startGDispersion"].GetInt());
	CC_ASSERT(json.HasMember("startB"));
	this->setStartB(json["startB"].GetInt());
	CC_ASSERT(json.HasMember("startBDispersion"));
	this->setStartBDispersion(json["startBDispersion"].GetInt());

	if (json.HasMember("middleAlpha")) {
		this->setMiddleAlpha(json["middleAlpha"].GetInt());
	}
	else {
		this->setMiddleAlpha(_startAlpha);
	}
	if (json.HasMember("middleAlphaDispersion")) {
		this->setMiddleAlphaDispersion(json["middleAlphaDispersion"].GetInt());
	}
	else {
		this->setMiddleAlphaDispersion(_startAlphaDispersion);
	}
	if (json.HasMember("middleR")) {
		this->setMiddleR(json["middleR"].GetInt());
	}
	else {
		this->setMiddleR(_startR);
	}
	if (json.HasMember("middleRDispersion")) {
		this->setMiddleRDispersion(json["middleRDispersion"].GetInt());
	}
	else {
		this->setMiddleRDispersion(_startRDispersion);
	}
	if (json.HasMember("middleG")) {
		this->setMiddleG(json["middleG"].GetInt());
	}
	else {
		this->setMiddleG(_startG);
	}
	if (json.HasMember("middleGDispersion")) {
		this->setMiddleGDispersion(json["middleGDispersion"].GetInt());
	}
	else {
		this->setMiddleGDispersion(_startGDispersion);
	}
	if (json.HasMember("middleB")) {
		this->setMiddleB(json["middleB"].GetInt());
	}
	else {
		this->setMiddleB(_startB);
	}
	if (json.HasMember("middleBDispersion")) {
		this->setMiddleBDispersion(json["middleBDispersion"].GetInt());
	}
	else {
		this->setMiddleBDispersion(_startBDispersion);
	}
	if (json.HasMember("middlePercent")) {
		this->setMiddlePercent(json["middlePercent"].GetInt());
	} else {
		this->setMiddlePercent(0);
	}

	CC_ASSERT(json.HasMember("endAlpha"));
	this->setEndAlpha(json["endAlpha"].GetInt());
	CC_ASSERT(json.HasMember("endAlphaDispersion"));
	this->setEndAlphaDispersion(json["endAlphaDispersion"].GetInt());
	CC_ASSERT(json.HasMember("endR"));
	this->setEndR(json["endR"].GetInt());
	CC_ASSERT(json.HasMember("endRDispersion"));
	this->setEndRDispersion(json["endRDispersion"].GetInt());
	CC_ASSERT(json.HasMember("endG"));
	this->setEndG(json["endG"].GetInt());
	CC_ASSERT(json.HasMember("endGDispersion"));
	this->setEndGDispersion(json["endGDispersion"].GetInt());
	CC_ASSERT(json.HasMember("endB"));
	this->setEndB(json["endB"].GetInt());
	CC_ASSERT(json.HasMember("endBDispersion"));
	this->setEndBDispersion(json["endBDispersion"].GetInt());
	CC_ASSERT(json.HasMember("check"));
	this->setCheck(json["check"].GetBool());
	CC_ASSERT(json.HasMember("touchDisappear"));
	this->setTouchDisappear(json["touchDisappear"].GetBool());
	CC_ASSERT(json.HasMember("disappearCount"));
	this->setDisappearCount(json["disappearCount"].GetInt());
	CC_ASSERT(json.HasMember("touchBound"));
	this->setTouchBound(json["touchBound"].GetBool());
	CC_ASSERT(json.HasMember("repulsion"));
	this->setRepulsion(json["repulsion"].GetDouble());
	CC_ASSERT(json.HasMember("templateId"));
	this->setTemplateId(json["templateId"].GetInt());
	return true;
}

const char *AnimeParticleData::getDescription()
{
	CC_ASSERT(_description);
	return _description->getCString();
}

#if defined(AGTK_DEBUG)
void AnimeParticleData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("description:%s", this->getDescription());
	CCLOG("playSe:%d", this->getPlaySe());
	CCLOG("playSeId:%d", this->getPlaySeId());
	CCLOG("playVoice:%d", this->getPlayVoice());
	CCLOG("playVoiceId:%d", this->getPlayVoiceId());
	CCLOG("duration300:%d", this->getDuration300());
	CCLOG("loop:%d", this->getLoop());
	CCLOG("emitVolume:%f", this->getEmitVolume());
	CCLOG("instantEmit:%d", this->getInstantEmit());
	CCLOG("previousEmit:%d", this->getPreviousEmit());
	CCLOG("lifetime300:%d", this->getLifetime300());
	CCLOG("lifetimeDispersion:%f", this->getLifetimeDispersion());
	CCLOG("direction:%f", this->getDirection());
	CCLOG("directionDispersion:%f", this->getDirectionDispersion());
	CCLOG("disableAntiAlias:%f", this->getDisableAntiAlias());
	CCLOG("speed:%f", this->getSpeed());
	CCLOG("speedDispersion:%f", this->getSpeedDispersion());
	CCLOG("startSize:%f", this->getStartSize());
	CCLOG("startSizeDispersion:%f", this->getStartSizeDispersion());
	CCLOG("endSize:%f", this->getEndSize());
	CCLOG("endSizeDispersion:%f", this->getEndSizeDispersion());
	CCLOG("startRotation:%f", this->getStartRotation());
	CCLOG("startRotationDispersion:%f", this->getStartRotationDispersion());
	CCLOG("endRotation:%f", this->getEndRotation());
	CCLOG("endRotationDispersion:%f", this->getEndRotationDispersion());
	CCLOG("endRotationIsRelative:%f", this->getEndRotationIsRelative());
	CCLOG("addMode:%f", this->getAddMode());
	CCLOG("emitterMode:%f", this->getEmitterMode());
	CCLOG("rotations:%f", this->getRotations());
	CCLOG("rotationsDispersion:%f", this->getRotationsDispersion());
	CCLOG("startRotationWidth:%f", this->getStartRotationWidth());
	CCLOG("startRotationWidthDispersion:%f", this->getStartRotationWidthDispersion());
	CCLOG("endRotationWidth:%f", this->getEndRotationWidth());
	CCLOG("endRotationWidthDispersion:%f", this->getEndRotationWidthDispersion());
	CCLOG("normalAccel:%f", this->getNormalAccel());
	CCLOG("normalAccelDispersion:%f", this->getNormalAccelDispersion());
	CCLOG("tangentAccel:%f", this->getTangentAccel());
	CCLOG("tangentAccelDispersion:%f", this->getTangentAccelDispersion());
	CCLOG("gravityDirection:%f", this->getGravityDirection());
	CCLOG("gravity:%f", this->getGravity());
	CCLOG("x:%f", this->getX());
	CCLOG("xDispersion:%f", this->getXDispersion());
	CCLOG("y:%f", this->getY());
	CCLOG("yDispersion:%f", this->getYDispersion());
	CCLOG("startAlpha:%d", this->getStartAlpha());
	CCLOG("startAlphaDispersion:%d", this->getStartAlphaDispersion());
	CCLOG("startR:%d", this->getStartR());
	CCLOG("startRDispersion:%d", this->getStartRDispersion());
	CCLOG("startG:%d", this->getStartG());
	CCLOG("startGDispersion:%d", this->getStartGDispersion());
	CCLOG("startB:%d", this->getStartB());
	CCLOG("startBDispersion:%d", this->getStartBDispersion());

	CCLOG("middleAlpha:%d", this->getMiddleAlpha());
	CCLOG("middleAlphaDispersion:%d", this->getMiddleAlphaDispersion());
	CCLOG("middleR:%d", this->getMiddleR());
	CCLOG("middleRDispersion:%d", this->getMiddleRDispersion());
	CCLOG("middleG:%d", this->getMiddleG());
	CCLOG("middleGDispersion:%d", this->getMiddleGDispersion());
	CCLOG("middleB:%d", this->getMiddleB());
	CCLOG("middleBDispersion:%d", this->getMiddleBDispersion());
	CCLOG("middlePercent:%d", this->getMiddlePercent());

	CCLOG("endAlpha:%d", this->getEndAlpha());
	CCLOG("endAlphaDispersion:%d", this->getEndAlphaDispersion());
	CCLOG("endR:%d", this->getEndR());
	CCLOG("endRDispersion:%d", this->getEndRDispersion());
	CCLOG("endG:%d", this->getEndG());
	CCLOG("endGDispersion:%d", this->getEndGDispersion());
	CCLOG("endB:%d", this->getEndB());
	CCLOG("endBDispersion:%d", this->getEndBDispersion());
	CCLOG("check:%d", this->getCheck());
	CCLOG("touchDisappear:%d", this->getTouchDisappear());
	CCLOG("disappearCount:%d", this->getDisappearCount());
	CCLOG("touchBound:%d", this->getTouchBound());
	CCLOG("repulsion:%f", this->getRepulsion());
	CCLOG("templateId:%d", this->getTemplateId());
}
#endif

//-------------------------------------------------------------------------------------------------------------------

/**
* @class AnimationData
*/
AnimationData::AnimationData()
{
	_name = nullptr;
	_resourceInfoList = nullptr;
	_motionList = nullptr;
	_frameList = nullptr;
	_particleList = nullptr;
	_memo = nullptr;
	_folder = false;
	_children = nullptr;

	_originType = kOriginLeftUp;
	_originX = 0;
	_originY = 0;

	_frameCount300 = 0;
	_infiniteLoop = false;
	_loopCount = 0;
	_reversePlay = false;
	_animationName = nullptr;
}

AnimationData::~AnimationData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_resourceInfoList);
	CC_SAFE_RELEASE_NULL(_motionList);
	CC_SAFE_RELEASE_NULL(_frameList);
	CC_SAFE_RELEASE_NULL(_particleList);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_children);
	CC_SAFE_RELEASE_NULL(_animationName);
}

bool AnimationData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("id"));
	this->setId(json["id"].GetInt());
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("folder"));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = AnimationData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	CC_ASSERT(json.HasMember("type"));
	this->setType((EnumAnimType)json["type"].GetInt());
	if (this->getType() == kMotion){
		if (json.HasMember("originType")) {
			this->setOriginType((EnumOriginType)json["originType"].GetInt());
		}
		if (json.HasMember("originX")) {
			this->setOriginX(json["originX"].GetInt());
		}
		if (json.HasMember("originY")) {
			this->setOriginY(json["originY"].GetInt());
		}
		CC_ASSERT(json.HasMember("resourceInfoList"));
		auto resourceInfoList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < json["resourceInfoList"].Size(); i++) {
			auto resourceInfo = ResourceInfoData::create(json["resourceInfoList"][i]);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
			resourceInfoList->setObject(resourceInfo, resourceInfo->getId());
#endif
		}
		this->setResourceInfoList(resourceInfoList);
		_resourceSetIdList.clear();
		_resourceSetNameList.clear();
		if (json.HasMember("resourceSetIdNameList")) {
			auto &list = json["resourceSetIdNameList"];
			int itemCount = list.Size() / 2;
			for (int i = 0; i < itemCount; i++) {
				_resourceSetIdList.push_back(list[i * 2 + 0].GetInt());
				_resourceSetNameList.push_back(list[i * 2 + 1].GetString());
			}
		}
		else {
			_resourceSetIdList.push_back(1);
			_resourceSetNameList.push_back("1");
		}

		if (json.HasMember("motionList")){
			auto motionList = cocos2d::__Dictionary::create();
			for (rapidjson::SizeType i = 0; i < json["motionList"].Size(); i++) {
				auto motionData = MotionData::create(json["motionList"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(motionList->objectForKey(motionData->getId()) == nullptr);
#endif
				motionList->setObject(motionData, motionData->getId());
			}
			this->setMotionList(motionList);
		} else {
			CC_ASSERT(json.HasMember("motionList"));
			auto motionList = cocos2d::__Dictionary::create();
			for (rapidjson::SizeType i = 0; i < json["motionList"].Size(); i++) {
				auto motionData = MotionData::create(json["motionList"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(motionList->objectForKey(motionData->getId()) == nullptr);
#endif
				motionList->setObject(motionData, motionData->getId());
			}
			this->setMotionList(motionList);
		}
	} else if (this->getType() == kEffect){
		if (json.HasMember("originType")) {
			this->setOriginType((EnumOriginType)json["originType"].GetInt());
		}
		if (json.HasMember("originX")) {
			this->setOriginX(json["originX"].GetInt());
		}
		if (json.HasMember("originY")) {
			this->setOriginY(json["originY"].GetInt());
		}
		if (json.HasMember("frameCount300")) {
			this->setFrameCount300(json["frameCount300"].GetInt());
		}
		if (json.HasMember("infiniteLoop")) {
			this->setInfiniteLoop(json["infiniteLoop"].GetBool());
		}
		if (json.HasMember("loopCount")) {
			this->setLoopCount(json["loopCount"].GetInt());
		}
		if (json.HasMember("reversePlay")) {
			this->setReversePlay(json["reversePlay"].GetBool());
		}

		CC_ASSERT(json.HasMember("frameList"));
		auto frameList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < json["frameList"].Size(); i++) {
			auto frameData = FrameData::create(json["frameList"][i]);
			frameList->setObject(frameData, frameData->getId());
		}
		this->setFrameList(frameList);

		CC_ASSERT(json.HasMember("resourceInfo"));
		auto resourceInfoList = cocos2d::__Dictionary::create();
		auto resourceInfo = ResourceInfoData::create(json["resourceInfo"]);
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
		resourceInfoList->setObject(resourceInfo, resourceInfo->getId());
#endif
		this->setResourceInfoList(resourceInfoList);
		_resourceSetIdList.clear();
		_resourceSetNameList.clear();
		if (json.HasMember("resourceSetIdNameList")) {
			auto &list = json["resourceSetIdNameList"];
			int itemCount = list.Size() / 2;
			for (int i = 0; i < itemCount; i++) {
				_resourceSetIdList.push_back(list[i * 2 + 0].GetInt());
				_resourceSetNameList.push_back(list[i * 2 + 1].GetString());
			}
		}
		else {
			_resourceSetIdList.push_back(1);
			_resourceSetNameList.push_back("1");
		}

		// モーションデータを偽装する
		auto motionList = cocos2d::__Dictionary::create();
		auto motionData = MotionData::create(this);
		motionList->setObject(motionData, motionData->getId());

		this->setMotionList(motionList);

		if (json.HasMember("animationName")) {
			this->setAnimationName(cocos2d::__String::create(json["animationName"].GetString()));
		}
	} else if (this->getType() == kParticle){
		CC_ASSERT(json.HasMember("particleList"));
		auto particleList = cocos2d::__Dictionary::create();
		for (rapidjson::SizeType i = 0; i < json["particleList"].Size(); i++) {
			auto particleData = AnimeParticleData::create(json["particleList"][i]);
#if defined(AGTK_DEBUG)
			CC_ASSERT(particleList->objectForKey(particleData->getId()) == nullptr);
#endif
			particleList->setObject(particleData, particleData->getId());
		}
		this->setParticleList(particleList);
	}
	CC_ASSERT(json.HasMember("memo"));
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	return true;
}

const char *AnimationData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *AnimationData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

const char *AnimationData::getAnimationName()
{
	CC_ASSERT(_animationName);
	return _animationName->getCString();
}

ResourceInfoData *AnimationData::getResourceInfoData(int id)
{
	CC_ASSERT(_resourceInfoList);
	if (id < 0) {
		auto keys = this->getResourceInfoList()->allKeys();
		if (keys == nullptr || keys->count() == 0) {
			return nullptr;
		}
// ATGK-NX
#ifdef STATIC_DOWN_CAST
		auto resourceInfoId = static_cast<cocos2d::Integer *>(keys->getObjectAtIndex(0));
#else
		auto resourceInfoId = dynamic_cast<cocos2d::Integer *>(keys->getObjectAtIndex(0));
#endif
		return dynamic_cast<ResourceInfoData *>(this->getResourceInfoList()->objectForKey(resourceInfoId->getValue()));
	}
	return dynamic_cast<ResourceInfoData *>(this->getResourceInfoList()->objectForKey(id));
}

agtk::data::MotionData *AnimationData::getMotionData(int id)
{
	if (this->getFolder()) {
		CC_ASSERT(0);
		return nullptr;
	}
	bool bExist = false;
	auto motionList = this->getMotionList();
	CC_ASSERT(motionList);
	auto keys = motionList->allKeys();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(keys, ref) {
// ATGK-NX
#ifdef STATIC_DOWN_CAST
		auto key = static_cast<cocos2d::Integer *>(ref);
#else
		auto key = dynamic_cast<cocos2d::Integer *>(ref);
#endif
		if (key->getValue() == id) {
			bExist = true;
			break;
		}
	}
	if (bExist == false) {
		return nullptr;
	}
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
	return static_cast<MotionData *>(this->getMotionList()->objectForKey(id));
#else
	return dynamic_cast<MotionData *>(this->getMotionList()->objectForKey(id));
#endif
}

agtk::data::DirectionData *AnimationData::getDirectionData(int motionId, int directionId)
{
	auto motionData = this->getMotionData(motionId);
	return motionData->getDirectionData(directionId);
}

cocos2d::Vec2 AnimationData::calcOriginPosition(cocos2d::Size& size)
{
	cocos2d::Vec2 origin;
	switch (this->getOriginType()) {
	case kOriginLeftUp:
		origin.x = 0.0f;
		origin.y = 0.0f;
		break;
	case kOriginCenter:
		origin.x = -size.width * 0.5f;
		origin.y = size.height * 0.5f;
		break;
	case kOriginFoot:
		origin.x = -size.width * 0.5f;
		origin.y = size.height;
		break;
	case kOriginXy:
		origin.x = -this->getOriginX();
		origin.y = this->getOriginY();
		break;
	default:CC_ASSERT(0);
	}
	return origin;
}

#if defined(AGTK_DEBUG)
void AnimationData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<AnimationData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("type:%d", this->getType());
	if (this->getType() == kMotion){
		CCLOG("originType:%d", this->getOriginType());
		CCLOG("originX:%d", this->getOriginX());
		CCLOG("originY:%d", this->getOriginY());
		auto keys = this->getResourceInfoList()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto resourceInfoData = dynamic_cast<ResourceInfoData *>(this->getResourceInfoList()->objectForKey(id->getValue()));
			resourceInfoData->dump();
		}
		if (_resourceSetIdList.size() != 1 || _resourceSetIdList[0] != 1 || _resourceSetNameList[0] != "1") {
			std::string s;
			char buf[16];
			for (int i = 0; i < (int)_resourceSetIdList.size(); i++) {
				if (i > 0) {
					s += ",";
				}
				snprintf(buf, sizeof(buf), "%d", _resourceSetIdList[i]);
				s += buf;
			}
			CCLOG("resourceSetIdList:%s", s.c_str());
			s.clear();
			for (int i = 0; i < (int)_resourceSetNameList.size(); i++) {
				if (i > 0) {
					s += ",";
				}
				snprintf(buf, sizeof(buf), "%s", _resourceSetNameList[i].c_str());
				s += buf;
			}
			CCLOG("resourceSetNameList:%s", s.c_str());
		}
		keys = this->getMotionList()->allKeys();
		ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto motionData = dynamic_cast<MotionData *>(this->getMotionList()->objectForKey(id->getValue()));
			motionData->dump();
		}
	} else if (this->getType() == kEffect){
		CCLOG("animationName:%s", this->getAnimationName());
	} else if (this->getType() == kParticle){
		auto keys = this->getParticleList()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto particleData = dynamic_cast<AnimeParticleData *>(this->getParticleList()->objectForKey(id->getValue()));
			particleData->dump();
		}
	}
	CCLOG("memo:%s", this->getMemo());
}
#endif

NS_DATA_END
NS_AGTK_END
