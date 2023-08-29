#ifndef __GUI_H__
#define	__GUI_H__

#include "cocos2d.h"
#include "Lib/Macros.h"
#include "Lib/Object.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
// GUI基底
class AGTKPLAYER_API Gui : public cocos2d::Node
{
public:
	const int NO_SETTING = -1; // 設定無し
protected:
	Gui();
	virtual ~Gui();

	agtk::Object * _targetObject; // 追従対象のオブジェクト
	cocos2d::ClippingRectangleNode* _clipping;

public:
	virtual void update(float delta) = 0;
	virtual void remove() = 0;
	virtual bool checkRemove(agtk::Object* object);	// 除去可能か確認する
	virtual bool getObjectStop(); // オブジェクト停止が発生するか？
	virtual bool getGameStop();	// ゲームの停止が発生するか？
	bool isDelete(); // 削除してもよいか？
	cocos2d::Vec2 getPositionTransform(cocos2d::Vec2 pos);
	bool isUseClipping();

#ifdef USE_REDUCE_RENDER_TEXTURE	//agusa-k
	virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
#endif

	agtk::Object* getTargetObject(); // 追従対象のオブジェクト
	bool _isDelete;
	bool _isClipping;
};

//-------------------------------------------------------------------------------------------------------------------
// テキスト表示用GUI
class AGTKPLAYER_API TextGui : public cocos2d::Node
{
protected:
	cocos2d::Color3B _color;	// カラー
	char _alpha;				// α値
	float _horzAlign;			// 水平方向揃え
	float _vertAlign;			// 垂直方向揃え

	float _textHorzAlign;		// テキスト用水平方向揃え

	float _textWidth;			// テキストの横幅
	float _textHeight;			// テキストの縦幅

public:
	CC_SYNTHESIZE_READONLY(std::string, _string, String);					// 表示する文字列
	CC_SYNTHESIZE_RETAIN(agtk::data::FontData*, _fontData, FontData);		// フォントデータ
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _lineNodeList, LineNodeList);	// 各行ごとのノード一覧
	CC_SYNTHESIZE_RETAIN(cocos2d::RenderTexture*, _renderTex, RenderTex);	// レンダーテクスチャ
	CC_SYNTHESIZE_RETAIN(cocos2d::Node*, _baseNode, BaseNode);				// 表示する文字列ノード

	CC_SYNTHESIZE(float, _width, Width);	// 表示幅
	CC_SYNTHESIZE(float, _height, Height);	// 表示幅
	CC_SYNTHESIZE(int, _letterSpace, LetterSpace); // 文字間
	CC_SYNTHESIZE(int, _lineSpace, LineSpace);	// 行間
	CC_SYNTHESIZE(int, _realHeight, RealHeight);	// テキストの実際の高さ
	CC_SYNTHESIZE(bool, _isUpdate, IsUpdate);	// 更新判定
protected:
	TextGui();
	virtual ~TextGui();
	virtual bool init(int fontId, cocos2d::Color3B color, char alpha);

public:
	CREATE_FUNC_PARAM3(TextGui, int, fontId, cocos2d::Color3B, color, char, alpha);
	void updateText(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB);
	void updateText(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB, float scrollY);

	void updateTextRender(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB);
	void updateTextRender(std::string text, int letterSpace, int lineSpace, float width, float height, int marginLR, int marginTB, float scrollY);

	void setAlign(float horz, float vert);
};

//-------------------------------------------------------------------------------------------------------------------
// メッセージウィンドウを表示するノード
class AGTKPLAYER_API MessageWindowNode : public cocos2d::Node
{
private:
	MessageWindowNode();
	virtual ~MessageWindowNode();
	virtual bool init(agtk::data::ObjectCommandMessageShowData* data);
	virtual bool init(agtk::data::ObjectCommandScrollMessageShowData* data);

	// 「無し」ウィンドウ作成
	cocos2d::Size createNone(bool useDefault, int width, int height);
	// テンプレートウィンドウ作成
	cocos2d::Size createTemplate(bool useDefault, int width, int height, int transparency, bool drawFrame, bool colorBlack);
	// 画像ウィンドウ作成
	cocos2d::Size createImage(bool useDefault, int width, int height, int imageId, int transparency);

public:
	CREATE_FUNC_PARAM(MessageWindowNode, agtk::data::ObjectCommandMessageShowData*, data);
	CREATE_FUNC_PARAM(MessageWindowNode, agtk::data::ObjectCommandScrollMessageShowData*, data);
};

