#pragma once
#include "CoreData.h"
#include <map>
#include <memory>
#include <atomic>

struct PinDefinition {
    std::string name;
    PinType type;
    PinType dataType;
    std::string linkedPropertyName;
};

struct NodeTemplate {
    std::string typeID;
    std::string displayName;
    std::string category;
    std::vector<PinDefinition> inputs;
    std::vector<PinDefinition> outputs;
    std::map<std::string, std::string> defaultProperties;
    std::vector<PropertyDefinition> propertyDefinitions;
};

class NodeFactory {
public:
    NodeFactory();
    ~NodeFactory() = default;

    static NodeFactory& GetInstance();

    bool LoadDefinitionsFromJson(const std::string& filePath);

    void RegisterBuiltinBehaviorNodes();

    std::unique_ptr<NodeInstance> CreateNode(const std::string& templateName, const VECTOR2& position, long long& nextNodeId);

    const std::map<std::string, NodeTemplate>& GetAllTemplates() const { return templates_; }

private:
    std::map<std::string, NodeTemplate> templates_;
    long long nextPinId_ = 1;

    void InitializeNodePins(NodeInstance& node, const NodeTemplate& tmpl);
};