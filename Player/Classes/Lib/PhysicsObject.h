#ifndef __PHYSICS_OBJECT_H__
#define	__PHYSICS_OBJECT_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Lib/Object.h"
#include "Data/SceneData.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：共通
class AGTKPLAYER_API PhysicsBase : public cocos2d::Node
{
public:
	enum {
		kMenuPhysicsLayerShift = 16,
	};
protected:
	/* 力学の範囲指定 */
	enum RangeType {
		kRangeTypeFan = 0,	// 円形指定
		kRangeTypeBand,		// 矩形指定
	};

	/* 力学データ */
	class ForceData : public cocos2d::Ref {
	public:
		ForceData() { _divMax = 0; _direction = Vec2::ZERO; _strength = 0; _distance = 0; _isConstant = true; _connectTarget = nullptr; };
		virtual ~ForceData() { CC_SAFE_RELEASE_NULL(_connectTarget); };
		CREATE_FUNC_PARAM(ForceData, int, layerId);
		bool init(int layerId) { _layerId = layerId; return true; };

		CC_SYNTHESIZE(int, _layerId, LayerId);//力のレイヤーID
		CC_SYNTHESIZE(int, _divMax, DivMax);//力の分割最大数
		CC_SYNTHESIZE(Vec2, _direction, Direction);//力の方向
		CC_SYNTHESIZE(float, _strength, Strength);//力の強さ
		CC_SYNTHESIZE(float, _distance, Distance);//力の影響する距離
		CC_SYNTHESIZE(bool, _isConstant, IsConstant);//距離に影響しないか？
		CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _connectTarget, ConnectTarget);//連結対象
	};

	// ベースへの接続(接着もしくは回転軸で使用)
	struct ConnectToBase {
		cocos2d::Node*	_dummyNode;	// 生成情報取得用のダミー
		cocos2d::Node*	_node;		// 接着もしくは回転軸のノード
	};
protected:
	PhysicsBase();
	virtual ~PhysicsBase();
public:
	CREATE_FUNC_PARAM(PhysicsBase, agtk::data::SceneData *, sceneData);
protected:
	virtual bool init(agtk::data::SceneData *sceneData);
	cocos2d::Node* createImageNode(int imgId, cocos2d::Node *mask, agtk::data::PhysicsBaseData::EnumPlacement type, float scaling, Vec2 offset, const char* defaultFileName);
	void drawDebugCircleOrFan(int angle, int fanAngle, int radius, const cocos2d::Color4F &color, cocos2d::DrawNode *drawNode);
	void drawDebugRectangle(const Vec2 &direction, float widthHalf, float distance, const cocos2d::Color4F &color, cocos2d::DrawNode *drawNode);
public:
	virtual void update(float dt) override {};
	virtual void clear();
	void setRotationFromTwoVec(Vec2 from, Vec2 to);
	virtual void showDebugVisible(bool isShow) {};
	void forceFollowPhysics();
