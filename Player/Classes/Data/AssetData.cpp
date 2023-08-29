#include "AssetData.h"
#include "Lib/Common.h"
#include "GameManager.h"
#include "json/stringbuffer.h"
#include "json/writer.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

std::string getFullFilename(std::string filename, const std::string &projectPath)
{
#ifdef _WINDOWS
	auto ch = filename[0];
	if (filename.length() >= 2 && (ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z') && filename[1] == ':') {
		return filename;
	}
// #AGTK-NX	
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	if (filename[0] == '/') {
		return filename;
	}
#endif
	return projectPath + filename;
}

#ifdef USE_PREVIEW
std::string join(const std::list<std::string> &strlist, const std::string &delim)
{
	std::string result;
	int len = 0;
	for (auto &str : strlist) {
		len += str.length() + delim.length();
	}
	len -= delim.length();
	result.reserve(len);
	bool first = true;
	for (auto &str : strlist) {
		if (first) {
			first = false;
			result = str;
		} else {
			result += delim;
			result += str;
		}
	}
	return result;
}

#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class FontData
 */
FontData::FontData()
{
	_id = 0;
	_name = nullptr;
	_folder = false;
	_children = nullptr;
	_localeSettings = nullptr;
}

FontData::~FontData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_children);
	CC_SAFE_RELEASE_NULL(_localeSettings);
}

bool FontData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = FontData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	auto localeSettings = cocos2d::__Dictionary::create();
	auto p = FontData::FontSetting::create(json);
	localeSettings->setObject(p, "mainLanguage");
	auto &jo = json["localeSettings"];
	for(auto it = jo.MemberBegin(); it != jo.MemberEnd(); it++){
		auto p = FontData::FontSetting::create(it->value);
#if defined(AGTK_DEBUG)
		CC_ASSERT(strcmp(it->name.GetString(), "mainLanguage") == 0 || localeSettings->objectForKey(it->name.GetString()) == nullptr);
#endif
		localeSettings->setObject(p, it->name.GetString());
	}
	this->setLocaleSettings(localeSettings);

	return true;
}

const char *FontData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

FontData::FontSetting *FontData::getMainFontSetting()
{
	auto projectData = GameManager::getInstance()->getProjectData();
	auto gameInformation = projectData->getGameInformation();
	auto fontSetting = dynamic_cast<FontData::FontSetting *>(_localeSettings->objectForKey(gameInformation->getMainLanguage()));
	if (!fontSetting) {
// #AGTK-NX
#if defined(STATIC_DOWN_CAST) && defined(FORCE_STATIC_DOWN_CAST)
		fontSetting = static_cast<FontData::FontSetting *>(_localeSettings->objectForKey("mainLanguage"));
#else
		fontSetting = dynamic_cast<FontData::FontSetting *>(_localeSettings->objectForKey("mainLanguage"));
#endif
	}
	return fontSetting;
}

const char *FontData::getFontName()
{
	auto fontSetting = getMainFontSetting();
	CC_ASSERT(fontSetting);
	return fontSetting->getFontName()->getCString();
}

const char *FontData::getTTFName()
{
	auto fontSetting = getMainFontSetting();
	CC_ASSERT(fontSetting);
	return fontSetting->getTTFName()->getCString();
}

const char *FontData::getLetterLayout()
{
	auto fontSetting = getMainFontSetting();
	CC_ASSERT(fontSetting);
	return fontSetting->getLetterLayout()->getCString();
}

const char *FontData::getLetterLayoutWithoutLF()
{
	auto fontSetting = getMainFontSetting();
	CC_ASSERT(fontSetting);
	return fontSetting->getLetterLayoutWithoutLF()->getCString();
}

