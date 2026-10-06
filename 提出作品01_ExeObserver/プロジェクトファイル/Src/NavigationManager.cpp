#include "NavigationManager.h"

#include "DetourCommon.h"
#include "DetourNavMeshBuilder.h"
#include "FbxMesh.h"
#include "MapBase.h"
#include "ObjectManager.h"
#include "RecastAlloc.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include "Sprite3D.h"

using namespace DirectX;

//---------------------------------------------------------------
// 
class BuildContext : public rcContext {
public:
  void doLog(const rcLogCategory category, const char *msg,
             const int len) override {
    // Simple logging
    if (category == RC_LOG_ERROR) {
      std::cerr << "[Recast Error]: " << msg << std::endl;
    } else {
      std::cout << "[Recast Log]: " << msg << std::endl;
    }
  }
};


NavigationManager &NavigationManager::GetInstance() {
  static NavigationManager instance;
  return instance;
}

//-----------------------------------------------------------------------------
// 初期化
NavigationManager::NavigationManager()
    : m_ctx(nullptr), m_triareas(nullptr), m_solid(nullptr), m_chf(nullptr),
      m_cset(nullptr), m_pmesh(nullptr), m_dmesh(nullptr), m_navMesh(nullptr),
      m_navQuery(nullptr) {
  m_ctx = new BuildContext();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// 終了処理
NavigationManager::~NavigationManager() {
  Cleanup();
  delete m_ctx;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ナビメッシュの構築
void NavigationManager::Initialize() {
	// Recast configuration setup
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ナビメッシュの構築
void NavigationManager::Cleanup() {
	// リキャスト関連のメモリを解放
    rcFreeHeightField(m_solid);
    m_solid = nullptr;
    rcFreeCompactHeightfield(m_chf);
    m_chf = nullptr;
    rcFreeContourSet(m_cset);
    m_cset = nullptr;
    rcFreePolyMesh(m_pmesh);
    m_pmesh = nullptr;
    rcFreePolyMeshDetail(m_dmesh);
    m_dmesh = nullptr;
    dtFreeNavMesh(m_navMesh);
    m_navMesh = nullptr;
    dtFreeNavMeshQuery(m_navQuery);
    m_navQuery = nullptr;

	// トライエリアのメモリを解放
    delete[] m_triareas;
    m_triareas = nullptr;
}
//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
// ジオメトリの抽出
bool NavigationManager::ExtractGeometry(MapManager *mapManager,
                                        InputGeom &geom) {
	// ジオメトリの初期化
    geom.verts.clear();
    geom.tris.clear();

    
    geom.bmin[0] = FLT_MAX;
    geom.bmin[1] = FLT_MAX;
    geom.bmin[2] = FLT_MAX;
    geom.bmax[0] = -FLT_MAX;
    geom.bmax[1] = -FLT_MAX;
    geom.bmax[2] = -FLT_MAX;

	// MapBaseオブジェクトを全て取得
    std::list<MapBase*> maps = ObjectManager::FindGameObjects<MapBase>();
    if (maps.empty()) {
        std::cerr << "[NavMesh] No MapBase objects found!" << std::endl;
        return false;
    }

	// 頂点と三角形の総数をカウント
    int totalVerts = 0;
    int totalTris = 0;

	//マニュアルオフセットの例（必要に応じて調整）
    float manualOffsetX = 0.0f;
    float manualOffsetY = 0.0f;
    float manualOffsetZ = 0.0f;

    char offLog[256];
    sprintf_s(offLog, "[NavMesh] Manual Offset applied: (%.2f, %.2f, %.2f)\n",
        manualOffsetX, manualOffsetY, manualOffsetZ);
    OutputDebugStringA(offLog);

	// 各MapBaseオブジェクトからジオメトリを抽出
    for (MapBase* map : maps) {
		// マップオブジェクトが持つメッシュを取得
        if (!map)
            continue;
        CFbxMesh* mesh = map->Mesh();
		// メッシュがない場合はスキップ
        if (!mesh)
            continue;
        MATRIX4X4 world = map->Matrix();

		// 各サブメッシュを処理
        for (DWORD i = 0; i < mesh->m_dwMeshNum; ++i) {
			// サブメッシュの頂点をワールド変換して追加
            const CFbxMeshArray& subMesh = mesh->m_pMeshArray[i];
            int currentVertOffset = totalVerts;

			//垂直方向のオフセットの適用（必要に応じて調整）
            for (DWORD v = 0; v < subMesh.m_dwVerticesNum; ++v) {
                VECTOR3 rawPos;
				// メッシュタイプに応じて頂点位置を取得
                if (mesh->m_nMeshType == 1 && subMesh.m_vStaticVerticesNormal) {
                    rawPos = subMesh.m_vStaticVerticesNormal[v].Pos;
                }
				// スキンメッシュの場合、頂点位置を取得
                else if (mesh->m_nMeshType == 2 && subMesh.m_vSkinVerticesNormal) {
                    rawPos = subMesh.m_vSkinVerticesNormal[v].Pos;
                }
				// それ以外のメッシュタイプの場合は、頂点位置をゼロにする（安全策）
                else {
                    rawPos = VECTOR3(0, 0, 0);
                }

				// ワールド変換を適用
                VECTOR3 worldPos = XMVector3TransformCoord(rawPos, world);

				// マニュアルオフセットの適用
                worldPos.x += manualOffsetX;
                worldPos.y += manualOffsetY;
                worldPos.z += manualOffsetZ;

				// ジオメトリに頂点を追加
                geom.verts.push_back(worldPos.x);
                geom.verts.push_back(worldPos.y);
                geom.verts.push_back(worldPos.z);

				// バウンディングボックスの更新
                if (worldPos.x < geom.bmin[0])
                    geom.bmin[0] = worldPos.x;
                if (worldPos.y < geom.bmin[1])
                    geom.bmin[1] = worldPos.y;
                if (worldPos.z < geom.bmin[2])
                    geom.bmin[2] = worldPos.z;
                if (worldPos.x > geom.bmax[0])
                    geom.bmax[0] = worldPos.x;
                if (worldPos.y > geom.bmax[1])
                    geom.bmax[1] = worldPos.y;
                if (worldPos.z > geom.bmax[2])
                    geom.bmax[2] = worldPos.z;

				// デバッグ: 特定のポイント（例: (0, -50)）がバウンディングボックス内にあるかをチェック
                totalVerts++;
            }

			// サブメッシュのインデックスを追加（両面分）
            for (DWORD t = 0; t < subMesh.m_dwIndicesNum / 3; ++t) {
                int i0 = (int)subMesh.m_nIndices[t * 3 + 0] + currentVertOffset;
                int i1 = (int)subMesh.m_nIndices[t * 3 + 1] + currentVertOffset;
                int i2 = (int)subMesh.m_nIndices[t * 3 + 2] + currentVertOffset;

				// 両面分の三角形を追加
                geom.tris.push_back(i0);
                geom.tris.push_back(i1);
                geom.tris.push_back(i2);

				// 逆向きの三角形も追加して両面をカバー
                geom.tris.push_back(i0);
                geom.tris.push_back(i2);
                geom.tris.push_back(i1);

				// デバッグ: 三角形の頂点位置を取得してバウンディングボックスを確認
                totalTris += 2;

				// デバッグ: 三角形の頂点位置を取得してバウンディングボックスを確認
                float v0x = geom.verts[i0 * 3 + 0];
                float v0z = geom.verts[i0 * 3 + 2];
                float v1x = geom.verts[i1 * 3 + 0];
                float v1z = geom.verts[i1 * 3 + 2];
                float v2x = geom.verts[i2 * 3 + 0];
                float v2z = geom.verts[i2 * 3 + 2];

				// デバッグ: 三角形のXZ平面でのバウンディングボックスを計算してログに出力
                float minX = min(v0x, min(v1x, v2x));
                float maxX = max(v0x, max(v1x, v2x));
                float minZ = min(v0z, min(v1z, v2z));
                float maxZ = max(v0z, max(v1z, v2z));
            }
        }
    }

	// ジオメトリの総数を設定
    geom.nverts = totalVerts;
    geom.ntris = totalTris;

	// デバッグ: 抽出されたジオメトリの情報をログに出力
    char buf[256];
    sprintf_s(buf, "[NavMesh] Extracted %d verts and %d tris.\n", geom.nverts,
        geom.ntris);
    OutputDebugStringA(buf);

    sprintf_s(buf,
        "[NavMesh] Bounds: Min(%.2f, %.2f, %.2f) Max(%.2f, %.2f, %.2f)\n",
        geom.bmin[0], geom.bmin[1], geom.bmin[2], geom.bmax[0],
        geom.bmax[1], geom.bmax[2]);
    OutputDebugStringA(buf);

    return (geom.nverts > 0);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ナビメッシュの構築
void NavigationManager::BuildNavMesh(MapManager *mapManager) {
	// 既存のナビメッシュデータをクリーンアップ
    Cleanup();

	// ジオメトリの抽出
    InputGeom geom;
    if (!ExtractGeometry(mapManager, geom)) {
        return;
    }

	// Recastの構築設定
    memset(&m_cfg, 0, sizeof(m_cfg));
    m_cfg.cs = 0.3f; // Cell size
    m_cfg.ch = 0.2f; // Cell height
    m_cfg.walkableSlopeAngle = 60.0f;
    m_cfg.walkableHeight = (int)ceilf(2.0f / m_cfg.ch); // Agent height ~2.0
    m_cfg.walkableClimb = (int)floorf(2.0f / m_cfg.ch); // Increased climb
    m_cfg.walkableRadius = (int)ceilf(0.5f / m_cfg.cs); // Agent radius ~0.5
    m_cfg.maxEdgeLen = (int)(12.0f / m_cfg.cs);
    m_cfg.maxSimplificationError = 1.3f;
    m_cfg.minRegionArea = (int)rcSqr(8);
    m_cfg.mergeRegionArea = (int)rcSqr(20);
    m_cfg.maxVertsPerPoly = 6;
    m_cfg.detailSampleDist = 6.0f < 0.9f ? 0 : m_cfg.cs * 6.0f;
    m_cfg.detailSampleMaxError = m_cfg.ch * 1.0f;

	// ジオメトリのバウンディングボックスをナビメッシュの構築範囲として設定
    m_cfg.bmin[0] = -600.0f;
    m_cfg.bmin[1] = -200.0f;
    m_cfg.bmin[2] = -600.0f;
    m_cfg.bmax[0] = 600.0f;
    m_cfg.bmax[1] = 200.0f;
    m_cfg.bmax[2] = 600.0f;

	// ナビメッシュのグリッドサイズを計算
    rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	// ナビメッシュの構築
    m_solid = rcAllocHeightfield();
    if (!m_solid ||
        !rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height,
            m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch)) {
        return;
    }

	// トライエリアのメモリを確保して初期化
    m_triareas = new unsigned char[geom.ntris];
    memset(m_triareas, 0, geom.ntris * sizeof(unsigned char));

	// ジオメトリの三角形をトライエリアにマーク
    rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, geom.verts.data(),
        geom.nverts, geom.tris.data(), geom.ntris,
        m_triareas);

	// トライエリアをもとに高さフィールドをラスタライズ
    if (!rcRasterizeTriangles(m_ctx, geom.verts.data(), geom.nverts,
        geom.tris.data(), m_triareas, geom.ntris, *m_solid,
        m_cfg.walkableClimb)) {
        return;
    }

	// ラスタライズされた高さフィールドをフィルタリングして、歩行可能なエリアを特定
    rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
    rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb,
        *m_solid);
    rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);

	// フィルタリングされた高さフィールドをもとに、コンパクトな高さフィールドを構築
    m_chf = rcAllocCompactHeightfield();
    if (!m_chf ||
        !rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight,
            m_cfg.walkableClimb, *m_solid, *m_chf)) {
        return;
    }

	// 歩行可能なエリアをさらに侵食して、エージェントの半径を考慮した安全なナビメッシュを構築
    if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf)) {
        return;
    }

	// コンパクトな高さフィールドをもとに、距離フィールドを構築して、
    // キャラクターがどれだけ歩行可能なエリアから離れているかを計算
    if (!rcBuildDistanceField(m_ctx, *m_chf)) {
        return;
    }

	// 距離フィールドをもとに、歩行可能なエリアを複数のリージョンに分割して、ナビメッシュの構築を効率化
    if (!rcBuildRegions(m_ctx, *m_chf, 0, m_cfg.minRegionArea,
        m_cfg.mergeRegionArea)) {
        return;
    }

	// リージョンをもとに、歩行可能なエリアの輪郭を抽出して、ナビメッシュのポリゴンを構築
    m_cset = rcAllocContourSet();
    if (!m_cset || !rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError,
        m_cfg.maxEdgeLen, *m_cset)) {
        return;
    }

	// 輪郭をもとに、ナビメッシュのポリゴンを構築
    m_pmesh = rcAllocPolyMesh();
    if (!m_pmesh ||
        !rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh)) {
        return;
    }

	// ポリゴンメッシュをもとに、ナビメッシュの詳細なジオメトリを構築
    m_dmesh = rcAllocPolyMeshDetail();
    if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist,
        m_cfg.detailSampleMaxError, *m_dmesh)) {
        return;
    }

	// ポリゴンのエリアタイプをもとに、ナビメッシュのフラグを設定
    for (int i = 0; i < m_pmesh->npolys; ++i) {
        if (m_pmesh->areas[i] == RC_WALKABLE_AREA) {
            m_pmesh->flags[i] = 1; // POLYFLAGS_WALKABLE
        }
    }

	// ポリゴンメッシュと詳細メッシュをもとに、Detourのナビメッシュデータを構築
    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));
    params.verts = m_pmesh->verts;
    params.vertCount = m_pmesh->nverts;
    params.polys = m_pmesh->polys;
    params.polyAreas = m_pmesh->areas;
    params.polyFlags = m_pmesh->flags;
    params.polyCount = m_pmesh->npolys;
    params.nvp = m_pmesh->nvp;

	// ナビメッシュのバウンディングボックスを設定
    params.bmin[0] = m_pmesh->bmin[0];
    params.bmin[1] = m_pmesh->bmin[1];
    params.bmin[2] = m_pmesh->bmin[2];
    params.bmax[0] = m_pmesh->bmax[0];
    params.bmax[1] = m_pmesh->bmax[1];
    params.bmax[2] = m_pmesh->bmax[2];

	// 詳細メッシュのデータを設定
    params.detailMeshes = m_dmesh->meshes;
    params.detailVerts = m_dmesh->verts;
    params.detailVertsCount = m_dmesh->nverts;
    params.detailTris = m_dmesh->tris;
    params.detailTriCount = m_dmesh->ntris;

	// ナビメッシュの構築に必要なパラメータを設定
    params.walkableHeight = (float)m_cfg.walkableHeight * m_cfg.ch;
    params.walkableRadius = (float)m_cfg.walkableRadius * m_cfg.cs;
    params.walkableClimb = (float)m_cfg.walkableClimb * m_cfg.ch;
    params.cs = m_cfg.cs;
    params.ch = m_cfg.ch;

	// タイルのユーザーデータやオフメッシュコネクションがない場合は、これらのパラメータをゼロに設定
    params.buildBvTree = true;

	// Detourのナビメッシュデータを構築
    unsigned char* navData = 0;
    int navDataSize = 0;

	// Detourのナビメッシュデータの構築に失敗した場合は、エラーをログに出力して終了
    if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
        return;
    }

	// Detourのナビメッシュデータをもとに、ナビメッシュを初期化
    m_navMesh = dtAllocNavMesh();
    if (!m_navMesh || dtStatusFailed(m_navMesh->init(navData, navDataSize,
        DT_TILE_FREE_DATA))) {
        dtFree(navData);
        return;
    }

	// Detourのナビメッシュクエリを初期化
    m_navQuery = dtAllocNavMeshQuery();
    if (!m_navQuery) {
        OutputDebugStringA("ERROR: Could not allocate Detour navmesh query\n");
        return;
    }
	// Detourのナビメッシュクエリの初期化に失敗した場合は、エラーをログに出力して終了
    if (dtStatusFailed(m_navQuery->init(m_navMesh, 2048))) {
        OutputDebugStringA("ERROR: Could not init Detour navmesh query\n");
        return;
    }
    OutputDebugStringA("NavMesh Query Initialized Successfully.\n");

	// デバッグ: ナビメッシュの構築が成功したことをログに出力
    char buf[256];
    sprintf_s(buf, "NavMesh Built Successfully. Polys: %d\n", m_pmesh->npolys);
    OutputDebugStringA(buf);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// パスの検索
