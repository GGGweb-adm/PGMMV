/**
 ForTreeCollision v0.0.1
 CollisionUtils.hpp

 cocos2d-x 3.x用の衝突判定ユーティリティクラス。

 Copyright (c) 2016 Kazuki Oda

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#ifndef CollisionUtils_hpp
#define CollisionUtils_hpp

#include <stdio.h>
#include "cocos2d.h"
/**
 * 線分
 */
typedef struct __line {
    cocos2d::Point* start;
    cocos2d::Point* end;
	int wallBit;
} CollisionLine;
/**
 * 四隅＋中心
 */
typedef struct __collision_corner {
    cocos2d::Point rightTop;
    cocos2d::Point leftTop;
    cocos2d::Point rightBottom;
    cocos2d::Point leftBottom;
    cocos2d::Point center;
    std::vector<cocos2d::Point*> points() {
        return {&rightTop, &leftTop, &rightBottom, &leftBottom, &center};
    }
    std::vector<CollisionLine> lines() {
        return {{&leftTop, &rightTop, 0x01},
                {&rightTop, &rightBottom, 0x04},
                {&rightBottom, &leftBottom, 0x08},
                {&leftBottom, &leftTop, 0x02}};
    }
} CollisionCorner;

/**
 * 衝突判定ユーティリティクラス。
 */
class CollisionUtils {
   public:
    /**
      矩形と点の衝突判定 (矩形の回転を考慮)
      @param target ターゲット
      @param point ポイントの位置
      @param wide 当たり判定を広げる
     */
    static bool containsPoint(cocos2d::Node* target,
                              const cocos2d::Point& point,
                              const cocos2d::Size& wide);

    /**
      矩形同士の衝突判定 (矩形同士の回転を考慮)
      @param target ターゲット
      @param self 自分自身
     */
    static bool intersectRect(cocos2d::Node* target, cocos2d::Node* self);

    /**
     回転を考慮した矩形の四隅の位置を返す。
     */
    static CollisionCorner getCorner(cocos2d::Node* node);

	/** 線がクロスしているかどうかを返す。 */
	static bool cross(CollisionLine* one, CollisionLine* two);

	/** 円と線分の衝突判定を行って交点と反射ベクトルを出力する **/
	static bool checkCircleHitSegmentAndGetIntersectionAndReflection(std::vector<CollisionLine>& wallList, CollisionLine* objLine, int wallBit, float size, float repulsion, const cocos2d::Vec2* speed, cocos2d::Vec2* crossPos, cocos2d::Vec2* reflection, bool checkInverseNormal = true);

	/** 線分交差判定で押し出される先を出力する **/
	static bool checkPushCross2(CollisionLine* wallLine, CollisionLine* objLine, cocos2d::Vec2* crossPos);

private:
    /** インスタンス化禁止 */
    CC_DISALLOW_COPY_AND_ASSIGN(CollisionUtils);
    CollisionUtils(){};
};

#endif /* CollisionUtils_hpp */
