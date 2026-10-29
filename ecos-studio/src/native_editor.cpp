#include <GLFW/glfw3.h>
#include "ecos_studio/asset_importer.hpp"
#include "ecos_studio/platform.hpp"
#include "ecos_studio/scene.hpp"
#include "ecos_studio/scene_document.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl2.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <cstdio>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#ifndef GL_ARRAY_BUFFER
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_FLOAT 0x1406
#define GL_UNSIGNED_INT 0x1405
#define GL_TRIANGLES 0x0004
#define GL_FALSE 0
#endif
using GLsizeiptr = std::ptrdiff_t;
using PFNGLCREATESHADERPROC = GLuint (APIENTRY*)(GLenum);
using PFNGLSHADERSOURCEPROC = void (APIENTRY*)(GLuint, GLsizei, const char* const*, const GLint*);
using PFNGLCOMPILESHADERPROC = void (APIENTRY*)(GLuint);
using PFNGLGETSHADERIVPROC = void (APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETSHADERINFOLOGPROC = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, char*);
using PFNGLCREATEPROGRAMPROC = GLuint (APIENTRY*)();
using PFNGLDELETEPROGRAMPROC = void (APIENTRY*)(GLuint);
using PFNGLATTACHSHADERPROC = void (APIENTRY*)(GLuint, GLuint);
using PFNGLLINKPROGRAMPROC = void (APIENTRY*)(GLuint);
using PFNGLGETPROGRAMIVPROC = void (APIENTRY*)(GLuint, GLenum, GLint*);
using PFNGLGETPROGRAMINFOLOGPROC = void (APIENTRY*)(GLuint, GLsizei, GLsizei*, char*);
using PFNGLUSEPROGRAMPROC = void (APIENTRY*)(GLuint);
using PFNGLGETUNIFORMLOCATIONPROC = GLint (APIENTRY*)(GLuint, const char*);
using PFNGLGETATTRIBLOCATIONPROC = GLint (APIENTRY*)(GLuint, const char*);
using PFNGLUNIFORMMATRIX4FVPROC = void (APIENTRY*)(GLint, GLsizei, GLboolean, const GLfloat*);
using PFNGLUNIFORM3FPROC = void (APIENTRY*)(GLint, GLfloat, GLfloat, GLfloat);
using PFNGLGENBUFFERSPROC = void (APIENTRY*)(GLsizei, GLuint*);
using PFNGLBINDBUFFERPROC = void (APIENTRY*)(GLenum, GLuint);
using PFNGLBUFFERDATAPROC = void (APIENTRY*)(GLenum, GLsizeiptr, const void*, GLenum);
using PFNGLDELETEBUFFERSPROC = void (APIENTRY*)(GLsizei, const GLuint*);
using PFNGLENABLEVERTEXATTRIBARRAYPROC = void (APIENTRY*)(GLuint);
using PFNGLDISABLEVERTEXATTRIBARRAYPROC = void (APIENTRY*)(GLuint);
using PFNGLVERTEXATTRIBPOINTERPROC = void (APIENTRY*)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*);
using PFNGLDRAWELEMENTSPROC = void (APIENTRY*)(GLenum, GLsizei, GLenum, const void*);
struct GpuApi {
 PFNGLCREATESHADERPROC CreateShader{}; PFNGLSHADERSOURCEPROC ShaderSource{}; PFNGLCOMPILESHADERPROC CompileShader{}; PFNGLGETSHADERIVPROC GetShaderiv{}; PFNGLGETSHADERINFOLOGPROC GetShaderInfoLog{};
 PFNGLCREATEPROGRAMPROC CreateProgram{}; PFNGLDELETEPROGRAMPROC DeleteProgram{}; PFNGLATTACHSHADERPROC AttachShader{}; PFNGLLINKPROGRAMPROC LinkProgram{}; PFNGLGETPROGRAMIVPROC GetProgramiv{}; PFNGLGETPROGRAMINFOLOGPROC GetProgramInfoLog{}; PFNGLUSEPROGRAMPROC UseProgram{};
 PFNGLGETUNIFORMLOCATIONPROC GetUniformLocation{}; PFNGLGETATTRIBLOCATIONPROC GetAttribLocation{}; PFNGLUNIFORMMATRIX4FVPROC UniformMatrix4fv{}; PFNGLUNIFORM3FPROC Uniform3f{};
 PFNGLGENBUFFERSPROC GenBuffers{}; PFNGLBINDBUFFERPROC BindBuffer{}; PFNGLBUFFERDATAPROC BufferData{}; PFNGLDELETEBUFFERSPROC DeleteBuffers{};
 PFNGLENABLEVERTEXATTRIBARRAYPROC EnableVertexAttribArray{}; PFNGLDISABLEVERTEXATTRIBARRAYPROC DisableVertexAttribArray{}; PFNGLVERTEXATTRIBPOINTERPROC VertexAttribPointer{}; PFNGLDRAWELEMENTSPROC DrawElements{};
 GLuint program{}; GLint mvp{-1}, model{-1}, albedo{-1}, position{-1}, normal{-1}; bool ready{false};
};
static GpuApi gpu;
static bool init_gpu_renderer(std::string& error) {
#define LOAD_GL(member, type, symbol) gpu.member = reinterpret_cast<type>(glfwGetProcAddress(symbol)); if (!gpu.member) { error = "OpenGL driver missing " symbol; return false; }
 LOAD_GL(CreateShader,PFNGLCREATESHADERPROC,"glCreateShader")
 LOAD_GL(ShaderSource,PFNGLSHADERSOURCEPROC,"glShaderSource")
 LOAD_GL(CompileShader,PFNGLCOMPILESHADERPROC,"glCompileShader")
 LOAD_GL(GetShaderiv,PFNGLGETSHADERIVPROC,"glGetShaderiv")
 LOAD_GL(GetShaderInfoLog,PFNGLGETSHADERINFOLOGPROC,"glGetShaderInfoLog")
 LOAD_GL(CreateProgram,PFNGLCREATEPROGRAMPROC,"glCreateProgram")
 LOAD_GL(DeleteProgram,PFNGLDELETEPROGRAMPROC,"glDeleteProgram")
 LOAD_GL(AttachShader,PFNGLATTACHSHADERPROC,"glAttachShader")
 LOAD_GL(LinkProgram,PFNGLLINKPROGRAMPROC,"glLinkProgram")
 LOAD_GL(GetProgramiv,PFNGLGETPROGRAMIVPROC,"glGetProgramiv")
 LOAD_GL(GetProgramInfoLog,PFNGLGETPROGRAMINFOLOGPROC,"glGetProgramInfoLog")
 LOAD_GL(UseProgram,PFNGLUSEPROGRAMPROC,"glUseProgram")
 LOAD_GL(GetUniformLocation,PFNGLGETUNIFORMLOCATIONPROC,"glGetUniformLocation")
 LOAD_GL(GetAttribLocation,PFNGLGETATTRIBLOCATIONPROC,"glGetAttribLocation")
 LOAD_GL(UniformMatrix4fv,PFNGLUNIFORMMATRIX4FVPROC,"glUniformMatrix4fv")
 LOAD_GL(Uniform3f,PFNGLUNIFORM3FPROC,"glUniform3f")
 LOAD_GL(GenBuffers,PFNGLGENBUFFERSPROC,"glGenBuffers")
 LOAD_GL(BindBuffer,PFNGLBINDBUFFERPROC,"glBindBuffer")
 LOAD_GL(BufferData,PFNGLBUFFERDATAPROC,"glBufferData")
 LOAD_GL(DeleteBuffers,PFNGLDELETEBUFFERSPROC,"glDeleteBuffers")
 LOAD_GL(EnableVertexAttribArray,PFNGLENABLEVERTEXATTRIBARRAYPROC,"glEnableVertexAttribArray")
 LOAD_GL(DisableVertexAttribArray,PFNGLDISABLEVERTEXATTRIBARRAYPROC,"glDisableVertexAttribArray")
 LOAD_GL(VertexAttribPointer,PFNGLVERTEXATTRIBPOINTERPROC,"glVertexAttribPointer")
 LOAD_GL(DrawElements,PFNGLDRAWELEMENTSPROC,"glDrawElements")
#undef LOAD_GL
 const char* vs="#version 120\nattribute vec3 aPosition; attribute vec3 aNormal; uniform mat4 uMvp; uniform mat4 uModel; varying vec3 vNormal; void main(){ vNormal=mat3(uModel)*aNormal; gl_Position=uMvp*vec4(aPosition,1.0); }";
 const char* fs="#version 120\nvarying vec3 vNormal; uniform vec3 uAlbedo; void main(){ vec3 n=normalize(vNormal); vec3 l=normalize(vec3(-0.4,0.85,0.3)); float diffuse=max(dot(n,l),0.0); float light=0.18+0.82*diffuse; gl_FragColor=vec4(uAlbedo*light,1.0); }";
 auto compile=[&](GLenum type,const char* code)->GLuint { GLuint sh=gpu.CreateShader(type);gpu.ShaderSource(sh,1,&code,nullptr);gpu.CompileShader(sh);GLint ok=0;gpu.GetShaderiv(sh,GL_COMPILE_STATUS,&ok);if(!ok){GLint n=0;gpu.GetShaderiv(sh,GL_INFO_LOG_LENGTH,&n);std::string log(static_cast<std::size_t>(std::max(n,1)), '\0');gpu.GetShaderInfoLog(sh,n,nullptr,log.data());error="Shader compile failed: "+log;return 0;}return sh; };
 GLuint v=compile(GL_VERTEX_SHADER,vs);if(!v)return false;GLuint f=compile(GL_FRAGMENT_SHADER,fs);if(!f)return false;
 gpu.program=gpu.CreateProgram();gpu.AttachShader(gpu.program,v);gpu.AttachShader(gpu.program,f);gpu.LinkProgram(gpu.program);GLint linked=0;gpu.GetProgramiv(gpu.program,GL_LINK_STATUS,&linked);
 if(!linked){GLint n=0;gpu.GetProgramiv(gpu.program,GL_INFO_LOG_LENGTH,&n);std::string log(static_cast<std::size_t>(std::max(n,1)), '\0');gpu.GetProgramInfoLog(gpu.program,n,nullptr,log.data());error="GPU renderer link failed: "+log;return false;}
 gpu.mvp=gpu.GetUniformLocation(gpu.program,"uMvp");gpu.model=gpu.GetUniformLocation(gpu.program,"uModel");gpu.albedo=gpu.GetUniformLocation(gpu.program,"uAlbedo");gpu.position=gpu.GetAttribLocation(gpu.program,"aPosition");gpu.normal=gpu.GetAttribLocation(gpu.program,"aNormal");if(gpu.position<0||gpu.normal<0){error="GPU shader attributes were optimized out or not linked";return false;}gpu.ready=true;return true;
}
struct GpuVertex { float p[3]; float n[3]; };
struct GpuMesh { GLuint vertex_buffer{}, index_buffer{}; GLsizei index_count{}; };
static std::unordered_map<const Mesh*, GpuMesh> gpu_mesh_cache;
static void clear_gpu_mesh_cache() {
    if (gpu.ready) for (const auto& entry : gpu_mesh_cache) {
        const GLuint buffers[2]{entry.second.vertex_buffer, entry.second.index_buffer};
        gpu.DeleteBuffers(2,buffers);
    }
    gpu_mesh_cache.clear();
}
struct Vertex { float x{}, y{}, z{}; };
struct Mesh { std::vector<Vertex> vertices; std::vector<std::uint32_t> triangles; };
struct SceneInstance { Mesh mesh; ecos::studio::Matrix4 world{ecos::studio::Matrix4::identity()}; std::string name; ecos::studio::NodeId node_id{}; };
struct EditorState {
    Mesh mesh;
    std::vector<SceneInstance> scene_instances;
    std::vector<std::string> available_mesh_assets;
    bool scene_loaded{false};
    std::unique_ptr<ecos::studio::SceneDocument> document;
    std::filesystem::path project_root, scene_path;
    ecos::studio::NodeId selected_node{};
    std::string status;
    ecos::studio::NodeId inspector_name_node{};
    char inspector_name[256]{};
    float yaw{25.0F}, pitch{18.0F}, distance{5.0F};
    double last_x{}, last_y{};
    bool dragging{false};
    std::string title{"ECOS Studio — Native 3D Viewport"};
};