//-------------------------------------------------------------------------------------------------------------------
// オブジェクトのパラメータ表示UI
class AGTKPLAYER_API ObjectParameterUi : public Gui
{
protected:
	ObjectParameterUi();
	virtual ~ObjectParameterUi();

protected:
	
	CC_SYNTHESIZE(agtk::data::ObjectAdditionalDisplayData *, _displayData, DisplayData); // 表示データ
	agtk::data::PlayVariableData * getPlayVariableData(bool UseParentVariable, int _variableObjectId, int _variableId); // 表示対象の変数データを取得
};

//-------------------------------------------------------------------------------------------------------------------
// テキストタイプのオブジェクトのパラメータ表示UI
class AGTKPLAYER_API ObjectParameterTextUi : public ObjectParameterUi
{
public:
	CC_SYNTHESIZE_RETAIN(agtk::TextGui*, _textGui, TextGui);

private:
	ObjectParameterTextUi();
	virtual ~ObjectParameterTextUi();
	virtual bool init(agtk::Object* targetObject, agtk::data::ObjectAdditionalDisplayData * displayData);

public:
	CREATE_FUNC_PARAM2(ObjectParameterTextUi, agtk::Object *, targetObject, agtk::data::ObjectAdditionalDisplayData * , displayData);
	virtual void update(float delta);
	virtual void remove();
};

//-------------------------------------------------------------------------------------------------------------------
// ゲージタイプのオブジェクトのパラメータ表示UI
class AGTKPLAYER_API ObjectParameterGaugeUi : public ObjectParameterUi
{
private:
	static const float DEFAULT_WIDTH;	// ゲージの基本の横幅
	static const float DEFAULT_HEIGTH;	// ゲージの基本の縦幅

	float width;	// 横幅
	float height;	// 縦幅

	float value;	// 値
	float valueMax; // 値最大値

	cocos2d::Color4F _color;	// カラー
	cocos2d::Color4F _bgColor;	// 背景カラー

private:
	CC_SYNTHESIZE_READONLY(cocos2d::DrawNode *, _drawNode, DrawNode);
	CC_SYNTHESIZE(int, _addOrder, AddOrder);

private:
	ObjectParameterGaugeUi();
	virtual ~ObjectParameterGaugeUi();
	virtual bool init(agtk::Object* targetObject, agtk::data::ObjectAdditionalDisplayData * displayData);

public:
	CREATE_FUNC_PARAM2(ObjectParameterGaugeUi, agtk::Object *, targetObject, agtk::data::ObjectAdditionalDisplayData *, displayData);
	virtual void update(float delta);
	virtual void remove();

	friend class DummyParameterGaugeUi;
};

//-------------------------------------------------------------------------------------------------------------------
// ゲージタイプのダミー用のパラメータ表示UI
class AGTKPLAYER_API DummyParameterGaugeUi : public cocos2d::Node
{
private:
	float width;	// 横幅
	float height;	// 縦幅

	float value;	// 値
	float valueMax; // 値最大値

	cocos2d::Color4F _color;	// カラー
	cocos2d::Color4F _bgColor;	// 背景カラー

	cocos2d::Vec2 _scale;
	cocos2d::Vec2 _adjustPos;
	float _rotation;

private:
	CC_SYNTHESIZE_READONLY(cocos2d::DrawNode *, _drawNode, DrawNode);
	agtk::Player *_player;//プレイヤー

private:
	DummyParameterGaugeUi();
	virtual ~DummyParameterGaugeUi();
	virtual bool init(agtk::Player* player, agtk::ObjectParameterGaugeUi *gaugeUi);

public:
	CREATE_FUNC_PARAM2(DummyParameterGaugeUi, agtk::Player *, player, agtk::ObjectParameterGaugeUi *, gaugeUi);
	virtual void update(float delta);
};

