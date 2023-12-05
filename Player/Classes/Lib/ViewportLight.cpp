#include "ViewportLight.h"
#include "Lib/Object.h"
#include "Lib/Scene.h"
#include "Lib/Collision.h"
#include "Manager/GameManager.h"
#include "Manager/PrimitiveManager.h"

NS_AGTK_BEGIN

//-------------------------------------------------------------------------------------------------------------------
TimerColor::TimerColor()
{
	_initValue = cocos2d::Color3B::BLACK;
	_value = cocos2d::Color3B::BLACK;
	_oldValue = cocos2d::Color3B::BLACK;
	_nextValue = cocos2d::Color3B::BLACK;
	_prevValue = cocos2d::Color3B::BLACK;
}

bool TimerColor::init(cocos2d::Color3B value)
{
	if (agtk::EventTimer::init() == false) {
		return false;
	}
	_initValue = value;
	_value = value;
	_prevValue = value;
	_nextValue = value;
	_oldValue = value;

	this->setProcessingFunc([&](float dt) {
		_oldValue = _value;
		_value.r = AGTK_LINEAR_INTERPOLATE(_prevValue.r, _nextValue.r, _seconds, _timer);
		_value.g = AGTK_LINEAR_INTERPOLATE(_prevValue.g, _nextValue.g, _seconds, _timer);
		_value.b = AGTK_LINEAR_INTERPOLATE(_prevValue.b, _nextValue.b, _seconds, _timer);
	});
	this->setEndFunc([&]() {
		_oldValue = _value;
		_value = _nextValue;
	});

	return true;
}

cocos2d::Color3B TimerColor::setValue(cocos2d::Color3B value, float seconds)
{
	auto state = this->getState();
	if (state == kStateIdle) {
		if (value == _value) {
			return _value;
		}
	}
	else {
		if (value == _nextValue) {
			return _value;
		}
	}

	_nextValue = value;
	_prevValue = _value;
	this->start(seconds);
	return _value;
}

cocos2d::Color3B TimerColor::addValue(cocos2d::Color3B value, float seconds)
{
	auto nowValue = this->getValue();
	this->setValue(cocos2d::Color3B(nowValue.r + value.r, nowValue.g + value.g, nowValue.b + value.b), seconds);
	return nowValue;
}

void TimerColor::reset()
{
	auto value = _initValue;
	_value = value;
	_oldValue = value;
	_prevValue = value;
	_nextValue = value;
}

//-------------------------------------------------------------------------------------------------------------------
ViewportLightTexture::ViewportLightTexture()
{
	_buffer = nullptr;
	_objectViewportLightSettingData = nullptr;
}

ViewportLightTexture::~ViewportLightTexture()
{
	CC_SAFE_DELETE_ARRAY(_buffer);
	CC_SAFE_RELEASE_NULL(_objectViewportLightSettingData);
}

bool ViewportLightTexture::init(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData)
{
	CC_ASSERT(objectViewportLightSettingData);
	if (this->createTexture(objectViewportLightSettingData) == false) {
		CC_ASSERT(0);
		return false;
	}
	this->setAliasTexParameters();
	this->setObjectViewportLightSettingData(objectViewportLightSettingData);
	return true;
}

bool ViewportLightTexture::checkSameTexture(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData)
{
	auto data1 = this->getObjectViewportLightSettingData();
	auto data2 = objectViewportLightSettingData;

	if (data1 == data2) {
		return true;
	}
	if (data1->getScaleX() != data2->getScaleX()) {
		return false;
	}
	if (data1->getScaleY() != data2->getScaleY()) {
		return false;
	}
	if (data1->getRadius() != data2->getRadius()) {
		return false;
	}
	if (data1->getDefocusCircumferenceFlag() != data2->getDefocusCircumferenceFlag()) {
		return false;
	}
	if (data1->getRotation() != data2->getRotation()) {
		return false;
	}
	if (data1->getAngleRange() != data2->getAngleRange()) {
		return false;
	}
	if (data1->getColoring() != data2->getColoring()) {
		return false;
	}
	if (data1->getA() != data2->getA()) {
		return false;
	}
	if (data1->getR() != data2->getR()) {
		return false;
	}
	if (data1->getG() != data2->getG()) {
		return false;
	}
	if (data1->getB() != data2->getB()) {
		return false;
	}
	if (data1->getDefocusSideFlag() != data2->getDefocusSideFlag()) {
		return false;
	}
	if (data1->getDefocusSide() != data2->getDefocusSide()) {
		return false;
	}
	return true;
}

