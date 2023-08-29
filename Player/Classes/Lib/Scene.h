#ifndef __SCENE_H__
#define	__SCENE_H__

#include "Lib/Macros.h"
#include "Lib/Camera.h"
#include "Lib/Shader.h"
#include "Lib/RenderTexture.h"
#include "Lib/Tile.h"
#include "Lib/Course.h"
#include "Lib/Object.h"
#include "Lib/CameraObject.h"
#include "Lib/BaseLayer.h"
#include "Lib/ViewportLight.h"
#include "Data/ProjectData.h"
#include "Data/SceneData.h"
#include "External/collision/CollisionDetaction.hpp"
#include <functional>
#include "Lib/Object.h"
#include "Lib/PhysicsObject.h"
#ifdef USE_SAR_OPTIMIZE_2
#include <map>
#endif

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneVariableTimer : public cocos2d::Ref
{
public:
	enum EnumCountType {
		kCountUp,
		kCountDown,
	};
private:
	SceneVariableTimer();
	virtual ~SceneVariableTimer();
public:
	CREATE_FUNC_PARAM3(SceneVariableTimer, agtk::data::PlayVariableData *, data, EnumCountType, type, agtk::Object *, object);
	void update(float dt);
	void start(double seconds);
	void end() { _duration = -1.0f; };
private:
	virtual bool init(agtk::data::PlayVariableData *data, EnumCountType type, agtk::Object *object);
private:
	CC_SYNTHESIZE(EnumCountType, _countType, CountType);
	CC_SYNTHESIZE_RETAIN(agtk::data::PlayVariableData *, _variableData, VariableData);
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _object, Object);
	float _duration;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneSprite : public cocos2d::Sprite
{
public:
	class OpacityTimer : public agtk::EventTimer
	{
	private:
		OpacityTimer() : agtk::EventTimer() { }
	public:
		CREATE_FUNC_PARAM(OpacityTimer, int, value);
	private:
		virtual bool init(int value) {
			if (agtk::EventTimer::init() == false) {
				return false;
			}
			_value = value;
			_prevValue = value;
			_nextValue = value;
			_oldValue = value;

			this->setProcessingFunc([&](float dt) {
				_oldValue = _value;
				_value = AGTK_LINEAR_INTERPOLATE(_prevValue, _nextValue, _seconds, _timer);
			});
			this->setEndFunc([&]() {
				_oldValue = _value;
				_value = _nextValue;
			});
			return true;
		}
	public:
		int setValue(int value, float seconds = 0.0f) {
			_nextValue = value;
			_prevValue = _value;
			this->start(seconds);
			return _value;
		}
		int getValue() { return _value; };
		bool isChanged() { return _value != _oldValue ? true : false; };
	private:
		int _value;
		int _nextValue;
		int _prevValue;
		int _oldValue;
	};
private:
	SceneSprite();
	virtual ~SceneSprite();
public:
	static SceneSprite *create(int imageId, int opacity, float seconds = 0.0f);
	virtual void update(float delta);
private:
	virtual bool init(int imageId, int opacity, float seconds);
private:
	CC_SYNTHESIZE(int, _imageId, ImageId);
	CC_SYNTHESIZE_RETAIN(OpacityTimer *, _opacityTimer, OpacityTimer);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneBackgroundSprite : public cocos2d::Sprite
{
public:
	static SceneBackgroundSprite *createWithTexture(cocos2d::Texture2D *texture);
public:
	virtual void setPosition(const Vec2 &position);
	virtual void setPosition(float x, float y);
	virtual const Vec2& getPosition() const;
private:
	CC_SYNTHESIZE(cocos2d::Vec2, _realPosition, RealPosition);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneBackground : public cocos2d::Node
{
private:
	SceneBackground();
	virtual ~SceneBackground();
public:
	CREATE_FUNC_PARAM(SceneBackground, agtk::Scene *, scene);
	virtual void update(float delta);
	void updateRenderer(float delta, cocos2d::Mat4 *viewMatrix);
	void createShader(cocos2d::Layer *layer);
	void setShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	agtk::Shader *getShader(Shader::ShaderKind kind);
	void removeShader(Shader::ShaderKind kind, float seconds = 0.0f);
	void createRenderTexture();
	void createObjectCommandRenderTexture();
	bool isRemoveRenderTexture();
	void removeRenderTexture();
	void createSceneSprite(int imageId, int opacity, cocos2d::Vec2 pos, float seconds);
	void removeSceneSprite(float seconds);
	void setMovePosition(const cocos2d::Vec2 &position);
	void adjustBackgroundSpritePosition();
private:
	virtual bool init(agtk::Scene *scene);
	cocos2d::Sprite *createSpriteBackground(cocos2d::Texture2D *texture2d);
private:
	agtk::Scene *_scene;
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _spriteBackgroundList, SpriteBackgroundList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _sceneSpriteList, SceneSpriteList);
	CC_SYNTHESIZE(cocos2d::Size, _sceneSize, SceneSize);
	CC_SYNTHESIZE(cocos2d::Size, _sceneBackgroundSize, SceneBackgroundSize);
	CC_SYNTHESIZE_RETAIN(agtk::Shader *, _shader, Shader);
	CC_SYNTHESIZE_RETAIN(agtk::RenderTextureCtrl *, _renderTexture, RenderTexture);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _removeShaderList, RemoveShaderList);
	unsigned char *_textureBuffer;
	CC_SYNTHESIZE_RETAIN(agtk::GifAnimation *, _gifAnimation, GifAnimation);	//シーン背景画像に設定したGIFアニメ(エディターでは背景画像にGIFを指定できない)
	cocos2d::Vec2 _sprite0Position;
	CC_SYNTHESIZE(int, _baseLayerZOrder, BaseLayerZOrder);
	CC_SYNTHESIZE(cocos2d::Vec2, _lastMovePosition, LastMovePosition);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneTopMostSprite : public cocos2d::Sprite
{
public:
	static SceneTopMostSprite *createWithTexture(cocos2d::Texture2D *texture);
public:
	virtual void setPosition(const Vec2 &position);
	virtual void setPosition(float x, float y);
	virtual const Vec2& getPosition() const;
private:
	CC_SYNTHESIZE(cocos2d::Vec2, _realPosition, RealPosition);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneTopMost : public cocos2d::Node
{
private:
	SceneTopMost();
	virtual ~SceneTopMost();
public:
	CREATE_FUNC_PARAM(SceneTopMost, agtk::Scene *, scene);
	virtual void update(float delta);
	void updateRenderer(float delta, cocos2d::Mat4 *viewMatrix);
	void innerUpdateRenderer(float delta, cocos2d::Mat4 *viewMatrix, int fromLayerId, int toLayerId, bool bBackgroundLayer, agtk::RenderTextureCtrl *renderTextureCtrl);
	void updateRenderer(float delta, cocos2d::Mat4 *viewMatrix, int fromLayerId, int toLayerId);
	void updateRendererFront(float delta, cocos2d::Mat4 *viewMatrix, int fromLayerId, int toLayerId);
	void updateRendererIgnoreMenu(float delta, cocos2d::Mat4 *viewMatrix);
	void updateRendererOnlyMenu(float delta, cocos2d::Mat4 *viewMatrix);
	void createShader(cocos2d::Layer *layer);
	void createWithMenuShader(cocos2d::Layer *layer);
	void setShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	void setWithMenuShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	agtk::Shader *getShader(Shader::ShaderKind kind);
	agtk::Shader *getWithMenuShader(Shader::ShaderKind kind);
	void removeShader(Shader::ShaderKind kind, float seconds = 0.0f);
	void removeWithMenuShader(Shader::ShaderKind kind, float seconds = 0.0f);
	void createRenderTexture();
	void createWithMenuRenderTexture();
	void createObjectCommandRenderTexture();
	void createObjectCommandWithMenuRenderTexture();
	void createOtherRenderTexture();
	void removeRenderTexture();
	void removeWithMenuRenderTexture();
	void removeOtherRenderTexture();
	void createSceneSprite(int imageId, int opacity, cocos2d::Vec2 pos, float seconds);
	void removeSceneSprite(float seconds);
	void setVisibleRenderTexture(bool visible);
private:
	virtual bool init(agtk::Scene *scene);
private:
	agtk::Scene *_scene;
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _spriteTopMostList, SpriteTopMostList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _sceneSpriteList, SceneSpriteList);
	CC_SYNTHESIZE(cocos2d::Size, _sceneSize, SceneSize);
	CC_SYNTHESIZE_RETAIN(agtk::RenderTextureCtrl *, _renderTexture, RenderTexture);
	CC_SYNTHESIZE_RETAIN(agtk::RenderTextureCtrl *, _renderTextureFront, RenderTextureFront);
	CC_SYNTHESIZE_RETAIN(agtk::RenderTextureCtrl *, _withMenuRenderTexture, WithMenuRenderTexture);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _removeShaderList, RemoveShaderList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _removeWithMenuShaderList, RemoveWithMenuShaderList);
	CC_SYNTHESIZE(int, _baseLayerZOrder, BaseLayerZOrder);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneGravity : public cocos2d::Ref
{
private:
	SceneGravity();
	virtual ~SceneGravity();
public:
	CREATE_FUNC_PARAM(SceneGravity, agtk::data::SceneData *, sceneData);
	virtual void update(float dt);
	void set(float gravity, float rotation, int duration300, bool bDurationUnlimited, bool isContainPhyiscsWorld = true);
private:
	virtual bool init(agtk::data::SceneData *sceneData);
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE(cocos2d::Vec2, _gravity, Gravity);
	CC_SYNTHESIZE(int, _duration300, Duration300);//効果時間（※<0は無制限）
	CC_SYNTHESIZE(float, _rotation, Rotation);//重力角度
	CC_SYNTHESIZE_RETAIN(agtk::TimerFloat *, _timerRotation, TimerRotation);//回転用重力角度
	float _duration;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneWater : public cocos2d::Ref
{
private:
	SceneWater();
	virtual ~SceneWater();
public:
	CREATE_FUNC_PARAM(SceneWater, agtk::data::SceneData *, scenData);
	virtual void update(float dt);
	void set(float water, int duration300, bool bDurationUnlimited);
private:
	virtual bool init(agtk::data::SceneData *sceneData);
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SecenData);
	CC_SYNTHESIZE(float, _water, Water);//水中効果(%)
	CC_SYNTHESIZE(int, _duration300, Duration300);//効果時間（※<0は無制限）
	int _duration;
};

//-------------------------------------------------------------------------------------------------------------------
//! @brief ゲームスピードを扱う
class AGTKPLAYER_API GameSpeed
{
public:

	// 雑多な定義
	enum {
		DURATION_UNLIMITED = -1, // 効果時間無限
	};
	enum State {
		eStateIdle,
		eStateStart,
		eStateUpdate,
		eStateEnd,
	};

public:
	GameSpeed();
	virtual ~GameSpeed();

	//! @brief 更新
	//! @return true:効果時間内または無制限 false:効果時間終了
	bool update(float dt);

	//! @brief 設定
	void set(float gameSpeed);
	void set(float gameSpeed, float duration300);

	float getTimeScale() const;
protected:
	void setTimeScale(float scale);
private:
	float _timeScale;
	float _timeScaleNext;
	float _timeScalePrev;
	CC_SYNTHESIZE(int, _duration300, Duration300);// 効果時間（※<0は無制限）
	CC_SYNTHESIZE(float, _duration, Duration);      // 効果経過時間
	CC_SYNTHESIZE(State, _state, State);
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(bool, _paused, Paused);         // プレビュー機能によるポーズ状態
#endif
};

//! @brief シーン内の GameSpeed を管理する
//! @note 「その他の実行アクション」「ゲームスピードを変更する」アクションが並列に処理される。
//!        対象の設定が同じ場合は単純にあとに実行したものが適用される。
//!        たとえば、対象Aに5秒間速度5倍にした直後3秒間速度3倍にした場合、5秒間速度5倍は適用されない。
//!        ただし対象の設定は違うが対象が結果的にA（同一）であった場合は、Aは3秒間速度3倍のあとに2秒間速度5倍になる。
class AGTKPLAYER_API SceneGameSpeed : public cocos2d::Ref
{
public:

	// ゲームスピード管理対象一覧
	enum Type {
		eTYPE_NONE = -1,
		eTYPE_EFFECT,
		eTYPE_TILE,
		eTYPE_MENU,
		eTYPE_OBJECT,
		eTYPE_TILE_OR_MENU,
	};

	// ゲームスピード管理対象詳細
	struct Target {
		Type _type{ eTYPE_NONE };    // 大雑把なゲームスピード変更対象
		int _targetObjectType{ -1 }; // どういうオブジェクトを対象にするか
		int _targetObjectGroup{ -1 };// オブジェクトグループを対象にする場合の対象のグループ
		int _targetObjectId{ -1 };	// オブジェクト種類を対象にする場合の対象の種類
		int _targetQualifierId{ -1 };
		cocos2d::RefPtr<cocos2d::__Array>  _targetObjectList;// 対象オブジェクトリスト（その瞬間に存在したオブジェクトを対象にする場合に使用）

		bool operator==(const Target& t) const {
			bool equal = true;
			equal &= _type == t._type;
			equal &= _targetObjectType == t._targetObjectType;
			equal &= _targetObjectGroup == t._targetObjectGroup;
			equal &= _targetObjectId == t._targetObjectId;
			equal &= _targetQualifierId == t._targetQualifierId;
			if (_targetObjectList && t._targetObjectList) {
				equal &= _targetObjectList->isEqualToArray(t._targetObjectList);
			}
			else {
				equal &= (!_targetObjectList && !t._targetObjectList);
			}
			return equal;
		}
	};

	// 管理データ
	struct Data {
		Target target; // 対象
		GameSpeed gs;  // 対象のスピード
	};

public:

	//! @brief 生成
	CREATE_FUNC_PARAM(SceneGameSpeed, agtk::data::SceneData *, scenData);

public:

	//! @brief 更新
	virtual void update(float df);

	//! @brief ゲームスピード設定
	void setTimeScale(Type type, float gameSpeed) {
		CC_ASSERT(type < eTYPE_OBJECT);
		set(type, -1, -1, -1, -1, nullptr, gameSpeed, _timeScales[type].gs.getDuration300());
	}
	void set(Type type, float gameSpeed, float duration) 
	{
		CC_ASSERT(type < eTYPE_OBJECT);
		set(type, -1, -1, -1, -1, nullptr, gameSpeed, duration);
	}
	void set(Type type, int targettingType, int targetObjectGroup, int targetObjectId, int targetQualifierId, cocos2d::RefPtr<cocos2d::Array> targetObjList,float gameSpeed, float duration);

	void set(const Target& t, const GameSpeed& gs);

	//! @brief ゲームスピードを返す
	float getTimeScale(agtk::Object const *object)const;
	float getTimeScale(Type type, agtk::Object const * object = nullptr)const;

	//! @brief  該当オブジェクトを対象から除外
	void removeObject(agtk::Object* object);

#ifdef USE_PREVIEW
	//! @brief 一時停止
	void setPaused(bool pause);
#endif

private:
	//! @brief コンストラクタ
	SceneGameSpeed();

	//! @brief デストラクタ
	virtual ~SceneGameSpeed();

	//! @brief 初期化
	virtual bool init(agtk::data::SceneData *sceneData);

	//! @brief 対象データを返す
	SceneGameSpeed::Data* find(const Target& target);

	//! @brief 削除
	void remove(const Data& data);

private:

	std::vector<Data> _timeScales; // 管理データ。後ろにあるものほど新しく追加されたもの。
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API SceneShake : public cocos2d::Ref
{
public:
	class ShakeTimer : public agtk::EventTimer
	{
	private:
		ShakeTimer() : agtk::EventTimer() { }
	public:
		CREATE_FUNC_PARAM(ShakeTimer, float, value);
	private:
		virtual bool init(float value) {
			if (agtk::EventTimer::init() == false) {
				return false;
			}
			_value = value;
			_prevValue = value;
			_nextValue = value;
			_oldValue = value;
			this->setProcessingFunc([&](float dt) {
				_oldValue = _value;
				_value = AGTK_LINEAR_INTERPOLATE(_prevValue, _nextValue, _seconds, _timer);
			});
			this->setEndFunc([&]() {
				_oldValue = _value;
				_value = _nextValue;
			});
			return true;
		}
	public:
		int setValue(float value, float seconds = 0.0f) {
			_nextValue = value;
			_prevValue = _value;
			this->start(seconds);
			return _value;
		}
		float getValue() { return _value; };
		bool isChanged() { return _value != _oldValue ? true : false; };
	private:
		float _value;
		float _nextValue;
		float _prevValue;
		float _oldValue;
	};
public:
	enum EnumFadeType {
		kFadeTypeNone,//なし
		kFadeTypeFadeNone,//フェードなし
		kFadeTypeFadeIn,//フェードイン
		kFadeTypeFadeOut,//フェードアウト
		kFadeTypeFadeInOut,//フェードイン・フェードアウト
		kFadeTypeMax,
	};
	enum EnumState {
		kStateIdle,
		kStateStart,
		kStateShaking,
		kStateEnd,
		kStateMax,
	};
private:
	SceneShake();
	virtual ~SceneShake();
public:
	CREATE_FUNC_PARAM(SceneShake, agtk::data::SceneData *, scenData);
	virtual void update(float df);
	void start(agtk::data::ObjectCommandSceneShakeData *objCommand);
	void stop();
	bool isShaking() { return _state != kStateIdle; }
private:
	virtual bool init(agtk::data::SceneData *sceneData);
	void updateFadeNone(float df);
	void updateFadeIn(float df);
	void updateFadeOut(float df);
	void updateFadeInOut(float df);
private:
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE(EnumFadeType, _fadeType, FadeType);
	CC_SYNTHESIZE(EnumState, _state, State);
	CC_SYNTHESIZE_RETAIN(agtk::data::ObjectCommandSceneShakeData *, _objCommand, ObjCommand);
	CC_SYNTHESIZE(cocos2d::Vec2, _moveXY, MoveXY);
	CC_SYNTHESIZE_RETAIN(ShakeTimer *, _shakeX, ShakeX);
	CC_SYNTHESIZE_RETAIN(ShakeTimer *, _shakeY, ShakeY);
	bool _flipX;
	bool _flipY;
	float _duration;
};

//-------------------------------------------------------------------------------------------------------------------
using DetectWallCollisionFunction = std::function<void(CollisionNode*)>;
class AGTKPLAYER_API SceneLayer : public cocos2d::Node
{
public:
	enum EnumType {
		kTypeScene,
		kTypeMenu,
		kTypeMax,
		kTypeAll = kTypeMax,
	};
private:
	SceneLayer();
	virtual ~SceneLayer();
public:
	static SceneLayer *create(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, bool isIgnoreCreateSameStartPointObj, bool bBlendAdditive, int startPointGroupIdx = 0);
	static SceneLayer *createMenu(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, cocos2d::Size sceneSize, bool bBlendAdditive);
	virtual void update(float delta);
#ifdef USE_REDUCE_RENDER_TEXTURE
	virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
#endif

	//オブジェクトの優先度を更新する
	void updateObjectPriority();
	void updateObjectDispPriority();

	void earlyUpdate(float delta);

	//カメラの座標更新後に呼び出される更新関数
	void lateUpdate(float delta);

	void end();

	// 上下のループ処理
	void loopVertical(bool fixedCamera);
	// 左右のループ処理
	void loopHorizontal(bool fixedCamera);

	void createShader(cocos2d::Layer *layer);
	void updateShader();
	void setShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	agtk::Shader *getShader(Shader::ShaderKind kind);
	void removeShader(Shader::ShaderKind kind, float seconds = 0.0f);
	bool isExist();
	bool isDisplay();
	void addCollisionDetection();
	void addCollisionDetaction(agtk::Object *object);
	void addObject(agtk::Object *object);
	void insertObject(agtk::Object *object, agtk::Object *targetObject);
	void removeObject(agtk::Object *object, bool bIgnoredReappearCondition = false, unsigned int removeOption = 0, bool bIgnoredRemoveObjectList = true);
	virtual void setScale(float scaleX, float scaleY);

	int publishObjectId(); // オブジェクトIDを発行
	void createSceneSprite(int imageId, int opacity, cocos2d::Vec2 pos, float seconds = 0.0f);
	void removeSceneSprite(float seconds = 0.0f);

	// 衝突するタイルリスト取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::vector<agtk::Tile *> getCollisionTileList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec = Vec2::ZERO);
	std::vector<agtk::Tile *> getCollisionTileOverlapList(cocos2d::Point boundMin, cocos2d::Point boundMax);
	std::vector<agtk::Tile *> getCollisionTileOverlapMaskList(cocos2d::Point boundMin, cocos2d::Point boundMax);
	std::vector<agtk::Tile *> getCollisionTileList(agtk::Vertex4 &v);
	std::vector<agtk::Tile *> getCollisionTile2List(agtk::Vertex4 &v);
#else
	cocos2d::__Array *getCollisionTileList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec = Vec2::ZERO);
	cocos2d::__Array *getCollisionTileOverlapList(cocos2d::Point boundMin, cocos2d::Point boundMax);
	cocos2d::__Array *getCollisionTileOverlapMaskList(cocos2d::Point boundMin, cocos2d::Point boundMax);
	cocos2d::__Array *getCollisionTileList(agtk::Vertex4 &v);
	cocos2d::__Array *getCollisionTile2List(agtk::Vertex4 &v);
#endif

	// 衝突する坂リスト取得
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	agtk::MtVector<agtk::Slope *> *getCollisionSlopeList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec);
#else
	cocos2d::__Array *getCollisionSlopeList(cocos2d::Point boundMin, cocos2d::Point boundMax, Vec2 moveVec);
#endif

	// 衝突する360度ループリスト取得取得
	cocos2d::__Array *getCollisionLoopCourseList(cocos2d::Point boundMin, cocos2d::Point boundMax);

	CollisionDetaction* getGroupCollisionDetection(int group)const;
	CollisionDetaction* getGroupRoughWallCollisionDetection(int group)const;
	CollisionDetaction* getGroupWallCollisionDetection(int group)const;

	// 他のシーンからのオブジェクト追加
	agtk::Object *addOtherSceneObject(agtk::Object * object, Vec2 apperPos, agtk::Object *parentObject = nullptr);

	virtual void addChild(Node *child);
	virtual void addChild(Node *child, int localZOrder);
	virtual void addChild(Node *child, int localZOrder, int tag);
	virtual void addChild(Node *child, int localZOrder, const std::string &name);
	virtual void removeChild(Node* child, bool cleanup = true);
	virtual void removeChildByTag(int tag, bool cleanup = true);
	virtual void removeChildByName(const std::string &name, bool cleanup = true);
	void removeRenderTexture();
	agtk::Scene *getScene() { return _scene; }
	int getObjectId() { return objectId; };
	void setIsVisible(bool visible);//表示ON/OFFフラグ
	bool isFadeout();//フェードアウト（非表示状態）
	bool isSlideout();//スライドアウト（画面外にスライドした状態）
	void createReappearObject();
	void updateWallCollision(Object *wallCollisionObject, const DetectWallCollisionFunction& func);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void updateThreadWallCollision(Object *wallCollisionObject, const DetectWallCollisionFunction& func);
#endif
	void updateCascadeOpacityEnabled(Node *parent, bool enabled);
	int getCountChildrenRecursively(cocos2d::Node* rootNode);
	bool isChildrenCountOver(cocos2d::Node *node, int &count);

public:
	virtual void setPosition(float x, float y);
	virtual void setPosition(cocos2d::Vec2 &position, float duration);
	void setFade(float alpha, float duration, bool bRemove = false);
	void reappearObjectByAction(int objectId);//アクションによるオブジェクトの復活
	void enableObjectByAction(int objectId);//アクションによるオブジェクトの有効化
	bool isMenuObjectStop();
private:
	virtual bool init(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, bool isIgnoreCreateSameStartPointObj, bool bBlendAdditive, int startPointGroupIdx);
	virtual bool initMenu(agtk::Scene *scene, agtk::data::SceneData *sceneData, int layerId, cocos2d::Size sceneSize, bool bBlendAdditive);
	void initChildren();
	bool initTileMapList(agtk::data::SceneData *sceneData, agtk::data::LayerData *layerData, bool isMemuLayer, cocos2d::Size *memuSize = nullptr);	// タイルマップ初期化
	bool initObject(agtk::data::SceneData *sceneData, int layerId, bool isMemuLayer, bool isIgnoreCreateSameStartPointObj = false, int startPointGroupIdx = 0);	// オブジェクト初期化
	bool initObjectList(agtk::data::SceneData *sceneData, int layerId, int sceneId, bool isMemuLayer, bool isIgnoreCreateSameStartPointObj = false, int startPointGroupIdx = 0);	// オブジェクトリスト初期化
	bool createRenderTexture(bool bBlendAdditive, int zOrder);
	void appearObject(agtk::data::ScenePartObjectData *scenePartObjectData); // オブジェクトの出現
	void reappearObject(agtk::data::ObjectReappearData *reappearData); // オブジェクトの再出現
	agtk::Object *createTakeoverStatesObject(agtk::ObjectTakeoverStatesData *data, agtk::data::ScenePartObjectData *scenePartObjectData);//オブジェクトの再出現
	bool checkNotReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData); // 復活できないオブジェクトか判定
	bool checkCommandReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData); //「アクションで復活」オブジェクトか判定
	bool checkSceneChangeReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData);//「消滅後の復活条件：シーンが切り替わった」オブジェクトか判定
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	agtk::data::ScenePartObjectData *SceneLayer::getInitiallyPlacedScenePartObjectDataForReappearData(agtk::data::ObjectReappearData *reappearData);
#endif
	bool checkObjectAppearCondition(agtk::data::ScenePartObjectData *scenePartObjectData); // オブジェクトの出現条件判定
	bool checkObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData);//シーン終了時の状態を維持するオブジェクトの判定
	agtk::ObjectTakeoverStatesData *getObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData);//シーン終了時の状態を維持するオブジェクトの情報を取得。
	bool checkStartPointObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData);//シーン終了時の状態を維持するオブジェクトの判定
	agtk::ObjectTakeoverStatesData *getStartPointObjectTakeoverStatesAsScene(agtk::data::ScenePartObjectData *scenePartObjectData);//シーン終了時の状態を維持するオブジェクトの情報を取得。
	bool checkObjectDisappearCondition(agtk::Object *object); // オブジェクトの消滅条件判定
	bool checkObjectReappearCondition(agtk::data::ObjectReappearData *reappearData); // オブジェクト消滅後の復活条件判定
	void sortObjectByLocalZOrder(cocos2d::Array* objectList);// Zオーダーを基準にオブジェクトのバブルソートを行う
	void callbackDetectionWallCollision(CollisionNode* collisionObject1, CollisionNode* collisionObject2);
	void removeSceneChangeReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData);
	void removeCommandReappearObject(agtk::data::ScenePartObjectData *scenePartObjectData);
