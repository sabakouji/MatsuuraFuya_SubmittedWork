#include "ObjectManager.h"
#include "GameObject.h"
#include "Time.h"
#include <algorithm>
#include <set>

namespace {
struct UpdateObject {
  GameObject *object;
  bool initialized;
  bool destroyMe;
  bool dontDestroy;
  int priority;
  bool active;
  UpdateObject()
      : object(nullptr), initialized(false), destroyMe(false),
        dontDestroy(false), priority(0), active(true) {}
};
struct DrawObject {
  GameObject *object;
  int order;
  float distQ;
  bool visible;
  DrawObject() : object(nullptr), order(0), distQ(0), visible(true) {}
};
std::vector<UpdateObject> updateObjects;
std::vector<DrawObject> drawObjects;
bool needSortUpdate;
bool needSortDraw;

// 全GameObjectポインタのキャッシュ。updateObjectsの増減時にdirty化し、
// GetAllObjectsRef()参照時のみ遅延再構築する（毎フレームのヒープ確保を回避）。
std::vector<GameObject *> allObjectsCache;
bool objectsDirty = true;
}; // namespace

void deleteDrawObject(GameObject *obj) {
  for (auto it = drawObjects.begin(); it != drawObjects.end();) {
    if ((*it).object == obj) {
      it = drawObjects.erase(it);
    } else {
      it++;
    }
  }
}

void ObjectManager::Start() {}

void ObjectManager::Update() {
  if (needSortUpdate) {
    std::sort(updateObjects.begin(), updateObjects.end(), [](UpdateObject &a, UpdateObject &b) {    return a.priority > b.priority;
    });
    needSortUpdate = false;
  }
  // Iterate by index: Update() may push_back into updateObjects causing
  // vector reallocation, which would invalidate iterators and references.
  for (size_t i = 0; i < updateObjects.size();) {
    GameObject *obj = updateObjects[i].object;
    if (!updateObjects[i].initialized) {
      obj->Start();
      updateObjects[i].initialized = true;
    }
    if (updateObjects[i].active) {
      obj->Update();
    }
    // Re-check by index because the vector may have been reallocated.
    if (i < updateObjects.size() && updateObjects[i].object == obj &&
        updateObjects[i].destroyMe) {
      deleteDrawObject(obj);
      updateObjects.erase(updateObjects.begin() + i);
      objectsDirty = true;
      continue;
    }
    ++i;
  }
}

void ObjectManager::Draw() {
  if (needSortDraw) {
    std::sort(drawObjects.begin(), drawObjects.end(), [](DrawObject &a, DrawObject &b) {
      return a.order > b.order || (a.order == b.order && a.distQ > b.distQ);
    });
    needSortDraw = false;
  }
  for (const DrawObject &node : drawObjects) {
    if (node.visible) {
      node.object->Draw();
    }
  }
}

void ObjectManager::Release() { DeleteAllGameObject(); }

void ObjectManager::ChangeScene() {
  std::set<GameObject *> deletedInScene;
  for (auto it = updateObjects.begin(); it != updateObjects.end();) {
    UpdateObject &node = *it;
    if (!node.dontDestroy) {
      deleteDrawObject(node.object);
      if (node.object != nullptr) {
        if (deletedInScene.find(node.object) == deletedInScene.end()) {
          delete node.object;
          deletedInScene.insert(node.object);
        }
      }
      it = updateObjects.erase(it);
      objectsDirty = true;
    } else
      it++;
  }
}

std::list<GameObject *> ObjectManager::GetAllObjects() {
  std::list<GameObject *> objs;
  for (const UpdateObject &obj : updateObjects) {
    objs.push_back(obj.object);
  }
  return objs;
}

const std::vector<GameObject *> &ObjectManager::GetAllObjectsRef() {
  if (objectsDirty) {
    allObjectsCache.clear();
    allObjectsCache.reserve(updateObjects.size());
    for (const UpdateObject &obj : updateObjects) {
      allObjectsCache.push_back(obj.object);
    }
    objectsDirty = false;
  }
  return allObjectsCache;
}

void ObjectManager::Push(GameObject *obj) {
  {
    // Check for duplicates
    bool found = false;
    for (auto &node : updateObjects) {
      if (node.object == obj) {
        found = true;
        break;
      }
    }
    if (!found) {
      UpdateObject node;
      node.object = obj;
      updateObjects.push_back(node);
      needSortUpdate = true;
      objectsDirty = true;
    }
  }
  {
    DrawObject node;
    node.object = obj;
    drawObjects.push_back(node);
    needSortDraw = true;
  }
}

void ObjectManager::Destroy(GameObject *obj) {
  for (UpdateObject &ou : updateObjects) {
    if (ou.object == obj)
      ou.destroyMe = true;
  }
}

void ObjectManager::SetDrawOrder(GameObject *obj, int _order) {
  for (DrawObject &od : drawObjects) {
    if (od.object == obj) {
      od.order = _order;
    }
  }
  needSortDraw = true;
}

void ObjectManager::SetPriority(GameObject *_obj, int _priority) {
  for (UpdateObject &ou : updateObjects) {
    if (ou.object == _obj) {
      ou.priority = _priority;
    }
  }
  needSortUpdate = true;
}

void ObjectManager::DeleteGameObject(GameObject *obj) {
  deleteDrawObject(obj);
  for (auto it = updateObjects.begin(); it != updateObjects.end();) {
    UpdateObject &node = (*it);
    if (node.object == obj) {
      // delete obj;
      obj->DestroyMe();
      it = updateObjects.erase(it);
      objectsDirty = true;
    } else
      it++;
  }
}

void ObjectManager::DeleteAllGameObject() {
  std::set<GameObject *> deletedObjects;
  for (auto it = updateObjects.begin(); it != updateObjects.end();) {
    UpdateObject &node = *it;
    if (node.object != nullptr) {
      if (deletedObjects.find(node.object) == deletedObjects.end()) {
        delete node.object;
        deletedObjects.insert(node.object);
      }
    }
    node.object = nullptr;
    it = updateObjects.erase(it);
  }
  updateObjects.clear();

  for (auto it = drawObjects.begin(); it != drawObjects.end();) {
    it = drawObjects.erase(it);
  }
  drawObjects.clear();
  objectsDirty = true;
}

void ObjectManager::DontDestroy(GameObject *obj, bool dont) {
  for (auto it = updateObjects.begin(); it != updateObjects.end(); it++) {
    UpdateObject &node = *it;
    if (node.object == obj) {
      node.dontDestroy = dont;
    }
  }
}

void ObjectManager::SetActive(GameObject *obj, bool active) {
  for (auto it = updateObjects.begin(); it != updateObjects.end(); it++) {
    UpdateObject &node = *it;
    if (node.object == obj) {
      node.active = active;
    }
  }
}

void ObjectManager::SetVisible(GameObject *obj, bool visible) {
  for (DrawObject &od : drawObjects) {
    if (od.object == obj) {
      od.visible = visible;
    }
  }
}

bool ObjectManager::IsExist(GameObject *obj) {
  for (DrawObject &od : drawObjects) {
    if (od.object == obj) {
      return true;
    }
  }
  return false;
}

void ObjectManager::SetEyeDist(GameObject *obj, const float &distQIn) {
  for (DrawObject &od : drawObjects) {
    if (od.object == obj && od.visible) {
      od.distQ = distQIn;
    }
  }
  needSortDraw = true;
}