protected:
	CC_SYNTHESIZE(int, _scenePartsId, ScenePartsId);//シーンパーツID
	CC_SYNTHESIZE(int, _parentScenePartId, ParentScenePartId);
	CC_SYNTHESIZE(bool, _isContacting, IsContacting);//何かに衝突しているフラグ
	CC_SYNTHESIZE(int, _layerId, LayerId);//レイヤーID
	CC_SYNTHESIZE(int, _sceneId, SceneId);//シーンID
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _rootObject, RootObject);//付随元のオブジェクトの参照
	CC_SYNTHESIZE(int, _priority, Priority);//サブ描画優先度
	CC_SYNTHESIZE(int, _dispPriority, DispPriority);//描画優先度
	CC_SYNTHESIZE_RETAIN(agtk::Object *, _followObject, FollowObject);//接続オブジェクト
	CC_SYNTHESIZE_RETAIN(cocos2d::PhysicsBody *, _followerPhysicsBody, FollowerPhysicsBody);//接続物理ボディ
	CC_SYNTHESIZE(std::list<ConnectToBase>, _connectToBaseList, ConnectToBaseList);//ベースへの接続リスト
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：円形
class AGTKPLAYER_API PhysicsDisk : public PhysicsBase
{
private:
	static const int CIRCLE_POINT_MAX = 36;//ポリゴンで描く円用のポイント最大数
protected:
	PhysicsDisk();
	virtual ~PhysicsDisk();
public:
	CREATE_FUNC_PARAM4(PhysicsDisk, agtk::data::PhysicsPartData *, physicsPartsData, agtk::data::SceneData *, sceneData, int, layerId, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
#ifdef USE_PREVIEW
	float getRotationZX() { return _rotationZ_X; }
#endif
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsDiskData *, _physicsData, PhysicsData);//円形の物理データ
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：四角形
class AGTKPLAYER_API PhysicsRectangle : public PhysicsBase
{
protected:
	PhysicsRectangle();
	virtual ~PhysicsRectangle();
public:
	CREATE_FUNC_PARAM4(PhysicsRectangle, agtk::data::PhysicsPartData *, physicsPartsData, agtk::data::SceneData *, sceneData, int, layerId, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
#ifdef USE_PREVIEW
	float getRotationZX(){ return _rotationZ_X; }
#endif
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsRectangleData *, _physicsData, PhysicsData);//四角形の物理データ
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：ロープ部品
class AGTKPLAYER_API PhysicsRopeParts : public PhysicsBase
{
public:
	static const int ContentHeightAddition = 8;
protected:
	const float DIVERGE_RATE = 0.85f;//力の発散割合
	const float MASS_INFLATION_RATE = 7.5f;//ロープの質量水増し率
	PhysicsRopeParts();
	virtual ~PhysicsRopeParts();
public:
	CREATE_FUNC_PARAM5(PhysicsRopeParts, agtk::data::PhysicsPartData *, physicsPartsData, agtk::data::SceneData *, sceneData, int, layerId, int, idx, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int idx, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
	void addContentSizeH(float h);
	void reArangeMass();
protected:
	CC_SYNTHESIZE(int, _idx, Idx);
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsRopeData *, _physicsData, PhysicsData);//ロープの物理データ
	CC_SYNTHESIZE_RETAIN(cocos2d::PhysicsBody *, _ignoreBody, IgnoreBody);//ロープとの衝突を回避する対象のボディ
	CC_SYNTHESIZE(Vec2, _oldPos, OldPos);
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：接着
class AGTKPLAYER_API PhysicsPin : public PhysicsBase
{
protected:
	PhysicsPin();
	virtual ~PhysicsPin();
public:
	CREATE_FUNC_PARAM4(PhysicsPin, agtk::data::PhysicsPartData *, physicsPartsData, agtk::data::SceneData *, sceneData, int, layerId, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsPinData *, _physicsData, PhysicsData);//接着の物理データ
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：ばね
class AGTKPLAYER_API PhysicsSpring : public PhysicsBase
{
protected:
	PhysicsSpring();
	virtual ~PhysicsSpring();
public:
	CREATE_FUNC_PARAM5(PhysicsSpring, agtk::data::PhysicsPartData *, physicsPartsData, agtk::data::SceneData *, sceneData, int, layerId, PhysicsJointSpring *, jointData, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, PhysicsJointSpring *jointData, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
	void setUp(float springLength);
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsSpringData *, _physicsData, PhysicsData);//ばねの物理データ
	PhysicsJointSpring * _jointData;//ジョイントデータ
};
//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：回転軸
class AGTKPLAYER_API PhysicsAxis : public PhysicsBase
{
protected:
	PhysicsAxis();
	virtual ~PhysicsAxis();
public:
	CREATE_FUNC_PARAM4(PhysicsAxis, agtk::data::PhysicsPartData *, physicsPartsData, agtk::data::SceneData *, sceneData, int, layerId, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsPartData *physicsPartsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsAxisData *, _physicsData, PhysicsData);//回転軸の物理データ
	CC_SYNTHESIZE_RETAIN(cocos2d::PhysicsBody *, _axisUpperTarget, AxisUpperTarget);//回転軸の上位対象リスト
	CC_SYNTHESIZE_RETAIN(cocos2d::PhysicsBody *, _axisLowerTarget, AxisLowerTarget);//回転軸の下位対象リスト
	CC_SYNTHESIZE(bool, _isBraking, IsBraking);//ブレーキ中フラグ
	CC_SYNTHESIZE(float, _rotationRate, RotationRate);//回転レート
	CC_SYNTHESIZE(float, _rotateDirectionValue, RotateDirectionValue);//回転方向値(-1 or 1)

