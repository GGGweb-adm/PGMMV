#ifndef __AGKT_ASSET_DATA_H__
#define	__AGKT_ASSET_DATA_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "json/document.h"

NS_AGTK_BEGIN
NS_DATA_BEGIN

std::string getFullFilename(std::string filename, const std::string &projectPath);
#ifdef USE_PREVIEW
std::string join(const std::list<std::string> &strlist, const std::string &delim);
#endif

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API FontData : public cocos2d::Ref
{
private:
	FontData();
	virtual ~FontData();
public:
	CREATE_FUNC_PARAM(FontData, const rapidjson::Value&, json);
	const char *getName();
	const char *getFontName();
	const char *getTTFName();
	const char *getLetterLayout();
	const char *getLetterLayoutWithoutLF();
#if defined(AGTK_DEBUG)
	void dump();
#endif
	class FontSetting : public cocos2d::Ref {
	public:
		FontSetting();
		virtual ~FontSetting();
		CREATE_FUNC_PARAM(FontSetting, const rapidjson::Value&, json);
	private:
		virtual bool init(const rapidjson::Value& json);
		CC_SYNTHESIZE(bool, _imageFontFlag, ImageFontFlag);
		CC_SYNTHESIZE(int, _imageId, ImageId);
		CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _fontName, FontName);
		CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _ttfName, TTFName);
		CC_SYNTHESIZE(int, _fontSize, FontSize);
		CC_SYNTHESIZE(bool, _antialiasDisabled, AntialiasDisabled);
		CC_SYNTHESIZE(int, _aliasThreshold, AliasThreshold);
		CC_SYNTHESIZE(bool, _fixedWidth, FixedWidth);
		CC_SYNTHESIZE(int, _hankakuWidth, HankakuWidth);
		CC_SYNTHESIZE(int, _zenkakuWidth, ZenkakuWidth);
		CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _letterLayout, LetterLayout);
		//Additional Data
		CC_SYNTHESIZE(int, _horzDivCount, HorzDivCount);
		CC_SYNTHESIZE(int, _vertDivCount, VertDivCount);
		CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _letterLayoutWithoutLF, LetterLayoutWithoutLF);
	};
	FontSetting *getMainFontSetting();
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _localeSettings, LocaleSettings);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ImageData : public cocos2d::Ref
{
public:
	enum Usege {
		Tile = 0x01,
		Item = 0x02,
		Animation = 0x04,
		Font = 0x08,
		Background = 0x10,
	};
private:
	ImageData();
	virtual ~ImageData();
public:
	CREATE_FUNC_PARAM2(ImageData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);
	CC_SYNTHESIZE(int, _horzDivCount, HorzDivCount);
	CC_SYNTHESIZE(int, _vertDivCount, VertDivCount);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	//Additional Data
	CC_SYNTHESIZE(int, _texWidth, TexWidth);
	CC_SYNTHESIZE(int, _texHeight, TexHeight);

};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API MovieData : public cocos2d::Ref
{
private:
	MovieData();
	virtual ~MovieData();
public:
	CREATE_FUNC_PARAM2(MovieData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);//ファイル名
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);//ファイル名(src)
	CC_SYNTHESIZE(int, _volume, Volume);//ボリューム 0～100(default=100)
	CC_SYNTHESIZE(int, _width, Width);//幅
	CC_SYNTHESIZE(int, _height, Height);//高さ
	CC_SYNTHESIZE(bool, _folder, Folder);//フォルダ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API BgmData : public cocos2d::Ref
{
private:
	BgmData();
	virtual ~BgmData();
public:
	CREATE_FUNC_PARAM2(BgmData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
	float getVolumeNormalize();//ボリュームを正規化(0.0～1.0f)
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);//ファイル名
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);//ファイル名(src)
	CC_SYNTHESIZE(int, _volume, Volume);//ボリューム 0～100(default=100)
	CC_SYNTHESIZE(int, _pan, Pan);//左右バランス -50:左,50:右 -50～50(default=0)
	CC_SYNTHESIZE(int, _pitch, Pitch);//音程 -50～50(default=0)
	CC_SYNTHESIZE(bool, _loop, Loop);//ループ
	CC_SYNTHESIZE(bool, _srcLoop, SrcLoop);//ループ(src)
	CC_SYNTHESIZE(bool, _folder, Folder);//フォルダ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SeData : public cocos2d::Ref
{
private:
	SeData();
	virtual ~SeData();
public:
	CREATE_FUNC_PARAM2(SeData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
	float getVolumeNormalize();//ボリュームを正規化(0.0～1.0f)
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);//ファイル名
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);//ファイル名(src)
	CC_SYNTHESIZE(int, _volume, Volume);//ボリューム 0～100(default=100)
	CC_SYNTHESIZE(int, _pan, Pan);//左右バランス -50:左,50:右 -50～50(default=0)
	CC_SYNTHESIZE(int, _pitch, Pitch);//音程 -50～50(default=0)
	CC_SYNTHESIZE(bool, _loop, Loop);//ループ
	CC_SYNTHESIZE(bool, _srcLoop, SrcLoop);//ループ(src)
	CC_SYNTHESIZE(bool, _folder, Folder);//フォルダ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API VoiceData : public cocos2d::Ref
{
private:
	VoiceData();
	virtual ~VoiceData();
public:
	CREATE_FUNC_PARAM2(VoiceData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
	float getVolumeNormalize();//ボリュームを正規化(0.0～1.0f)
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);//ファイル名
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);//ファイル名
	CC_SYNTHESIZE(int, _volume, Volume);//ボリューム 0～100(default=100)
	CC_SYNTHESIZE(int, _pan, Pan);//左右バランス -50:左,50:右 -50～50(default=0)
	CC_SYNTHESIZE(int, _pitch, Pitch);//音程 -50～50(default=0)
	CC_SYNTHESIZE(bool, _loop, Loop);//ループ
	CC_SYNTHESIZE(bool, _srcLoop, SrcLoop);//ループ(src)
	CC_SYNTHESIZE(bool, _folder, Folder);//フォルダ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API VariableData : public cocos2d::Ref
{
private:
	VariableData();
	virtual ~VariableData();
public:
	CREATE_FUNC_PARAM(VariableData, const rapidjson::Value&, json);
	static VariableData *create(unsigned int id, std::string name, double initialValue, bool toBeSaved, std::string memo);
	const char *getName();
	const char *getMemo();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(unsigned int id, std::string name, double initialValue, bool toBeSaved, std::string memo);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(double, _initialValue, InitialValue);
	CC_SYNTHESIZE(bool, _toBeSaved, ToBeSaved);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SwitchData : public cocos2d::Ref
{
private:
	SwitchData();
	virtual ~SwitchData();
public:
	CREATE_FUNC_PARAM(SwitchData, const rapidjson::Value&, json);
	static SwitchData *create(unsigned int id, std::string name, bool initialValue, bool toBeSaved, std::string memo);
	const char *getName();
	const char *getMemo();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json);
	virtual bool init(unsigned int id, std::string name, bool initialValue, bool toBeSaved, std::string memo);
private:
	// プロジェクトデータ
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE(bool, _initialValue, InitialValue);
	CC_SYNTHESIZE(bool, _toBeSaved, ToBeSaved);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _memo, Memo);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TextData : public cocos2d::Ref
{
private:
	TextData();
	virtual ~TextData();
public:
	CREATE_FUNC(TextData);
	CREATE_FUNC_PARAM2(TextData, const rapidjson::Value&, json, cocos2d::__Array *, languageList);
	const char *getText(const char *locale);

#if defined(AGTK_DEBUG)
	void dump();
#endif

private:
	virtual bool init();
	virtual bool init(const rapidjson::Value& json, cocos2d::__Array * languageList);

private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);

	CC_SYNTHESIZE(unsigned int, _fontId, FontId);
	CC_SYNTHESIZE(int, _letterSpacing, LetterSpacing);
	CC_SYNTHESIZE(int, _lineSpacing, LineSpacing);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _textList, TextList);

	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API AnimationOnlyData : public cocos2d::Ref
{
public:
	enum DataType {
		None,
		Gif,
		Spine,
		SpriteStudio,
	};
public:
	class AnimationInfoData : public cocos2d::Ref
	{
	private:
		AnimationInfoData();
		virtual ~AnimationInfoData();
	public:
		CREATE_FUNC_PARAM(AnimationInfoData, const rapidjson::Value&, json);
		const char *getName();
#if defined(AGTK_DEBUG)
		void dump();
#endif
	private:
		virtual bool init(const rapidjson::Value& json);
	private:
		CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
		CC_SYNTHESIZE(unsigned int, _totalFrameCount, TotalFrameCount);
		CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _keyframeList, KeyframeList);
		CC_SYNTHESIZE(int, _imageWidth, ImageWidth);
		CC_SYNTHESIZE(int, _imageHeight, ImageHeight);
		CC_SYNTHESIZE(int, _originX, OriginX);
		CC_SYNTHESIZE(int, _originY, OriginY);
	};