bool ViewportLightTexture::createTexture(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData)
{
	//RGBA8888を作成。
	float scaleX = objectViewportLightSettingData->getScaleX() * 0.01f;
	float scaleY = objectViewportLightSettingData->getScaleY() * 0.01f;
	int width = int(objectViewportLightSettingData->getRadius() * 2.0f * scaleX);
	int height = int(objectViewportLightSettingData->getRadius() * 2.0f * scaleY);
	int pn = 4;

	//ぼかし
#define USE_SIMPLE_DEFOCUS
#ifdef USE_SIMPLE_DEFOCUS
	int defocusCircumference = 0;
	if (objectViewportLightSettingData->getDefocusCircumferenceFlag()) {
		defocusCircumference = objectViewportLightSettingData->getDefocusCircumference();
	}
#else
	int defocusStrength = 0;
	if (objectViewportLightSettingData->getDefocusCircumferenceFlag()) {
		defocusStrength = -objectViewportLightSettingData->getRadius() * objectViewportLightSettingData->getDefocusCircumference() / 100;
	}
#endif

	auto data = new unsigned char[width * height * pn];
	memset(data, 0x00, width * height * pn);
	cocos2d::Vec2 pos(width * 0.5f, height * 0.5f);
	unsigned int segments = (unsigned int)((pos.x + pos.y) * M_PI * 0.5f);

	float angle = agtk::Scene::getAngleCocos2dFromScene(objectViewportLightSettingData->getRotation());
#define USE_SIMPLE_DEFOCUS
#ifdef USE_SIMPLE_DEFOCUS
	float radius = objectViewportLightSettingData->getRadius();
#else
	float radius = objectViewportLightSettingData->getRadius() + (int)(defocusStrength > 0 ? defocusStrength : 0);//defocusStrengthが正の場合は半径増
#endif
	auto shape = agtk::PolygonShape::createFan(
		pos,
		radius,
		angle,
		objectViewportLightSettingData->getAngleRange(),
		scaleX,
		scaleY,
		segments
	);

	auto timer = agtk::Timer::create();
	timer->start();

#define USE_SIMPLE_DEFOCUS
#ifdef USE_SIMPLE_DEFOCUS
	float defocusStart = (100.0f - defocusCircumference) / 100.0f;
	float defocusEnd = 1.0f;
#endif
	//色指定で塗る。
	bool bColoring = objectViewportLightSettingData->getColoring() && objectViewportLightSettingData->getA() > 0;
	cocos2d::Color4B color;
	if (bColoring) {
		color = cocos2d::Color4B(
			objectViewportLightSettingData->getR(),
			objectViewportLightSettingData->getG(),
			objectViewportLightSettingData->getB(),
			objectViewportLightSettingData->getA()
		);
		float radiusX = radius * scaleX;
		float radiusY = radius * scaleY;

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
#ifdef USE_FAST_INTERSECTS_FAN_POINT
				bool flag = PolygonShape::intersectsFanPoint(pos, angle, objectViewportLightSettingData->getAngleRange(), radiusX, radiusY, cocos2d::Vec2(x, y));
#else
				bool flag = PolygonShape::intersectsFanPoint(shape, radiusX, radiusY, cocos2d::Vec2(x, y));
#endif
				if (flag) {
					cocos2d::Color4B *c = (cocos2d::Color4B *)&data[y * width * pn + x * pn];
					*c = color;
#ifdef USE_SIMPLE_DEFOCUS
					if (defocusCircumference > 0) {
						auto unitLen = cocos2d::Vec2((x - pos.x) / radiusX, (y - pos.y) / radiusY).length();
						if (unitLen >= defocusStart && unitLen <= defocusEnd) {
							auto alpha = (int)(255 * (defocusEnd - unitLen) / (defocusEnd - defocusStart));
							c->a = alpha;
						}
					}
#endif
				}
			}
		}
	}
	timer->stop();
	timer->dump();

