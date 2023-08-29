/**
 ForTreeCollision v0.0.1
 CollisionComponent.hpp

 Copyright (c) 2016 Kazuki Oda

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#ifndef CollisionComponent_hpp
#define CollisionComponent_hpp

#include <stdio.h>
#include "cocos2d.h"
#include "Lib/Macros.h"
#include "CollisionDetaction.hpp"

/**
  衝突判定クラスに自身を同期するコンポーネント。
  衝突判定を行うNodeにaddComponentすることで、
  フィールドに追加された際と除去された際に自動で衝突判定クラスへの追加／除去が
  行われます。
  (使用例)
   -----------------------------
   spr = Sprite::create();
   spr->addComponent(CollisionComponent::create());
   field->addChild(spr); // このタイミングでCollisionDetactionに登録される
   field->removeChild(spr); // このタイミングでCollisionDetactionから除去される
   -----------------------------
   (他のremoveChildメソッドや、Node::removeFromParent(),
    RemoveSelfを利用した場合も同様に除去されます)
 */
class CollisionComponent : public cocos2d::Component {
   private:
    /** 衝突判定クラス。 **/
    CollisionDetaction* m_detaction;
    /** 衝突判定用ノード。 **/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CollisionNode m_node;
#else
    CollisionNode* m_node;
#endif
    /** グループ。敵や味方、弾などの判別として利用 **/
    unsigned int m_group;

   public:
    /** コンストラクタ。 **/
    CollisionComponent()
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		: m_detaction(nullptr), m_node(), m_group(UINT_MAX){};
#else
        : m_detaction(nullptr), m_node(nullptr), m_group(UINT_MAX){};
#endif
    /** デストラクタ。 **/
    virtual ~CollisionComponent();
    /** インスタンスを作成する。**/
    static CollisionComponent* create(CollisionDetaction* detaction,
                                      unsigned int group);
	void setGroup(unsigned int group) {
		m_group = group;
	}
	enum EnumGroup {
		kGroupWall,
		kGroupRoughWall,
		kGroupHit,
		kGroupAttack,
		kGroupPortal,
		kGroupObjectCenter,
		kGroupMax
	};
	static const std::string &getCollisionComponentName(int group);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	CollisionNode *getCollisionNode() { return &m_node; }
#else
	CollisionNode *getCollisionNode(){ return m_node; }
#endif

    /** Nodeがフィールドに追加された際に呼ばれる。 **/
    void onEnter() override;
    /** Nodeがフィールドから除去された際に呼ばれる。 **/
    void onExit() override;

   protected:
    /** 初期化。 **/
    virtual bool initWithDetaction(CollisionDetaction* detaction,
                                   unsigned int group);
};

#endif /* CollisionComponent_hpp **/