#if defined(AGTK_DEBUG)
void FontData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<FontData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	cocos2d::DictElement *el;
	CCDICT_FOREACH(_localeSettings, el) {
		CCLOG("locale:%s", el->getStrKey());
		auto p = dynamic_cast<FontData::FontSetting *>(el->getObject());
		CCLOG("imageFontFlag:%d", p->getImageFontFlag());
		CCLOG("imageId:%d", p->getImageId());
		CCLOG("fontName:%s", p->getFontName());
		CCLOG("TTFName:%s", p->getTTFName());
		CCLOG("fontSize:%d", p->getFontSize());
		CCLOG("antialiasDisabled:%d", p->getAntialiasDisabled());
		CCLOG("aliasThreshold:%d", p->getAliasThreshold());
		CCLOG("fixedWidth:%d", p->getFixedWidth());
		CCLOG("hankakuWidth:%d", p->getHankakuWidth());
		CCLOG("zenkakuWidth:%d", p->getZenkakuWidth());
		CCLOG("letterLayout:%s", p->getLetterLayout());
		//Additional Data
		CCLOG("horzDivCount:%d", p->getHorzDivCount());
		CCLOG("vertDivCount:%d", p->getVertDivCount());
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
* @class FontSetting
*/
FontData::FontSetting::FontSetting()
{
	_imageFontFlag = false;
	_imageId = 0;
	_fontName = nullptr;
	_ttfName = nullptr;
	_fontSize = 0;
	_antialiasDisabled = false;
	_aliasThreshold = 126;
	_hankakuWidth = 0;
	_zenkakuWidth = 0;
	_letterLayout = nullptr;
	_horzDivCount = 1;
	_vertDivCount = 1;
	_letterLayoutWithoutLF = nullptr;
}

FontData::FontSetting::~FontSetting()
{
	CC_SAFE_RELEASE_NULL(_ttfName);
	CC_SAFE_RELEASE_NULL(_letterLayout);
	CC_SAFE_RELEASE_NULL(_letterLayoutWithoutLF);
}

bool FontData::FontSetting::init(const rapidjson::Value& json)
{
	this->setImageFontFlag(json["imageFontFlag"].GetBool());
	this->setImageId(json["imageId"].GetInt());
	this->setFontName(cocos2d::__String::create(json["fontName"].GetString()));
	this->setTTFName(cocos2d::__String::create(json["ttfName"].GetString()));
	this->setFontSize(json["fontSize"].GetInt());
	if (json.HasMember("antialiasDisabled")) {
		this->setAntialiasDisabled(json["antialiasDisabled"].GetBool());
	}
	if (json.HasMember("aliasThreshold")) {
		this->setAliasThreshold(json["aliasThreshold"].GetInt());
	}
	this->setFixedWidth(json["fixedWidth"].GetBool());
	this->setHankakuWidth(json["hankakuWidth"].GetInt());
	this->setZenkakuWidth(json["zenkakuWidth"].GetInt());
	this->setLetterLayout(cocos2d::__String::create(json["letterLayout"].GetString()));

	//ImageFontFlag: ON
	if (this->getImageFontFlag()) {
		std::string letter = this->getLetterLayout()->getCString();
		int horzDivCount = 1;//横
		int vertDivCount = 1;//縦
		int offset = 0;

		std::vector<std::string> v;
		std::stringstream ss(letter);
		std::string buffer;
		while (std::getline(ss, buffer)) {
			v.push_back(buffer);
		}

		// 縦の分割数を設定
		vertDivCount = v.size();

		std::u32string u32Str;

		for (unsigned int i = 0; i < v.size(); i++) {
			std::string u8Str = v[i];

			if (StringUtils::UTF8ToUTF32(u8Str, u32Str)) {
				// 横の分割数を設定
				if (horzDivCount < static_cast<int>(u32Str.length())) {
					horzDivCount = u32Str.length();
				}
			}
		}

		this->setVertDivCount(vertDivCount);
		this->setHorzDivCount(horzDivCount);

		//改行を破棄する
		std::string tmpString = json["letterLayout"].GetString();
		size_t c;
		CCLOG("%s", tmpString.c_str());
		while ((c = tmpString.find_first_of("\n")) != std::string::npos) {
			tmpString.erase(c, 1);
		}
		CCLOG("%s", tmpString.c_str());
		this->setLetterLayoutWithoutLF(cocos2d::__String::create(tmpString));

		/*
		while (1) {
			offset = letter.find("\n", offset);
			if (vertDivCount == 1) {
				//一行目から文字数を取得して、テクスチャの横方向の分割数を取得する。
				std::string line = letter.substr(0, offset);
				horzDivCount = GetStringLength(line);
			}
			if (offset == std::string::npos) {
				break;
			}
			offset++;
			vertDivCount++;
		}
		this->setVertDivCount(vertDivCount);
		this->setHorzDivCount(horzDivCount);

		//改行を破棄する
		std::string tmpString = json["letterLayout"].GetString();
		size_t c;
		CCLOG("%s", tmpString.c_str());
		while ((c = tmpString.find_first_of("\n")) != std::string::npos) {
			tmpString.erase(c, 1);
		}
		CCLOG("%s", tmpString.c_str());
		this->setLetterLayoutWithoutLF(cocos2d::__String::create(tmpString));
		*/
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class ImageData
 */
ImageData::ImageData()
{
	_id = 0;
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_horzDivCount = 0;
	_vertDivCount = 0;
	_folder = false;
	_children = nullptr;

	_texWidth = 0;
	_texHeight = 0;
}

ImageData::~ImageData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_children);
}

bool ImageData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = ImageData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::__String::create(projectPath + json["filename"].GetString()));
#endif
	this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));
	this->setHorzDivCount(json["horzDivCount"].GetInt());
	this->setVertDivCount(json["vertDivCount"].GetInt());

	//get image size
