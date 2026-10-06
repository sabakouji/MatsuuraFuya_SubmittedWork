// 70行目 [Start] 初期化
// 78行目 [Update] 更新処理
// 90行目 [Draw] 描画処理
// 112行目 [ChangeScene] シーン変更
// 132行目 [GetCurrentSceneName] 現在のシーン名取得
#include "SceneManager.h"
#include "ECS/World.h"
#include "ObjectManager.h"
#include "SceneFactory.h"
#include "Time.h"
#include "sceneBase.h"
#include <Windows.h>

#include <stack>
#include <vector>

namespace {
std::string currentName_;
std::string nextName_;
std::unique_ptr<SceneBase> currentScene_;
std::unique_ptr<SceneFactory> factory_; // Factory

// History Stack
std::vector<std::string> sceneStack_;

// DeltaTime用
LARGE_INTEGER freq;
LARGE_INTEGER current;
float deltaTime;
float unscaledDeltaTime;
static const int REC_SIZE = 60;
float record[REC_SIZE]; // 60回分のリングバッファ
int recCount = 0;

// Time Scale
float timeScale_ = 1.0f;

// Reload Request
bool reloadRequested_ = false;

void timeInit() {
  bool ret = QueryPerformanceFrequency(&freq);
  assert(ret);
  QueryPerformanceCounter(&current);
}

void timeUpdate() {
  LARGE_INTEGER last = current;
  QueryPerformanceCounter(&current);
  float t =
      static_cast<float>(current.QuadPart - last.QuadPart) / freq.QuadPart;
  float t2 = t;
  // deltaTimeは、平均フレームレートの2倍を超えないようにする
  if (recCount >= REC_SIZE) {
    float sum = 0;
    for (int i = 0; i < REC_SIZE; i++)
      sum += record[i];
    sum /= REC_SIZE;
    if (t2 > sum * 3.0f)
      t2 = sum * 3.0f;
    //			if (t2 <= sum * 1.1f)
    //				t2 = sum;
  }
  record[recCount % REC_SIZE] = t;
  recCount++;
  unscaledDeltaTime = t2;
  deltaTime = t2 * timeScale_;
}
}; // namespace

void SceneManager::SetTimeScale(float scale) { timeScale_ = scale; }

float SceneManager::GetTimeScale() { return timeScale_; }

void SceneManager::Start() {
  timeInit();
  nextName_ = "TitleScene";
  currentName_ = "TitleScene";
  factory_ = std::make_unique<SceneFactory>();
  currentScene_ = factory_->CreateFirst(); // resetではなく直接代入
}

void SceneManager::Update() {
  if (nextName_ != currentName_ || reloadRequested_) {
    currentScene_.reset();
    ObjectManager::ChangeScene();

    // ECSのエンティティ（パーティクル・弾・敵ボディ）を破棄する。
    // ObjectManager::ChangeScene()はGameObjectしか破棄しないため、
    // ここでクリアしないとパーティクル等がWorldに残り続ける。
    // EffectManagerはDontDestroyで生き残り毎フレームDrawされる一方、
    // ParticleSystemはPlayScene::Update()からしか動かないので、
    // 残ったパーティクルは寿命が減らず次のシーンに描画され続けてしまう。
    World::GetInstance().Clear();

    // If reloading, nextName_ is nominally the same as currentName_ (or
    // whatever was set) Actually if reloadRequested_ is true, we just want to
    // recreate currentName_. If nextName_ changed AND reload requested,
    // nextName_ takes precedence? Let's assume Reload() doesn't change
    // nextName_, allowing "same scene" transition.

    // If we rely on nextName_, ChangeScene sets it.
    // But if we want same scene, nextName_ == currentName_ usually means no op.
    // So we add reloadRequested_ to the check.

    // However, we must ensure nextName_ is correct.
    if (reloadRequested_) {
      nextName_ = currentName_;
    }

    currentScene_ = factory_->Create(nextName_);
    currentName_ = nextName_;
    reloadRequested_ = false;
  }
  if (currentScene_)
    currentScene_->Update();
}

void SceneManager::Draw() {
  timeUpdate();

  if (currentScene_ != nullptr)
    currentScene_->Draw();
}

void SceneManager::Release() {
  if (currentScene_ != nullptr) {
    currentScene_.reset();
    currentScene_ = nullptr;
  }
  factory_.reset();
}

SceneBase *SceneManager::CurrentScene() { return currentScene_.get(); }

void SceneManager::SetCurrentScene(SceneBase * /*scene*/) {
  // 使用禁止：所有権が曖昧になりクラッシュ原因になる
  assert(!"Do not use SetCurrentScene(raw*). Pass std::unique_ptr instead.");
}

void SceneManager::ChangeScene(const std::string &sceneName, bool pushHistory) {
  if (pushHistory && !currentName_.empty()) {
    sceneStack_.push_back(currentName_);
  }
  nextName_ = sceneName;
}

void SceneManager::ReturnScene() {
  if (sceneStack_.empty())
    return;

  std::string prev = sceneStack_.back();
  sceneStack_.pop_back();

  ChangeScene(prev, false); // Don't push current when returning
}

std::string SceneManager::GetPreviousSceneName() {
  if (sceneStack_.empty())
    return "";
  return sceneStack_.back();
}

std::string SceneManager::GetCurrentSceneName() { return currentName_; }

float SceneManager::DeltaTime() { return deltaTime; }

float SceneManager::UnscaledDeltaTime() { return unscaledDeltaTime; }

void SceneManager::Reload() { reloadRequested_ = true; }

void SceneManager::Exit() { PostQuitMessage(0); }
