#pragma once
/// <summary>
/// シーンの切り替えを管理するクラス
/// </summary>
#include <list>
#include <string>

class SceneFactory;
class SceneBase;

/// <summary>
/// 現在のシーンを呼び出している名前空間
/// </summary>
namespace SceneManager {
void Start();
void Update();
void Draw();
void Release();

/// <summary>
/// 現在のシーンを取得する
/// </summary>
SceneBase *CurrentScene();

/// <summary>
/// 現在のシーンとして登録する
/// </summary>
void SetCurrentScene(SceneBase *scene);

/// <summary>
/// Change Scene
/// </summary>
/// <param name="sceneName">Scene Name</param>
/// <param name="pushHistory">Push to history stack</param>
void ChangeScene(const std::string &sceneName, bool pushHistory = true);

/// <summary>
/// Return to previous scene
/// </summary>
void ReturnScene();

/// <summary>
/// Get Previous Scene Name
/// </summary>
std::string GetPreviousSceneName();

/// <summary>
/// Get Current Scene Name
/// </summary>
std::string GetCurrentSceneName();

/// <summary>
/// Set Time Scale (1.0f = normal, 0.0f = stop)
/// </summary>
void SetTimeScale(float scale);

/// <summary>
/// Get Time Scale
/// </summary>
float GetTimeScale();

/// <summary>
/// 前のフレームからの経過時間（秒）
/// </summary>
float DeltaTime();

/// <summary>
/// 前のフレームからの経過時間（秒）(TimeScale非考慮)
/// </summary>
float UnscaledDeltaTime();

/// <summary>
/// Reload Current Scene
/// </summary>
void Reload();

void Exit();

} // namespace SceneManager