// #AGTK-NX #AGTK-WIN
#if defined(USE_BG_PROJECT_LOAD) || defined(USE_PRELOAD_TEX)
	if (ImageMtTask::getInstance()->getEnable()) {
		cocos2d::Image* img = new cocos2d::Image();
		img->initWithImageFileLateSetup(this->getFilename(), this);
	}
	else {
#endif
	cocos2d::Image img;
	img.initWithImageFile(this->getFilename());
	this->setTexWidth(img.getWidth());
	this->setTexHeight(img.getHeight());
// #AGTK-NX #AGTK-WIN
#if defined(USE_BG_PROJECT_LOAD) || defined(USE_PRELOAD_TEX)
	}
#endif

	return true;
}

const char *ImageData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *ImageData::getFilename()
{
	if (_filename == nullptr) {
		return nullptr;
	}
	return _filename->getCString();
}

const char *ImageData::getSrcFilename()
{
	if (_srcFilename == nullptr) {
		return nullptr;
	}
	return _srcFilename->getCString();
}

#if defined(AGTK_DEBUG)
void ImageData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<ImageData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("horzDivCount:%d", this->getHorzDivCount());
	CCLOG("vertDivCount:%d", this->getVertDivCount());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class MovieData
 */
MovieData::MovieData()
{
	_id = 0;
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_volume = 100;
	_folder = false;
	_children = nullptr;
	_width = 1;
	_height = 1;
}

MovieData::~MovieData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_children);
}

bool MovieData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = MovieData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::__String::create(projectPath + json["filename"].GetString()));
#endif
	this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));
	this->setVolume(json["volume"].GetInt());
	this->setWidth(json["width"].GetInt());
	this->setHeight(json["height"].GetInt());
	return true;
}

const char *MovieData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *MovieData::getFilename()
{
	if (_filename == nullptr) {
		return nullptr;
	}
	return _filename->getCString();
}

const char *MovieData::getSrcFilename()
{
	if (_srcFilename == nullptr) {
		return nullptr;
	}
	return _srcFilename->getCString();
}

#if defined(AGTK_DEBUG)
void MovieData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<MovieData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("volume:%d", this->getVolume());
	CCLOG("width:%d", this->getWidth());
	CCLOG("height:%d", this->getHeight());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
* @class BgmData
*/
BgmData::BgmData()
{
	_id = 0;
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_volume = 100;
	_pan = 0;
	_pitch = 0;
	_loop = false;
	_srcLoop = false;
	_folder = false;
	_children = nullptr;
}

BgmData::~BgmData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_children);
}

bool BgmData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = BgmData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::__String::create(projectPath + json["filename"].GetString()));
#endif
	this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));
	this->setVolume(json["volume"].GetInt());
	this->setPan(json["pan"].GetInt());
	this->setPitch(json["pitch"].GetInt());
	this->setLoop(json["loop"].GetBool());
	this->setSrcLoop(json["srcLoop"].GetBool());
	return true;
}

const char *BgmData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *BgmData::getFilename()
{
	if (_filename == nullptr) {
		return nullptr;
	}
	return _filename->getCString();
}

const char *BgmData::getSrcFilename()
{
	if (_srcFilename == nullptr) {
		return nullptr;
	}
	return _srcFilename->getCString();
}

float BgmData::getVolumeNormalize()
{
	return (float)this->getVolume() * 0.01f;
}

