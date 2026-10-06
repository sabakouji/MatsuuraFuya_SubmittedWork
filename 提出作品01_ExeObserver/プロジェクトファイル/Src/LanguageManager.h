#pragma once
#include "GameObject.h"
#include <map>
#include <string>
#include <vector>


class LanguageManager : public GameObject {
public:
  enum LanguageCode { JP = 0, EN, MAX };

  LanguageManager();
  ~LanguageManager() override;

  // Load data
  void Load();

  // Set current language
  void SetLanguage(LanguageCode code);

  // Get localized text
  std::string GetText(std::string id);

private:
  LanguageCode m_currentLanguage;
  std::map<std::string, std::vector<std::string>> m_textMap;
};