#ifdef USE_SIMPLE_DEFOCUS
#else
	//ぼかし処理
	if ((objectViewportLightSettingData->getDefocusCircumferenceFlag() && defocusStrength != 0) || objectViewportLightSettingData->getDefocusSideFlag()) {
		auto data2 = new unsigned char[width * height * pn];
		memcpy(data2, data, width * height * pn);
		//円周をぼかす
		if (objectViewportLightSettingData->getDefocusCircumferenceFlag() && defocusStrength != 0) {
			int radiusMin = objectViewportLightSettingData->getRadius() + (int)(defocusStrength < 0 ? defocusStrength : 0);
			auto shapeMin = agtk::PolygonShape::createFan(
				pos,
				radiusMin,
				angle,
				objectViewportLightSettingData->getAngleRange(),
				scaleX,
				scaleY,
				segments
			);
			timer->start();
#define USE_AREA_SUM	//領域のα値を領域がズレた分の差分だけ更新していくようにする。
#ifdef USE_AREA_SUM
			struct {
				int minX;
				int minY;
				int maxX;
				int maxY;
				unsigned int suma;
				int cnt;
			} info;
			memset(&info, 0, sizeof(info));
#endif
			for (int y = 0; y < height; y++) {
#ifdef USE_AREA_SUM
				for (int h = 0; h < width; h++) {
					int x = (y & 1) == 0 ? h : (width - 1 - h);
#else
				for (int x = 0; x < width; x++) {
#endif
					int length = (pos - cocos2d::Vec2(x, y)).getLength();
					int strength = length - radiusMin;
					if (strength <= 0) {
						continue;
					}
#ifdef USE_AREA_SUM
					bool flag = PolygonShape::intersectsFanPoint(pos, angle, objectViewportLightSettingData->getAngleRange(), radius * scaleX, radius * scaleY, cocos2d::Vec2(x, y));
					//bool flag = data[y * width * pn + x * pn + 3];
					if (flag == false) {
						continue;
					}
					flag = PolygonShape::intersectsFanPoint(pos, angle, objectViewportLightSettingData->getAngleRange(), radiusMin * scaleX, radiusMin * scaleY, cocos2d::Vec2(x, y));
					if (flag == true) {
						continue;
					}
#else
					bool flag = PolygonShape::intersectsFanPoint(shape, radius * scaleX, radius * scaleY, cocos2d::Vec2(x, y));
					if (flag == false) {
						continue;
					}
					flag = PolygonShape::intersectsFanPoint(shapeMin, radiusMin * scaleX, radiusMin * scaleY, cocos2d::Vec2(x, y));
					if (flag == true) {
						continue;
					}
#endif
					//ぼかしのためにα値を再計算。
#ifdef USE_AREA_SUM
					auto minX = x - strength;
					auto minY = y - strength;
					auto maxX = x + strength;
					auto maxY = y + strength;
					if (minX < 0) {
						minX = 0;
					}
					if (minY < 0) {
						minY = 0;
					}
					if (maxX >= width) {
						maxX = width - 1;
					}
					if (maxY >= height) {
						maxY = height - 1;
					}
					if (minX > maxX || minY > maxY) {
						continue;
					}
					if(info.cnt == 0 || minX > info.maxX || minY > info.maxY || maxX < info.minX || maxY < info.minY) {
						//普通に算出
						info.suma = 0;
						for (int v = minY; v <= maxY; v++) {
							for (int u = minX; u <= maxX; u++) {
								info.suma += data[v * width * pn + u * pn + 3];
							}
						}
						//info.cnt = (maxX - minX + 1) * (maxY - minY + 1);
					}
					else {
						//差分算出
						//減らす
						while (info.minY < minY) {
							for (int u = info.minX; u <= info.maxX; u++) {
								info.suma -= data[info.minY * width * pn + u * pn + 3];
							}
							//info.cnt -= info.maxX - info.minX + 1;
							info.minY++;
						}
						while (info.maxY > maxY) {
							for (int u = info.minX; u <= info.maxX; u++) {
								info.suma -= data[info.maxY * width * pn + u * pn + 3];
							}
							//info.cnt -= info.maxX - info.minX + 1;
							info.maxY--;
						}
						while (info.minX < minX) {
							for (int v = info.minY; v <= info.maxY; v++) {
								info.suma -= data[v * width * pn + info.minX * pn + 3];
							}
							//info.cnt -= info.maxY - info.minY + 1;
							info.minX++;
						}
						while (info.maxX > maxX) {
							for (int v = info.minY; v <= info.maxY; v++) {
								info.suma -= data[v * width * pn + info.maxX * pn + 3];
							}
							//info.cnt -= info.maxY - info.minY + 1;
							info.maxX--;
						}
						//加える
						while (info.minY > minY) {
							info.minY--;
							for (int u = info.minX; u <= info.maxX; u++) {
								info.suma += data[info.minY * width * pn + u * pn + 3];
							}
							//info.cnt += info.maxX - info.minX + 1;
						}
						while (info.maxY < maxY) {
							info.maxY++;
							for (int u = info.minX; u <= info.maxX; u++) {
								info.suma += data[info.maxY * width * pn + u * pn + 3];
							}
							//info.cnt += info.maxX - info.minX + 1;
						}
						while (info.minX > minX) {
							info.minX--;
							for (int v = info.minY; v <= info.maxY; v++) {
								info.suma += data[v * width * pn + info.minX * pn + 3];
							}
							//info.cnt += info.maxY - info.minY + 1;
						}
						while (info.maxX < maxX) {
							info.maxX++;
							for (int v = info.minY; v <= info.maxY; v++) {
								info.suma += data[v * width * pn + info.maxX * pn + 3];
							}
							//info.cnt += info.maxY - info.minY + 1;
						}
					}
					//int alpha = info.suma / info.cnt;
					int alpha = info.suma / ((strength * 2 + 1) * (strength * 2 + 1));
#else
					int suma = 0;
					int cnt = 0;
					for (int k = -strength; k <= strength; k++) {
						for (int l = -strength; l <= strength; l++) {
							if (0 <= y + k && y + k < height && 0 <= x + l && x + l < width) {
								suma += data[(y + k) * width * pn + (x + l) * pn + 3];
							}
							cnt++;
						}
					}
					int alpha = suma / cnt;
#endif
					//長さは９０％辺りからアルファ値を減衰する。
					int radiusAttenuation = radius * 9 / 10;
					if (length - radiusAttenuation >= 0) {
						alpha = AGTK_LINEAR_INTERPOLATE(alpha, 0, radius - radiusAttenuation, length - radiusAttenuation);
					}
					data2[y * width * pn + x * pn + 3] = alpha;
				}
			}
			timer->stop();
			timer->dump();
		}
		//側面をぼかす
		if (objectViewportLightSettingData->getDefocusSideFlag() && (0 < objectViewportLightSettingData->getAngleRange() && objectViewportLightSettingData->getAngleRange() < 360)) {
			std::function<float(cocos2d::Vec2, cocos2d::Vec2, cocos2d::Vec2)> GetDistanceLinePoint = [](cocos2d::Vec2 a, cocos2d::Vec2 b, cocos2d::Vec2 p) {
				cocos2d::Vec2 ab, ap;
				ab = b - a;
				ap = p - a;
				float d = (ab.x * ap.x + ab.y * ap.y);
				float l = ab.getLengthSq();
				float r = d / l;
				if (r <= 0) {
					return (a - p).getLength();
				}
				else if (r >= 1) {
					return (b - p).getLength();
				}
				cocos2d::Vec2 v(a.x + r * ab.x, a.y + r * ab.y);
				return (v - p).getLength();
			};

			int strengthMax = objectViewportLightSettingData->getRadius() * objectViewportLightSettingData->getDefocusSide() / 100;

			std::function<void(cocos2d::Vec2, cocos2d::Vec2, bool, bool)> makeDefocusSide = [&](cocos2d::Vec2 v1, cocos2d::Vec2 v2, bool bClockwise, bool bInside) {
				auto np1 = (v2 - v1).getNormalized();
				auto np2 = np1.rotateByAngle(cocos2d::Vec2::ZERO, CC_DEGREES_TO_RADIANS(bClockwise ? -90 : 90));

				cocos2d::Vec2 vertices[4];
				vertices[0] = v1 + -np1 * strengthMax;
				vertices[1] = v2;
				vertices[2] = v2 + np2 * strengthMax;
				vertices[3] = v1 + np2 * strengthMax - np1 * strengthMax;

				agtk::PolygonShape *shapeRectangle = nullptr;
				if (bClockwise) {
					shapeRectangle = agtk::PolygonShape::createRectangle(vertices[3], vertices[2], vertices[1], vertices[0], 0);
				}
				else {
					shapeRectangle = agtk::PolygonShape::createRectangle(vertices[0], vertices[1], vertices[2], vertices[3], 0);
				}
				if (bInside == false) {
					for (int y = 0; y < height; y++) {
						for (int x = 0; x < width; x++) {
							bool flag = PolygonShape::intersectsPoint(shapeRectangle, cocos2d::Vec2(x, y));
							if (flag) {
								cocos2d::Color4B *c = (cocos2d::Color4B *)&data[y * width * pn + x * pn];
								*c = color;
							}
						}
					}
				}
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						bool flag = PolygonShape::intersectsPoint(shapeRectangle, cocos2d::Vec2(x, y));
						if (flag) {
							int distance = GetDistanceLinePoint(vertices[0], vertices[1], cocos2d::Vec2(x, y));
							int strength = bInside ? strengthMax - distance : distance;
							if (strength > strengthMax) strength = strengthMax;
							if (strength < 0) continue;
							int sumr = 0;
							int sumg = 0;
							int sumb = 0;
							int suma = 0;
							int cnt = 0;
							for (int k = -strength; k <= strength; k++) {
								for (int l = -strength; l <= strength; l++) {
									if (0 <= y + k && y + k < height && 0 <= x + l && x + l < width) {
										sumr += data[(y + k) * width * pn + (x + l) * pn + 0];
										sumg += data[(y + k) * width * pn + (x + l) * pn + 1];
										sumb += data[(y + k) * width * pn + (x + l) * pn + 2];
										suma += data[(y + k) * width * pn + (x + l) * pn + 3];
									}
									cnt++;
								}
							}
							//色を入れる色もマスクされる。
							data2[y * width * pn + x * pn + 0] = color.r;// sumr / cnt;
							data2[y * width * pn + x * pn + 1] = color.g;// sumg / cnt;
							data2[y * width * pn + x * pn + 2] = color.b;// sumb / cnt;
							data2[y * width * pn + x * pn + 3] = suma / cnt;
						}
					}
				}
			};
			if (strengthMax > 0) {
				//outside
				makeDefocusSide(shape->_vertices[0], shape->_vertices[1], true, false);
				makeDefocusSide(shape->_vertices[0], shape->_vertices[shape->_segments - 1], false, false);
			}
			else {
				//inside
				makeDefocusSide(shape->_vertices[0], shape->_vertices[1], false, true);
				makeDefocusSide(shape->_vertices[0], shape->_vertices[shape->_segments - 1], true, true);
			}

#if 0
			int strength = 5;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					int sumr = 0;
					int sumg = 0;
					int sumb = 0;
					int suma = 0;
					int cnt = 0;
					for (int k = -strength; k <= strength; k++) {
						for (int l = -strength; l <= strength; l++) {
							if (0 <= y + k && y + k < height && 0 <= x + l && x + l < width) {
								sumr += data[(y + k) * width * pn + (x + l) * pn + 0];
								sumg += data[(y + k) * width * pn + (x + l) * pn + 1];
								sumb += data[(y + k) * width * pn + (x + l) * pn + 2];
								suma += data[(y + k) * width * pn + (x + l) * pn + 3];
							}
							cnt++;
						}
					}
					//色を入れる色もマスクされる。
					//data2[y * width * pn + x * pn + 0] = sumr / (cnt - 1);
					//data2[y * width * pn + x * pn + 1] = sumg / (cnt - 1);
					//data2[y * width * pn + x * pn + 2] = sumb / (cnt - 1);
					data2[y * width * pn + x * pn + 3] = suma / (cnt - 1);
				}
			}