#if defined(AGTK_DEBUG)
void BgmData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<BgmData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("volume:%d,%f", this->getVolume(), this->getVolumeNormalize());
	CCLOG("pan:%d", this->getPan());
	CCLOG("pitch:%d", this->getPitch());
	CCLOG("loop:%d", this->getLoop());
	CCLOG("srcLoop:%d", this->getSrcLoop());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class SeData
 */
SeData::SeData()
{
	_id = 0;
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_volume = 100;
	_pan = 0;
	_pitch = 0;
	_loop = false;
	_srcLoop = false;
	_folder = false;
	_children = nullptr;
}

SeData::~SeData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_children);
}

bool SeData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = SeData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::__String::create(projectPath + json["filename"].GetString()));
#endif
	this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));
	this->setVolume(json["volume"].GetInt());
	this->setPan(json["pan"].GetInt());
	this->setPitch(json["pitch"].GetInt());
	this->setLoop(json["loop"].GetBool());
	this->setSrcLoop(json["srcLoop"].GetBool());
	return true;
}

const char *SeData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *SeData::getFilename()
{
	CC_ASSERT(_filename);
	return _filename->getCString();
}

const char *SeData::getSrcFilename()
{
	CC_ASSERT(_srcFilename);
	return _srcFilename->getCString();
}

float SeData::getVolumeNormalize()
{
	return (float)this->getVolume() * 0.01f;
}

#if defined(AGTK_DEBUG)
void SeData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<SeData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("volume:%d,%f", this->getVolume(), this->getVolumeNormalize());
	CCLOG("pan:%d", this->getPan());
	CCLOG("pitch:%d", this->getPitch());
	CCLOG("loop:%d", this->getLoop());
	CCLOG("srcLoop:%d", this->getSrcLoop());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class VoiceData
 */
VoiceData::VoiceData()
{
	_id = 0;
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_volume = 100;
	_pan = 0;
	_pitch = 0;
	_loop = false;
	_srcLoop = false;
	_folder = false;
	_children = nullptr;
}

VoiceData::~VoiceData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_children);
}

bool VoiceData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = VoiceData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::__String::create(projectPath + json["filename"].GetString()));
#endif
	this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));
	this->setVolume(json["volume"].GetInt());
	this->setPan(json["pan"].GetInt());
	this->setPitch(json["pitch"].GetInt());
	this->setLoop(json["loop"].GetBool());
	this->setSrcLoop(json["srcLoop"].GetBool());
	return true;
}

const char *VoiceData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *VoiceData::getFilename()
{
	CC_ASSERT(_filename);
	return _filename->getCString();
}

const char *VoiceData::getSrcFilename()
{
	CC_ASSERT(_srcFilename);
	return _srcFilename->getCString();
}

float VoiceData::getVolumeNormalize()
{
	return (float)this->getVolume() * 0.01f;
}

#if defined(AGTK_DEBUG)
void VoiceData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<VoiceData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("volume:%d,%f", this->getVolume(), this->getVolumeNormalize());
	CCLOG("pan:%d", this->getPan());
	CCLOG("pitch:%d", this->getPitch());
	CCLOG("loop:%d", this->getLoop());
	CCLOG("srcLoop:%d", this->getSrcLoop());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class VariableData
 */
VariableData::VariableData()
{
	_id = 0;
	_name = nullptr;
	_initialValue = 0;
	_toBeSaved = false;
	_memo = nullptr;
	_folder = false;
	_children = nullptr;
}

VariableData::~VariableData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_children);
}

VariableData *VariableData::create(unsigned int id, std::string name, double initialValue, bool toBeSaved, std::string memo)
{
	auto p = new (std::nothrow) VariableData();
	if (p && p->init(id, name, initialValue, toBeSaved, memo)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool VariableData::init(unsigned int id, std::string name, double initialValue, bool toBeSaved, std::string memo)
{
	this->setId(id);
	this->setName(cocos2d::__String::create(name));
	this->setInitialValue(initialValue);
	this->setToBeSaved(toBeSaved);
	this->setMemo(cocos2d::__String::create(memo));
	return true;
}

bool VariableData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = VariableData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	this->setInitialValue(json["initialValue"].GetDouble());
	this->setToBeSaved(json["toBeSaved"].GetBool());
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	return true;
}

const char *VariableData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *VariableData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

#if defined(AGTK_DEBUG)
void VariableData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder::%s", this->getFolder() ? "true" : "false");
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<VariableData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("initialValue:%f", this->getInitialValue());
	CCLOG("toBeSaved:%s", this->getToBeSaved() ? "true" : "false");
	CCLOG("memo:%s", this->getMemo());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class SwitchData
 */
SwitchData::SwitchData()
{
	_id = 0;
	_name = nullptr;
	_initialValue = false;
	_toBeSaved = false;
	_memo = nullptr;
	_folder = false;
	_children = nullptr;
}

SwitchData::~SwitchData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_memo);
	CC_SAFE_RELEASE_NULL(_children);
}