private:
	void _addChild(Node *child, int localZOrder, int tag);
	void _addChild(Node *child, int localZOrder, const std::string &name);

	// 物理関係 -------------------------------------------------------------------------------
public:
	void createPhysicsObjectWithObject(agtk::Object *object);//オブジェクトに紐付いた物理オブジェクトの生成
private:
	// 接続対象設定クラス
	class ConnectTarget : public cocos2d::Ref {
	public:
		ConnectTarget() { _target = nullptr; };
		virtual ~ConnectTarget() { CC_SAFE_RELEASE_NULL(_target); };
		CREATE_FUNC(ConnectTarget);
		bool init() { return true; };
		CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _target, Target);
		CC_SYNTHESIZE(cocos2d::Vec2, _anchor, Anchor);
		CC_SYNTHESIZE(cocos2d::Vec2, _pos, Pos);
	};

	// ロープの接触回避チェック用クラス
	class ConnectRope : public cocos2d::Ref {
	public:
		ConnectRope() { _target = nullptr; };
		virtual ~ConnectRope() { CC_SAFE_RELEASE_NULL(_target); };
		CREATE_FUNC(ConnectRope);
		bool init() { return true; };
		CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _target, Target);
		CC_SYNTHESIZE(cocos2d::Vec2, _prePos, PrePos);
		CC_SYNTHESIZE(cocos2d::Vec2, _endPos, EndPos);
	};

	void createPhysicsObject(agtk::data::SceneData *sceneData);//物理オブジェクトの生成
	cocos2d::Node *createStaticPhysicNode(const cocos2d::Vec2 pos, bool isConvertPos = true);//物理オブジェクトをシーンに固定する用のノードを生成
	cocos2d::Node *getConnectedTarget(int scenePartsId, int connectSubId, cocos2d::__Array* phyiscsObjectList, bool *isObject);//指定のシーンパーツIDの接続対象となるノードを取得
	agtk::PhysicsBase *getConnectedPhysicsPartsFromList(cocos2d::__Array* list, int scenePartsId);//指定のリストから物理パーツを取得
	cocos2d::Node *checkConnectToRope(cocos2d::Node *target, int connectId, int connectIdx, cocos2d::Vec2 *anchor, cocos2d::Vec2 *pos, cocos2d::__Array *physicsObjList, bool* isRope);//ロープと接続しているかチェック
	void checkIgnoreRopeCollision(cocos2d::__Array *ropeList, cocos2d::__Array *ignoreTargetList);//ロープの衝突回避チェック
	void getObjectAnchorAndPos(agtk::Object *object, Vec2 *pos, Vec2 *anchor);// オブジェクトから座標とアンカーポイントを取得
	cocos2d::Vec2 rotateAnchor(Vec2 anchor, float angle);// アンカーを指定の角度に回転させて返す
	void jointTwoConnectTarget(ConnectTarget *pre, ConnectTarget *cur);//二つの接続対象をジョイントする
	void registDrawListForPhysicsPartsWithObjectByDrawPriority(agtk::Object *object, agtk::PhysicsBase *physicBase);//オブジェクトと物理パーツの描画優先度を元にオブジェクトの描画用リストへ登録

	struct PinnedPhysicsInfo {
		agtk::SceneLayer::ConnectTarget *upper;
		agtk::SceneLayer::ConnectTarget *lower;
	};
	void createPinJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object, std::list<PinnedPhysicsInfo> & pinnedPhysicsInfoList);//接着ジョイントの生成
	void createPhysicsJointPin(agtk::SceneLayer::ConnectTarget *upper, agtk::SceneLayer::ConnectTarget *lower);
	void createRopeJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object);//ロープジョイントの生成
	void createSpringJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object);//バネジョイントの生成
	void createAxisJoint(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object);//回転軸ジョイントの生成
	void createExplode(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object);//爆発の生成
	void createForce(agtk::data::PhysicsPartData *physicsPartData, agtk::data::SceneData *sceneData, int layerId, cocos2d::__Array *physicsObjList, agtk::Object *object);//引力・斥力の生成

	void createGroupCollisionDetections(bool isNeedCommon);
	void createGroupRoughWallCollisionDetections();
	void createGroupWallCollisionDetections();

	void setBlendAdditiveRenderTextureSprite();

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	static void objectUpdateWallCollisionThread(ThreadManager::ThreadInfo *threadInfo);
	struct ObjectUpdateThreadInfo {
		int i;							// スレッド番号
		std::vector<Object *> *tmpObjectVector;
		int begin;
		int end;
		float delta;					// delta
		Object *wallCollisionObject;								// updateWallCollisionの引数保存用
		const DetectWallCollisionFunction* detectWallCollisionFunc;	// updateWallCollisionの引数保存用
	};
