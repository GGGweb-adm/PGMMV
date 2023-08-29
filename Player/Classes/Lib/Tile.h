#ifndef __TILE_H__
#define	__TILE_H__

#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Data/ProjectData.h"
#include "External/collision/CollisionDetaction.hpp"
#include "Lib/Player/GifPlayer.h"

NS_AGTK_BEGIN
//-------------------------------------------------------------------------------------------------------------------
class SceneLayer;
class Object;
class Camera;
class AGTKPLAYER_API Tile : public cocos2d::Sprite
{
public:
	enum EnumType {
		kTypeTile,
		kTypeLimitTile,// 行動範囲制限用タイル
	};
protected:
	Tile();
	virtual ~Tile();
public:
	static Tile *create(agtk::data::Tile* tileData, cocos2d::Texture2D* texture, agtk::SceneLayer *sceneLayer, cocos2d::Size tileScale);

	virtual void update(float delta);
	void setupPhysicsBody(int wallBit);
	int getWallBit();
	int getGroupBit()const;
	int getGroup()const;
	bool isNeedUpdate();//updateが必要か？
public:
	void convertToWorldSpaceVertex4(agtk::Vertex4 &vertex4);
	void convertToLayerSpaceVertex4(agtk::Vertex4 &vertex4);
	cocos2d::Rect convertToWorldSpaceRect();
	cocos2d::Rect convertToLayerSpaceRect();
	agtk::SceneLayer *getSceneLayer() { return _sceneLayer; }
	void setObjectOverlapped(bool flag, agtk::Object *object = nullptr);
protected:
	virtual bool init(agtk::data::Tile *tileData, cocos2d::Texture2D *texture, agtk::SceneLayer *sceneLayer, cocos2d::Size tileScale);
	bool checkSwitchVariableCondition(cocos2d::__Array *switchVariableConditionList);
	void changeSwitchVariableAssign(cocos2d::__Array *switchVariableAssignList);
	cocos2d::Vec2 getAnchoredPosition();
protected:
	agtk::SceneLayer *_sceneLayer;
	CC_SYNTHESIZE(int, _sceneId, sceneId);
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(int, _tilesetId, TilesetId);
	CC_SYNTHESIZE(int, _x, X);
	CC_SYNTHESIZE(int, _y, Y);
	CC_SYNTHESIZE(int, _tileX, TileX);
	CC_SYNTHESIZE(int, _tileY, TileY);
	CC_SYNTHESIZE_RETAIN(agtk::data::TileData *, _tileData, TileData);
	CC_SYNTHESIZE(cocos2d::Size, _tileSize, TileSize);
	CC_SYNTHESIZE(cocos2d::Size, _tileScale, TileScale);
	CC_SYNTHESIZE(bool, _changeState, ChangeState);
	CC_SYNTHESIZE(float, _delta, Delta);
	CC_SYNTHESIZE_READONLY(bool, _touch, Touch);
	CC_SYNTHESIZE(int, _touchCount, TouchCount);			// このタイルに接触しているオブジェクト数
	CC_SYNTHESIZE(int, _touchCountMax, TouchCountMax);		// このタイルに接触しているオブジェクト最大数
	CC_SYNTHESIZE(bool, _touchTrigger, TouchTrigger);
	CC_SYNTHESIZE_READONLY(bool, _touchRelease, TouchRelease);
	CC_SYNTHESIZE(bool, _touchAttackBox, TouchAttackBox);//攻撃判定が触れているフラグ
	CC_SYNTHESIZE(EnumType, _type, Type);
	struct ObjectOverlapped {
	private:
		bool _flag;
		agtk::Object *_object;
	public:
		ObjectOverlapped();
		virtual ~ObjectOverlapped();
	public:
		void set(bool flag, agtk::Object *object);
		void reset();
		bool flag() { return _flag; };
		agtk::Object *object() { return _object; };
		void setFlag(bool flag) { _flag = flag; };
	};
	ObjectOverlapped _objectOverlapped;
	//CC_SYNTHESIZE(bool, _objectOverlapped, ObjectOverlapped);//オブジェクトが重なったフラグ
	CC_SYNTHESIZE(int, _physicsMargedTileX, PhysicsMargedTileX);//物理オブジェクトがマージ先のタイルX
	CC_SYNTHESIZE(int, _physicsMargedTileY, PhysicsMargedTileY);//物理オブジェクトがマージ先のタイルY
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _debugNode, DebugNode);
	CC_SYNTHESIZE(bool, _removeFlag, RemoveFlag);
	CC_SYNTHESIZE(agtk::data::TilesetData::EnumTilesetType, _tilesetType, TilesetType);
	CC_SYNTHESIZE(int, _beforeChangeGroup, BeforeChangeGroup);	//タイルが変化する前のグループを保持。次のフレームでクリアされる。

	// デバッグ用表示
	void showDebugVisible(bool isShow);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API LimitTile : public Tile
{
private:
	LimitTile();
public:
	CREATE_FUNC_PARAM2(LimitTile, int, layerId, cocos2d::Size, tileSize);
	virtual void update(float delta);
	bool isNeedCheck(agtk::Object const * obj)const;// obj と衝突判定をしていいか
private:
	virtual bool init(int layerId, cocos2d::Size tileSize);
private:
	CC_SYNTHESIZE(agtk::Object const *, _object, Object); // シーンに対する行動範囲制限なら nullptr 、オブジェクトに対する行動範囲制限ならそのオブジェクトのポインタ
	CC_SYNTHESIZE(cocos2d::Vec2, _initPos, InitPos); // 初期の位置。移動前の位置ともいう。
};