SwitchData *SwitchData::create(unsigned int id, std::string name, bool initialValue, bool toBeSaved, std::string memo)
{
	auto p = new (std::nothrow) SwitchData();
	if (p && p->init(id, name, initialValue, toBeSaved, memo)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool SwitchData::init(unsigned int id, std::string name, bool initialValue, bool toBeSaved, std::string memo)
{
	this->setId(id);
	this->setName(cocos2d::__String::create(name));
	this->setInitialValue(initialValue);
	this->setToBeSaved(toBeSaved);
	this->setMemo(cocos2d::__String::create(memo));
	return true;
}

bool SwitchData::init(const rapidjson::Value& json)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = SwitchData::create(json["children"][i]);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	this->setInitialValue(json["initialValue"].GetBool());
	this->setToBeSaved(json["toBeSaved"].GetBool());
	this->setMemo(cocos2d::__String::create(json["memo"].GetString()));
	return true;
}

const char *SwitchData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *SwitchData::getMemo()
{
	CC_ASSERT(_memo);
	return _memo->getCString();
}

#if defined(AGTK_DEBUG)
void SwitchData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if(this->getFolder() && this->getChildren()){
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<SwitchData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("initialValue:%d", this->getInitialValue());
	CCLOG("toBeSaved:%d", this->getToBeSaved());
	CCLOG("memo:%s", this->getMemo());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
* @class TextData
*/
TextData::TextData()
{
	_id = -1;
	_name = nullptr;
	_fontId = -1;
	_letterSpacing = 0;
	_lineSpacing = 0;
	_textList = nullptr;
	_folder = false;
	_children = nullptr;
}

TextData::~TextData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_textList);
	CC_SAFE_RELEASE_NULL(_children);
}
bool TextData::init()
{
	return true;
}

bool TextData::init(const rapidjson::Value& json, cocos2d::__Array * languageList)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = TextData::create(json["children"][i], languageList);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	this->setFontId(json["fontId"].GetInt());
	this->setLetterSpacing(json["letterSpacing"].GetInt());
	this->setLineSpacing(json["lineSpacing"].GetInt());


	auto dic = cocos2d::__Dictionary::create();

	for (int i = 0; i < languageList->count(); i++) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto key = static_cast<cocos2d::__String *>(languageList->getObjectAtIndex(i))->getCString();
#else
		auto key = dynamic_cast<cocos2d::__String *>(languageList->getObjectAtIndex(i))->getCString();
#endif

		if (json["text"].HasMember(key)) {

			auto val = cocos2d::__String::create(json["text"][key].GetString());
			dic->setObject(val, key);
		}
	}

	this->setTextList(dic);

	return true;
}

//取得したいテキストのロケールを_localeに指定する。_locale == nullptrのときはデフォルトを取得する。
const char* TextData::getText(const char *_locale)
{
	if (_textList != nullptr) {
		// 言語設定を取得
		std::string locale = std::string(_locale ? _locale : GameManager::getInstance()->getProjectData()->getGameInformation()->getMainLanguage());

		auto string = dynamic_cast<cocos2d::__String *>(_textList->objectForKey(locale));

		// 文字列の取得に成功した場合
		if (string) {
			return string->getCString();
		}
	}

	return "";
}

#if defined(AGTK_DEBUG)
void TextData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<TextData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}

	CCLOG("fontId:%d", this->getFontId());
	CCLOG("letterSpacing:%d", this->getLetterSpacing());
	CCLOG("lineSpacing:%d", this->getLineSpacing());
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class AnimationOnlyData::AnimationInfoData
 */
AnimationOnlyData::AnimationInfoData::AnimationInfoData()
{
	_name = nullptr;
	_keyframeList = nullptr;

	// 初期値は512サイズとする
	_imageWidth = 512;
	_imageHeight = 512;
	_originX = 0;
	_originY = 0;
}