	bool _ignoreMortor;//モーター動作OFFフラグ
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：爆発
class AGTKPLAYER_API PhysicsExprosion : public PhysicsBase
{
protected:
	PhysicsExprosion();
	virtual ~PhysicsExprosion();
public:
	CREATE_FUNC_PARAM4(PhysicsExprosion, agtk::data::PhysicsExplosionData *, physicsData, agtk::data::SceneData *, sceneData, int, layerId, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsExplosionData *physicsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
	virtual void showDebugVisible(bool isShow) override;
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsExplosionData *, _physicsData, PhysicsData);//爆発の物理データ
	CC_SYNTHESIZE(bool, _isFan, IsFan);//扇形か？
	CC_SYNTHESIZE(bool, _isActive, IsActive);//爆発中か？
	CC_SYNTHESIZE(bool, _isActiveOnce, IsActiveOnce);//爆発中か？(発動切り替えが無い場合の一度きり発動用)
	CC_SYNTHESIZE(float, _duration300, Duration300);//爆発経過時間
	CC_SYNTHESIZE(cocos2d::Vec2, _connectedAnchor, ConnectedAnchor);//連結座標アンカー
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _connectedTarget, ConnectedTarget);//連結元対象
	CC_SYNTHESIZE(float, _connectedInitAngle, ConnectedInitAngle);//連結元の初期角度
	CC_SYNTHESIZE(float, _forceDistance, ForceDistance);//力の及ぶ距離
};

//-------------------------------------------------------------------------------------------------------------------
//! 物理オブジェクト：引力・斥力
class AGTKPLAYER_API PhysicsAttraction : public PhysicsBase
{
protected:
	static const int ATTRACT_DIRECTION = -1;	// 引力方向
	static const int REPULSIVE_DIRECTION = 1;	// 斥力方向
protected:
	PhysicsAttraction();
	virtual ~PhysicsAttraction();
public:
	CREATE_FUNC_PARAM4(PhysicsAttraction, agtk::data::PhysicsForceData *, physicsData, agtk::data::SceneData *, sceneData, int, layerId, int, parentScenePartId);
protected:
	virtual bool init(agtk::data::PhysicsForceData *physicsData, agtk::data::SceneData *sceneData, int layerId, int parentScenePartId);
public:
	virtual void update(float dt) override;
	virtual void clear();
	virtual void showDebugVisible(bool isShow) override;
protected:
	CC_SYNTHESIZE_RETAIN(agtk::data::PhysicsForceData *, _physicsData, PhysicsData);//引力・斥力の物理データ
	CC_SYNTHESIZE(bool, _isFan, IsFan);//扇形か？
	CC_SYNTHESIZE(cocos2d::Vec2, _connectedAnchor, ConnectedAnchor);//連結座標アンカー
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _connectedTarget, ConnectedTarget);//連結元対象
	CC_SYNTHESIZE(float, _connectedInitAngle, ConnectedInitAngle);//連結元の初期角度
	CC_SYNTHESIZE(bool, _isActiveAttract, IsActiveAttract);//引力発生フラグ
	CC_SYNTHESIZE(bool, _isActiveRepulsive, IsActiveRepulsive);//斥力発生フラグ
	CC_SYNTHESIZE(float, _forceDistance, ForceDistance);//力の及ぶ距離
};

NS_AGTK_END

#endif	//__OBJECT_H__
