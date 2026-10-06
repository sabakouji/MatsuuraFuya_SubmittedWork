#include "CoreData.h"
#include <iostream>

//-----------------------------------------------------------------------------
// ピンタイプの互換性チェック
bool IsCompatible(PinType type1, PinType type2) {
  bool isControl1 =
      (type1 == PinType::ControlIn || type1 == PinType::ControlOut);
  bool isControl2 =
      (type2 == PinType::ControlIn || type2 == PinType::ControlOut);

  if (isControl1 && isControl2) {
    return (type1 != type2);
  }

  if (isControl1 != isControl2) {
    return false;
  }

  auto normalizeType = [](PinType t) -> PinType {
    if (t == PinType::DataIn_Int || t == PinType::DataOut_Int)
      return PinType::DataIn_Int;
    if (t == PinType::DataIn_Float || t == PinType::DataOut_Float)
      return PinType::DataIn_Float;
    if (t == PinType::DataIn_String || t == PinType::DataOut_String)
      return PinType::DataIn_String;
    return t;
  };

  PinType normType1 = normalizeType(type1);
  PinType normType2 = normalizeType(type2);

  if (normType1 == normType2 && normType1 != PinType::DataIn_Generic) {
    return (type1 != type2);
  }

  bool isData1 = !isControl1;
  bool isData2 = !isControl2;

  if (isData1 && isData2) {
    if ((normType1 == PinType::DataIn_Generic ||
         normType2 == PinType::DataIn_Generic) ||
        (normType1 == PinType::DataOut_Generic ||
         normType2 == PinType::DataOut_Generic)) {
      return (type1 != type2);
    }
  }

  return false;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// リンクの整合性チェック
bool LinkInstance::IsValid(
    const std::map<std::string, PinInstance> &allPins) const {
  auto itStart = allPins.find(startPinId);
  auto itEnd = allPins.find(endPinId);

  if (itStart == allPins.end() || itEnd == allPins.end()) {
    std::cerr << "ERROR: Link pins not found. Start: " << startPinId
              << ", End: " << endPinId << std::endl;
    return false;
  }

  const PinInstance &startPin = itStart->second;
  const PinInstance &endPin = itEnd->second;

  if (startPin.IsInput() == endPin.IsInput()) {
    std::cerr << "ERROR: Invalid link direction. Both pins are "
              << (startPin.IsInput() ? "Input" : "Output") << std::endl;
    return false;
  }

  if (!IsCompatible(startPin.type, endPin.type)) {
    std::cerr << "ERROR: Incompatible pin types for link." << std::endl;
    return false;
  }

  if (endPin.IsInput() && endPin.IsDataPin() && !endPin.linkedPinIds.empty()) {
    std::cerr << "ERROR: Data input pin already connected." << std::endl;
    return false;
  }

  return true;
}
//-----------------------------------------------------------------------------