bool NavigationManager::FindPath(const VECTOR3 &startPos, const VECTOR3 &endPos,
                                 std::vector<VECTOR3> &outPath) {
	// ナビメッシュクエリが初期化されていない場合は、エラーをログに出力して終了
    if (!m_navMesh || !m_navQuery)
        return false;

	// 開始位置と終了位置をRecast/Detourの形式に変換
    float startPosRecast[3] = { startPos.x, startPos.y, startPos.z };
    float endPosRecast[3] = { endPos.x, endPos.y, endPos.z };

	// ナビメッシュ上の最近傍のポイントを検索するための検索範囲を設定
    float extents[3] = {
        200.0f, 200.0f,
        200.0f };

	// ナビメッシュ上の開始位置と終了位置の最近傍のポイントを検索
    dtPolyRef startRef, endRef;

	// デバッグ: 最近傍のポイントを検索する前の開始位置と終了位置をログに出力
    float startPt[3], endPt[3];

	// ナビメッシュ上の開始位置の最近傍のポイントを検索
    m_navQuery->findNearestPoly(startPosRecast, extents, &m_filter, &startRef,
        startPt);

	// デバッグ: 最近傍のポイントを検索した後の開始位置と終了位置をログに出力
    char dbg[512];
    sprintf_s(dbg,
        "[NavLog] FindPath Start. Pos(%.2f, %.2f, %.2f) Ref:%u "
        "Nearest(%.2f, %.2f, %.2f)\n",
        startPos.x, startPos.y, startPos.z, (unsigned int)startRef,
        startPt[0], startPt[1], startPt[2]);
    OutputDebugStringA(dbg);

	// ナビメッシュ上の終了位置の最近傍のポイントを検索
    m_navQuery->findNearestPoly(endPosRecast, extents, &m_filter, &endRef, endPt);

	// デバッグ: 最近傍のポイントを検索した後の開始位置と終了位置をログに出力
    sprintf_s(dbg,
        "[NavLog] FindPath End. Pos(%.2f, %.2f, %.2f) Ref:%u Nearest(%.2f, "
        "%.2f, %.2f)\n",
        endPos.x, endPos.y, endPos.z, (unsigned int)endRef, endPt[0],
        endPt[1], endPt[2]);
    OutputDebugStringA(dbg);

	// 開始位置または終了位置の最近傍のポイントが見つからない場合は、エラーをログに出力して終了
    if (!startRef || !endRef) {
        if (!startRef) {
            sprintf_s(dbg,
                "[NavLog] ERROR: Failed to find START poly ref. Pos(%.2f, "
                "%.2f, %.2f)\n",
                startPos.x, startPos.y, startPos.z);
            OutputDebugStringA(dbg);
        }
        if (!endRef) {
            sprintf_s(dbg,
                "[NavLog] ERROR: Failed to find END poly ref. Pos(%.2f, %.2f, "
                "%.2f)\n",
                endPos.x, endPos.y, endPos.z);
            OutputDebugStringA(dbg);
        }
        return false;
    }

	// ナビメッシュ上の開始位置から終了位置へのパスを検索
    dtPolyRef path[256];
    int pathCount = 0;

	// デバッグ: パス検索の開始をログに出力
    dtStatus status = m_navQuery->findPath(startRef, endRef, startPt, endPt,
        &m_filter, path, &pathCount, 256);

	// デバッグ: パス検索の結果をログに出力
    if (dtStatusFailed(status)) {
        OutputDebugStringA("[NavLog] ERROR: findPath returned failure status.\n");
    }

	// パスが見つからない場合は、エラーをログに出力して終了
    if (pathCount <= 0)
        return false;

	// パス上のポイントを直線化して、キャラクターが移動するための実際の経路を生成
    float straightPath[256 * 3];
    unsigned char straightPathFlags[256];
    dtPolyRef straightPathRefs[256];
    int straightPathCount = 0;

	// デバッグ: パスの直線化の開始をログに出力
    m_navQuery->findStraightPath(startPt, endPt, path, pathCount, straightPath,
        straightPathFlags, straightPathRefs,
        &straightPathCount, 256, 0);

	// デバッグ: 直線化されたパスのポイント数をログに出力
    outPath.clear();
    for (int i = 0; i < straightPathCount; ++i) {
        outPath.push_back({ straightPath[i * 3], straightPath[i * 3 + 1],
                           straightPath[i * 3 + 2] });
    }

	// デバッグ: 直線化されたパスのポイント数と最初のポイントをログに出力
    char buf[512];
    sprintf_s(buf, "[NavigationManager] Path found. Points: %zu\n",
        outPath.size());
    OutputDebugStringA(buf);

	// デバッグ: パスの最初のポイントをログに出力
    if (!outPath.empty()) {
        sprintf_s(buf, "  Start: (%.2f, %.2f, %.2f)\n", startPos.x, startPos.y,
            startPos.z);
        OutputDebugStringA(buf);
        sprintf_s(buf, "  End: (%.2f, %.2f, %.2f)\n", endPos.x, endPos.y, endPos.z);
        OutputDebugStringA(buf);
        sprintf_s(buf, "  Next Point: (%.2f, %.2f, %.2f)\n", outPath[0].x,
            outPath[0].y, outPath[0].z);
        OutputDebugStringA(buf);
    }

    return true;
}

void NavigationManager::Update() {}

void NavigationManager::Draw() {
}