/**
 * @brief LimitTile 上下左右をまとめたもの
 */
class AGTKPLAYER_API LimitTileSet{
public:
	//! @brief 上下左右の LimitTile をもった LimitTileSet を作成
	static std::unique_ptr<LimitTileSet> create(int layerId, cocos2d::Rect rect, cocos2d::Size tileSize, agtk::data::SceneData const * sceneData);

	//! @brief カメラに追従するように移動
	void UpdatePosByCamera(agtk::Camera* camera);

	//! @brief スイッチが有効か？（オブジェクト用）
	//! @param existSwtich スイッチが設定されているかどうか
	bool isSwitchEnable(bool* existSwitch = nullptr) const;

	//! @brief 上下左右の LimitTile を返す
	cocos2d::RefPtr<cocos2d::__Array> const & getLimitTileList() const { return _limitTileList; }

	LimitTileSet() {};
private:

	//! @brief 上下左右の LimitTile を設定
	void setLimitTileList(cocos2d::Array* a) { _limitTileList = a; }

private:
	cocos2d::RefPtr<cocos2d::__Array> _limitTileList;// 左右上下の順番の LimitTile
	CC_SYNTHESIZE(agtk::data::SceneData const*, _sceneData, SceneData); 
	CC_SYNTHESIZE(agtk::data::ObjectMoveRestrictionSettingData const *, _settingData, SettingData);
	CC_SYNTHESIZE(cocos2d::Size, _screenSize, ScreenSize);
	CC_SYNTHESIZE(cocos2d::Size, _tileSize, TileSize);
};

/**
 * @brief 同一のオブジェクトに対する LimitTileSet をまとめたもの。
*/
class AGTKPLAYER_API LimitTileSetList {
public:
	//! @brief 生成
	static std::unique_ptr<LimitTileSetList> create();

	//! @brief LimitTileSet の追加
	void add(std::unique_ptr<LimitTileSet>& lts);

	//! @brief 有効な LimitTileSet を返す
	LimitTileSet * getEnableLimitTileSet()const;