AnimationOnlyData::AnimationInfoData::~AnimationInfoData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_keyframeList);
}

bool AnimationOnlyData::AnimationInfoData::init(const rapidjson::Value& json)
{
	CC_ASSERT(json.HasMember("name"));
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	CC_ASSERT(json.HasMember("totalFrameCount"));
	this->setTotalFrameCount(json["totalFrameCount"].GetInt());
	if (json.HasMember("imageWidth")) {
		this->setImageWidth(json["imageWidth"].GetInt());
	}
	if (json.HasMember("imageHeight")) {
		this->setImageHeight(json["imageHeight"].GetInt());
	}
	if (json.HasMember("originX")) {
		this->setOriginX(json["originX"].GetInt());
	}
	if (json.HasMember("originY")) {
		this->setOriginY(json["originY"].GetInt());
	}
	CC_ASSERT(json.HasMember("keyframeList"));
	auto arr = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < json["keyframeList"].Size(); i++) {
		auto key = cocos2d::Integer::create(json["keyframeList"][i].GetInt());
		arr->addObject(key);
	}
	this->setKeyframeList(arr);
	return true;
}

const char *AnimationOnlyData::AnimationInfoData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

#if defined(AGTK_DEBUG)
void AnimationOnlyData::AnimationInfoData::dump()
{
	CCLOG("name:%s", this->getName());
	CCLOG("totalFrameCount:%d", this->getTotalFrameCount());
	CCLOG("imageWidth:%d", this->getImageWidth());
	CCLOG("imageHeight:%d", this->getImageHeight());
	CCLOG("originX:%d", this->getOriginX());
	CCLOG("originY:%d", this->getOriginY());
	CCLOG("keyframeList");
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getKeyframeList(), ref) {
		auto id = dynamic_cast<cocos2d::Integer *>(ref);
		CCLOG("  %d", id->getValue());
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
 * @class AnimationOnlyData
 */
AnimationOnlyData::AnimationOnlyData()
{
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_animationInfoList = nullptr;
	_children = nullptr;
	_dataType = DataType::None;
	_binFilename = nullptr;
	_atlasFilename = nullptr;
}

AnimationOnlyData::~AnimationOnlyData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_animationInfoList);
	CC_SAFE_RELEASE_NULL(_children);
	CC_SAFE_RELEASE_NULL(_binFilename);
	CC_SAFE_RELEASE_NULL(_atlasFilename);
}

bool AnimationOnlyData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = AnimationOnlyData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
	if (json.HasMember("animationInfoList")) {
		{
			CC_ASSERT(json.HasMember("filename"));
#ifdef USE_PREVIEW
			this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
			this->setFilename(cocos2d::String::create(projectPath + json["filename"].GetString()));
#endif
			CC_ASSERT(json.HasMember("srcFilename"));
			this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));

			//SpriteStudio
			if ((strstr(this->getFilename(), ".sspj") != NULL) || (strstr(this->getFilename(), ".SSPJ") != NULL)) {
				_dataType = DataType::SpriteStudio;
				std::string fname = this->getFilename();
				int ext_i = fname.find_last_of(".");
				std::string file_ssbp = fname.substr(0, ext_i + 1) + std::string("ssbp");
				this->setBinFilename(cocos2d::__String::create(file_ssbp.c_str()));
			}
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#endif
			//GIF
			else if ((strstr(this->getFilename(), ".gif") != NULL) || (strstr(this->getFilename(), ".GIF") != NULL)) {
				_dataType = DataType::Gif;
			}
			//Spine
			else if ((strstr(this->getFilename(), ".json") != NULL) || (strstr(this->getFilename(), ".JSON") != NULL)) {
				_dataType = DataType::Spine;
				std::string fname = this->getFilename();
				int ext_i = fname.find_last_of(".");
				std::string file_atlas = fname.substr(0, ext_i + 1) + std::string("atlas");
				this->setAtlasFilename(cocos2d::__String::create(file_atlas.c_str()));
			}
			else {
				CC_ASSERT(0);
			}

			auto arr = cocos2d::__Array::create();
			for (rapidjson::SizeType i = 0; i < json["animationInfoList"].Size(); i++) {
				auto animationInfoData = AnimationOnlyData::AnimationInfoData::create(json["animationInfoList"][i]);
				if (animationInfoData == nullptr) {
					return false;
				}
				arr->addObject(animationInfoData);
			}
			this->setAnimationInfoList(arr);
			return true;
		}
	}
	else {
		std::string path;
		char num[16];
		sprintf(num, "%03d", this->getId());
		path = projectPath + std::string("animations") + std::string("/") + std::string(num) + std::string(".json");
		return this->loadJson(path, projectPath);
	}
}