#endif
public:
#ifdef USE_REDUCE_RENDER_TEXTURE
	// _running変更用Nodeクラス
	class changeRunningNode : public cocos2d::Node {
	public:
		changeRunningNode() {};
		virtual ~changeRunningNode() {};
		CREATE_FUNC(changeRunningNode);
		bool init() { return true; };
		void setRunning(bool running) { _running = running; }
		void insertChildByIndex(Node *child, int localZOrder, int index);
	protected:
		void insertChildHelper(Node* child, int index, int localZOrder, int tag, const std::string &name, bool setTag);
	};

	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _tileMapNode, TileMapNode);
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _tileMapList, TileMapList);
#ifdef USE_REDUCE_RENDER_TEXTURE
	CC_SYNTHESIZE_RETAIN(changeRunningNode *, _objectSetNode, ObjectSetNode);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _objectFrontNode, ObjectFrontNode);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _additiveParticleNode, AdditiveParticleNode);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _additiveParticleBacksideNode, AdditiveParticleBacksideNode);
#endif
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
public:
	CC_SYNTHESIZE(bool, _isObjectListUpdated, IsObjectListUpdated);
	void addIdMap(agtk::Object* object);
	void removeIdMap(agtk::Object* object);
#ifdef USE_SAR_OPTIMIZE_2
	cocos2d::__Array *getObjectAll(int objectId);
	agtk::Object *getObjectAll(int objectId, int instanceId);