static bool load_obj(const std::string& path, Mesh& out, std::string& error) {
    std::ifstream in(path);
    if (!in) { error = "Cannot open OBJ: " + path; return false; }
    Mesh next; std::string line;
    while (std::getline(in, line)) {
        std::istringstream row(line); std::string tag; row >> tag;
        if (tag == "v") {
            Vertex v; if (!(row >> v.x >> v.y >> v.z) ||
                !std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z)) {
                error = "Invalid vertex in OBJ"; return false;
            }
            next.vertices.push_back(v);
        } else if (tag == "f") {
            std::vector<std::uint32_t> face; std::string token;
            while (row >> token) {
                const auto slash = token.find('/'); const auto index_text = token.substr(0, slash);
                char* end = nullptr; const long raw = std::strtol(index_text.c_str(), &end, 10);
                if (end == index_text.c_str() || *end != '\0' || raw == 0) {
                    error = "Invalid OBJ face index"; return false;
                }
                const long resolved = raw > 0 ? raw - 1 : static_cast<long>(next.vertices.size()) + raw;
                if (resolved < 0 || static_cast<std::size_t>(resolved) >= next.vertices.size()) {
                    error = "OBJ face index out of range"; return false;
                }
                face.push_back(static_cast<std::uint32_t>(resolved));
            }
            if (face.size() < 3) { error = "OBJ face has fewer than three vertices"; return false; }
            for (std::size_t i=1; i+1<face.size(); ++i) {
                next.triangles.push_back(face[0]); next.triangles.push_back(face[i]); next.triangles.push_back(face[i+1]);
            }
        }
    }
    if (next.vertices.empty() || next.triangles.empty()) { error = "OBJ must contain vertices and faces"; return false; }
    if (next.vertices.size() > 2000000 || next.triangles.size() > 12000000) { error = "OBJ exceeds native preview limits"; return false; }
    Vertex lo=next.vertices.front(), hi=lo;
    for (const auto& v:next.vertices) { lo.x=std::min(lo.x,v.x);lo.y=std::min(lo.y,v.y);lo.z=std::min(lo.z,v.z);hi.x=std::max(hi.x,v.x);hi.y=std::max(hi.y,v.y);hi.z=std::max(hi.z,v.z); }
    const float span=std::max({hi.x-lo.x,hi.y-lo.y,hi.z-lo.z,0.0001F});
    const float cx=(hi.x+lo.x)*0.5F,cy=(hi.y+lo.y)*0.5F,cz=(hi.z+lo.z)*0.5F;
    for (auto& v:next.vertices) { v.x=(v.x-cx)*2/span;v.y=(v.y-cy)*2/span;v.z=(v.z-cz)*2/span; }
    out=std::move(next); return true;
}
static void draw_mesh(const Mesh& mesh);
static bool load_ecos_scene(const std::filesystem::path& project_root,
                            const ecos::studio::SceneGraph& graph,
                            std::vector<SceneInstance>& instances,
                            std::vector<std::string>& mesh_asset_ids, std::string& error) {
    auto registry = ecos::studio::load_imported_asset_registry(project_root);
    if (!registry) { error = "Asset registry load failed: " + registry.error().message; return false; }

    std::vector<std::string> next_asset_ids;
    for (const auto& asset : registry.value().assets())
        if (asset.type == "mesh.obj") next_asset_ids.push_back(asset.id);
    std::sort(next_asset_ids.begin(), next_asset_ids.end());
    std::vector<SceneInstance> next;
    for (const auto& node : graph.nodes()) {
        if (node.mesh_asset_id.empty()) continue;
        const auto* asset = registry.value().find(node.mesh_asset_id);
        if (!asset) { error = "Scene node '" + node.name + "' references missing mesh asset '" + node.mesh_asset_id + "'"; return false; }
        if (asset->type != "mesh.obj") { error = "Scene asset '" + node.mesh_asset_id + "' is not a mesh.obj asset"; return false; }
        auto source = ecos::studio::platform::read_text_file(project_root / asset->path);
        if (!source) { error = "Cannot read mesh asset '" + node.mesh_asset_id + "': " + source.error().message; return false; }
        auto imported = ecos::studio::import_obj_mesh(source.value());
        if (!imported) { error = "Cannot import mesh asset '" + node.mesh_asset_id + "': " + imported.error().message; return false; }
        SceneInstance instance;
        instance.name = node.name;
        instance.node_id = node.id;
        instance.mesh.vertices.reserve(imported.value().positions.size());
        for (const auto& p : imported.value().positions) instance.mesh.vertices.push_back({p.x,p.y,p.z});
        instance.mesh.triangles = imported.value().indices;
        auto world = graph.world_matrix(node.id);
        if (!world) { error = "Cannot resolve world transform for node '" + node.name + "': " + world.error().message; return false; }
        instance.world = world.value();
        next.push_back(std::move(instance));
    }
    instances = std::move(next);
    mesh_asset_ids = std::move(next_asset_ids);
    return true;
}
static bool save_scene_file(EditorState& state) {
    if (!state.document || state.scene_path.empty()) return false;
    auto serialized = ecos::studio::serialize_scene(state.document->scene_id(), state.document->scene());
    if (!serialized) { state.status = serialized.error().message; return false; }
    const auto temporary = state.scene_path.string() + ".tmp";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) { state.status = "Cannot create scene save file"; return false; }
        output << serialized.value();
        if (!output) { state.status = "Failed writing scene save file"; return false; }
    }
    std::error_code ec;