private:
	AnimationOnlyData();
	virtual ~AnimationOnlyData();
public:
	CREATE_FUNC_PARAM2(AnimationOnlyData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
	const char *getBinFilename();
	const char *getAtlasFilename();
	AnimationInfoData *getAnimationInfoData(int id);
	AnimationInfoData *getAnimationInfoData(std::string name);
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
	bool loadJson(std::string path, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _animationInfoList, AnimationInfoList);
	CC_SYNTHESIZE(bool, _folder, Folder);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
	//Additional Data
	CC_SYNTHESIZE_READONLY(DataType, _dataType, DataType);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _binFilename, BinFilename);
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _atlasFilename, AtlasFilename);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PluginData : public cocos2d::Ref
{
private:
	PluginData();
	virtual ~PluginData();
public:
	CREATE_FUNC_PARAM2(PluginData, const rapidjson::Value&, json, const std::string &, projectPath);
	const char *getName();
	const char *getFilename();
	const char *getSrcFilename();
	const char *getInternalJson();
	const char *getParamValueJson();
#if defined(AGTK_DEBUG)
	void dump();
#endif
private:
	virtual bool init(const rapidjson::Value& json, const std::string &projectPath);
private:
	CC_SYNTHESIZE(unsigned int, _id, Id);//ID
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _name, Name);//名前
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _filename, Filename);//ファイル名
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _srcFilename, SrcFilename);//ファイル名(src)
	CC_SYNTHESIZE(bool, _enabled, Enabled);//機能を有効化
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _internalJson, InternalJson);//内部パラメータJSON
	CC_SYNTHESIZE_RETAIN(cocos2d::__String *, _paramValueJson, ParamValueJson);//パラメータ設定値JSON
	CC_SYNTHESIZE(bool, _folder, Folder);//フォルダ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _children, Children);
};

NS_DATA_END
NS_AGTK_END

#endif	//__AGKT_ASSET_DATA_H__
