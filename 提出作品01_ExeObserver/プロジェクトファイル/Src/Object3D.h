#pragma once
#include "Animator.h"
#include "FbxMesh.h"
#include "GameObject.h"
#include "MeshCollider.h"

class Transform {
public:
  VECTOR3 position;
  VECTOR3 rotation;
  VECTOR3 scale;
  Transform() {
    position = VECTOR3(0, 0, 0);
    rotation = VECTOR3(0, 0, 0); // ラジアン角
    scale = VECTOR3(1, 1, 1);
  }
  const MATRIX4X4 matrix() const {
    MATRIX4X4 scaleM = XMMatrixScaling(scale.x, scale.y, scale.z);
    MATRIX4X4 rotX = XMMatrixRotationX(rotation.x);
    MATRIX4X4 rotY = XMMatrixRotationY(rotation.y);
    MATRIX4X4 rotZ = XMMatrixRotationZ(rotation.z);
    MATRIX4X4 trans = XMMatrixTranslation(position.x, position.y, position.z);
    return scaleM * rotZ * rotX * rotY * trans;
  }
  VECTOR3 Right() const {
    // Y軸回転のみで右方向ベクトルを返す（XZ平面）
    float y = rotation.y;
    return VECTOR3(cosf(y), 0, -sinf(y));
  }
};

class SphereCollider {
public:
  VECTOR3 center;
  float radius;
  SphereCollider() {
    center = VECTOR3(0, 0, 0);
    radius = 0.0f;
  }
};

class Object3D : public GameObject {
public:
  Object3D();
  virtual ~Object3D();
  virtual void Update() override;
  virtual void Draw() override;

  const VECTOR3 Position() const { return transform.position; };
  const VECTOR3 Rotation() const { return transform.rotation; };
  const VECTOR3 Scale() const { return transform.scale; };
  const MATRIX4X4 Matrix() const {
    return transform.matrix();
  } // transform.matrix() is not const-qualified in Transform struct (inline),
    // so we need to fix Transform as well or implement here.
    // However, Transform::matrix() calculates and returns a value, so it should
  // be const. Looking at Transform struct above: const MATRIX4X4 matrix() { ...
  // } - it is NOT const method.

  void SetPosition(const VECTOR3 &pos);
  void SetPosition(float x, float y, float z);
  void SetRotation(const VECTOR3 &pos);
  void SetRotation(float x, float y, float z);
  void SetScale(const VECTOR3 &pos);
  void SetScale(float x, float y, float z);

  virtual SphereCollider Collider();

  /// <summary>
  /// 球とメッシュの当たり判定をする
  /// 当たった場合にのみ、pushに押し返す場所を返す
  /// </summary>
  /// <param name="sphere">球体</param>
  /// <param name="push">押し返す座標を格納する場所</param>
  /// <returns>当たった場合にtrue</returns>
  virtual bool HitSphereToMeshPush(const SphereCollider &sphere,
                                   VECTOR3 *push = nullptr);

  /// <summary>
  /// 球とメッシュの当たり判定をする
  /// 当たった場合は、衝突情報を返す
  /// </summary>
  /// <param name="sphere">球体</param>
  /// <param name="collOut">衝突情報を格納する場所</param>
  /// <returns>当たった場合にtrue</returns>
  virtual bool HitSphereToMesh(const SphereCollider &sphere,
                               MeshCollider::CollInfo *collOut = nullptr);

  /// <summary>
  /// 直線とメッシュの当たり判定をする
  /// 当たった場合は、衝突情報を返す
  /// </summary>
  /// <param name="from">直線の始点</param>
  /// <param name="to">直線の終点</param>
  /// <param name="collOut">衝突情報を格納する場所</param>
  /// <returns>当たった場合にtrue</returns>
  virtual bool HitLineToMesh(const VECTOR3 &from, const VECTOR3 &to,
                             MeshCollider::CollInfo *collOut = nullptr);

  /// <summary>
  /// 球と球の当たり判定をする
  /// 自分の球は、Collider()で取得する
  /// </summary>
  /// <param name="target">相手の球</param>
  /// <param name="withY">falseにするとYの座標差を無視する</param>
  /// <returns>重なり量</returns>
  virtual float HitSphereToSphere(const SphereCollider &target,
                                  bool withY = true);

  /// <summary>
  /// 球と球の当たり判定をする
  /// 当たった場合は、pushに押し返す場所を返す
  /// </summary>
  /// <param name="target">相手の球</param>
  /// <param name="withY">falseにするとYの座標差を無視する</param>
  /// <returns>当たった場合にtrue</returns>
  virtual bool HitSphereToSpherePush(const SphereCollider &target,
                                     bool withY = true,
                                     VECTOR3 *push = nullptr);

  /// <summary>
  /// メッシュのアドレスを返す
  /// </summary>
  /// <returns>メッシュのアドレス</returns>
  CFbxMesh *Mesh() { return mesh; }

  /// <summary>
  /// コリジョンメッシュのアドレスを返す
  /// </summary>
  /// <returns>コリジョンメッシュのアドレス</returns>
  MeshCollider *MeshCol() { return meshCol; }

  /// <summary>
  /// アニメーターのアドレスを返す
  /// </summary>
  /// <returns>アニメーターのアドレス</returns>
  Animator *GetAnimator() { return animator; }

  /// <summary>
  /// コリジョンメッシュのBall情報を返す
  /// </summary>
  /// <returns>SphereCollider</returns>
  SphereCollider GetSphereCollider();

  /// <summary>
  /// トゥーンシェーディング（輪郭線付き）で描画するかを設定する
  /// メッシュは複数のオブジェクトで共有されることがあるため、
  /// 有効・無効の切り替えはオブジェクト単位で保持する
  /// </summary>
  /// <param name="enabled">トゥーンシェーディングで描画するならtrue</param>
  void SetToonEnabled(bool enabled) { toonEnabled = enabled; }

  /// <summary>
  /// トゥーンシェーディングで描画するかを返す
  /// </summary>
  /// <returns>トゥーンシェーディングで描画するならtrue</returns>
  bool ToonEnabled() const { return toonEnabled; }

  /// <summary>
  /// 輪郭線の設定を行う
  /// 設定はメッシュに保持されるため、同じメッシュを共有する
  /// 全てのオブジェクトに反映される
  /// </summary>
  /// <param name="enabled">輪郭線を描画するならtrue</param>
  /// <param name="color">輪郭線の色</param>
  /// <param name="width">輪郭線の太さ(ピクセル単位)</param>
  void SetOutline(bool enabled, const VECTOR4 &color = VECTOR4(0, 0, 0, 1),
                  float width = 2.0f);

protected:
  CFbxMesh *mesh;
  Animator *animator;
  MeshCollider *meshCol;
  Transform transform;
  bool toonEnabled;   // トゥーンシェーディングで描画するか
};