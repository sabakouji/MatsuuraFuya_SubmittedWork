#pragma once
/// <summary>
/// Game Object Base Class
/// </summary>
/// <author>N.Hanai</author>

#include "../Src/GameMain.h"
#include "ObjectManager.h"
#include "SceneBase.h"
#include <string>

class GameObject {
public:
  GameObject() : pParent(nullptr), tag("") { ObjectManager::Push(this); }
  GameObject(GameObject *object) : pParent(object), tag("") {
    ObjectManager::Push(this);
  }
  virtual ~GameObject() {}

  /// <summary>
  /// Object Start (Called before first Update)
  /// </summary>
  virtual void Start() {}

  /// <summary>
  /// Frame Update
  /// </summary>
  virtual void Update() {}

  /// <summary>
  /// Frame Draw
  /// </summary>
  virtual void Draw() {}

  /// <summary>
  /// Destroy this instance (Removed before next Update)
  /// </summary>
  virtual void DestroyMe() { ObjectManager::Destroy(this); }

  /// <summary>
  /// Don't destroy on scene change
  /// </summary>
  void DontDestroyMe() { ObjectManager::DontDestroy(this); }

  /// <summary>
  /// Set Execution Priority (Higher is earlier)
  /// </summary>
  /// <param name="pri">Priority</param>
  void SetPriority(int pri) { ObjectManager::SetPriority(this, pri); }

  /// <summary>
  /// Set Draw Order (Higher is later/front for 2D)
  /// </summary>
  /// <param name="odr">Order</param>
  void SetDrawOrder(int odr) { ObjectManager::SetDrawOrder(this, odr); }

  /// <summary>
  /// Set Tag (Only one)
  /// </summary>
  /// <param name="_tag">Tag</param>
  void SetTag(std::string _tag) { tag = _tag; }

  /// <summary>
  /// Check Tag
  /// </summary>
  /// <param name="_tag">Tag</param>
  /// <returns>True if match</returns>
  bool IsTag(std::string _tag) const { return tag == _tag; }

  /// <summary>
  /// Get Parent
  /// </summary>
  /// <returns>Parent Pointer</returns>
  GameObject *Parent() const { return pParent; }

  /// <summary>
  /// Set Parent
  /// </summary>
  /// <returns>Parent Pointer</returns>
  void SetParent(GameObject *_parent) { pParent = _parent; }

  /// <summary>
  /// ObjectManager::SetActive Proxy
  /// </summary>
  void SetActive(bool active) { ObjectManager::SetActive(this, active); }

  /// <summary>
  /// ObjectManager::SetVisible Proxy
  /// </summary>
  void SetVisible(bool visible) { ObjectManager::SetVisible(this, visible); }

private:
  GameObject *pParent; // Parent Object
  std::string tag;     // Tag
};

template <class C> C *Instantiate() {
  C *obj = new C;
  return obj;
};

template <class C> C *Instantiate(GameObject *parent) {
  C *obj = new C(parent);
  return obj;
};

template <class C> C *SingleInstantiate() {
  C *obj = ObjectManager::FindGameObject<C>();
  if (obj == nullptr) {
    obj = Instantiate<C>();
  }
  return obj;
}
