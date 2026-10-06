#pragma once
#include "CoreData.h"
#include <string>
#include <variant>

class Executor;

// Helper variant to hold any supported data type
using GenericValue = std::variant<int, float, bool, std::string>;

class NodeEvaluator {
public:
  // Main evaluation methods
  static bool EvaluateBool(const NodeInstance *node, Executor *executor);
  static float EvaluateFloat(const NodeInstance *node, Executor *executor);
  static int EvaluateInt(const NodeInstance *node, Executor *executor);
  static std::string EvaluateString(const NodeInstance *node,
                                    Executor *executor);
  static GenericValue EvaluateGeneric(const NodeInstance *node,
                                      Executor *executor);

private:
  // Helper to get input source node
  static const NodeInstance *GetInputNode(const NodeInstance *node,
                                          const std::string &pinName,
                                          Executor *executor);

  // Helper to compare generic values
  static bool CompareValues(const GenericValue &lhs, const GenericValue &rhs,
                            const std::string &op);
};