#endif
private:
	std::multimap<int, agtk::Object*> _objectIdMap;
	std::map<int, agtk::Object*> _instanceIdMap;
#endif
private:
	agtk::Scene *_scene;
	CC_SYNTHESIZE_RETAIN(agtk::data::LayerData *, _layerData, LayerData);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _objectList, ObjectList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _createObjectList, CreateObjectList);//生成オブジェクトリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _uncreateObjectList, UncreateObjectList); // 未生成オブジェクトリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _deleteObjectList, DeleteObjectList); // 消滅したオブジェクトリスト 

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _portalObjectList, PortalObjectList);//ポータルオブジェクトリスト

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _physicsObjectList, PhysicsObjectList);//物理オブジェクトリスト

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _slopeList, SlopeList);// 配置されている坂リスト

	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _loopCourseList, LoopCourseList);// 配置されている360度ループリスト

	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _groupCollisionDetections, GroupCollisionDetections);
	CC_SYNTHESIZE_RETAIN(CollisionDetaction *, _commonCollisionDetection, CommonCollisionDetection);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _groupRoughWallCollisionDetections, GroupRoughWallCollisionDetections);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _groupWallCollisionDetections, GroupWallCollisionDetections);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CC_SYNTHESIZE(Object *, _wallCollisionObject, WallCollisionObject);
