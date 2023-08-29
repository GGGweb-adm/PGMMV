#ifndef __COLLISION_H__
#define	__COLLISION_H__

#include "cocos2d.h"
#include "json/document.h"
#include "Lib/Macros.h"
#include "Lib/Common.h"
#include "Manager/PrimitiveManager.h"
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#include "Lib/Slope.h"
#endif
#include "Manager/ThreadManager.h"

#if defined(AGUSA_K)
#define USE_WALL_DEBUG_DISPLAY
#endif

NS_AGTK_BEGIN

extern float TILE_COLLISION_THRESHOLD;

extern void setTileCollisionThreshold(float threshold);

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PolygonShape : public cocos2d::Ref
{
private:
	PolygonShape();
	virtual ~PolygonShape();
public:
	static PolygonShape *create(cocos2d::Vec2 pos, const cocos2d::Vec2 *vertices, unsigned int segments);
	static PolygonShape *createCircle(cocos2d::Vec2 pos, float radius, unsigned int segments = 32);//円
	static PolygonShape *createEllipse(cocos2d::Vec2 pos, float radiusX, float radiusY, float angle, unsigned int segments = 32);//楕円
	static PolygonShape *createRectangle(cocos2d::Rect rect, float angle);//矩形
	static PolygonShape *createRectangle(cocos2d::Vec2 pos, cocos2d::Size size, float angle);//矩形
	static PolygonShape *createRectangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, cocos2d::Vec2 v4, float angle);//矩形
	static PolygonShape *createRectangleCenterPoint(cocos2d::Vec2 pos, cocos2d::Size size, cocos2d::Vec2 centerPoint, float angle);//矩形
	static PolygonShape *createFan(cocos2d::Vec2 pos, float radius, float angle, float arcAngle, unsigned int segments = 32);//扇
	static PolygonShape *createFan(cocos2d::Vec2 pos, float radius, float angle, float arcAngle, float scaleX, float scaleY, unsigned int segments = 32);//扇
	static PolygonShape *createTriangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, float angle);//三角形
	void set(cocos2d::Vec2 pos, const cocos2d::Vec2 *vertices, unsigned int segments);
	void reset();
	void setPosition(cocos2d::Vec2 pos);
	void setAngle(float angle);
#if defined(AGTK_DEBUG)
	void dump();
#endif
public:
	static bool intersectsPoint(PolygonShape *p, cocos2d::Vec2 pos);
	static bool intersectsLine(PolygonShape *p, cocos2d::Vec2 v1, cocos2d::Vec2 v2);
	static bool intersectsRect(PolygonShape *p, cocos2d::Rect rect);
	static bool intersectsTriangle(PolygonShape *p, cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3);
	static bool intersectsPolygonShape(PolygonShape *p1, PolygonShape *p2);
	static bool intersectsFunPolygonShape(PolygonShape *fan, PolygonShape *ps);
	static bool intersectsFanPoint(PolygonShape *fan, cocos2d::Vec2 pos);
	static bool intersectsFanPoint(const PolygonShape *fan, float radiusX, float radiusY, const cocos2d::Vec2& pos);
#define USE_FAST_INTERSECTS_FAN_POINT
#ifdef USE_FAST_INTERSECTS_FAN_POINT
	static bool intersectsFanPoint(const cocos2d::Vec2& center, float angle, float arcAngle, float radiusX, float radiusY, const cocos2d::Vec2& pos);
#endif
private:
	virtual bool initCircle(cocos2d::Vec2 pos, float radius, unsigned int segments);
	virtual bool initEllipse(cocos2d::Vec2 pos, float radiusX, float radiusY, float angle, unsigned int segments);
	virtual bool initRectangle(cocos2d::Vec2 pos, cocos2d::Size size, float angle);
	virtual bool initRectangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, cocos2d::Vec2 v4, float angle);
	virtual bool initRectangleCenterPoint(cocos2d::Vec2 pos, cocos2d::Size size, cocos2d::Vec2 centerPoint, float angle);
	virtual bool initFan(cocos2d::Vec2 pos, float radius, float angle, float arcAngle, float scaleX, float scaleY, unsigned int segments);
	virtual bool initTriangle(cocos2d::Vec2 v1, cocos2d::Vec2 v2, cocos2d::Vec2 v3, float angle);
	cocos2d::Rect getRect();