#ifdef _WIN32
    if (!MoveFileExW(std::filesystem::path(temporary).c_str(), state.scene_path.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        state.status = "Cannot replace scene file (Windows error " + std::to_string(GetLastError()) + ")";
        return false;
    }
#else
    std::filesystem::rename(temporary, state.scene_path, ec);
    if (ec) { state.status = "Cannot replace scene file: " + ec.message(); return false; }
#endif
    auto marked_saved = state.document->save();
    if (!marked_saved) { state.status = marked_saved.error().message; return false; }
    state.status = "Scene saved";
    return true;
}
static void draw_hierarchy_node(EditorState& state, ecos::studio::NodeId id) {
    const auto* node = state.document->scene().find(id);
    if (!node) return;
    ImGui::PushID(static_cast<int>(id & 0x7fffffffU));
    const bool leaf = node->children.empty();
    const ImGuiTreeNodeFlags flags = (leaf ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen : 0)
        | (state.selected_node == id ? ImGuiTreeNodeFlags_Selected : 0)
        | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow;
    const bool open = ImGui::TreeNodeEx(node->name.c_str(), flags);
    if (ImGui::IsItemClicked()) state.selected_node = id;
    if (!leaf && open) {
        const auto children = node->children;
        for (const auto child : children) draw_hierarchy_node(state, child);
        ImGui::TreePop();
    }
    ImGui::PopID();
}
static void refresh_scene_meshes(EditorState& state) {
    if (!state.document) return;
    std::vector<SceneInstance> next;
    std::vector<std::string> asset_ids;
    std::string error;
    if (!load_ecos_scene(state.project_root, state.document->scene(), next, asset_ids, error)) {
        state.status = error;
        return;
    }
    clear_gpu_mesh_cache();
    state.scene_instances = std::move(next);
    state.available_mesh_assets = std::move(asset_ids);
}
static void refresh_scene_instance_transforms(EditorState& state) {
    if (!state.document) return;
    for (auto& instance : state.scene_instances) {
        auto world = state.document->scene().world_matrix(instance.node_id);
        if (world) instance.world = world.value();
        const auto* node = state.document->scene().find(instance.node_id);
        if (node) instance.name = node->name;
    }
}
static void draw_editor_ui(EditorState& state) {
    ImGui::SetNextWindowPos(ImVec2(12, 12), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(270, 520), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("ECOS — World Outliner")) {
        if (state.document) {
            if (ImGui::Button("Save") && save_scene_file(state)) {}
            ImGui::SameLine();
            if (ImGui::Button("Undo")) {
                auto result = state.document->undo();
                state.status = result ? "Undo applied" : result.error().message;
                if (result) refresh_scene_meshes(state);
            }
            ImGui::SameLine();
            if (ImGui::Button("Redo")) {
                auto result = state.document->redo();
                state.status = result ? "Redo applied" : result.error().message;
                if (result) refresh_scene_meshes(state);
            }
            if (ImGui::Button("Add Entity")) {
                auto created = state.document->create_node("Entity");
                if (created) { state.selected_node = created.value(); state.status = "Entity created"; }
                else state.status = created.error().message;
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete") && state.selected_node != 0) {
                auto result = state.document->delete_subtree(state.selected_node);
                state.status = result ? "Entity deleted" : result.error().message;
                state.selected_node = 0;
                if (result) refresh_scene_meshes(state);
            }
            ImGui::Separator();
            for (const auto root : state.document->scene().roots()) draw_hierarchy_node(state, root);
            if (!state.status.empty()) { ImGui::Separator(); ImGui::TextWrapped("%s", state.status.c_str()); }
        } else {
            ImGui::TextWrapped("Open a project scene with --project and --scene to edit its hierarchy and transforms.");
            ImGui::TextWrapped("Standalone OBJ preview remains available from the command line.");
        }
    }
    ImGui::End();

    if (state.document) {
        ImGui::SetNextWindowPos(ImVec2(296, 12), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(320, 360), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("ECOS — Inspector")) {
            const auto* node = state.document->scene().find(state.selected_node);
            if (!node) ImGui::TextUnformatted("Select an entity in the World Outliner.");
            else {
                ImGui::Text("Entity ID: %llu", static_cast<unsigned long long>(node->id));
                if (state.inspector_name_node != node->id) {
                    state.inspector_name_node = node->id;
                    std::snprintf(state.inspector_name, sizeof(state.inspector_name), "%s", node->name.c_str());
                }
                if (ImGui::InputText("Name", state.inspector_name, sizeof(state.inspector_name))) {
                    auto result = state.document->rename_node(node->id, state.inspector_name);
                    state.status = result ? "Entity renamed" : result.error().message;
                }
                ImGui::Separator();
                auto transform = node->local_transform;
                float position[3]{transform.position.x,transform.position.y,transform.position.z};
                float rotation[3]{transform.rotation_euler_radians.x,transform.rotation_euler_radians.y,transform.rotation_euler_radians.z};
                float scale[3]{transform.scale.x,transform.scale.y,transform.scale.z};
                bool changed = ImGui::InputFloat3("Position", position, "%.3f");
                changed = ImGui::InputFloat3("Rotation (rad)", rotation, "%.3f") || changed;
                changed = ImGui::InputFloat3("Scale", scale, "%.3f") || changed;
                if (changed) {
                    transform.position={position[0],position[1],position[2]};
                    transform.rotation_euler_radians={rotation[0],rotation[1],rotation[2]};
                    transform.scale={scale[0],scale[1],scale[2]};
                    auto result=state.document->set_transform(node->id,transform);
                    state.status=result ? "Transform updated" : result.error().message;
                    refresh_scene_instance_transforms(state);
                }
                ImGui::Separator();
                const char* current_mesh = node->mesh_asset_id.empty() ? "(None)" : node->mesh_asset_id.c_str();
                if (ImGui::BeginCombo("Mesh Asset", current_mesh)) {
                    if (ImGui::Selectable("(None)", node->mesh_asset_id.empty())) {
                        auto result = state.document->set_mesh_asset(node->id, "");
                        state.status = result ? "Mesh assignment cleared" : result.error().message;
                        if (result) refresh_scene_meshes(state);
                    }
                    for (const auto& asset_id : state.available_mesh_assets) {
                        const bool selected = node->mesh_asset_id == asset_id;
                        if (ImGui::Selectable(asset_id.c_str(), selected)) {
                            auto result = state.document->set_mesh_asset(node->id, asset_id);
                            state.status = result ? "Mesh asset assigned" : result.error().message;
                            if (result) refresh_scene_meshes(state);
                        }
                        if (selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                ImGui::TextWrapped("Material asset: %s", node->material_asset_id.empty() ? "(none)" : node->material_asset_id.c_str());
            }
        }
        ImGui::End();
    }
}
static void draw_scene_instances(const std::vector<SceneInstance>& instances) {
    for (const auto& instance : instances) {
        float matrix[16]{};
        for (int row=0; row<4; ++row)
            for (int col=0; col<4; ++col)
                matrix[col*4+row] = instance.world.values[static_cast<std::size_t>(row*4+col)];
        glPushMatrix();
        glMultMatrixf(matrix);
        draw_mesh(instance.mesh);
        glPopMatrix();
    }
}
static void scroll_callback(GLFWwindow* window, double, double y) {
    auto* s=static_cast<EditorState*>(glfwGetWindowUserPointer(window));
    s->distance=std::clamp(s->distance*static_cast<float>(std::pow(0.88,y)),1.2F,80.0F);
}
static void mouse_button_callback(GLFWwindow* w,int button,int action,int) {
    auto* s=static_cast<EditorState*>(glfwGetWindowUserPointer(w));
    if(button==GLFW_MOUSE_BUTTON_LEFT){s->dragging=action==GLFW_PRESS;glfwGetCursorPos(w,&s->last_x,&s->last_y);}
}
static void cursor_callback(GLFWwindow* w,double x,double y) {
    auto* s=static_cast<EditorState*>(glfwGetWindowUserPointer(w));
    if(s->dragging){s->yaw+=static_cast<float>(x-s->last_x)*0.45F;s->pitch=std::clamp(s->pitch+static_cast<float>(y-s->last_y)*0.45F,-89.0F,89.0F);}
    s->last_x=x;s->last_y=y;
}
static void key_callback(GLFWwindow* w,int key,int,int action,int) {
    if(action!=GLFW_PRESS)return;
    auto* s=static_cast<EditorState*>(glfwGetWindowUserPointer(w));
    if(key==GLFW_KEY_ESCAPE)glfwSetWindowShouldClose(w,GLFW_TRUE);
    if(key==GLFW_KEY_R){s->yaw=25;s->pitch=18;s->distance=5;}
}
static void draw_grid() {
    glLineWidth(1.0F);glBegin(GL_LINES);
    for(int i=-20;i<=20;++i){
        const float c=(i==0)?0.34F:0.15F;
        glColor3f(c,c+0.02F,c+0.04F);
        glVertex3f(static_cast<float>(i),0,-20);glVertex3f(static_cast<float>(i),0,20);
        glVertex3f(-20,0,static_cast<float>(i));glVertex3f(20,0,static_cast<float>(i));
    }
    glEnd();
    glLineWidth(2.0F);glBegin(GL_LINES);
    glColor3f(0.95F,0.28F,0.30F);glVertex3f(0,0.01F,0);glVertex3f(2,0.01F,0);
    glColor3f(0.25F,0.88F,0.48F);glVertex3f(0,0.01F,0);glVertex3f(0,2,0);
    glColor3f(0.30F,0.55F,1.0F);glVertex3f(0,0.01F,0);glVertex3f(0,0.01F,2);
    glEnd();
}
static void draw_mesh(const Mesh& mesh) {
    if (!gpu.ready || mesh.triangles.empty()) return;
    auto found=gpu_mesh_cache.find(&mesh);
    if(found==gpu_mesh_cache.end()) {
        std::vector<GpuVertex> vertices;
        vertices.reserve(mesh.triangles.size());
        std::vector<std::uint32_t> indices;
        indices.reserve(mesh.triangles.size());
        for(std::size_t i=0;i+2<mesh.triangles.size();i+=3){
            const auto a=mesh.vertices[mesh.triangles[i]],b=mesh.vertices[mesh.triangles[i+1]],c=mesh.vertices[mesh.triangles[i+2]];
            const float ux=b.x-a.x,uy=b.y-a.y,uz=b.z-a.z,vx=c.x-a.x,vy=c.y-a.y,vz=c.z-a.z;
            float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;const float len=std::sqrt(nx*nx+ny*ny+nz*nz);
            if(len>1e-8F){nx/=len;ny/=len;nz/=len;}else{nx=0;ny=1;nz=0;}
            for(auto idx:{mesh.triangles[i],mesh.triangles[i+1],mesh.triangles[i+2]}){
                const auto& p=mesh.vertices[idx];vertices.push_back({{p.x,p.y,p.z},{nx,ny,nz}});indices.push_back(static_cast<std::uint32_t>(indices.size()));
            }
        }
        GpuMesh cached;gpu.GenBuffers(1,&cached.vertex_buffer);
        gpu.BindBuffer(GL_ARRAY_BUFFER,cached.vertex_buffer);
        gpu.BufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(GpuVertex)),vertices.data(),GL_STATIC_DRAW);
        gpu.GenBuffers(1,&cached.index_buffer);
        gpu.BindBuffer(GL_ELEMENT_ARRAY_BUFFER,cached.index_buffer);
        gpu.BufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(indices.size()*sizeof(std::uint32_t)),indices.data(),GL_STATIC_DRAW);
        cached.index_count=static_cast<GLsizei>(indices.size());
        found=gpu_mesh_cache.emplace(&mesh,cached).first;
    }
    const auto& cached=found->second;
    gpu.BindBuffer(GL_ARRAY_BUFFER,cached.vertex_buffer);
    gpu.BindBuffer(GL_ELEMENT_ARRAY_BUFFER,cached.index_buffer);
    gpu.EnableVertexAttribArray(static_cast<GLuint>(gpu.position));gpu.EnableVertexAttribArray(static_cast<GLuint>(gpu.normal));
    gpu.VertexAttribPointer(static_cast<GLuint>(gpu.position),3,GL_FLOAT,GL_FALSE,sizeof(GpuVertex),reinterpret_cast<const void*>(offsetof(GpuVertex,p)));
    gpu.VertexAttribPointer(static_cast<GLuint>(gpu.normal),3,GL_FLOAT,GL_FALSE,sizeof(GpuVertex),reinterpret_cast<const void*>(offsetof(GpuVertex,n)));
    gpu.UseProgram(gpu.program);
    GLfloat model[16]{},projection[16]{},mvp[16]{};
    glGetFloatv(GL_MODELVIEW_MATRIX,model);glGetFloatv(GL_PROJECTION_MATRIX,projection);
    for(int row=0;row<4;++row)for(int col=0;col<4;++col){mvp[col*4+row]=0;for(int k=0;k<4;++k)mvp[col*4+row]+=projection[k*4+row]*model[col*4+k];}
    gpu.UniformMatrix4fv(gpu.mvp,1,GL_FALSE,mvp);gpu.UniformMatrix4fv(gpu.model,1,GL_FALSE,model);gpu.Uniform3f(gpu.albedo,0.28F,0.58F,0.95F);
    gpu.DrawElements(GL_TRIANGLES,cached.index_count,GL_UNSIGNED_INT,nullptr);
    gpu.DisableVertexAttribArray(static_cast<GLuint>(gpu.position));gpu.DisableVertexAttribArray(static_cast<GLuint>(gpu.normal));
    gpu.UseProgram(0);gpu.BindBuffer(GL_ARRAY_BUFFER,0);gpu.BindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}
