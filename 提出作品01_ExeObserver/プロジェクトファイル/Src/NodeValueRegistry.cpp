#include "NodeValueRegistry.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

NodeValueRegistry &NodeValueRegistry::Instance() {
  static NodeValueRegistry s_instance;
  return s_instance;
}

std::string NodeValueRegistry::MakeKey(const std::string &typeId,
                                       const std::string &propName) {
  return typeId + "|" + propName;
}

static std::string TrimWS(const std::string &s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos)
    return std::string();
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

void NodeValueRegistry::Clear() {
  m_values.clear();
  m_byType.clear();
}

bool NodeValueRegistry::Load(const std::string &csvPath) {
  Clear();
  std::ifstream ifs(csvPath);
  if (!ifs.is_open()) {
    std::cerr << "WARNING: NodeValueRegistry::Load - Failed to open " << csvPath
              << std::endl;
    return false;
  }

  std::string line;
  int lineNo = 0;
  while (std::getline(ifs, line)) {
    ++lineNo;
    std::string trimmed = TrimWS(line);
    if (trimmed.empty() || trimmed[0] == '#')
      continue;

    // Header row: skip if first column reads "TypeId" (case-insensitive).
    std::stringstream ss(trimmed);
    std::string col1, col2, col3;
    if (!std::getline(ss, col1, ','))
      continue;
    if (!std::getline(ss, col2, ','))
      continue;
    if (!std::getline(ss, col3, ','))
      continue;

    col1 = TrimWS(col1);
    col2 = TrimWS(col2);
    col3 = TrimWS(col3);

    if (col1.empty() || col2.empty())
      continue;

    if (col1 == "TypeId" && col2 == "PropertyName")
      continue;

    m_values[MakeKey(col1, col2)] = col3;
    m_byType[col1].emplace_back(col2, col3);
  }

  std::cout << "INFO: NodeValueRegistry loaded " << m_values.size()
            << " entries from " << csvPath << std::endl;
  return true;
}

bool NodeValueRegistry::Has(const std::string &typeId,
                            const std::string &propName) const {
  return m_values.find(MakeKey(typeId, propName)) != m_values.end();
}

std::string NodeValueRegistry::Get(const std::string &typeId,
                                   const std::string &propName,
                                   const std::string &defaultValue) const {
  auto it = m_values.find(MakeKey(typeId, propName));
  if (it == m_values.end())
    return defaultValue;
  return it->second;
}

void NodeValueRegistry::ApplyOverrides(
    const std::string &typeId,
    std::map<std::string, std::string> &properties) const {
  auto it = m_byType.find(typeId);
  if (it == m_byType.end())
    return;
  for (const auto &kv : it->second) {
    properties[kv.first] = kv.second;
  }
}