public:
	cocos2d::Vec2 _position, _initPosition;
	cocos2d::Vec2 *_vertices, *_initVertices;
	cocos2d::Rect _rect;
	unsigned int _segments;
	float _angle, _initAngle;
protected:
	void copy(cocos2d::Vec2 **vertices, cocos2d::Vec2 &pos);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Shape : public cocos2d::Ref
{
public:
	enum Type {
		None,
		Rect,
		Circle,
		Ellipse,
	};
	Shape() {
		_type = None;
	}
public:
	static Shape *createWithRectangle(float x, float y, float w, float h, float angle);
	static Shape *createWithCircle(float x, float y, float radius);
	static Shape *createWithEllipse(float x, float y, float rx, float ry, float angle);
	virtual cocos2d::Rect getRect() = 0;
	virtual PolygonShape *getPolygonShape() = 0;
private:
	CC_SYNTHESIZE(Shape::Type, _type, Type);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Rectangle : public Shape
{
public:
	Rectangle();
	Rectangle(cocos2d::Rect rect, float angle);
	Rectangle(float x, float y, float w, float h, float angle);
	Rectangle(cocos2d::Vec2 position, cocos2d::Size size, float angle);
public:
	virtual cocos2d::Rect getRect();
	virtual PolygonShape *getPolygonShape();
public:
	cocos2d::Rect _rect;
	float _angle;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Circle : public Shape
{
public:
	Circle();
	Circle(cocos2d::Vec2 position, float radius);
	Circle(float x, float y, float radius);
public:
	virtual cocos2d::Rect getRect();
	virtual PolygonShape *getPolygonShape();
public:
	cocos2d::Vec2 _position;
	float _radius;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Ellipse: public Shape
{
public:
	Ellipse();
	Ellipse(cocos2d::Vec2 position, cocos2d::Vec2 radiusXY, float angle);
	Ellipse(float x, float y, float rx, float ry, float angle);
public:
	virtual cocos2d::Rect getRect();
	virtual PolygonShape *getPolygonShape();
public:
	cocos2d::Vec2 _position;
	cocos2d::Vec2 _radiusXY;
	float _angle;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API Collision : public cocos2d::Ref
{
public:
	Collision();
public:
	static Collision *create(const rapidjson::Value& json);
	static Shape *getShape(Collision *c);
	static Shape *getLinear(Collision *c1, Collision *c2, float range, float pos);
	static Shape *getReLinear(Collision *c1, Collision *c2, float range, float pos);
protected:
	virtual bool init(const rapidjson::Value& json);
	static Shape::Type getType(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(bool, _ignored, Ignored);//判定有無
	CC_SYNTHESIZE(bool, _samePrevFrame, SamePrevFrame);//前フレームと同じ
	CC_SYNTHESIZE(Shape::Type, _type, Type);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API CollisionRect: public Collision
{
public:
	CollisionRect();
	CREATE_FUNC_PARAM(CollisionRect, const rapidjson::Value&, json);
	cocos2d::Rect GetCollisionRect(cocos2d::Size sz);
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(cocos2d::Rect, _rect, Rect);//位置(%),サイズ(%)
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API CollisionRectangle : public Collision
{
public:
	CollisionRectangle();
	virtual ~CollisionRectangle();
	CREATE_FUNC_PARAM(CollisionRectangle, const rapidjson::Value&, json);
	cocos2d::Rect getRect();
	cocos2d::Rect getRect(cocos2d::Size sz);
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(agtk::Rectangle, _rectangle, Rectangle);
	CC_SYNTHESIZE_RETAIN(agtk::Shape *, _shape, Shape);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API CollisionCircle : public Collision
{
private:
	CollisionCircle();
	virtual ~CollisionCircle();
public:
	CREATE_FUNC_PARAM(CollisionCircle, const rapidjson::Value&, json);
	cocos2d::Rect getRect();
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(agtk::Circle, _circle, Circle);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API CollisionEllipse : public Collision
{
private:
	CollisionEllipse();
	virtual ~CollisionEllipse();
public:
	CREATE_FUNC_PARAM(CollisionEllipse, const rapidjson::Value&, json);
	cocos2d::Rect getRect();
private:
	virtual bool init(const rapidjson::Value& json);
private:
	CC_SYNTHESIZE(agtk::Ellipse, _ellipse, Ellipse);
};

typedef std::vector<std::pair<int, agtk::Tile *>> HitWallTiles;

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
class Object;
class Tile;
class AGTKPLAYER_API WallHitInfo
{
public:
	WallHitInfo() { id = 0; object = nullptr; }
public:
	void initHit(agtk::Object *obj = nullptr) {
		hitLeft = boundMin.x - 10000;
		hitRight = boundMax.x + 10000;
		hitDown = boundMin.y - 10000;
		hitUp = boundMax.y + 10000;
		openLeftMin = boundMin.y - 10000;
		openLeftMax = boundMax.y + 10000;
		openRightMin = boundMin.y - 10000;
		openRightMax = boundMax.y + 10000;
		openDownMin = boundMin.x - 10000;
		openDownMax = boundMax.x + 10000;
		openUpMin = boundMin.x - 10000;
		openUpMax = boundMax.x + 10000;
		wallLeft = boundMax.x + 10000;
		wallRight = boundMin.x - 10000;
		wallDown = boundMax.y + 10000;
		wallUp = boundMax.y - 10000;
		move = cocos2d::Vec2::ZERO;
		size = boundMax - boundMin;
		object = obj;
		hitTiles.clear();
	}
	WallHitInfo &operator=(const WallHitInfo &data) {
		this->id = data.id;
		this->center = data.center;
		this->boundMin = data.boundMin;
		this->boundMax = data.boundMax;
		this->size = data.size;
		this->hitLeft = data.hitLeft;
		this->hitRight = data.hitRight;
		this->hitDown = data.hitDown;
		this->hitUp = data.hitUp;
		this->openLeftMin = data.openLeftMin;
		this->openRightMin = data.openRightMin;
		this->openDownMin = data.openDownMin;
		this->openUpMin = data.openUpMin;
		this->openLeftMax = data.openLeftMax;
		this->openRightMax = data.openRightMax;
		this->openDownMax = data.openDownMax;
		this->openUpMax = data.openUpMax;
		this->wallLeft = data.wallLeft;
		this->wallRight = data.wallRight;
		this->wallDown = data.wallDown;
		this->wallUp = data.wallUp;
		this->move = data.move;
		this->wallVertex4 = data.wallVertex4;
		this->object = data.object;
		this->hitTiles = data.hitTiles;
		return *this;
	}
	void adjustHit() {
		if (hitLeft > center.x) {
			hitLeft = center.x;
		}
		if (hitRight < center.x) {
			hitRight = center.x;
		}
		if (hitDown > center.y) {
			hitDown = center.y;
		}
		if (hitUp < center.y) {
			hitUp = center.y;
		}
	}
	cocos2d::Rect getRect() { return cocos2d::Rect(boundMin, size); }
	void dump();
	int id;
	cocos2d::Point center;
	cocos2d::Point boundMin;
	cocos2d::Point boundMax;
	cocos2d::Size size;
	float hitLeft, hitRight, hitDown, hitUp;
	//float openLeft, openRight, openDown, openUp;
	//Point openLeft, openRight, openDown, openUp;
	float openLeftMin, openLeftMax;
	float openRightMin, openRightMax;
	float openDownMin, openDownMax;
	float openUpMin, openUpMax;
	float wallLeft, wallRight, wallDown, wallUp;
	cocos2d::Point move;
	Vertex4 wallVertex4;
	agtk::Object *object;
	agtk::HitWallTiles hitTiles;
};

//-------------------------------------------------------------------------------------------------------------------
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
class AGTKPLAYER_API WallHitInfoGroup
#else
class AGTKPLAYER_API WallHitInfoGroup : public cocos2d::Ref
#endif
{
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
public:
	WallHitInfoGroup() { _object = nullptr; };
#else
private:
	WallHitInfoGroup() : cocos2d::Ref() { _object = nullptr; _errorWallHitInfo.id = -1; };
#endif
	virtual ~WallHitInfoGroup() { _object = nullptr; };
public:
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	static WallHitInfoGroup *create(agtk::Object *object) {
		auto p = new(std::nothrow) WallHitInfoGroup();
		if (p && p->init(object)) {
			return p;
		}
		delete p;
		return nullptr;
	}
#else
	CREATE_FUNC_PARAM(WallHitInfoGroup, agtk::Object *, object);
#endif
	std::vector<WallHitInfo>& getWallHitInfoList() { return _wallHitInfoList; }
	unsigned int getWallHitInfoListCount() { return _wallHitInfoList.size(); }
	WallHitInfo &getWallHitInfo(int id);
	WallHitInfo &findWallHitInfo(int id);
	void addWallHitInfo(std::vector<Vertex4> &vertex4List);
	void addWallHitInfo(agtk::Vertex4 v);
	void addWallHitInfo(std::vector<WallHitInfo> &wallHitInfoList);
	void addWallHitInfo(WallHitInfo &wallHitInfo);
	void remove();
public:
	agtk::Object *getObject() { return _object; }
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
private:
#endif
	virtual bool init(agtk::Object *object);
	void mergeRect();
private:
	std::vector<WallHitInfo> _wallHitInfoList;
	CC_SYNTHESIZE(cocos2d::Rect, _rect, Rect);
	CC_SYNTHESIZE(cocos2d::Point, _center, Center);
	CC_SYNTHESIZE(cocos2d::Point, _boundMin, BoundMin);
	CC_SYNTHESIZE(cocos2d::Point, _boundMax, BoundMax);
	agtk::Object *_object;
	WallHitInfo _errorWallHitInfo;
};

#if defined(USE_WALL_DEBUG_DISPLAY)
//-------------------------------------------------------------------------------------------------------------------
class ObjectWallDebugDisplay : public cocos2d::Ref
{
public:
	class WallRect {
	public:
		cocos2d::Vec2 boundMin;
		cocos2d::Vec2 boundMax;
		WallRect(const cocos2d::Vec2 &boundMin, const cocos2d::Vec2 &boundMax) {
			this->boundMin = boundMin;
			this->boundMax = boundMax;
		}
		WallRect &operator=(const WallRect &data) {
			boundMin = data.boundMin;
			boundMax = data.boundMax;
			return *this;
		}
	};

private:
	ObjectWallDebugDisplay();
	virtual ~ObjectWallDebugDisplay();

public:
	CREATE_FUNC_PARAM(ObjectWallDebugDisplay, cocos2d::Node *, node);
	void destroy();
	void reset();
	void update(float dt);
	void addTileWall(cocos2d::Vec2 &boundMin, cocos2d::Vec2 &boundMax);
	void addObjectWall(cocos2d::Vec2 &boundMin, cocos2d::Vec2 &boundMax);

private:
	virtual bool init(cocos2d::Node *node);

protected:
	PrimitiveNode *getNextTilePrimitiveNode();
	PrimitiveNode *getNextObjectPrimitiveNode();

protected:
	CC_SYNTHESIZE(cocos2d::Vec2, _boundMin, BoundMin);
	CC_SYNTHESIZE(cocos2d::Vec2, _boundMax, BoundMax);
	CC_SYNTHESIZE(cocos2d::Vec2, _wallCenter, WallCenter);
	CC_SYNTHESIZE(float, _hitLeft, HitLeft);
	CC_SYNTHESIZE(float, _hitRight, HitRight);
	CC_SYNTHESIZE(float, _hitUp, HitUp);
	CC_SYNTHESIZE(float, _hitDown, HitDown);
	CC_SYNTHESIZE(float, _openLeftMin, OpenLeftMin);
	CC_SYNTHESIZE(float, _openLeftMax, OpenLeftMax);
	CC_SYNTHESIZE(float, _openRightMin, OpenRightMin);
	CC_SYNTHESIZE(float, _openRightMax, OpenRightMax);
	CC_SYNTHESIZE(float, _openUpMin, OpenUpMin);
	CC_SYNTHESIZE(float, _openUpMax, OpenUpMax);
	CC_SYNTHESIZE(float, _openDownMin, OpenDownMin);
	CC_SYNTHESIZE(float, _openDownMax, OpenDownMax);
	CC_SYNTHESIZE_RETAIN(cocos2d::Node *, _node, Node);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _primitiveNodeList, PrimitiveNodeList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _tilePrimitiveNodeList, TilePrimitiveNodeList);
	CC_SYNTHESIZE_RETAIN(cocos2d::Array *, _objectPrimitiveNodeList, ObjectPrimitiveNodeList);
	CC_SYNTHESIZE(std::vector<WallRect>, _tileWallList, TileWallList);
	CC_SYNTHESIZE(std::vector<WallRect>, _objectWallList, ObjectWallList);
	CC_SYNTHESIZE(int, _tilePrimitiveNodeIndex, TilePrimitiveNodeIndex);
	CC_SYNTHESIZE(int, _objectPrimitiveNodeIndex, ObjectPrimitiveNodeIndex);
};
#else
class ObjectWallDebugDisplay;
#endif

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
void getWallCenterAndBound(cocos2d::Vec2 pos, cocos2d::Vec2 size, cocos2d::Point *center, cocos2d::Point *boundMin, cocos2d::Point *boundMax);
void getWallCenterAndBound(cocos2d::Node *node, Point *center, Point *boundMin, Point *boundMax);
bool checkWallHit(WallHitInfo &wallHitInfo, const Point &pos, const Size &size, bool isTile, ObjectWallDebugDisplay *debug);

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkBuriedWall(cocos2d::Rect& objectRect, std::vector<agtk::Tile *> &tileList);
bool pushBackBuriedWall(agtk::Object* object, cocos2d::Rect& objectRect, cocos2d::Vec2& move, std::vector<agtk::Tile *> &tileList);
#else
bool checkBuriedWall(cocos2d::Rect& objectRect, cocos2d::Array* tileList);
bool pushBackBuriedWall(agtk::Object* object, cocos2d::Rect& objectRect, cocos2d::Vec2& move, cocos2d::Array* tileList);
#endif
bool pushBackBuriedObjectWall(cocos2d::Rect& objectPrevRect, cocos2d::Rect& objectCrntRect, cocos2d::Vec2& move, agtk::WallHitInfoGroup* infoGroup);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkBuriedWallDownPos(cocos2d::Rect& objectRect, float y, std::vector<agtk::Tile *> &tileList);
#else
bool checkBuriedWallDownPos(cocos2d::Rect& objectRect, float y, cocos2d::Array* tileList);
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, int tileThreshold, std::vector<agtk::Tile *> &tileList, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, bool falling, bool slope, agtk::HitWallTiles& hitTiles);
void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, agtk::WallHitInfoGroup* infoGroup, Vec2& moveVec, Vec2 nowMoveVec);
void newCheckWallHit(Rect& crntRect, Rect slopeCheckRect, bool& hitX, bool& hitY, int tileThreshold, std::vector<agtk::Tile *> &tileList, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, agtk::Object* object, bool falling, bool slope, agtk::HitWallTiles& hitTiles);
#else
void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, int tileThreshold, cocos2d::Array* tileList, cocos2d::Array* slopeList, cocos2d::Array* passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, bool falling, bool slope);
void newCheckWallHit(WallHitInfo &wallHitInfo, bool& hitX, bool& hitY, agtk::WallHitInfoGroup* infoGroup, Vec2& moveVec, Vec2 nowMoveVec);
void newCheckWallHit(Rect& crntRect, Rect slopeCheckRect, bool& hitX, bool& hitY, int tileThreshold, cocos2d::Array* tileList, cocos2d::Array* slopeList, cocos2d::Array* passableSlopeList, Vec2& moveVec, Vec2& tileThresholdMoveVec, Rect& oldRect, agtk::Object* object, bool falling, bool slope);
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool newCheckSlopeHit(WallHitInfo &wallHitInfo, Rect& oldRect, bool jumping, bool& hitUp, bool& hitDown, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeTouchedList, Vec2& moveVec, bool isSlip, float touchFrame300);
bool newCheckSlopeHit(agtk::Object* object, bool& hitUp, bool& hitDown, agtk::MtVector<agtk::Slope *> *slopeList, Vec2& moveVec, bool isSlip, std::vector<agtk::Tile *> &tileList);
void newCheckSlopeHit(WallHitInfo &wallHitInfo, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *touchedSlopeList);
#else
bool newCheckSlopeHit(WallHitInfo &wallHitInfo, Rect& oldRect, bool jumping, bool& hitUp, bool& hitDown, cocos2d::Array* slopeList, cocos2d::Array* passableSlopeTouchedList, Vec2& moveVec, bool isSlip, float touchFrame300);
bool newCheckSlopeHit(agtk::Object* object, bool& hitUp, bool& hitDown, cocos2d::Array* slopeList, Vec2& moveVec, bool isSlip, cocos2d::Array *tileList);
void newCheckSlopeHit(WallHitInfo &wallHitInfo, cocos2d::Array* slopeList, cocos2d::Array* touchedSlopeList);
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
float pushBackBuriedWallX(cocos2d::Rect& objectRect, float moveX, std::vector<agtk::Tile *> &tileList);
float pushBackBuriedWallY(cocos2d::Rect& objectRect, float moveY, std::vector<agtk::Tile *> &tileList);
#else
float pushBackBuriedWallX(cocos2d::Rect& objectRect, float moveX, cocos2d::Array* tileList);
float pushBackBuriedWallY(cocos2d::Rect& objectRect, float moveY, cocos2d::Array* tileList);
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkGroundAndWallHit(Rect &objRect, Vec2& dist, std::vector<agtk::Tile *> &tileList, Rect& oldRect, agtk::Object* object, bool checkWall, bool jumping);
#else
bool checkGroundAndWallHit(Rect &objRect, Vec2& dist, cocos2d::Array* tileList, Rect& oldRect, agtk::Object* object, bool checkWall, bool jumping);
#endif
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
bool checkGroundSlope(Rect &objRect, float& dist, agtk::MtVector<agtk::Slope *> *slopeList);
#else
bool checkGroundSlope(Rect &objRect, float& dist, cocos2d::Array* slopeList);
#endif

cocos2d::Vec2 getPushObjectVec(cocos2d::Vec2 pushObjMoveVec, cocos2d::Rect& pushObjRect, agtk::WallHitInfoGroup* pushedObjInfoGroup, bool& bBuriedInWallFlag, bool bPushedMove);

// #AGTK-NX #AGTK-WIN
#ifdef USE_SAR_FIX_AND_OPTIMIZE_1
int checkWallHitDivIndex(agtk::Object* object, Rect& crntRect, cocos2d::Vec2 move, int wallHitInfoId, int crntIndex, int div, std::vector<agtk::Tile *> &tileList, agtk::MtVector<agtk::Slope *> *slopeList, agtk::MtVector<agtk::Slope *> *passableSlopeList);
#endif

//-------------------------------------------------------------------------------------------------------------------
void showDebugLine(cocos2d::Vec2 &p1, cocos2d::Vec2 &p2, cocos2d::Color4F &color, float duration, int localZOrder = 1000);
void showDebugRect(cocos2d::Rect &rect, cocos2d::Color4F &color, float duration, int localZOrder = 1000);
void showDebugPolygon(cocos2d::Vec2 *vertices, int length, cocos2d::Color4F &fillColor, cocos2d::Color4F &borderColor, float duration, int localZOrder = 1000);
void showDebugCircle(cocos2d::Vec2 pos, float radius, cocos2d::Color4F &color, float duration, int localZOrder = 1000);

NS_AGTK_END

#endif	//__COLLISION_H__
