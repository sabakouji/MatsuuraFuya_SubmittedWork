// 79行目 [PlayScene] コンストラクタ
// 190行目 [~PlayScene] デストラクタ
// 196行目 [Update] 更新処理
// 275行目 [Draw] 描画処理

#include "PlayScene.h"
#include "ECS/World.h"
#include "ECS/Systems/ParticleSystem.h"
#include "ECS/Systems/BulletSystem.h"
#include "ECS/Systems/EnemyCollisionSystem.h"
#include "ECS/Systems/EnemyPhysicsSystem.h"
#include "AudioManager.h"
#include "BattleSpawner.h"
#include "Camera.h"
#include "CsvReader.h"
#include "DataCarrier.h"
#include "DisplayInfo.h"
#include "EffectManager.h"
#include "EnemyManager.h"
#include "EventManager.h"
#include "Executor.h"
#include "FadeObject.h" // Added include
#include "Goal.h"
#include "Item.h"
#include "Main.h"
#include "MapManager.h"
#include "MyImgui.h"
#include "NavigationManager.h"
#include "NodeValueRegistry.h"
#include "PauseObject.h"
#include "Player.h"
#include "ScreenshotManager.h"
#include "Sprite3D.h"
#include "TextReader.h"
#include "WeaponManager.h"
#include <assert.h>
#include <fstream>


namespace {
const std::string FirstScriptName = "Data/Script/MapField.txt";

class StartButtonObject : public Object3D {
public:
  CSpriteImage *m_startBtn;
  CSprite *m_sprite;

  StartButtonObject() {
    m_startBtn = new CSpriteImage(GameDevice()->m_pShader);
    m_startBtn->Load("Data/Image/Execute_Button.png");
    m_sprite = new CSprite();
    // Set Draw Order: Scene=0, Button=-500, Fade=-1000, DisplayInfo=-10000
    // We want Button > Scene, but Button < Fade.
    // So -500 is good.
    ObjectManager::SetDrawOrder(this, -500);
  }

  ~StartButtonObject() {
    SAFE_DELETE(m_startBtn);
    SAFE_DELETE(m_sprite);
  }

  void Draw() override {
    if (m_startBtn && m_sprite) {
      float w = (float)m_startBtn->m_dwImageWidth;
      float h = (float)m_startBtn->m_dwImageHeight;
      float x = (WINDOW_WIDTH - w) / 2.0f;
      float y = WINDOW_HEIGHT - h;

      m_sprite->Draw(m_startBtn, x, y, 0, 0, w, h);
    }
  }

  // Helper for input check
  void GetButtonRect(float &outX, float &outY, float &outW, float &outH) {
    if (m_startBtn) {
      outW = (float)m_startBtn->m_dwImageWidth;
      outH = (float)m_startBtn->m_dwImageHeight;
      outX = (WINDOW_WIDTH - outW) / 2.0f;
      outY = WINDOW_HEIGHT - outH;
    }
  }
};

}; // namespace

