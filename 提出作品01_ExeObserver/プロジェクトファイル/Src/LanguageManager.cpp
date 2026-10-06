#include "LanguageManager.h"
#include "CsvReader.h"

LanguageManager::LanguageManager() : m_currentLanguage(JP) {
  DontDestroyMe();
  Load();
}

LanguageManager::~LanguageManager() {}

void LanguageManager::Load() {
  CsvReader csv("Data/Lang/Language.csv");
  unsigned int lines = csv.GetLines();

  m_textMap.clear();

  // Start from 1 to skip header
  for (unsigned int i = 1; i < lines; i++) {
    std::string id = csv.GetString(i, 0);
    if (id.empty())
      continue;

    std::vector<std::string> texts;
    // Col 1 is JP, Col 2 is EN based on CSV structure: ID,JP,EN
    texts.push_back(csv.GetString(i, 1)); // JP index 0
    texts.push_back(csv.GetString(i, 2)); // EN index 1

    m_textMap[id] = texts;
  }
}

void LanguageManager::SetLanguage(LanguageCode code) {
  if (code >= 0 && code < MAX) {
    m_currentLanguage = code;
  }
}

std::string LanguageManager::GetText(std::string id) {
  if (m_textMap.find(id) != m_textMap.end()) {
    if ((size_t)m_currentLanguage < m_textMap[id].size()) {
      return m_textMap[id][m_currentLanguage];
    }
  }
  return id; // Fallback to ID
}