#endif
		}

		delete[] data;
		data = nullptr;
		data = data2;
	}
#endif

	if (this->initWithData(data, width * height * pn, cocos2d::Texture2D::PixelFormat::RGBA8888, width, height, cocos2d::Size(width, height)) == false) {
		CC_ASSERT(0);
		return false;
	}
	_buffer = data;
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
ViewportLightSprite::ViewportLightSprite()
{
	_object = nullptr;
	_texture = nullptr;
	_lightShape = nullptr;
	_intensityColor = nullptr;
	_viewportLight = nullptr;
	_viewportLightTexture = nullptr;
	_objectViewportLightSettingData = nullptr;
}

ViewportLightSprite::~ViewportLightSprite()
{
	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_texture);
	CC_SAFE_RELEASE_NULL(_lightShape);
	CC_SAFE_RELEASE_NULL(_intensityColor);
	CC_SAFE_RELEASE_NULL(_viewportLight);
	CC_SAFE_RELEASE_NULL(_viewportLightTexture);
	CC_SAFE_RELEASE_NULL(_objectViewportLightSettingData);
}

bool ViewportLightSprite::init(agtk::Object *object, agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData)
{
	if (object) {
		CC_SAFE_RETAIN(object);
		_object = object;
	}
// #AGTK-NX
#if (CC_TARGET_PLATFORM == CC_PLATFORM_NX)
#else
	if (cocos2d::Sprite::create() == false) {
#endif
		CC_ASSERT(0);
		return false;
	}
	this->setObjectViewportLightSettingData(objectViewportLightSettingData);

	//shape
	float angle = agtk::Scene::getAngleMathFromScene(objectViewportLightSettingData->getRotation());
	float scaleX = objectViewportLightSettingData->getScaleX() * 0.01f;
	float scaleY = objectViewportLightSettingData->getScaleY() * 0.01f;
	int width = int(objectViewportLightSettingData->getRadius() * 2.0f * scaleX);
	int height = int(objectViewportLightSettingData->getRadius() * 2.0f * scaleY);
	int segments = 128;
	auto shape = agtk::PolygonShape::createFan(
		cocos2d::Vec2(width * 0.5f, height * 0.5f),
		objectViewportLightSettingData->getRadius(),
		angle,
		objectViewportLightSettingData->getAngleRange(),
		scaleX,
		scaleY,
		segments
	);
	if (shape == nullptr) {
		return false;
	}
	this->setLightShape(shape);

	//intensity color
	cocos2d::Color3B c = cocos2d::Color3B::WHITE;
	if (objectViewportLightSettingData->getIntensityFlag()) {
		int intensity = objectViewportLightSettingData->getIntensityOffset();
		c.r = 0xff * intensity / 100;
		c.g = 0xff * intensity / 100;
		c.b = 0xff * intensity / 100;
	}
	auto color = agtk::TimerColor::create(c);
	if (color == nullptr) {
		return false;
	}
	this->setIntensityColor(color);
	return true;
}