#else
	CC_SYNTHESIZE_RETAIN(Object *, _wallCollisionObject, WallCollisionObject);
#endif
	CC_SYNTHESIZE(const DetectWallCollisionFunction*, _detectWallCollisionFunc, DetectWallCollisionFunc);
	CC_SYNTHESIZE(bool, _wallCollisionInit, WallCollisionInit);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _sceneSpriteList, SceneSpriteList);
	CC_SYNTHESIZE(bool, _blendAdditive, BlendAdditive);
	CC_SYNTHESIZE_RETAIN(agtk::RenderTextureCtrl *, _renderTexture, RenderTexture);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _removeShaderList, RemoveShaderList);//シェーダー破棄リスト
	CC_SYNTHESIZE(bool, _blendAdditiveFlag, BlendAdditiveFlag);
	int objectId;
	CC_SYNTHESIZE(int, _activeFlg, ActiveFlg);//動作ON/OFFフラグ
	CC_SYNTHESIZE_READONLY(bool, _isVisible, IsVisible);//表示ON/OFFフラグ
	CC_SYNTHESIZE(EnumType, _type, Type);//タイプ（シーン、メニュー）
	CC_SYNTHESIZE_RETAIN(agtk::TimerFloat *, _alphaValue, AlphaValue);
	CC_SYNTHESIZE_RETAIN(agtk::TimerVec2 *, _positionValue, PositionValue);
	CC_SYNTHESIZE(bool, _removeSelfFlag, RemoveSelfFlag);
	bool _isFirstCollisionCheck;
	CC_SYNTHESIZE(bool, _isShaderColorDarkMask, IsShaderColorDarkMask);
	CC_SYNTHESIZE(int, _initChildrenCount, InitChildrenCount);
	CC_SYNTHESIZE(int, _menuWorkMargin, MenuWorkMargin);//メニューが非表示になってもオブジェクトを動かす余剰フレーム数。

	// -----------------------------------------------------------------------------
	// 重なり演出用
