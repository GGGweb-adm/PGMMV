#ifndef __FONT_MANAGER_H__
#define	__FONT_MANAGER_H__

#include "cocos2d.h"
#include "GameManager.h"

USING_NS_CC;

class AGTKPLAYER_API FontManager : public cocos2d::Ref
{
private:
	FontManager();
	virtual ~FontManager();
	static FontManager *_fontManager;
public:
	static FontManager* getInstance();
	static void purge();
public:
	cocos2d::Label *createWithArialFont(const std::string& text, int fontSize);
	cocos2d::Label *createWithTTF(const std::string& text, const std::string& ttf, int fontSize);
	cocos2d::Label *createOrSetWithFontData(const std::string& text, agtk::data::FontData *fontData, int iniSize, int fontSize,cocos2d::Label* label=nullptr);

	float getImageFontWidth(agtk::data::FontData* fontData);
	float getImageFontHeight(agtk::data::FontData* fontData);

	static std::string getFontFilePath(const std::string &fontName);
};

#endif	//__FONT_MANAGER_H__