PlayScene::PlayScene() : m_isPaused(true), m_isStartWait(true) {
  // BTノード数値/真偽値プロパティを外部CSVから読み込む
  NodeValueRegistry::Instance().Load("Data/NodeValueData/NodeValues.csv");

  // ECS システム登録（シーン開始時に1回）
  // 活性: パーティクル＋弾＋敵ボディ（押し合い）。
  // 敵のFSM思考・アニメ・FBX描画はOOP維持（段階的ハイブリッド）。
  World& world = World::GetInstance();
  // SceneManagerのシーン切替でもWorld::Clear()を呼んでいるが、ここでも必ず呼ぶ。
  // Clear()はsystems_も空にするため、これを省くとPlaySceneに入り直すたびに
  // Systemが多重登録され、パーティクル等が多重更新される。
  world.Clear();
  world.RegisterSystem(std::make_unique<ParticleSystem>());
  world.RegisterSystem(std::make_unique<BulletSystem>());
  world.RegisterSystem(std::make_unique<EnemyCollisionSystem>());
  world.RegisterSystem(std::make_unique<EnemyPhysicsSystem>());

  SingleInstantiate<EnemyManager>();
  SingleInstantiate<WeaponManager>();
  SingleInstantiate<EffectManager>();
  SingleInstantiate<EventManager>();

  Instantiate<MapManager>();

  DataCarrier *dc = ObjectManager::FindGameObject<DataCarrier>();
  if (dc->ScriptName() == "") {
    dc->SetScriptName(FirstScriptName);
  }

  // FORCE CLEANUP: Ensure no stale Executor/Camera/DisplayInfo remain
  // This handles the case where ObjectManager::ChangeScene failed to clear
  // them.
  {
    auto executors = ObjectManager::FindGameObjects<Executor>();
    for (auto *obj : executors)
      ObjectManager::DeleteGameObject(obj);

    auto cameras = ObjectManager::FindGameObjects<Camera>();
    for (auto *obj : cameras)
      ObjectManager::DeleteGameObject(obj);

    auto displays = ObjectManager::FindGameObjects<DisplayInfo>();
    for (auto *obj : displays)
      ObjectManager::DeleteGameObject(obj);
  }

  TextReader *txt = new TextReader(dc->ScriptName());

  for (int i = 0; i < txt->GetLines(); i++) {
    std::string str = txt->GetString(i, 0);
    Object3D *obj = nullptr;

    if (str == "Player") {
      obj = Instantiate<Executor>();
      VECTOR3 pos, rot;
      pos.x = txt->GetFloat(i, 1);
      pos.y = txt->GetFloat(i, 2);
      pos.z = txt->GetFloat(i, 3);
      obj->SetPosition(pos.x, pos.y, pos.z);
      rot.x = 0;
      rot.y = txt->GetFloat(i, 4);
      rot.z = 0;
      obj->SetRotation(rot.x, rot.y, rot.z);
    } else if (str.substr(0, 3) == "Map") {
      ObjectManager::FindGameObject<MapManager>()->MakeMap(txt, i);
    } else if (str.substr(0, 5) == "Enemy") {
      ObjectManager::FindGameObject<EnemyManager>()->Spawn(txt, i);
    } else if (str.substr(0, 5) == "Event") {
      ObjectManager::FindGameObject<EventManager>()->MakeEvent(txt, i);
    } else if (str.substr(0, 4) == "Item") {
      Item *itemObj = Instantiate<Item>();
      itemObj->MakeItem(txt, i);
    } else if (str.substr(0, 4) == "Goal") {
      Goal *goalObj = Instantiate<Goal>();
      goalObj->MakeGoal(txt, i);
    } else if (str == "BattleStage") {
      BattleSpawner *spawner = Instantiate<BattleSpawner>();
      spawner->MakeSpawner(txt, i);
    } else {
      // assert(false);
    }
  }

  m_pauseObj = Instantiate<PauseObject>();
  m_pauseObj->SetActive(false);
  m_pauseObj->SetVisible(false);

  Instantiate<Camera>();
  Instantiate<DisplayInfo>();

  AudioManager::StopAll();
  AudioManager::Audio("BGM_Stage")->Play(AUDIO_LOOP);

  SAFE_DELETE(txt);

  NavigationManager::GetInstance().Initialize();
  NavigationManager::GetInstance().BuildNavMesh(
      ObjectManager::FindGameObject<MapManager>());

  m_btnObj = Instantiate<StartButtonObject>();

  FadeObject *fade = ObjectManager::FindGameObject<FadeObject>();

  if (fade) {
    fade->StartFadeIn(1.0f, [fade]() {
      ObjectManager::DontDestroy(fade, false); // Allow destruction
      ObjectManager::Destroy(fade);            // Destroy safely after fade in
    });
  } else {
  }

  SceneManager::SetTimeScale(0.0f);
  DisplayInfo *di = ObjectManager::FindGameObject<DisplayInfo>();
  if (di) {
    di->SetPaused(true);
  }
}

PlayScene::~PlayScene() {
  SceneManager::SetTimeScale(1.0f);
  // m_btnObj is managed by ObjectManager, so no need to delete here.
  // SAFE_DELETE(m_btnObj);
}

void PlayScene::Update() {
  if (m_isStartWait) {
    // Check for Start Button Click
    if (GameDevice()->m_pDI->CheckMouse(KD_TRG, DIM_LBUTTON)) {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(GameDevice()->m_pMain->m_hWnd, &pt);

      float w = 0, h = 0, x = 0, y = 0;
      if (m_btnObj) {
        ((StartButtonObject *)m_btnObj)->GetButtonRect(x, y, w, h);
      }

      if (pt.x >= x && pt.x <= x + w && pt.y >= y && pt.y <= y + h) {
        m_isStartWait = false;
        m_isPaused = false;
        SceneManager::SetTimeScale(1.0f);

        // Remove button
        ObjectManager::Destroy(m_btnObj);
        m_btnObj = nullptr;

        DisplayInfo *di = ObjectManager::FindGameObject<DisplayInfo>();
        if (di)
          di->SetPaused(false);
      }
    }
  }

  if (GameDevice()->m_pDI->CheckKey(KD_TRG, DIK_TAB)) {
    m_isPaused = !m_isPaused;
    if (!m_isPaused) {
      if (m_isStartWait && m_btnObj) {
        ObjectManager::Destroy(m_btnObj);
        m_btnObj = nullptr;
      }
      m_isStartWait = false;
    }

    DisplayInfo *di = ObjectManager::FindGameObject<DisplayInfo>();
    if (di) {
      di->SetPaused(m_isPaused);
    }

    if (m_isPaused) {
      SceneManager::SetTimeScale(0.0f);
    } else {
      SceneManager::SetTimeScale(1.0f);
    }
  }

  // ECS システム一括更新（Movement/Enemy/Animation/Bullet/Particle）
  World::GetInstance().UpdateSystems();
}

void PlayScene::Draw() {
  SceneBase::Draw();
  NavigationManager::GetInstance().Draw();
}