public:
	void updateRenderer(float delta, cocos2d::Mat4 *viewMatrix, cocos2d::__Array *lowerLayerObjList, bool ignoreVisibleObject);
};

//-------------------------------------------------------------------------------------------------------------------
using DetectWallCollisionFunction = std::function<void(CollisionNode*)>;
class AGTKPLAYER_API Scene : public cocos2d::Node
{
public:
	static const int START_SCENE_ID = -1;//「START」のシーンID
	enum EnumCreateType {
		kCreateTypeNone,
		kCreateTypeScreenFlow,
		kCreateTypePortal,
		kCreateTypeLoad,
	};
	enum EnumPhase {
		kPhaseNone,
		kPhaseStart,
		kPhasePlaying,
		kPhaseStop,
		kPhaseEnd,
	};
private:
	Scene();
	virtual ~Scene();
public:
	static Scene *create(agtk::data::SceneData *sceneData, int startPointGroupIdx, EnumCreateType type = kCreateTypeScreenFlow);
	void start(cocos2d::Layer *layer, bool bSetupScreen = true);
	void stop();
	void end(bool forceGC = true);
	virtual void update(float delta);
	void updateVisit(float delta);
	agtk::RenderTextureCtrl *getRenderTextureCtrl(int layerId);
	agtk::SceneLayer *getSceneLayer(int layerId);
	agtk::SceneLayer *getSceneLayerFront();

// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	cocos2d::__Array *getObjectAllReference(SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax);//全てのオブジェクト配列の参照を取得する。
#endif
	cocos2d::__Array *getObjectAll(SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax);//全てのオブジェクトを取得する。
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	agtk::NrArray *getObjectAllNrArray(SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax);//全てのオブジェクトを取得する。
#endif
	cocos2d::__Array *getObjectAll(int objectId, SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax, int SceneLayerId = -1);//指定オブジェクトIDからオブジェクトを取得する。
	cocos2d::__Array *getObjectAll(cocos2d::Rect rect, SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax, int SceneLayerId = -1);//領域からオブジェクトを取得する。
	cocos2d::__Array *getObjectAllObjGroup(agtk::data::ObjectData::EnumObjGroup type, bool isOperatable = false, SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax);//タイプからオブジェクトを取得する。
	cocos2d::__Array *getObjectAllLocked(int objectId, SceneLayer::EnumType sceneLayerType);// = SceneLayer::kTypeMax);//ロックしたオブジェクトを取得する。
	cocos2d::RefPtr<cocos2d::__Array> getObjectAllFront(SceneLayer::EnumType sceneLayerType, int layerId);// layerId より手前のレイヤーのオブジェクトを取得する。
	std::vector<cocos2d::Node *> getPhysicObjectAll(int isAffectOtherObjects, int isAffectedByOtherObjects, int isFollowConnectedPhysics, SceneLayer::EnumType sceneLayerType = SceneLayer::kTypeMax);

