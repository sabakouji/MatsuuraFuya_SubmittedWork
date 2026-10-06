#pragma once
#include "MapManager.h"
#include "Object3D.h"
#include <memory>
#include <vector>

// Recast & Detour Headers
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Recast.h"

// Helper struct for input geometry
struct InputGeom {
  std::vector<float> verts;
  std::vector<int> tris;
  float bmin[3];
  float bmax[3];
  int nverts;
  int ntris;
};

class NavigationManager {
public:
  static NavigationManager &GetInstance();

  void Initialize();
  void BuildNavMesh(MapManager *mapManager);
  void Update();
  void Draw(); // Debug draw
  void Cleanup();

  bool FindPath(const VECTOR3 &startPos, const VECTOR3 &endPos,
                std::vector<VECTOR3> &outPath);
  bool RandomPoint(VECTOR3 &outPoint);

private:
  NavigationManager();
  ~NavigationManager();

  // Recast configuration
  rcConfig m_cfg;
  rcContext *m_ctx;

  // NavMesh data
  unsigned char *m_triareas;
  rcHeightfield *m_solid;
  rcCompactHeightfield *m_chf;
  rcContourSet *m_cset;
  rcPolyMesh *m_pmesh;
  rcPolyMeshDetail *m_dmesh;

  // Detour data
  dtNavMesh *m_navMesh;
  dtNavMeshQuery *m_navQuery;
  dtQueryFilter m_filter;

  // Helper to extract geometry from MapManager
  bool ExtractGeometry(MapManager *mapManager, InputGeom &geom);
};
