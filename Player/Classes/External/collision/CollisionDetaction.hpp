/**
 ForTreeCollision v0.0.1
 CollisionDetection.hpp

 cocos2d-x 3.x用の4分木衝突判定クラス。
 以下公開されているソースコードをcocos2d-x v3用に修正＆拡張したものです。
 https://github.com/gear1554/CollisionDetectionTest

 Copyright (c) 2016 Kazuki Oda

 This software is released under the MIT License.
 http://opensource.org/licenses/mit-license.php
*/

#ifndef ForTreeCollisionDetection_hpp
#define ForTreeCollisionDetection_hpp

#include <stdio.h>
#include <unordered_set>
#include "cocos2d.h"
#include "Lib/Macros.h"
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#include "Lib/MtVector.h"
#endif
#define CLINER4TREEMANAGER_MAXLEVEL 9

/**
 * 衝突判定のノード。
 */
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
class CollisionNode {
private:
	/** 衝突判定オブジェクト **/
	CC_SYNTHESIZE_READONLY(cocos2d::Node*, m_node, Node);
	/** グループ **/
	CC_SYNTHESIZE(unsigned int, m_group, Group);
	CC_SYNTHESIZE(int, m_level, Level);

public:
	/** デストラクタ。 **/
	virtual ~CollisionNode() {
		CC_SAFE_RELEASE_NULL(m_node);
	}
	/** コンストラクタ。 **/
	CollisionNode() : m_node(nullptr), m_group(0), m_level(-1) {
	}

	void setNode(cocos2d::Node *node) {
		if (m_node) {
			m_node->release();
			m_node = nullptr;
		}
		if (node) {
			node->retain();
			m_node = node;
		}
	}
#else
class CollisionNode : public cocos2d::Ref {
   private:
    /** 衝突判定オブジェクト **/
    CC_SYNTHESIZE_READONLY(cocos2d::Node*, m_node, Node);
    /** グループ **/
	CC_SYNTHESIZE(unsigned int, m_group, Group);
	CC_SYNTHESIZE(int, m_level, Level);

   public:
    /** デストラクタ。 **/
    virtual ~CollisionNode() {
        CC_SAFE_RELEASE_NULL(m_node);
    }
    /** コンストラクタ。 **/
	CollisionNode() : m_node(nullptr), m_group(0), m_level(-1) {
	}

    /** nodeをフィールドから除去する。 **/
    void removeFromParent() {
        if (m_node) {
            m_node->removeFromParent();
        }
    }

    /** インスタンスを生成する。 **/
    static CollisionNode* create(cocos2d::Node* node, unsigned int group) {
        auto ref = new CollisionNode();
        node->retain();
        ref->autorelease();
        ref->m_node = node;
        ref->m_group = group;
        return ref;
    }
#endif
};

/**
 * 衝突判定ノードのリスト。
 */
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
class CollisionNodeList {
#else
class CollisionNodeList : public cocos2d::Ref {
#endif
   private:
    /** ノードリスト **/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
   agtk::MtVector<CollisionNode *> m_list;
#else
    cocos2d::Vector<CollisionNode*> m_list;
#endif

   public:
    /** インスタンスを生成する。 **/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
    CREATE_FUNC(CollisionNodeList);
#endif
    /** リストの数を得る。 **/
    size_t count() const {
        return m_list.size();
    }
    /** リストをクリアする。 **/
    void clear() {
        m_list.clear();
    }
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
#else
    /** イテレータの定義 **/
    cocos2d::Vector<CollisionNode*>::iterator begin() {
        return m_list.begin();
    }
    /** イテレータの定義 **/
    cocos2d::Vector<CollisionNode*>::iterator end() {
        return m_list.end();
    }
    /** イテレータの定義 **/
    cocos2d::Vector<CollisionNode*>::const_iterator begin() const {
        return m_list.begin();
    }
    /** イテレータの定義 **/
    cocos2d::Vector<CollisionNode*>::const_iterator end() const {
        return m_list.end();
    }
#endif
    /** オブジェクトが存在するか **/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	bool containsObject(CollisionNode *node) {
		auto size = m_list.size();
		for(int i = 0; i < (int)size; i++){
			auto &n = m_list[i];
			if (n == node) {
				return true;
			}
		}
		return false;
#else
    bool containsObject(CollisionNode* node) {
        return m_list.contains(node);
#endif
    }
    /** オブジェクトを追加する。 **/
    void addObject(CollisionNode* node) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		m_list.push_back(node);
#else
        m_list.pushBack(node);
#endif
    }
    /** インデックスのオブジェクトを返す。 **/
    CollisionNode* objectAtIndex(size_t size) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		return m_list[size];
#else
        return m_list.at(size);
#endif
    }
    /** Nodeに紐づくオブジェクトを除去する。 **/
    void erase(cocos2d::Node* node) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		if (m_list.size() == 0)
			return;
		else if (node == nullptr)
			return;