void ViewportLightSprite::update(float delta)
{
	auto data = this->getObjectViewportLightSettingData();
	CC_ASSERT(data);

	//set position
	cocos2d::Vec2 pos = Scene::getPositionCocos2dFromScene(_object->getPosition());
	switch (data->getPositionType()) {
	case agtk::data::ObjectViewportLightSettingData::EnumPositionType::kPositionCenter://このオブジェクトの中心
		pos = Scene::getPositionCocos2dFromScene(_object->getCenterPosition());
		break;
	case agtk::data::ObjectViewportLightSettingData::EnumPositionType::kPositionFoot://このオブジェクトの足元
		pos = Scene::getPositionCocos2dFromScene(_object->getFootPosition());
		break;
	case agtk::data::ObjectViewportLightSettingData::EnumPositionType::kPositionUseConnection: {//接続点を使用
		if (data->getConnectionId() > 0) {
			agtk::Vertex4 connectPoint;
			if (_object->getTimeline(agtk::data::TimelineInfoData::kTimelineConnection, data->getConnectionId(), connectPoint) == false) {
				break;
			}
			pos = connectPoint.p1;
		}
		break; }
	}
	pos.x += data->getAdjustX();
	pos.y -= data->getAdjustY();
	this->setPosition(pos);
	this->getLightShape()->setPosition(pos);

	//戦車・車タイプ
	/*
	auto objectData = this->getObject()->getObjectData();
	if (objectData->getMoveType() == agtk::data::ObjectData::kMoveTank || objectData->getMoveType() == agtk::data::ObjectData::kMoveCar) {
		auto player = this->getObject()->getPlayer();
		if (player != nullptr) {
			float degree = 0.0f;
			if (player->isAutoGeneration()) {
				float tmpDegree = player->getCenterRotation();
				degree = agtk::Scene::getAngleCocos2dFromScene(tmpDegree);
			} else {
				degree = data->getRotation() + (data->getRotation() - 180);
			}
			this->setRotation(degree);
			auto shape = this->getLightShape();
			shape->setAngle(degree);
		}
	}
	*/
	//check switch
	bool bSwitch = checkSwitch();
	this->setVisible(bSwitch);

	if (this->isVisible() && !this->getViewportLightTexture() && this->getViewportLight()) {
		//テクスチャが無いので作成する。
		auto viewportLightTexture = this->getViewportLight()->getViewportLightTexture(this->getObjectViewportLightSettingData());
		this->setViewportLightTexture(viewportLightTexture);
	}

	this->getIntensityColor()->update(delta);
	if (data->getIntensityFlag()) {
		auto intensity = this->getIntensityColor();
		this->setColor(intensity->getValue());
	}
}

void ViewportLightSprite::updateDarkShader(float intensity)
{
	CC_ASSERT(0.0f <= intensity && intensity <= 1.0f);
	auto data = this->getObjectViewportLightSettingData();
	if (data->getIntensityFlag() == false) {
		return;
	}

	//intensity color
	cocos2d::Color3B c = cocos2d::Color3B::WHITE;
	if (data->getIntensityFlag()) {
		float intensityOffset = (float)data->getIntensityOffset();
		int value = (int)AGTK_LINEAR_INTERPOLATE(255.0f, (255.0f * intensityOffset / 100.0f), 1.0f, intensity);
		c.r = value;
		c.g = value;
		c.b = value;
	}
	this->getIntensityColor()->setValue(c);
}

bool ViewportLightSprite::checkSwitch()
{
	auto data = this->getObjectViewportLightSettingData();
	//check switch
	bool bSwitch = false;
	auto playObjectData = _object->getPlayObjectData();
	if (data->getObjectSwitch()) {
		if (data->getObjectSwitchId() == -1) {//無し
			bSwitch = true;
		}
		else {
			auto switchData = playObjectData->getSwitchData(data->getObjectSwitchId());
			if (switchData != nullptr) {
				bSwitch = switchData->getValue();
			}
		}
	}
	else {
		auto projectPlayData = GameManager::getInstance()->getPlayData();
		if (data->getSystemSwitchId() == -1) {//無し
			bSwitch = true;
		}
		else {
			auto switchData = projectPlayData->getSwitchData(agtk::data::PlayData::COMMON_PLAY_DATA_ID, data->getSystemSwitchId());
			if (switchData != nullptr) {
				bSwitch = switchData->getValue();
			}
		}
	}
	return bSwitch;
}

bool ViewportLightSprite::intersects(agtk::Object *targetObject)
{
	if (checkSwitch() == false) {
		return false;
	}
	auto lightShape = this->getLightShape();
	std::vector<Vertex4> wallCollisionList;
	targetObject->getTimelineList(agtk::data::TimelineInfoData::kTimelineWall, wallCollisionList);
	if (wallCollisionList.size() > 0) {
		for (auto wallInfo : wallCollisionList) {
			auto polyShape = agtk::PolygonShape::createRectangle(wallInfo.getRect(), 0);
			if (PolygonShape::intersectsFunPolygonShape(lightShape, polyShape)) {
				return true;
			}
		}
	}
	return false;
}

void ViewportLightSprite::setViewportLight(agtk::ViewportLight *viewportLight)
{
	if (viewportLight) {
		CC_SAFE_RETAIN(viewportLight);
		_viewportLight = viewportLight;
	}
}

