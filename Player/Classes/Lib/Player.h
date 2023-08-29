#ifndef __PLAYER_H__
#define	__PLAYER_H__

#include "Lib/Macros.h"
#include "Lib/Player/BasePlayer.h"
#include "Lib/Player/ImagePlayer.h"
#include "Lib/Player/GifPlayer.h"
#include "Lib/Player/SpinePlayer.h"
#include "Lib/Player/SSPlayer.h"
#include "Lib/RenderTexture.h"
#include "Data/AnimationData.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
/**
* @brief オブジェクト（フィルター効果、無敵、被ダメージ）に表示する画像を管理するクラス。
*/
class AGTKPLAYER_API PlayerSprite : public cocos2d::Sprite
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
			this->setEndFunc([&](){
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
	PlayerSprite();
	virtual ~PlayerSprite();
public:
	static PlayerSprite *create(int imageId, int opacity, float seconds = 0.0f);
	virtual void update(float delta);
private:
	virtual bool init(int imageId, int opacity, float seconds);
private:
	CC_SYNTHESIZE(int, _imageId, ImageId);
	CC_SYNTHESIZE_RETAIN(OpacityTimer *, _opacityTimer, OpacityTimer);
};

//-------------------------------------------------------------------------------------------------------------------
/**
 * @brief モーションアニメーションを管理するクラス。
 */
class AGTKPLAYER_API Player : public cocos2d::Node
{
public:
	class CenterNode : public cocos2d::Node {
	private:
		CenterNode() : cocos2d::Node() {
			_bFirstSetPosition = true;
		}
	public:
		CREATE_FUNC(CenterNode);
	private:
		virtual bool init() { return Node::init(); }
	public:
		virtual void setPosition(const Vec2 &position) {
			CenterNode::setPosition(position.x, position.y);
		}
		virtual void setPosition(float x, float y) {
			if (_bFirstSetPosition) {
				_bFirstSetPosition = false;
				_oldPosition = cocos2d::Vec2(x, y);
			}
			else {
				_oldPosition = this->getPosition();
			}
			Node::setPosition(x, y);
		}
		void update(float dt) {
			if (_animePositionFlag) {
				_animePositionFlag = false;
				return;
			}
			this->setPosition(this->getPosition());
		}
	private:
		CC_SYNTHESIZE(cocos2d::Vec2, _oldPosition, OldPosition);
		CC_SYNTHESIZE(bool, _animePositionFlag, AnimePositionFlag);
		bool _bFirstSetPosition;
	};
public:
	enum EnumCollidedBit {
		kCollidedBitNone = 0x00,//無し
		kCollidedBitUp = 0x01,//上
		kCollidedBitLeft = 0x02,//左
		kCollidedBitRight = 0x04,//右
		kCollidedBitDown = 0x08,//下
		kCollidedBitAll = 0x0f,//全て（上左右下）
	};
	enum {
		kInvalidResourceSetId = -2
	};
private:
	//シェーダー情報
	class ShaderInfo : public cocos2d::Ref {
	public:
		ShaderInfo() {
			_kind = Shader::kShaderDefault;
			_shaderValue = nullptr;
		}
		virtual ~ShaderInfo() {
			CC_SAFE_RELEASE_NULL(_shaderValue);
		}
	public:
		CREATE_FUNC_PARAM(ShaderInfo, agtk::Shader *, shader);
	protected:
		virtual bool init(agtk::Shader *shader) {
			setKind(shader->getKind());
			setShaderValue(agtk::ShaderValue::create(0.0f, true));
			*this->_shaderValue = *shader->getValue();
			if (getKind() == Shader::kShaderColorRgba) {
				setRgbaColor(shader->getShaderRgbaColorBase());
			}
			return true;
		}
	public:
		void apply(agtk::Shader *shader) {
			if (getKind() == shader->getKind()) {
				*shader->getValue() = *_shaderValue;
				if (getKind() == Shader::kShaderColorRgba) {
					shader->setShaderRgbaColor(getRgbaColor());
				}
			}
		}
	private:
		CC_SYNTHESIZE(Shader::ShaderKind, _kind, Kind);
		CC_SYNTHESIZE_RETAIN(agtk::ShaderValue *, _shaderValue, ShaderValue);
		CC_SYNTHESIZE(cocos2d::Color4B, _rgbaColor, RgbaColor);
	};

private:
	Player();
	virtual ~Player();
public:
	CREATE_FUNC_PARAM(Player, agtk::data::AnimationData *, animationData);
	virtual void update(float dt);
	virtual void play(int actionNo, int actionDirectNo, bool bTakeoverFrames = false, bool bIgnoredSound = false);
	void setResourceSetId(int resourceSetId);
	void setResourceSetName(string resourceSetName);
	int getResourceSetIdByName(string resourceSetName);

	// 現在再生中のアニメを再生し続ける
	void continuePlayCurrentAnime();

	// 再生し続けているアニメを停止する停止する
	void stopAnimeContinuePlaying();

private:
	virtual bool init(agtk::data::AnimationData *animationData);
	void createBasePlayer(agtk::data::ResourceInfoData *resourceInfoData, int imageId);
	void removeBasePlayer();
	cocos2d::Vec2 calcAnchorPoint(cocos2d::Size& size);
public:
	cocos2d::Vec2 addPosition(const cocos2d::Vec2 add);
	virtual void setPosition(const Vec2 &position, bool fromPhysics = false);
	virtual void setPosition(float x, float y, bool fromPhysics = false);
	virtual void setRotation(float rotation);
	virtual const Vec2& getPosition() const;
	float addRotation(float rotation);
	virtual float getRotation() const;
	virtual void setScale(float x, float y);
	virtual float getScaleX() const;
	virtual float getScaleY() const;
	void setCenterRotation(float rotation);
	float getCenterRotation();
	float getAutoGenerationRotation();
	bool isAutoGeneration();
	bool isFlipY();
	cocos2d::Vec2 getDispPosition();
	cocos2d::Vec2 getCenterNodePosition();
	cocos2d::Vec2 getCenterNodePosition2();
protected:
	cocos2d::Vec2 getCenterOffset();
	cocos2d::Vec2 getCenterPositionForSpriteStudio();
public:
	agtk::BasePlayer *getBasePlayer() { return _basePlayer; };
	agtk::Shader *setShader(Shader::ShaderKind kind, float value, float seconds = 0.0f);
	agtk::Shader *setShader(Shader::ShaderKind kind, float value, float seconds, float timer);
	agtk::Shader *setShader(Shader::ShaderKind kind, agtk::ShaderValue *shaderValue);
	agtk::Shader *setShader(Shader::ShaderKind kind, const rapidjson::Value &shaderData);
	void removeShader(Shader::ShaderKind kind, float seconds = 0.0f);
	void removeAllShader();
	agtk::Shader *getShader(Shader::ShaderKind kind);
	void setExecActionSprite(int imageId, int opacity = 255, float seconds = 0.0f);
	void removeExecActionSprite(float seconds);
	int getResourceInfoId(int actionNo, int actionDirectNo);
	int getCurrentResourceInfoId();
	bool compareResourceInfo(int actionNo, int actionDirectNo);
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	void setVisibleTimeline(agtk::data::TimelineInfoData::EnumTimelineType type, bool visible);
	void setVisibelTimelineAll(bool visible);
#endif
	int getCurrentActionNo();
	int getCurrentDirectionNo();
	bool getTimelineBackside(int id);
	bool getTimelineValid(int id);
	agtk::data::DirectionData *getCurrentDirectionData();
public:
	bool convertToLayerSpaceTimelineVertex4(int id, agtk::Vertex4 &vertex4, EnumCollidedBit bit = kCollidedBitNone, int dot = 0);
	void convertToLayerSpaceTimelineRect(int id, cocos2d::Rect &rect);
	void convertToLayerSpaceTimelineVertex4List(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List, EnumCollidedBit bit = kCollidedBitNone, int dot = 0);
	void convertToLayerSpaceTimelineRectList(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<cocos2d::Rect> &rectList);
	cocos2d::Vec2 convertToLayerSpacePosition(cocos2d::Vec2 pos, bool bUpdate = false);
	cocos2d::Vec2 convertToLayerSpacePosition2(cocos2d::Vec2 pos, bool bUpdate = false);
	void getTimelineVertListCache(agtk::data::TimelineInfoData::EnumTimelineType type, std::vector<agtk::Vertex4> &vertex4List, EnumCollidedBit bit = kCollidedBitNone, int dot = 0);
	void updateTimelineVertListCache(agtk::data::TimelineInfoData::EnumTimelineType type, bool wallAreaAttackWhenInvincible = false, EnumCollidedBit bit = kCollidedBitNone, int dot = 0);
	void VertListCacheClear(agtk::data::TimelineInfoData::EnumTimelineType type);
	bool isUpdateTimelineVertListCache(agtk::data::TimelineInfoData::EnumTimelineType type);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void lock();
	void unlock();
#endif

	virtual void visit(Renderer *renderer, const Mat4& parentTransform, uint32_t parentFlags);
private:
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	void createDisplayTimeline();
	void removeDisplayTimelineAll();
	void removeDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType type);
	void updateDisplayTimelineAll();
	void updateDisplayTimeline(agtk::data::TimelineInfoData::EnumTimelineType type);
#endif
	void createAnimationTimelineList();
	bool convertToWorldSpaceTimelineVertex4(agtk::AnimationTimeline *animationTimeline, agtk::Vertex4 &vertex4, EnumCollidedBit bit, int dot);
	cocos2d::Vec2 getTimelinePosition(cocos2d::Rect rect, cocos2d::Size size);
	agtk::Vertex4 getTimelineRectVertex4(cocos2d::Rect rect, cocos2d::Size size);
	agtk::Vertex4 getTimelineRectCenterVertex4(cocos2d::Rect rect, cocos2d::Size size, cocos2d::Vec2 scale);
	int getTimelineCountMax(agtk::data::TimelineInfoData::EnumTimelineType type);
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	PrimitiveNode *getTimelinePrimitiveNode(agtk::data::TimelineInfoData::EnumTimelineType type, int id);
	cocos2d::__Array *getTimelineNodeList(agtk::data::TimelineInfoData::EnumTimelineType type);
#endif
	cocos2d::__Array *getAnimeTimelineList(agtk::data::TimelineInfoData::EnumTimelineType type);
	std::vector<agtk::Vertex4> *getTimelineVertList(agtk::data::TimelineInfoData::EnumTimelineType type);
	cocos2d::Rect *getTimelineRectList(agtk::data::TimelineInfoData::EnumTimelineType type);

	void createRenderTexture();

	virtual const Mat4& getNodeToParentTransform() const;
	void updateTransformCache();

private:
	CC_SYNTHESIZE_RETAIN(agtk::data::AnimationData *, _animationData, AnimationData);
	CC_SYNTHESIZE_RETAIN(CenterNode *, _centerNode, CenterNode);//中心点ノード
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _originNode, OriginNode);//原点ノード
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _autoGenerationNode, AutoGenerationNode);//自動生成ノード
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _autoGenerationCenterNode, AutoGenerationCenterNode);//自動生成用中心ノード
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _visibleCtrlNode, VisibleCtrlNode);//アニメーションタイムライン上の表示コントロール用ノード
	agtk::BasePlayer* _basePlayer;
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _nodePlayer, NodePlayer);
	CC_SYNTHESIZE(BasePlayer::Type, _type, Type);
	CC_SYNTHESIZE_RETAIN(agtk::RenderTextureCtrl *, _renderTexture, RenderTexture);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _removeShaderList, RemoveShaderList);//シェーダー破棄リスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _execActionSpriteList, ExecActionSpriteList);  //オブジェクトのフィルター効果の「画像を表示」で指定された画像スプライトリスト
	CC_SYNTHESIZE(cocos2d::Vec2, _realPosition, RealPosition);	//アニメプレイヤーに設定したcocos2d-x座標系の位置。Nodeの表示位置と異なる場合がある。
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _transformNode, TransformNode);
// #AGTK-NX
#if defined(USE_PREVIEW) || ((CC_TARGET_PLATFORM == CC_PLATFORM_NX) && defined(AGTK_DEBUG))
//#ifdef USE_PREVIEW
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _timelineWallNodeList, TimelineWallNodeList);			//壁判定    ノードリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _timelineHitNodeList, TimelineHitNodeList);			//当たり判定ノードリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _timelineAttackNodeList, TimelineAttackNodeList);		//攻撃判定  ノードリスト
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _timelineConnectNodeList, TimelineConnectNodeList);	//接続点    ノードリスト
#endif
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _animeTimelineWallList, AnimeTimelineWallList);		//壁判定    アニメーションタイムライン
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _animeTimelineHitList, AnimeTimelineHitList);			//当たり判定アニメーションタイムライン
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _animeTimelineAttackList, AnimeTimelineAttackList);	//攻撃判定  アニメーションタイムライン
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _animeTimelineConnectList, AnimeTimelineConnectList);	//接続点    アニメーションタイムライン
	CC_SYNTHESIZE(std::vector<agtk::Vertex4>, _wallVertListCache, WallVertListCache);				// 壁判定    Vertex4リスト
	CC_SYNTHESIZE(std::vector<agtk::Vertex4>, _hitVertListCache, HitVertListCache);					// 当たり判定Vertex4リスト
	CC_SYNTHESIZE(std::vector<agtk::Vertex4>, _attackVertListCache, AttackVertListCache);			// 攻撃判定  Vertex4リスト
	CC_SYNTHESIZE(std::vector<agtk::Vertex4>, _connectVertListCache, ConnectVertListCache);			// 接続点    Vertex4リスト
	CC_SYNTHESIZE(cocos2d::Rect, _wallVertRectCache, WallVertRectCache);							// 壁判定    Rectリスト
	CC_SYNTHESIZE(cocos2d::Rect, _hitVertRectCache, HitVertRectCache);								// 当たり判定Rectリスト
	CC_SYNTHESIZE(cocos2d::Rect, _attackVertRectCache, AttackVertRectCache);						// 攻撃判定  Rectリスト
	CC_SYNTHESIZE(cocos2d::Rect, _connectVertRectCache, ConnectVertRectCache);						// 接続点    Rectリスト

	CC_SYNTHESIZE(bool, _wallAreaAttackFlag, WallAreaAttackFlag);
	CC_SYNTHESIZE(cocos2d::Size, _renderTextureSize, RenderTextureSize);//オフスクリーンテクスチャのサイズ
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _clippingPreAdjustNode, ClippingPreAdjustNode);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _clippingPostAdjustNode, ClippingPostAdjustNode);
	CC_SYNTHESIZE_RETAIN(cocos2d::ClippingRectangleNode *, _clippingRectangleNode, ClippingRectangleNode);

	bool _isContinuePlayingAnime;	// アニメを継続再生中か？
	int _nextActionNo;				// アニメ継続再生中にplay関数で設定されたときに使用する変数
	int _nextActionDirectionNo;		// アニメ継続再生中にplay関数で設定されたときに使用する変数

	Mat4 _parentToWorldTransformCache;		// ワールド座標変換行列キャッシュ
	Mat4 _parentToWorldTransformCache2;
	Mat4 _playerToSceneLayerTransformCache;	// シーンレイヤー座標変換行列キャッシュ
	bool _isUpdateTransformCache;			// 座標変換行列キャッシュの更新フラグ
	mutable bool _updateTransformDirty;		// getNodeToParentTransformの更新フラグ
	CC_SYNTHESIZE(cocos2d::Vec2, _playerScale, PlayerScale);
	CC_SYNTHESIZE(float, _centerRotationOld, CenterRotationOld);	// 一つ前のCenterRotationを保存
	CC_SYNTHESIZE(cocos2d::Vec2, _centerPosition, CenterPosition);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::mutex _mutex;
#endif
	bool _fromPhysics;
	int _resourceSetId;
	int _frameCount;
	//シェーダー処理中のリクエスト(setShader)は、処理中を保持するように（removeShader時に復活）
	CC_SYNTHESIZE(bool, _stockShaderInfoFlag, StockShaderInfoFlag);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _stockShaderInfoList, StockShaderInfoList);
};

NS_AGTK_END

#endif	//__PLAYER_H__