	agtk::BaseLayer *getBaseLayer();
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_3
	agtk::Object *getObjectInstance(int objectId, int instanceId, SceneLayer::EnumType sceneType = SceneLayer::kTypeMax, int sceneLayerId = -1);
#else
	agtk::Object *getObjectInstance(int objectId, int instanceId, SceneLayer::EnumType sceneType = SceneLayer::kTypeMax);
#endif
	agtk::Object *getObjectInstanceByName(int objectId, const char *name);
	virtual void setScale(float scaleX, float scaleY);
	int getObjectInstanceId(int objectId, bool bSceneMenu);
	int getObjectInstanceId(agtk::Object *object);
	int getObjectInstanceId(bool bSceneMenu);
#ifdef USE_PREVIEW
	void setVisibleObjectDebugDisplayArea(bool bVisible);
#endif
#ifdef AGTK_DEBUG
	void setVisibleObjectDebugDisplayPlayer(bool bVisible);
#endif
	agtk::Object *getObjectLocked(int objectId);//指定オブジェクトIDでロックされているオブジェクトを取得する
#ifdef USE_PREVIEW
	cocos2d::__Array *getScenePhysicsObject(int scenePartId); 
#endif

	agtk::OthersCourse *getOthersCourse(int courseId);//指定コースを取得
	void setShader(int layerId, Shader::ShaderKind kind, float value, float seconds = 0.0f);
	void removeShader(int layerId, Shader::ShaderKind kind, float seconds = 0.0f);
	void pauseShader();
	void resumeShader();

	void changeCamera(agtk::CameraObject* cameraObject); // カメラを指定したカメラオブジェクトに切り替える

	agtk::SceneLayer *getMenuLayer(int layerId);

	int getTopPrioritySceneLayer();
	// シーン一時停止用
	CC_SYNTHESIZE(int, _waitDuration300, WaitDuration300);
	void startVariableTimer(agtk::data::PlayVariableData *data, bool bCountUp, double seconds, agtk::Object *object);
	void endVariableTimer(agtk::data::PlayVariableData *data);
	void removeObjectVariableTimer(agtk::Object *object);

	// シーンが持つレンダーテクスチャリストの更新
	void updateSceneRenderTextureList(float delta, bool ignoreVisibleObject = false, int layerId = -1);
	void updateSceneRenderTextureListIgnoreMenu(float delta, bool ignoreVisibleObject = false);
	void updateSceneRenderTextureListOnlyMenu(float delta, bool ignoreVisibleObject = false);
#ifdef USE_PREVIEW
	void showDebugLimitArea(bool bShow);
	void showDebugLimitCamera(bool bShow);
#endif
	int getObjectInstanceCount(int objectId);
	int setObjectInstanceCount(int objectId, int count);
	int incrementObjectInstanceCount(int objectId);
	int decrementObjectInstanceCount(int objectId);
	void updateObjectInstanceCount(int objectId);//シーンに配置された同インスタンス数をオブジェクトに反映。
	void setTakeOverMenuObject(cocos2d::__Array *menuObjectList);
	agtk::Object *getObjectPlayer();
private:
	virtual bool init(agtk::data::SceneData *scene, int startPointGroupIdx, EnumCreateType type);
	void updateVariableTimer(float dt);
	agtk::CameraObject *getInitialCameraObject();	// 初期カメラのオブジェクトを取得