void ViewportLightSprite::setViewportLightTexture(agtk::ViewportLightTexture *texture)
{
	if (texture) {
		CC_SAFE_RETAIN(texture);
		_viewportLightTexture = texture;
	}
	_batchNode = nullptr;

	_recursiveDirty = false;
	setDirty(false);

	_opacityModifyRGB = true;

	_blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;

	_flippedX = _flippedY = false;

	// default transform anchor: center
	setAnchorPoint(Vec2::ANCHOR_MIDDLE);

	// zwoptex default values
	_offsetPosition.setZero();

	// clean the Quad
	memset(&_quad, 0, sizeof(_quad));

	// Atlas: Color
	_quad.bl.colors = Color4B::WHITE;
	_quad.br.colors = Color4B::WHITE;
	_quad.tl.colors = Color4B::WHITE;
	_quad.tr.colors = Color4B::WHITE;

	cocos2d::Sprite::setTexture(texture);
	cocos2d::Sprite::setTextureRect(cocos2d::Rect(cocos2d::Vec2::ZERO, texture->getContentSize()));
	this->getTexture()->setAliasTexParameters();
}

//-------------------------------------------------------------------------------------------------------------------
ViewportLightObject::ViewportLightObject()
{
	_object = nullptr;
	_viewportLight = nullptr;
	_viewportLightSpriteList = nullptr;
}

ViewportLightObject::~ViewportLightObject()
{
	CC_SAFE_RELEASE_NULL(_object);
	CC_SAFE_RELEASE_NULL(_viewportLight);
	CC_SAFE_RELEASE_NULL(_viewportLightSpriteList);
}

ViewportLightObject *ViewportLightObject::create(agtk::Object *object, agtk::ViewportLight *viewportLight, agtk::SceneLayer *sceneLayer)
{
	auto p = new (std::nothrow) ViewportLightObject();
	if (p && p->init(object, viewportLight, sceneLayer)) {
		p->autorelease();
		return p;
	}
	CC_SAFE_DELETE(p);
	return nullptr;
}

bool ViewportLightObject::init(agtk::Object *object, agtk::ViewportLight *viewportLight, agtk::SceneLayer *sceneLayer)
{
	if (object) {
		CC_SAFE_RETAIN(object);
		_object = object;
	}
	else {
		CC_ASSERT(0);
		return false;
	}
	if (viewportLight) {
		CC_SAFE_RETAIN(viewportLight);
		_viewportLight = viewportLight;
	}
	else {
		CC_ASSERT(0);
		return false;
	}
	auto objectData = object->getObjectData();
	if (!objectData->getViewportLightSettingFlag()) {
		CC_ASSERT(0);
		return false;
	}
	if (objectData->getViewportLightSettingList()->count() == 0) {
		CC_ASSERT(0);
		return false;
	}
	auto objectViewportLightSettingList = objectData->getViewportLightSettingList();
	auto dic = cocos2d::__Dictionary::create();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(objectViewportLightSettingList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto objectViewportLightSettingData = static_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#else
		auto objectViewportLightSettingData = dynamic_cast<agtk::data::ObjectViewportLightSettingData *>(el->getObject());
#endif
		auto sprite = agtk::ViewportLightSprite::create(object, objectViewportLightSettingData);
		sprite->setViewportLight(viewportLight);
		dic->setObject(sprite, el->getIntKey());
		// ACT2-6457対応 視野をレイヤーの最前面に設定
		sceneLayer->addChild(sprite, 10);
	}
	this->setViewportLightSpriteList(dic);
	return true;
}

void ViewportLightObject::update(float delta)
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		viewportLightSprite->update(delta);
	}
}

void ViewportLightObject::updateDarkShader(float intensity)
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		viewportLightSprite->updateDarkShader(intensity);
	}
}

void ViewportLightObject::visit()
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		auto data = viewportLightSprite->getObjectViewportLightSettingData();
		if (data->getViewport() && !data->getColoring()) {
			continue;//視野で指定色で塗るフラグOFF
		}
		viewportLightSprite->visit();
	}
}

agtk::ViewportLightSprite *ViewportLightObject::getViewportLightSprite(int id)
{
	return dynamic_cast<agtk::ViewportLightSprite *>(this->getViewportLightSpriteList()->objectForKey(id));
}

void ViewportLightObject::removeSprite(agtk::SceneLayer *sceneLayer)
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		sceneLayer->removeChild(viewportLightSprite);
	}
}

bool ViewportLightObject::checkSwitch()
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		
		if (viewportLightSprite->checkSwitch())
		{
			return true;
		}
	}
	return false;
}

// 指定色で塗るがONの視野、あるいは、照明の設定を含んでいる場合にtrueを返す。
bool ViewportLightObject::containsVisibleViewportLight()
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		auto data = viewportLightSprite->getObjectViewportLightSettingData();
		if (data->getViewport() == false || data->getColoring()) {
			// 照明、あるいは、指定色で塗るがON
			return true;
		}
	}
	return true;
}

// スイッチがONの　指定色で塗るがONの視野、あるいは、照明の設定　を含んでいる場合にtrueを返す。
bool ViewportLightObject::containsVisibleViewportLightSwitch()
{
	auto viewportLightSpriteList = this->getViewportLightSpriteList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSpriteList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSprite = static_cast<agtk::ViewportLightSprite *>(el->getObject());
#else
		auto viewportLightSprite = dynamic_cast<agtk::ViewportLightSprite *>(el->getObject());
#endif
		auto data = viewportLightSprite->getObjectViewportLightSettingData();
		if (data->getViewport() == false || data->getColoring()) {
			// 照明、あるいは、指定色で塗るがON
			if (viewportLightSprite->checkSwitch()) {
				//スイッチがON
				return true;
			}
		}
	}
	return true;
}

//-------------------------------------------------------------------------------------------------------------------
ViewportLightSceneLayer::ViewportLightSceneLayer()
{
	_sceneLayer = nullptr;
	_viewportLightObjectList = nullptr;
	_renderTexture = nullptr;
	_isPreviousSwitch = false;
}

