#pragma once
// NodeValueRegistry
// External CSV-driven numeric/bool parameter store for BehaviorTree nodes.
// Loaded once at PlayScene start. The visual editor does NOT edit these values
// (inline numeric input is disabled); only Enum (pull-down) parameters such as
// targetBox/boxType remain editable.
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

class NodeValueRegistry {
public:
  static NodeValueRegistry &Instance();

  // Load CSV from path. Returns true on successful open.
  // CSV format: TypeId,PropertyName,Value  (lines starting with '#' ignored).
  bool Load(const std::string &csvPath);

  // Clear all loaded entries.
  void Clear();

  bool Has(const std::string &typeId, const std::string &propName) const;

  // Returns CSV value as string. Returns defaultValue if missing.
  std::string Get(const std::string &typeId, const std::string &propName,
                  const std::string &defaultValue = "") const;

  // Apply CSV overrides into the given properties map (overwrites matching keys).
  void ApplyOverrides(const std::string &typeId,
                      std::map<std::string, std::string> &properties) const;

private:
  NodeValueRegistry() = default;
  NodeValueRegistry(const NodeValueRegistry &) = delete;
  NodeValueRegistry &operator=(const NodeValueRegistry &) = delete;

  static std::string MakeKey(const std::string &typeId,
                             const std::string &propName);

  // Key: "TypeId|PropertyName" -> Value (string form, parsed at use site).
  std::unordered_map<std::string, std::string> m_values;
  // Per-typeId list of (propName, value) pairs for fast ApplyOverrides.
  std::unordered_map<std::string,
                     std::vector<std::pair<std::string, std::string>>>
      m_byType;
};