	bool checkChangeCamera(); // カメラ切り替えの確認
	void updateCamera(float dt);// カメラの更新

	// シーンの上下左右を繋げる処理
	void procLoop();

	void createReappearObject();

public:
	// 強制オブジェクトインスタンスID更新
	//! 主にデータロード後に使用
	void forceUpdateObjectInstanceId(int instanceId, bool bMenuScene);

private:
	CC_SYNTHESIZE_RETAIN(cocos2d::Layer *, _layer, Layer);
	CC_SYNTHESIZE_RETAIN(agtk::data::SceneData *, _sceneData, SceneData);
	CC_SYNTHESIZE_RETAIN(agtk::SceneBackground *, _sceneBackground, SceneBackground);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _sceneLayerList, SceneLayerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _menuLayerList, MenuLayerList);
	CC_SYNTHESIZE_RETAIN(agtk::SceneTopMost *, _sceneTopMost, SceneTopMost);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Dictionary *, _renderTextureCtrlList, RenderTextureCtrlList);
	CC_SYNTHESIZE_RETAIN(agtk::Camera *, _camera, Camera);
	CC_SYNTHESIZE_RETAIN(agtk::SceneGravity *, _gravity, Gravity);
	CC_SYNTHESIZE_RETAIN(agtk::SceneWater *, _water, Water);
	CC_SYNTHESIZE_RETAIN(agtk::SceneGameSpeed *, _gameSpeed, GameSpeed);
	CC_SYNTHESIZE_RETAIN(agtk::SceneShake *, _shake, Shake);
	CC_SYNTHESIZE_RETAIN(agtk::ViewportLight *, _viewportLight, ViewportLight);
	CC_SYNTHESIZE(cocos2d::Vec2, _sceneSize, SceneSize);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _variableTimerList, VariableTimerList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array*, _cameraObjectList, CameraObjectList);	// シーン上に配置されているカメラ一覧一覧
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _courseList, CourseList); // シーン上に配置されているコース一覧
	CC_SYNTHESIZE(bool, _sceneCreateSkipFrameFlag, SceneCreateSkipFrameFlag);//シーン遷移時に生成したシーンのスキップフラグ（シーン遷移時のみ使用）

	int _objectInstanceId;//インスタンスID（オブジェクトが作成されるとオブジェクトのインスタンスIDに割り振られる）
	int _objectInstanceMenuId;//インスタンスID（※メニューシーン用）
	CC_SYNTHESIZE(float, _elapsedTime, ElapsedTime);//シーンが開始してからの経過時間
#ifdef USE_PREVIEW
	CC_SYNTHESIZE(int, _previewObjectId, PreviewObjectId);//プレビュー対象のオブジェクトID
	CC_SYNTHESIZE(int, _previewInstanceId, PreviewInstanceId);//プレビュー対象のオブジェクトのインスタンスID
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _watchPhysicsPartIdList, WatchPhysicsPartIdList);
	CC_SYNTHESIZE(bool, _physicsRequestd, PhysicsRequested);
#endif
#ifdef USE_PREVIEW
	CC_SYNTHESIZE_RETAIN(cocos2d::Sprite *, _debugLimitAreaSprite, DebugLimitAreaSprite);
	CC_SYNTHESIZE_RETAIN(cocos2d::Sprite *, _debugLimitCameraSprite, DebugLimitCameraSprite);
#endif
	CC_SYNTHESIZE_RETAIN(agtk::CameraObject*, _currentCameraObject, CurrentCameraObject);// 現在使用中のカメラオブジェクト
	CC_SYNTHESIZE(bool, _cameraUpdated, CameraUpdated);	//シーンが生成されてからupdateCamera()が呼ばれたことがあるかどうか。
	CC_SYNTHESIZE(EnumCreateType, _createType, CreateType);
	CC_SYNTHESIZE(EnumPhase, _scenePhase, ScenePhase);
	CC_SYNTHESIZE(bool, _ignoredUpdateActionFlag, IgnoredUpdateActionFlag);//アクション無効フラグ
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_1
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _sceneObjectListCache, SceneObjectListCache);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _menuObjectListCache, MenuObjectListCache);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _allObjectListCache, AllObjectListCache);
#endif

public:
	CC_SYNTHESIZE(bool, _requestSwitchInit, RequestSwitchInit);//変数・スイッチ初期化要求スイッチ
	CC_SYNTHESIZE(bool, _requestSwitchReset, RequestSwitchReset);//リセット要求スイッチ
	CC_SYNTHESIZE(bool, _requestSwitchSaveFile, RequestSwitchSaveFile);//セーブ要求スイッチ
	CC_SYNTHESIZE(bool, _requestSwitchDeleteFile, RequestSwitchDeleteFile);//削除要求スイッチ
	CC_SYNTHESIZE(bool, _requestSwitchCopyFile, RequestSwitchCopyFile);//コピー要求スイッチ
	CC_SYNTHESIZE(bool, _requestSwitchLoadFile, RequestSwitchLoadFile);//ロード要求スイッチ
public:
	static cocos2d::Vec2 getPositionSceneFromCocos2d(const cocos2d::Vec2 &pos, agtk::data::SceneData const *sceneData);
	static cocos2d::Vec2 getPositionCocos2dFromScene(const cocos2d::Vec2 &pos, agtk::data::SceneData const *sceneData);
	static cocos2d::Vec2 getPositionSceneFromCocos2d(const cocos2d::Vec2 &pos, agtk::Scene *scene = nullptr);
	static cocos2d::Vec2 getPositionCocos2dFromScene(const cocos2d::Vec2 &pos, agtk::Scene *scene = nullptr);
	static cocos2d::Rect getRectSceneFromCocos2d(const cocos2d::Rect &rect, agtk::Scene *scene = nullptr);
	static cocos2d::Rect getRectCocos2dFromScene(const cocos2d::Rect &rect, agtk::Scene *scene = nullptr);
	static float getAngleCocos2dFromScene(float angle);
	static float getAngleSceneFromCocos2d(float angle);
	static float getAngleMathFromScene(float angle);
	static float getAngleSceneFromMath(float angle);
	static float getAngleMathFromCocos2d(float angle);
};

NS_AGTK_END

#endif	//__SCENE_H__