ViewportLightSceneLayer::~ViewportLightSceneLayer()
{
	CC_SAFE_RELEASE_NULL(_sceneLayer);
	CC_SAFE_RELEASE_NULL(_viewportLightObjectList);
	CC_SAFE_RELEASE_NULL(_renderTexture);
}

bool ViewportLightSceneLayer::init(agtk::SceneLayer *sceneLayer, agtk::ViewportLight *viewportLight)
{
	if (sceneLayer) {
		CC_SAFE_RETAIN(sceneLayer);
		_sceneLayer = sceneLayer;
	}
	auto viewportLightObjectList = cocos2d::__Array::create();
	if (viewportLightObjectList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setViewportLightObjectList(viewportLightObjectList);

	auto objectList = sceneLayer->getObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(objectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto object = static_cast<agtk::Object *>(ref);
#else
		auto object = dynamic_cast<agtk::Object *>(ref);
#endif
		auto objectData = object->getObjectData();
		if (objectData->getViewportLightSettingFlag() && objectData->getViewportLightSettingList()->count() > 0) {
			auto viewportLightObject = ViewportLightObject::create(object, viewportLight, sceneLayer);
			this->getViewportLightObjectList()->addObject(viewportLightObject);
		}
	}

	return true;
}

//いずれかのSwitchでONのものがある。
bool ViewportLightSceneLayer::isSwitch()
{
	if (_sceneLayer->getIsShaderColorDarkMask()) {
		return true;
	}

	auto viewportLightObjectList = this->getViewportLightObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
		auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif

		if (viewportLightObject->checkSwitch()) {
			return true;
			break;
		}
	}

	return false;
}

//表示されるものを含んでいて、いずれかのスイッチがON　のものが含まれていたらTrueを返す。
bool ViewportLightSceneLayer::containsVisibleViewportLightSwitch()
{
	if (_sceneLayer->getIsShaderColorDarkMask()) {
		return true;
	}

	auto viewportLightObjectList = this->getViewportLightObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
		auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif

		if (viewportLightObject->containsVisibleViewportLightSwitch()) {
			//スイッチがONの表示されるものを含んでいる。
			return true;
		}
	}

	return false;
}

bool ViewportLightSceneLayer::containsVisibleViewportLight()
{
	//スイッチのON/OFFに関わらず、リストの中で一つでも指定色で塗るがONの設定または、照明設定がある場合はTrueを返す。
	auto viewportLightObjectList = this->getViewportLightObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
		auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
		if (viewportLightObject->containsVisibleViewportLight()) {
			//指定色で塗るがONの設定または、照明設定
			return true;
		}
	}
	return false;
}

void ViewportLightSceneLayer::createRenderTexture()
{
	auto visibleSize = Director::getInstance()->getVisibleSize();
	auto renderTexture = cocos2d::RenderTexture::create(visibleSize.width, visibleSize.height);
	this->setRenderTexture(renderTexture);

	renderTexture->setAnchorPoint(cocos2d::Vec2(0, 0));
	renderTexture->getSprite()->getTexture()->setAliasTexParameters();
	renderTexture->setPosition(visibleSize.width * 0.5f, visibleSize.height * 0.5f);
}

void ViewportLightSceneLayer::removeRenderTexture()
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		renderTexture->removeFromParentAndCleanup(true);
		this->setRenderTexture(nullptr);
	}
}

void ViewportLightSceneLayer::createShader(cocos2d::Layer *layer)
{
	// ACT2-6467 暗闇シェーダーがある場合のみ視野シェーダーを有効にする
	auto sceneLayer = this->getSceneLayer();

	auto rtCtrl = sceneLayer->getRenderTexture();
	if (rtCtrl) {
		rtCtrl->setLayer(layer);

		auto renderTexture = this->getRenderTexture();

		auto shader = sceneLayer->getShader(agtk::Shader::kShaderColorDarkMask);
		if (shader == nullptr) {
			return;
		}

		sceneLayer->setIsShaderColorDarkMask(false);
		shader->setMaskTexture(renderTexture->getSprite()->getTexture());
	}
}

void ViewportLightSceneLayer::removeShader()
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture) {
		auto sceneLayer = this->getSceneLayer();
		sceneLayer->removeShader(agtk::Shader::kShaderColorDarkMask);
	}
}

void ViewportLightSceneLayer::update(float delta)
{
	auto renderTexture = this->getRenderTexture();
	if (renderTexture == nullptr) {
		// switchがONからOFFに切り替わったら非表示にするため1回のみ更新
		if (_isPreviousSwitch) {
			_isPreviousSwitch = false;

			auto viewportLightObjectList = this->getViewportLightObjectList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
				auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
				viewportLightObject->update(delta);
			}
			return;
		}
		//switchがONで表示物が1つもない設定の場合
		if (this->isSwitch() && this->containsVisibleViewportLight() == false) {
			auto viewportLightObjectList = this->getViewportLightObjectList();
			cocos2d::Ref *ref;
			CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
				auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
				auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
				viewportLightObject->update(delta);
			}
		}
		return;
	}

	_isPreviousSwitch = true;

	auto viewportLightObjectList = this->getViewportLightObjectList();
	cocos2d::Ref *ref;

	//dark shader
	auto sceneLayer = this->getSceneLayer();
	auto shader = sceneLayer->getShader(agtk::Shader::kShaderColorDarkMask);
	if (shader != nullptr) {
		auto intensity = shader->getIntensity();
		CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
			auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
			viewportLightObject->updateDarkShader(intensity);
		}
	}

	//update
	CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
		auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
		viewportLightObject->update(delta);
	}

	if (this->containsVisibleViewportLight()) {
		renderTexture->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
		CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
			auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
			viewportLightObject->visit();
		}
		renderTexture->end();
	}
}