//-------------------------------------------------------------------------------------------------------------------
// 「表示するテキスト」で表示するテキストUI
class AGTKPLAYER_API ActionCommandMessageTextUi : public Gui
{
private:
	float _duration300;	// 残り表示時間
	int _actionId;		// 表示開始時のアクションID
	bool _waitOneFrame;	// 1フレーム待機するフラグ(生成後に更新が呼ばれる為、キー入力判定が即座に行われてしまうのを回避する用)

	agtk::Object* _lockObject;	// ロック対象オブジェクト

public:
	CC_SYNTHESIZE_RETAIN(agtk::TextGui*, _textGui, TextGui);
	CC_SYNTHESIZE_RETAIN(agtk::MessageWindowNode*, _messageWindowNode, MessageWindowNode);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandMessageShowData*, _data, Data); // 表示データ
	agtk::Object *getLockObject() { return _lockObject; };

private:
	ActionCommandMessageTextUi();
	virtual ~ActionCommandMessageTextUi();
	virtual bool init(agtk::Object* targetObject, agtk::Object* lockObject, agtk::data::ObjectCommandMessageShowData* showData);
	agtk::data::PlayVariableData* getPlayVariableData(); //　表示対象の変数を表示
	void createObject(bool isInit);
	void createMessageTextUi();
public:

	CREATE_FUNC_PARAM3(ActionCommandMessageTextUi, agtk::Object*, targetObject, agtk::Object*, lockObject, agtk::data::ObjectCommandMessageShowData*, showData);
	virtual void update(float delta);
	virtual void remove();
	virtual bool checkRemove(agtk::Object* object);
	virtual bool getObjectStop();
	virtual bool getGameStop();

private:
};

//-------------------------------------------------------------------------------------------------------------------
// 「スクロールメッセージを設定」で表示するテキストUI
class AGTKPLAYER_API ActionCommandScrollMessageTextUi : public Gui
{
private:
	int _actionId;	// 表示開始時のアクションID

	agtk::Object* _lockObject;	// ロック対象オブジェクト

	float _scrollY; // スクロール値
	float _scrollYMax;

	bool _isAllowScrollSpeedUp;//スクロール速度アップの許可フラグ(スクロールメッセージが生成される前からキーが押されている場合にそのまま加速してしまわないようにする為用)
public:
	CC_SYNTHESIZE_RETAIN(agtk::TextGui*, _textGui, TextGui);
	CC_SYNTHESIZE_RETAIN(agtk::MessageWindowNode*, _messageWindowNode, MessageWindowNode);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandScrollMessageShowData*, _data, Data); // 表示データ
	agtk::Object *getLockObject() { return _lockObject; };

private:
	ActionCommandScrollMessageTextUi();
	virtual ~ActionCommandScrollMessageTextUi();
	virtual bool init(agtk::Object* targetObject, agtk::Object* lockObject, agtk::data::ObjectCommandScrollMessageShowData* showData);
	void createObject(bool isInit);

public:
	CREATE_FUNC_PARAM3(ActionCommandScrollMessageTextUi, agtk::Object*, targetObject, agtk::Object*, lockObject, agtk::data::ObjectCommandScrollMessageShowData*, showData);
	virtual void update(float delta);
	virtual void remove();
	virtual bool checkRemove(agtk::Object* object);
	virtual bool getObjectStop();
	virtual bool getGameStop();
};


//-------------------------------------------------------------------------------------------------------------------
// テキストの行ごとに表示するノード
class AGTKPLAYER_API TextLineNode : public cocos2d::Node
{
public:
	CC_SYNTHESIZE_RETAIN(agtk::data::FontData*, _fontData, FontData);

public:
	TextLineNode();
	virtual ~TextLineNode();
	virtual bool init(std::string str, agtk::data::FontData* fontData, int letterSpace, int iniSize, const cocos2d::Color3B &iniColor, int *pSize, cocos2d::Color3B *pColor);

	bool isZenkaku(std::string str);

public:
	CREATE_FUNC_PARAM7(TextLineNode, std::string, str, agtk::data::FontData*, fontData, int, letterSpace, int, iniSize, const cocos2d::Color3B &, iniColor, int *, pSize, cocos2d::Color3B *, pColor);

private:
	CC_SYNTHESIZE_READONLY(float, _width, Width);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _nodeList, NodeList);
};

NS_AGTK_END

#endif	//__GUI_H__