static Mesh starter_cube() {
    Mesh m;
    m.vertices={{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
    m.triangles={0,2,1,0,3,2,4,5,6,4,6,7,0,1,5,0,5,4,3,7,6,3,6,2,0,4,7,0,7,3,1,2,6,1,6,5};
    return m;
}
int main(int argc,char** argv) {
    EditorState state;
    state.mesh=starter_cube();
    std::string mesh_path;
    for (int i=1; i<argc; ++i) {
        const std::string arg=argv[i];
        if (arg=="--project" && i+1<argc) state.project_root=argv[++i];
        else if (arg=="--scene" && i+1<argc) state.scene_path=argv[++i];
        else if (arg=="--help" || arg=="-h") {
            std::cout << "ECOS Studio native editor\n"
                      << "  ecos-studio-editor [mesh.obj]\n"
                      << "  ecos-studio-editor --project <project-dir> --scene <scene.ecscene>\n";
            return 0;
        } else if (!arg.empty() && arg[0]!='-') mesh_path=arg;
        else { std::cerr << "Unknown or incomplete argument: " << arg << '\n'; return 2; }
    }
    if (!state.scene_path.empty()) {
        if (state.project_root.empty()) { std::cerr << "--scene requires --project\n"; return 2; }
        auto serialized=ecos::studio::platform::read_text_file(state.scene_path);
        if (!serialized) { std::cerr << serialized.error().message << '\n'; return 2; }
        state.document=std::make_unique<ecos::studio::SceneDocument>();
        auto loaded=state.document->load(serialized.value());
        if (!loaded) { std::cerr << "Scene load failed: " << loaded.error().message << '\n'; return 2; }
        std::string error;
        if (!load_ecos_scene(state.project_root,state.document->scene(),state.scene_instances,state.available_mesh_assets,error)) {
            std::cerr << error << '\n'; return 2;
        }
        const auto roots=state.document->scene().roots();
        if (!roots.empty()) state.selected_node=roots.front();
        state.scene_loaded=true;
        state.title += " — " + state.scene_path.filename().string();
    } else if (!mesh_path.empty()) {
        std::string error;
        if(!load_obj(mesh_path,state.mesh,error)){std::cerr<<error<<'\n';return 2;}
        state.title+=" — "+mesh_path;
    }
    if(!glfwInit()){std::cerr<<"GLFW initialization failed\n";return 2;}
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,2);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,1);
#endif
    glfwWindowHint(GLFW_DEPTH_BITS,24);glfwWindowHint(GLFW_RESIZABLE,GLFW_TRUE);
    GLFWwindow* window=glfwCreateWindow(1440,900,state.title.c_str(),nullptr,nullptr);
    if(!window){glfwTerminate();std::cerr<<"Cannot create native OpenGL viewport window\n";return 2;}
    glfwMakeContextCurrent(window);glfwSwapInterval(1);glfwSetWindowUserPointer(window,&state);
    std::string gpu_error;
    if(!init_gpu_renderer(gpu_error)){std::cerr<<gpu_error<<'\n';glfwDestroyWindow(window);glfwTerminate();return 3;}
    glfwSetScrollCallback(window,scroll_callback);glfwSetMouseButtonCallback(window,mouse_button_callback);glfwSetCursorPosCallback(window,cursor_callback);glfwSetKeyCallback(window,key_callback);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL2_Init();
    glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glClearColor(0.075F,0.09F,0.12F,1.0F);
    while(!glfwWindowShouldClose(window)){
        int width=1,height=1;glfwGetFramebufferSize(window,&width,&height);height=std::max(height,1);width=std::max(width,1);
        glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);glLoadIdentity();const double aspect=static_cast<double>(width)/height;const double nearp=0.05,farp=500.0,top=nearp*std::tan(60.0*3.141592653589793/360.0);
        glFrustum(-top*aspect,top*aspect,-top,top,nearp,farp);
        glMatrixMode(GL_MODELVIEW);glLoadIdentity();glTranslatef(0,0,-state.distance);glRotatef(state.pitch,1,0,0);glRotatef(state.yaw,0,1,0);
        draw_grid();if(state.scene_loaded)draw_scene_instances(state.scene_instances);else if(!state.mesh.triangles.empty())draw_mesh(state.mesh);
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        draw_editor_ui(state);
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);glfwPollEvents();
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    clear_gpu_mesh_cache();
    if(gpu.program){gpu.DeleteProgram(gpu.program);}
    glfwDestroyWindow(window);glfwTerminate();return 0;
}
