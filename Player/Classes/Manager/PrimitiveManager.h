#ifndef __PRIMITIVE_MANAGER_H__
#define	__PRIMITIVE_MANAGER_H__

#include "Lib/Macros.h"

USING_NS_CC;

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PrimitiveTimer : public cocos2d::Ref
{
private:
	PrimitiveTimer() {};
	virtual ~PrimitiveTimer() {};
public:
	CREATE_FUNC_PARAM(PrimitiveTimer, double, duration);
	virtual void update(float delta) {
		_timer += delta;
	}
	bool isDuration() {
		return _duration >= _timer;
	}
private:
	bool init(double duration) {
		_duration = duration;
		_timer = 0.0;
		return true;
	}
private:
	CC_SYNTHESIZE(double, _duration, Duration);
	CC_SYNTHESIZE_READONLY(double, _timer, Timer);
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PrimitiveNode : public DrawNode
{
private:
	PrimitiveNode();
	virtual ~PrimitiveNode();

public:
	enum Type {
		Disk,
		Line,
		Rectangle,
		Circle,
		Plate,
		Ellipse,
		Polygon,
	};
	void setDisk(float x, float y, float radius);
	void setLine(float x1, float y1, float x2, float y2);
	void setRectangle(float x, float y, float w, float h);
	void setCircle(float x, float y, float radius);
	void setPlate(float x, float y, float w, float h);
	void setEllipse(float x, float y, float rx, float ry, float angle);
	void setPolygon(float x, float y, cocos2d::Vec2 *vertices, unsigned int segments);
public:
	void setRGBA(float r, float g, float b, float a);
	void setFillRGBA(float r, float g, float b, float a);
	void setTimer(double duration);
public:
	void getDisk(float *x, float *y, float *radius);
	void getLine(float *x1, float *y1, float *x2, float *y2);
	void getRectangle(float *x, float *y, float *w, float *h);
	void getCircle(float *x, float *y, float *radius);
	void getPlate(float *x, float *y, float *w, float *h);
	void getPolygon(cocos2d::Vec2 **vertices, unsigned int *segments);
public:
	static PrimitiveNode *createWithType(PrimitiveNode::Type type);
private:
	virtual bool initWithType(PrimitiveNode::Type type);
	enum Type _type;
	union {
		struct {
			float x, y;
			float radius;
		} disk,circle;
		struct {
			float x1, y1;
			float x2, y2;
		} line;
		struct {
			float x, y;
			float w, h;
		} rect,plate;
		struct {
			float x, y;
			float rx, ry;
			float angle;
		} ellipse;
		struct {
			float x, y;
			cocos2d::Vec2 *vertices;
			unsigned int segments;
		} polygon;
	} _data;
	cocos2d::Color4F _color;
	cocos2d::Color4F _fillColor;
	bool _update;
	CC_SYNTHESIZE_RETAIN(PrimitiveTimer *, _primTimer, PrimTimer);
	friend class PrimitiveManager;
};

//-------------------------------------------------------------------------------------------------------------------
class AGTKPLAYER_API PrimitiveManager : public cocos2d::Ref
{
private:

	// コンストラクタ
	PrimitiveManager();

	// シングルトンのインスタンス
	static PrimitiveManager *_primitiveManager;

public:

	// デストラクタ
	virtual ~PrimitiveManager();
	void init();

	// インスタンスを取得する。
	static PrimitiveManager* getInstance();

	// シングルトンを破棄する。
	static void purge();

	PrimitiveNode *createDisk(float x, float y, float radius, cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	PrimitiveNode *createLine(float x1, float y1, float x2, float y2, cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	PrimitiveNode *createRectangle(float x, float y, float width, float height, cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	PrimitiveNode *createCircle(float x, float y, float radius, cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	PrimitiveNode *createPlate(float x, float y, float width, float height, cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	PrimitiveNode *createEllipse(float x, float y, float rx, float ry, float angle, cocos2d::Color4F color = cocos2d::Color4F::WHITE);
	PrimitiveNode *createPolygon(float x, float y, cocos2d::Vec2 *vertices, unsigned int segments, cocos2d::Color4F color = cocos2d::Color4F::WHITE, cocos2d::Color4F fillColor = cocos2d::Color4F(0.0f, 0.0f, 0.0f, 0.0f));

	void removeAll();
	void remove(cocos2d::Node *node);
	void removeFromParent(cocos2d::Node *node);
	virtual void update(float delta);

	void setTimer(PrimitiveNode *node, double duration);
private:
	CC_SYNTHESIZE_RETAIN(cocos2d::__Array *, _primList, PrimList);
	CC_SYNTHESIZE(double, _timer, Timer);
};

#endif	//__PRIMITIVE_MANAGER_H__