const char *AnimationOnlyData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *AnimationOnlyData::getFilename()
{
	CC_ASSERT(_filename);
	return _filename->getCString();
}

const char *AnimationOnlyData::getSrcFilename()
{
	CC_ASSERT(_srcFilename);
	return _srcFilename->getCString();
}

const char *AnimationOnlyData::getBinFilename()
{
	CC_ASSERT(_binFilename);
	return _binFilename->getCString();
}

const char *AnimationOnlyData::getAtlasFilename()
{
	CC_ASSERT(_atlasFilename);
	return _atlasFilename->getCString();
}

AnimationOnlyData::AnimationInfoData *AnimationOnlyData::getAnimationInfoData(int id)
{
	return dynamic_cast<AnimationOnlyData::AnimationInfoData *>(this->getAnimationInfoList()->getObjectAtIndex(id));
}

AnimationOnlyData::AnimationInfoData *AnimationOnlyData::getAnimationInfoData(std::string name)
{
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getAnimationInfoList(), ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto p = static_cast<AnimationOnlyData::AnimationInfoData *>(ref);
#else
		auto p = dynamic_cast<AnimationOnlyData::AnimationInfoData *>(ref);
#endif
		if (name.compare(p->getName()) == 0) {
			return p;
		}
	}
	return nullptr;
}

bool AnimationOnlyData::loadJson(std::string path, const std::string &projectPath)
{
	auto jsonData = cocos2d::FileUtils::getInstance()->getStringFromFile(path);
	if (jsonData.length() == 0) {
		return false;
	}

	rapidjson::Document doc;
	doc.Parse(jsonData.c_str());
	bool error = doc.HasParseError();
	if (error) {
		return false;
	}
	CC_ASSERT(doc.HasMember("filename"));
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(doc["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::String::create(projectPath + doc["filename"].GetString()));
#endif
	CC_ASSERT(doc.HasMember("srcFilename"));
	this->setSrcFilename(cocos2d::__String::create(doc["srcFilename"].GetString()));

	//SpriteStudio
	if ((strstr(this->getFilename(), ".sspj") != NULL) || (strstr(this->getFilename(), ".SSPJ") != NULL)) {
		_dataType = DataType::SpriteStudio;
		std::string fname = this->getFilename();
		int ext_i = fname.find_last_of(".");
		std::string file_ssbp = fname.substr(0, ext_i + 1) + std::string("ssbp");
		this->setBinFilename(cocos2d::__String::create(file_ssbp.c_str()));
	}
	//GIF
	else if ((strstr(this->getFilename(), ".gif") != NULL) || (strstr(this->getFilename(), ".GIF") != NULL)) {
		_dataType = DataType::Gif;
	}
	//Spine
	else if ((strstr(this->getFilename(), ".json") != NULL) || (strstr(this->getFilename(), ".JSON") != NULL)) {
		_dataType = DataType::Spine;
		std::string fname = this->getFilename();
		int ext_i = fname.find_last_of(".");
		std::string file_atlas = fname.substr(0, ext_i + 1) + std::string("atlas");
		this->setAtlasFilename(cocos2d::__String::create(file_atlas.c_str()));
	}
	else {
		CC_ASSERT(0);
	}

	CC_ASSERT(doc.HasMember("animationInfoList"));
	auto arr = cocos2d::__Array::create();
	for (rapidjson::SizeType i = 0; i < doc["animationInfoList"].Size(); i++) {
		auto animationInfoData = AnimationOnlyData::AnimationInfoData::create(doc["animationInfoList"][i]);
		if (animationInfoData == nullptr) {
			return false;
		}
		arr->addObject(animationInfoData);
	}
	this->setAnimationInfoList(arr);
	return true;
}

#if defined(AGTK_DEBUG)
void AnimationOnlyData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<AnimationOnlyData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("keyframeList");
	cocos2d::Ref *ref = nullptr;
	CCARRAY_FOREACH(this->getAnimationInfoList(), ref) {
		auto p = dynamic_cast<AnimationOnlyData::AnimationInfoData *>(ref);
		p->dump();
	}
}
#endif

//-------------------------------------------------------------------------------------------------------------------
/**
* @class PluginData
*/
PluginData::PluginData()
{
	_id = 0;
	_name = nullptr;
	_filename = nullptr;
	_srcFilename = nullptr;
	_enabled = false;
	_internalJson = nullptr;
	_paramValueJson = nullptr;
	_children = nullptr;
}

PluginData::~PluginData()
{
	CC_SAFE_RELEASE_NULL(_name);
	CC_SAFE_RELEASE_NULL(_filename);
	CC_SAFE_RELEASE_NULL(_srcFilename);
	CC_SAFE_RELEASE_NULL(_internalJson);
	CC_SAFE_RELEASE_NULL(_paramValueJson);
	CC_SAFE_RELEASE_NULL(_children);
}

bool PluginData::init(const rapidjson::Value& json, const std::string &projectPath)
{
	this->setId(json["id"].GetInt());
	this->setName(cocos2d::__String::create(json["name"].GetString()));
	this->setFolder(json["folder"].GetBool());
	if (this->getFolder()) {
		auto dic = cocos2d::__Dictionary::create();
		if (json.HasMember("children")) {
			for (rapidjson::SizeType i = 0; i < json["children"].Size(); i++) {
				auto p = PluginData::create(json["children"][i], projectPath);
#if defined(AGTK_DEBUG)
				CC_ASSERT(dic->objectForKey(p->getId()) == nullptr);
#endif
				dic->setObject(p, p->getId());
			}
		}
		this->setChildren(dic);
		return true;
	}
#ifdef USE_PREVIEW
	this->setFilename(cocos2d::__String::create(getFullFilename(json["filename"].GetString(), projectPath)));
#else
	this->setFilename(cocos2d::__String::create(projectPath + json["filename"].GetString()));
#endif
	this->setSrcFilename(cocos2d::__String::create(json["srcFilename"].GetString()));
	this->setEnabled(json["enabled"].GetBool());
#if 1
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		json["internalJson"].Accept(writer);
		this->setInternalJson(cocos2d::__String::create(buffer.GetString()));
	}
	{
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		json["paramValueJson"].Accept(writer);
		this->setParamValueJson(cocos2d::__String::create(buffer.GetString()));
	}
#else
	this->setInternalJson(cocos2d::__String::create(json["internalJson"].GetString()));
	this->setParamValueJson(cocos2d::__String::create(json["paramValueJson"].GetString()));
#endif
	return true;
}

