#include "FontManager.h"
#include "scripting/js-bindings/manual/ScriptingCore.h"

ScriptingCore::js_cocos2dx_LabelTTF_fontNameCallbackType sLastFontNameCallback = nullptr;

FontManager* FontManager::_fontManager = nullptr;
FontManager::FontManager()
{
	sLastFontNameCallback = ScriptingCore::js_cocos2dx_LabelTTF_setFontNameCallback(getFontFilePath);
}

FontManager::~FontManager()
{
	ScriptingCore::js_cocos2dx_LabelTTF_setFontNameCallback(sLastFontNameCallback);
}

FontManager* FontManager::getInstance()
{
	if (!_fontManager) {
		_fontManager = new FontManager();
	}
	return _fontManager;
}

void FontManager::purge()
{
	if (!_fontManager)
		return;

	FontManager *fm = _fontManager;
	_fontManager = NULL;
	fm->release();
}

cocos2d::Label *FontManager::createWithArialFont(const std::string& text, int fontSize)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	auto label = Label::createWithSystemFont(text, "Arial", fontSize);
#endif
	label->setAnchorPoint(Vec2(0, 0));
	label->setHorizontalAlignment(TextHAlignment::LEFT);
	label->setVerticalAlignment(TextVAlignment::TOP);
	label->getTexture()->setAliasTexParameters();
	return label;
}

cocos2d::Label *FontManager::createWithTTF(const std::string& text, const std::string& ttf, int fontSize)
{
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	cocos2d::TTFConfig config("fonts/arial.ttf", fontSize, GlyphCollection::DYNAMIC);
#endif
	auto label = Label::createWithTTF(config, text);
	label->setAnchorPoint(Vec2(0, 0));
	label->setHorizontalAlignment(TextHAlignment::LEFT);
	label->setVerticalAlignment(TextVAlignment::TOP);
	label->getTexture()->setAliasTexParameters();
	return label;
}

std::string FontManager::getFontFilePath(const std::string &fontName)
{
	std::string filePath = fontName;
	static std::string extensions[] = { ".ttf", ".ttc" };
	for (std::string extension : extensions) {
		if (filePath.length() < 4) break;
// #AGTK-NX
#if CC_TARGET_PLATFORM != CC_PLATFORM_NX
		if (stricmp(filePath.substr(filePath.length() - 4).c_str(), extension.c_str()) == 0) {
#else
#endif
			filePath = filePath.substr(0, filePath.length() - 4);
		}
	}
	auto projectPath = GameManager::getProjectPathFromProjectFile(GameManager::getInstance()->getProjectFilePath()->getCString());
	for (std::string extension : extensions) {
		if (FileUtils::getInstance()->isFileExist(filePath + extension)) {
			return filePath + extension;
		}
		if (FileUtils::getInstance()->isFileExist(projectPath + filePath + extension)) {
			return projectPath + filePath + extension;
		}
	}
	// フォントファイルが見つからない場合
	CCASSERT(0, "font file is not exist");
	return fontName;
}

cocos2d::Label *FontManager::createOrSetWithFontData(const std::string& text, agtk::data::FontData *fontData, int iniSize, int fontSize,cocos2d::Label* label )
{
	if (fontSize == 0) {
		return nullptr;
	}
	auto project = GameManager::getInstance()->getProjectData();
	if (fontData->getMainFontSetting()->getImageFontFlag()) {
		if (fontData->getMainFontSetting()->getImageId() < 0) {
			return nullptr;
		}
		auto imageData = project->getImageData(fontData->getMainFontSetting()->getImageId());
		int itemWidth = fontData->getMainFontSetting()->getHorzDivCount() ? imageData->getTexWidth() / fontData->getMainFontSetting()->getHorzDivCount() : 0;
		int itemHeight = fontData->getMainFontSetting()->getVertDivCount() ? imageData->getTexHeight() / fontData->getMainFontSetting()->getVertDivCount() : 0;
		//label = Label::createWithCharMap(imageData->getFilename(), itemWidth, itemHeight, fontData->getLetterLayoutWithoutLF());
		if (label) {
			label->setCharMap(imageData->getFilename(), itemWidth, itemHeight, fontData->getLetterLayout());
		}
		else {
			label = Label::createWithCharMap(imageData->getFilename(), itemWidth, itemHeight, fontData->getLetterLayout());
		}
		if (label == nullptr) {
			return nullptr;
		}
		label->setString(text);
		label->setScaleX((float)fontSize / iniSize);
		label->setScaleY((float)fontSize / iniSize);
	}
	else {
		// TTFファイルあるいはTTCファイルを読み込む
		std::string filePath = "fonts/";
		filePath += fontData->getFontName();

		cocos2d::TTFConfig config(
			"",
			fontSize,
			GlyphCollection::DYNAMIC
		);
		if (fontData->getMainFontSetting()->getAntialiasDisabled()) {
			config.aliasThreshold = fontData->getMainFontSetting()->getAliasThreshold();
		}

		config.fontFilePath = getFontFilePath(filePath);
		if (label) {
			label->initWithTTF(config, text);
		}
		else {
			label = Label::createWithTTF(config, text);
		}

	}
	label->setAnchorPoint(Vec2(0, 0));
	label->setHorizontalAlignment(TextHAlignment::LEFT);
	label->setVerticalAlignment(TextVAlignment::TOP);
	label->getTexture()->setAliasTexParameters();
	return label;
}

float FontManager::getImageFontWidth(agtk::data::FontData* fontData)
{
	auto project = GameManager::getInstance()->getProjectData();

	if (fontData->getMainFontSetting()->getImageFontFlag()) {
		auto imageData = project->getImageData(fontData->getMainFontSetting()->getImageId());
		return imageData->getTexWidth() / fontData->getMainFontSetting()->getHorzDivCount();
	}

	return 0;
}

float FontManager::getImageFontHeight(agtk::data::FontData* fontData)
{
	auto project = GameManager::getInstance()->getProjectData();

	if (fontData->getMainFontSetting()->getImageFontFlag()) {
		auto imageData = project->getImageData(fontData->getMainFontSetting()->getImageId());
		if (!imageData) {
			return 0;
		}
		return imageData->getTexHeight() / fontData->getMainFontSetting()->getVertDivCount();
	}

	return 0;
}
