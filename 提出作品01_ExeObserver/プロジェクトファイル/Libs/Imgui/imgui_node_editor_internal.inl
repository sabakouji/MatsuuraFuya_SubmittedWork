//------------------------------------------------------------------------------
// imgui_node_editor_internal.inl
// Stub: inline implementations declared in imgui_node_editor_internal.h
//------------------------------------------------------------------------------

namespace ax {
namespace NodeEditor {
namespace Detail {

inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

inline ImVec2 ImGui_GetMouseClickPos(ImGuiMouseButton buttonIndex)
{
    return ImGui::GetIO().MouseClickedPos[buttonIndex];
}

} // namespace Detail
} // namespace NodeEditor
} // namespace ax