const char *PluginData::getName()
{
	CC_ASSERT(_name);
	return _name->getCString();
}

const char *PluginData::getFilename()
{
	if (_filename == nullptr) {
		return nullptr;
	}
	return _filename->getCString();
}

const char *PluginData::getSrcFilename()
{
	if (_srcFilename == nullptr) {
		return nullptr;
	}
	return _srcFilename->getCString();
}

const char *PluginData::getInternalJson()
{
	if (_internalJson == nullptr) {
		return nullptr;
	}
	return _internalJson->getCString();
}

const char *PluginData::getParamValueJson()
{
	if (_paramValueJson == nullptr) {
		return nullptr;
	}
	return _paramValueJson->getCString();
}

#if defined(AGTK_DEBUG)
void PluginData::dump()
{
	CCLOG("id:%d", this->getId());
	CCLOG("name:%s", this->getName());
	CCLOG("folder:%d", this->getFolder());
	if (this->getFolder() && this->getChildren()) {
		auto keys = this->getChildren()->allKeys();
		cocos2d::Ref *ref = nullptr;
		CCARRAY_FOREACH(keys, ref) {
			auto id = dynamic_cast<cocos2d::Integer *>(ref);
			auto data = dynamic_cast<PluginData *>(this->getChildren()->objectForKey(id->getValue()));
			data->dump();
		}
		return;
	}
	CCLOG("filename:%s", this->getFilename());
	CCLOG("srcFilename:%s", this->getSrcFilename());
	CCLOG("enabled:%d", this->getFolder());
	CCLOG("internalJson:%s", this->getInternalJson());
	CCLOG("paramValueJson:%s", this->getParamValueJson());
}
#endif


NS_DATA_END
NS_AGTK_END