void ViewportLightSceneLayer::removeObject(agtk::Object *object)
{
	auto viewportLightObjectList = this->getViewportLightObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
		auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
		if (viewportLightObject->getObject() == object) {
			viewportLightObject->removeSprite(this->getSceneLayer());
			viewportLightObjectList->removeObject(viewportLightObject);
		}
	}
}

agtk::ViewportLightObject *ViewportLightSceneLayer::getViewportLightObject(agtk::Object *object)
{
	auto viewportLightObjectList = this->getViewportLightObjectList();
	cocos2d::Ref *ref;
	CCARRAY_FOREACH(viewportLightObjectList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightObject = static_cast<ViewportLightObject *>(ref);
#else
		auto viewportLightObject = dynamic_cast<ViewportLightObject *>(ref);
#endif
		if (viewportLightObject->getObject() == object) {
			return viewportLightObject;
		}
	}
	return nullptr;
}

//-------------------------------------------------------------------------------------------------------------------
ViewportLight::ViewportLight()
{
	_scene = nullptr;
	_viewportLightSceneLayerList = nullptr;
	_viewportLightTextureList = nullptr;
	_baseLayer = nullptr;
}

ViewportLight::~ViewportLight()
{
	CC_SAFE_RELEASE_NULL(_viewportLightSceneLayerList);
	CC_SAFE_RELEASE_NULL(_viewportLightTextureList);
}

bool ViewportLight::init(agtk::Scene *scene)
{
	CC_ASSERT(scene);
	CC_ASSERT(scene->getReferenceCount() > 0);
	_scene = scene;
	auto sceneLayerList = cocos2d::__Dictionary::create();
	if (sceneLayerList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setViewportLightSceneLayerList(sceneLayerList);
	auto textureList = cocos2d::__Array::create();
	if (textureList == nullptr) {
		CC_ASSERT(0);
		return false;
	}
	this->setViewportLightTextureList(textureList);
	return true;
}

void ViewportLight::createViewportLight()
{
	auto scene = this->getScene();
	CC_ASSERT(scene->getReferenceCount() > 0);
	//scenelayer
	auto sceneLayerList = scene->getSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(sceneLayerList, el) {
		if (el->getIntKey() <= 0) continue;
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto sceneLayer = static_cast<agtk::SceneLayer *>(el->getObject());
#else
		auto sceneLayer = dynamic_cast<agtk::SceneLayer *>(el->getObject());
#endif
		auto viewportLightSceneLayer = agtk::ViewportLightSceneLayer::create(sceneLayer, this);
		this->getViewportLightSceneLayerList()->setObject(viewportLightSceneLayer, el->getIntKey());
	}
}

void ViewportLight::removeViewportLight()
{
	//scenelayer
	auto viewportLightSceneLayerList = this->getViewportLightSceneLayerList();
	viewportLightSceneLayerList->removeAllObjects();
}

void ViewportLight::update(float delta)
{
	CC_ASSERT(_scene);

	auto viewportLightSceneLayerList = this->getViewportLightSceneLayerList();
	cocos2d::DictElement *el;

	// レンダーの作成か削除
	{
		CCDICT_FOREACH(viewportLightSceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto viewportLightSceneLayer = static_cast<agtk::ViewportLightSceneLayer *>(el->getObject());
#else
			auto viewportLightSceneLayer = dynamic_cast<agtk::ViewportLightSceneLayer *>(el->getObject());
#endif
			if (viewportLightSceneLayer->containsVisibleViewportLightSwitch()) {
				if (viewportLightSceneLayer->getRenderTexture() == nullptr) {
					viewportLightSceneLayer->createRenderTexture();
				}
				viewportLightSceneLayer->createShader(_baseLayer);
			}
			else {
				if (viewportLightSceneLayer->getRenderTexture()) {
					viewportLightSceneLayer->removeShader();
					viewportLightSceneLayer->removeRenderTexture();
				}
			}
		}
	}

	// 更新
	{
		CCDICT_FOREACH(viewportLightSceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
			auto viewportLightSceneLayer = static_cast<agtk::ViewportLightSceneLayer *>(el->getObject());
#else
			auto viewportLightSceneLayer = dynamic_cast<agtk::ViewportLightSceneLayer *>(el->getObject());
#endif
			viewportLightSceneLayer->update(delta);
		}
	}
}

ViewportLightSceneLayer *ViewportLight::getViewportLightSceneLayer(int layerId)
{
	return dynamic_cast<ViewportLightSceneLayer*>(this->getViewportLightSceneLayerList()->objectForKey(layerId));
}

ViewportLightTexture *ViewportLight::getViewportLightTexture(agtk::data::ObjectViewportLightSettingData *objectViewportLightSettingData)
{
	cocos2d::Ref *ref;
	auto viewportLightTextureList = this->getViewportLightTextureList();
	CCARRAY_FOREACH(viewportLightTextureList, ref) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto texture = static_cast<agtk::ViewportLightTexture *>(ref);
#else
		auto texture = dynamic_cast<agtk::ViewportLightTexture *>(ref);
#endif
		if (texture->checkSameTexture(objectViewportLightSettingData)) {
			return texture;
		}
	}
	auto texture = agtk::ViewportLightTexture::create(objectViewportLightSettingData);
	if (texture) {
		viewportLightTextureList->addObject(texture);
	}
	return texture;
}

void ViewportLight::removeShader()
{
	auto viewportLightSceneLayerList = this->getViewportLightSceneLayerList();
	cocos2d::DictElement *el;
	CCDICT_FOREACH(viewportLightSceneLayerList, el) {
// #AGTK-NX
#ifdef STATIC_DOWN_CAST
		auto viewportLightSceneLayer = static_cast<agtk::ViewportLightSceneLayer *>(el->getObject());
#else
		auto viewportLightSceneLayer = dynamic_cast<agtk::ViewportLightSceneLayer *>(el->getObject());
#endif
		viewportLightSceneLayer->removeShader();
	}
}

NS_AGTK_END