		if (auto _node = find(node)) {
			auto size = m_list.size();
			for (int i = 0; i < (int)size; i++) {
				auto &n = m_list[i];
				if (n == _node) {
					n = nullptr;
					if (_node != nullptr) {
						break;
					}
				}
			}
		}
#else
        if (auto _node = find(node)) {
            m_list.eraseObject(_node);
        }
#endif
    }
    /** Nodeに紐づくオブジェクトを探す。 **/
    CollisionNode* find(cocos2d::Node* node) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		auto size = m_list.size();
		for (int i = 0; i < (int)size; i++) {
			if (m_list[i]->getNode() == node) {
				return m_list[i];
			}
		}
#else
        auto it = std::find_if(
            m_list.begin(), m_list.end(),
            [node](CollisionNode* one) { return one->getNode() == node; });
        if (it != end()) {
            return *it;
        }
#endif
        return nullptr;
    }
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	bool setnull(CollisionNode *node) {
		auto size = m_list.size();
		for (int i = 0; i < (int)size; i++) {
			auto &n = m_list[i];
			if (n == node) {
				n = nullptr;
				return true;
			}
		}
		return false;
	}
	bool setnull(cocos2d::Node* node) {
		if (auto _node = find(node)) {
			auto size = m_list.size();
			for (int i = 0; i < (int)size; i++) {
				auto &n = m_list[i];
				if (n == _node) {
					n = nullptr;
					return true;
				}
			}
		}
		return false;
	}
#endif

    /** オブジェクトを除去する。 **/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void erase(CollisionNode *node) {
		auto size = m_list.size();
		for (int i = 0; i < (int)size; i++) {
			auto &n = m_list[i];
			if (n == node) {
				n = nullptr;
				// nullptrを消す場合はすべて検索する
				if (node != nullptr) {
					break;
				}
			}
		}
    }
#else
    void erase(CollisionNode* node) {
        m_list.eraseObject(node);
    }
#endif
    /** 最後のオブジェクトを除去する。 **/
    void removeLastObject() {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		m_list.pop_back();
#else
        m_list.popBack();
#endif
    }

   private:
    bool init() {
        return true;
    }
};

/**
 * 衝突判定ノードのマップ。
 */
class CollisionNodeMap : public cocos2d::Ref {
   private:
    /** マップ **/
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::map<size_t, CollisionNodeList *> m_list;
#else
    cocos2d::Map<size_t, CollisionNodeList*> m_list;
#endif
    /** マップのサイズ **/
    size_t m_size;

   public:
    /** コンストラクタ。 **/
    CollisionNodeMap() : m_list(), m_size(0){};
    /** インスタンスを生成する。 **/
    CREATE_FUNC(CollisionNodeMap);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	~CollisionNodeMap() {
		removeAllObjects();
	}
#endif
    /** 配列内のリスト内要素をクリアする。 **/
    void refresh() {
        for (auto space : m_list) {
            space.second->clear();
        }
    }
    /** 全てのオブジェクトを除去する。 **/
    void removeAllObjects() {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		for (auto &item : m_list) {
			delete item.second;
		}
#endif
        m_list.clear();
        m_size = 0;
    }
    /** オブジェクトを追加する。 **/
    void addObject(CollisionNodeList* list) {
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
		m_list.insert(std::make_pair(m_size, list));
#else
        m_list.insert(m_size, list);
#endif
        m_size++;
    }
    /** 要素数を返す。 **/
    size_t count() {
        return m_size;
    }
    /** 要素を得る。 **/
    CollisionNodeList* get(size_t index) {
        return m_list.at(index);
    }

   private:
    bool init() {
        return true;
    }
};

/** 衝突判定用の関数定義 **/
using DetectCollisionFunc = std::function<void(CollisionNode*, CollisionNode*)>;
/**
 * 4分木衝突判定クラス。
 */
class CollisionDetaction : public cocos2d::Ref {
   public:
    /**
     * コンストラクタ。
     */
    CollisionDetaction();

    /**
     * デストラクタ。
     */
    ~CollisionDetaction();