	// コピーコンストラクタ禁止
	LimitTileSetList(LimitTileSetList&) = delete;
	// ムーブコンストラクタ
	LimitTileSetList(LimitTileSetList&& ltsl) 
	{
		for (auto& l : ltsl._limitTileSetList){
			_limitTileSetList.push_back(std::move(l));
		}
	}
	// コピー代入演算禁止
	LimitTileSetList& operator=(LimitTileSetList&) = delete;
	// ムーブ代入演算子
	LimitTileSetList& operator=(LimitTileSetList&& ltsl) {
		for (auto& l : ltsl._limitTileSetList) {
			_limitTileSetList.push_back(std::move(l));
		}
		return *this;
	}
private:
	LimitTileSetList() {}
private:
	std::vector<std::unique_ptr<LimitTileSet>> _limitTileSetList;
};
//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Subtile : public cocos2d::Sprite
{
private:
	Subtile();
	virtual ~Subtile();
public:
	CREATE_FUNC_PARAM3(Subtile, agtk::data::Tile *, tile, cocos2d::Texture2D*, texture, int, layerId);
#if 0
	virtual void update(float delta);
#endif
private:
	virtual bool init(agtk::data::Tile* tile, cocos2d::Texture2D* texture, int layerId);
#if 0
	cocos2d::Vec2 getAnchoredPosition();
#endif
private:
	CC_SYNTHESIZE(int, _layerId, LayerId);
	CC_SYNTHESIZE(int, _tilesetId, TilesetId);
	CC_SYNTHESIZE(int, _x, X);
	CC_SYNTHESIZE(int, _y, Y);
	CC_SYNTHESIZE_RETAIN(agtk::data::TileData *, _tileData, TileData);
	CC_SYNTHESIZE(cocos2d::Size, _tileSize, TileSize);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Tileset : public cocos2d::SpriteBatchNode
{
private:
	Tileset();
	virtual ~Tileset();
public:
	CREATE_FUNC_PARAM2(Tileset, data::TilesetData*, tilesetData, agtk::SceneLayer *, sceneLayer);
	virtual void update(float delta);
	bool isNeedUpdate();
private:
	virtual bool init(data::TilesetData* tilesetData, agtk::SceneLayer *sceneLayer);
private:
	agtk::SceneLayer *_sceneLayer;
	CC_SYNTHESIZE_RETAIN(data::TilesetData*, _tilesetData, TilesetData);
	CC_SYNTHESIZE_RETAIN(agtk::GifAnimation *, _gifAnimation, GifAnimation);	//タイルセット画像に設定したGIFアニメ(エディターではタイルセット画像にGIFを指定できない)
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API TileMap : public cocos2d::Node
{
private:
	TileMap();
	virtual ~TileMap();
public:
	CREATE_FUNC_PARAM2(TileMap, cocos2d::Array *, tileList, agtk::SceneLayer *, sceneLayer);
	void addCollisionDetaction(CollisionDetaction *collision);
	virtual void update(float delta);
	void initTileMap(int sceneHorzTileCount, int sceneVertTileCount);
	Tile *getTile(int tileX, int tileY);
	Tile *getTile2(int tileX, int tileY);
	void removeTileReference(int tileX, int tileY, const cocos2d::Size* tileSize);
	void removeTile2Reference(int tileX, int tileY, const cocos2d::Size *tileSize);
	void addTileReference(agtk::Tile *tile);
	cocos2d::__Array *getLimitTileList();

private:
	virtual bool init(cocos2d::__Array *tileList, agtk::SceneLayer *sceneLayer);
	void mergeTilePhysicsBox(cocos2d::__Array *list, const cocos2d::Vec2 maxTilesInScene);
	CC_SYNTHESIZE(int, _tileMapWidth, TileMapWidth);
	CC_SYNTHESIZE(int, _tileMapHeight, TileMapHeiht);
	Tile **_tileMap;
	Tile **_tileMap2;
	agtk::SceneLayer *_sceneLayer;

	std::unique_ptr<LimitTileSet> _limitTileSet;
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _childrenList, ChildrenList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _updatedTilesetList, UpdatedTilesetList);
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _updatedTileList, UpdatedTileList);
};

NS_AGTK_END

#endif	//__TILE_H__
