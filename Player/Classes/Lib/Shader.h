#ifndef __SHADER_H__
#define	__SHADER_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API ShaderValue : public agtk::EventTimer
{
private:
	ShaderValue();
	virtual ~ShaderValue();
public:
	CREATE_FUNC_PARAM2(ShaderValue, float, value, bool, ignored);
private:
	virtual bool init(float value, bool ignored);
public:
	float setValue(float value, float seconds = 0.0f);
	float getValue();
	float getNextValue() { return _nextValue; };
	bool isChanged();
	ShaderValue &operator=(const ShaderValue &shaderValue);
	void getJsonData(rapidjson::Value &jsonData, rapidjson::Document::AllocatorType& allocator);
	void setJsonData(const rapidjson::Value &jsonData);
public:
	float getTimer() { return EventTimer::_timer; };
	void setTimer(float timer) { EventTimer::_timer = timer; };
private:
	float _value;
	float _nextValue;
	float _prevValue;
	float _oldValue;
	CC_SYNTHESIZE(bool, _ignored, Ignored);
	CC_SYNTHESIZE(float, _seconds, Seconds);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Shader: public cocos2d::Ref
{
public:
	enum ShaderKind {
		kShaderCRTMonitor = 0,//アナログテレビ
		kShaderBlur,//ぼかし
		kShaderNoisy,//ノイズ
		kShaderMosaic,//モザイク
		kShaderColorGray,//モノクロ
		kShaderColorSepia,//セピア
		kShaderColorNega,//ネガ
		kShaderColorGameboy,//ゲームボーイ調
		kShaderColorDark,//暗闇
		kShaderColorDarkMask,//暗闇（マスク用）
		kShaderColorChromaticAberration,//色収差
		kShaderColorSolidColor,//ベタ塗
		kShaderColorRgba,//RGBA
		kShaderColorAfterimageRbga,//残像用RGBA
		kShaderColorSilhouetteimageRbga,//シルエット用RGBA
		kShaderColorSilhouetteimageRbgaMultiply,//シルエット用RGBA乗算
		kShaderTextureRepeat,//テクスチャリピート
		kShaderAlphaMask,//αマスク
		kShaderImage,//イメージ
		kShaderDefault,
		kShaderTransparency,//透明
		kShaderMax,
	};
private:
	Shader();
	virtual ~Shader();
public:
	static Shader *create(cocos2d::Size size, ShaderKind Kind);
	static Shader *createShaderKind(cocos2d::Node *node, cocos2d::Size size, ShaderKind kind, int hierarchyLimit);

	// パーティクルに使用する残像用のシェーダーを生成する
	static cocos2d::GLProgramState *createShaderParticleAfterimage();

private:
	virtual bool init(cocos2d::Size size, ShaderKind kind);
	virtual bool initShaderKind(cocos2d::Node *node, cocos2d::Size size, ShaderKind kind, int hierarchyLimit);
	cocos2d::GLProgramState *createShader(ShaderKind kind);
	cocos2d::GLProgramState *createShaderDefault();
	cocos2d::GLProgramState *createShaderColorGray();
	cocos2d::GLProgramState *createShaderColorSepia();
	cocos2d::GLProgramState *createShaderColorNega();
	cocos2d::GLProgramState *createShaderColorGameBoy();
	cocos2d::GLProgramState *createShaderCRTMonitor();
	cocos2d::GLProgramState *createShaderBlur();
	cocos2d::GLProgramState *createShaderNoisy();
	cocos2d::GLProgramState *createShaderMosaic();
	cocos2d::GLProgramState *createShaderColorDark();
	cocos2d::GLProgramState *createShaderColorDarkMask();
	cocos2d::GLProgramState *createShaderColorChromaticAberration();
	cocos2d::GLProgramState *createShaderColorSolidColor();
	cocos2d::GLProgramState *createShaderColorRgba();
	cocos2d::GLProgramState *createShaderColorAfterimageAlpha();
	cocos2d::GLProgramState *createShaderColorShilhouetteimage();
	cocos2d::GLProgramState *createShaderColorMultiplyShilhouetteimage();
	cocos2d::GLProgramState *createShaderTextureRepeat();
	cocos2d::GLProgramState *createShaderAlphaMask();
	cocos2d::GLProgramState *createShaderImage();
	cocos2d::GLProgramState *createShaderTransparency();
public:
	virtual void update(float delta);
	void pause();
	void resume();
public:
	float setIntensity(float v, float seconds = 0.0f);
	float getIntensity();
	bool setIgnored(bool ignored);
	bool getIgnored();
	void setShaderSolidColor(cocos2d::Color3B color);
	void setShaderRgbaColor(cocos2d::Color4B color);
	void setShaderAlpha(float alpha);
	cocos2d::Color4B getShaderRgbaColor();
	cocos2d::Color4B getShaderRgbaColorBase() { return _rgbaColor; };
	void setShaderTextureRepeat(cocos2d::Size textureSize, cocos2d::Size resolutionSize);
	void setMaskTexture(cocos2d::Texture2D *texture2d);
	void resetMaskTexture();
	cocos2d::Texture2D *getMaskTexture() { return _maskTexture; };
	void setShaderProgram(cocos2d::Node *node, int hierarchyLimit = -1);
private:
	void updateIntensity();
	void updateShader(float delta);
	bool updateUniforms(cocos2d::GLProgram* &program);
	void setShaderProgramRecursive(cocos2d::Node *node, cocos2d::GLProgramState *programState, int hierarchy = 0, int hierarchyLimit = -1);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::GLProgramState *, _programState, ProgramState);
	CC_SYNTHESIZE_RETAIN(cocos2d::GLProgram *, _glProgram, GLProgram);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _valueList, ValueList);
	CC_SYNTHESIZE_READONLY(ShaderKind, _kind, Kind);
	CC_SYNTHESIZE_RETAIN(agtk::ShaderValue *, _value, Value);
	CC_SYNTHESIZE(cocos2d::Size, _shaderSize, ShaderSize);
	CC_SYNTHESIZE_RETAIN(cocos2d::Ref *, _userData, UserData);
	float _counter;
	cocos2d::Color4B _rgbaColor;
	cocos2d::Texture2D *_maskTexture;
	bool _pauseFlag;
	bool _resumeFlag;
	bool _updateFlag;
	bool _updateUniformsFlag;

	//「ぼかし」シェーダーパラメーター。
	struct BlurParam {
		float _blurRadius;
		float _sampleNum;
		float _intensity;
		void init(float blurRadius, float sampleNum, float intensity) {
			_blurRadius = blurRadius;
			_sampleNum = sampleNum;
			_intensity = intensity;
		}
		void set(float intensity) { _intensity = intensity; };
		float blurRadius() { return _blurRadius; };
		float sampleNum() { return _sampleNum; };
		float intensity() { return _intensity; }
	} _blurParam;
};

NS_AGTK_END

#endif	//__SHADER_H__