	std::string name;
   private:
    /** 衝突判定 **/
    DetectCollisionFunc m_func;
    /** m_field内のexit時のキャッシュ **/
    std::function<void()> m_onexit;
    /** 衝突判定フィールド **/
    cocos2d::Node* m_field;
    /** 4分木分割配列 **/
    CollisionNodeMap* m_spaceArray;
    /** 衝突判定オブジェクトリスト **/
    CollisionNodeList* m_gameObjectArray;
    /** 4分木分割の最小セルのサイズ **/
    cocos2d::Size m_unitSize;
    /** セルの数 **/
    int m_dwCellNum;
    /** 4分木分割のレベル **/
    int m_uiLevel;
    /** 4分木分割のレベルに応じた要素数 **/
    unsigned int m_iPow[CLINER4TREEMANAGER_MAXLEVEL + 1];
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	std::mutex m_mtx;
#ifdef USE_MULTITHREAD_MEASURE
	int m_stateChangeCounter;
#endif
#endif

   public:
    /** シングルトンを得る。 **/
    static CollisionDetaction* getDefaultDetaction();
    /** 衝突判定クラスを作成する。 **/
    static CollisionDetaction* create();
    /**
      初期化。
      @param field 衝突判定フィールドのターゲット
      @param level 4分木分割の階層レベル。レベル9より多くは指定できない
      @param autoclean fieldが親ノードから除去された際に自動で後処理をする
      @param func 衝突判定時の処理

      [Notice]
      autocleanは、fieldのsetonExitTransitionDidStartCallbackを
      使用しています。事前に登録されているものはキャッシュしますが、
      この処理を呼び出した後にfield::setonExitTransitionDidStartCallbackを
      呼び出して登録した場合はリセットされるためautocleanされません。
     */
    virtual bool init(cocos2d::Node* field, int level, bool autoclean,
                      const DetectCollisionFunc& func);
    /** リセット。 **/
    virtual void reset();

    /** 衝突判定を更新。 **/
    virtual void update();
	void updateSpaceStatus();
	void initForSingle();
	void updateSingle(CollisionNode *collisionObject);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_5
	void removeSingle(CollisionNode *collisionObject);
#endif
	void scanCollisionDetectionSingle(int spaceIndex, CollisionNode *collisionObject, bool checkChild);
	void updateSingleWithoutScan(CollisionNode *collisionObject);
	void scanSingle(CollisionNode *collisionObject);
	int calcLevel(CollisionNode *collisionObject);

    /** ノードを追加する。 **/
    virtual void add(CollisionNode* node);
    /** ノードを除去する。 **/
    virtual void remove(CollisionNode* node);
    /** ノードを除去する。 **/
    virtual void remove(cocos2d::Node* node);

   protected:
    /** 4分木空間に配置する。 **/
    virtual void updateSpaceStatus(CollisionNode* collisionObject);

    /** 衝突判定を行う。 **/
    virtual void scanCollisionDetection(int spaceIndex,
                                        CollisionNodeList* stackArray);

    /** 衝突したかどうかを判別する。 **/
    virtual void checkHit(CollisionNode* CollisionObject1,
                          CollisionNode* collisionObject2);

    /** リスト同士で衝突判定を行う。**/
    virtual void checkHitSpaceCell(CollisionNodeList* array1,
                                   CollisionNodeList* array2);
#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	void mtxLock();
	void mtxUnlock();
#endif

#ifdef USE_MULTITHREAD_OBJECT_UPDATE
	virtual void resetLocal();
	virtual void addLocal(CollisionNode* node);
	virtual void updateLocal();
	void updateSpaceStatusLocal();
	void initForSingleLocal();
	void updateSingleLocal(CollisionNode *collisionObject);
// #AGTK-NX
#ifdef USE_SAR_OPTIMIZE_5
	void removeSingleLocal(CollisionNode *collisionObject);
#endif
	void scanCollisionDetectionSingleLocal(int spaceIndex, CollisionNode *collisionObject, bool checkChild);
	void scanSingleLocal(CollisionNode *collisionObject);
	int calcLevelLocal(CollisionNode *collisionObject);
	virtual void updateSpaceStatusLocal(CollisionNode* collisionObject);
	virtual void scanCollisionDetectionLocal(int spaceIndex,
		CollisionNodeList* stackArray);
	virtual void checkHitLocal(CollisionNode* CollisionObject1,
		CollisionNode* collisionObject2);
	virtual void checkHitSpaceCellLocal(CollisionNodeList* array1,
		CollisionNodeList* array2);
public:
#ifdef USE_MULTITHREAD_MEASURE
	void setStateChangeCounter(int i);
	int getStateChangeCounter();
#endif
	void deleteNullptrm_spaceArray();

#endif
#ifdef USE_SAR_TEST_0
	void dumpLocal(int spaceIndex);
	void dump();
#endif
};

#endif /* ForTreeCollisionDetection_hpp **/
