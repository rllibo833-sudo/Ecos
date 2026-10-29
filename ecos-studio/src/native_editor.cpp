#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Vertex { float x{}, y{}, z{}; };
struct Mesh { std::vector<Vertex> vertices; std::vector<std::uint32_t> triangles; };
struct EditorState { Mesh mesh; float yaw{25.0F}, pitch{18.0F}, distance{5.0F}; double last_x{}, last_y{}; bool dragging{false}; std::string title{"ECOS Studio — Native 3D Viewport"}; };

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
    glBegin(GL_TRIANGLES);
    for(std::size_t i=0;i+2<mesh.triangles.size();i+=3){
        const auto a=mesh.vertices[mesh.triangles[i]],b=mesh.vertices[mesh.triangles[i+1]],c=mesh.vertices[mesh.triangles[i+2]];
        const float ux=b.x-a.x,uy=b.y-a.y,uz=b.z-a.z,vx=c.x-a.x,vy=c.y-a.y,vz=c.z-a.z;
        float nx=uy*vz-uz*vy,ny=uz*vx-ux*vz,nz=ux*vy-uy*vx;const float len=std::sqrt(nx*nx+ny*ny+nz*nz);
        if(len>1e-8F){nx/=len;ny/=len;nz/=len;}else{nx=0;ny=1;nz=0;}
        const float light=std::clamp(0.25F+0.75F*std::max(0.0F,nx*0.35F+ny*0.8F+nz*0.45F),0.18F,1.0F);
        glColor3f(0.28F*light,0.58F*light,0.95F*light);glNormal3f(nx,ny,nz);
        for(auto idx:{mesh.triangles[i],mesh.triangles[i+1],mesh.triangles[i+2]}){const auto& v=mesh.vertices[idx];glVertex3f(v.x,v.y,v.z);}
    }
    glEnd();
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
    if(argc>1){std::string error;if(!load_obj(argv[1],state.mesh,error)){std::cerr<<error<<'\n';return 2;}state.title+=" — "+std::string(argv[1]);}
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
    glfwSetScrollCallback(window,scroll_callback);glfwSetMouseButtonCallback(window,mouse_button_callback);glfwSetCursorPosCallback(window,cursor_callback);glfwSetKeyCallback(window,key_callback);
    glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glClearColor(0.075F,0.09F,0.12F,1.0F);
    while(!glfwWindowShouldClose(window)){
        int width=1,height=1;glfwGetFramebufferSize(window,&width,&height);height=std::max(height,1);width=std::max(width,1);
        glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);glLoadIdentity();const double aspect=static_cast<double>(width)/height;const double nearp=0.05,farp=500.0,top=nearp*std::tan(60.0*3.141592653589793/360.0);
        glFrustum(-top*aspect,top*aspect,-top,top,nearp,farp);
        glMatrixMode(GL_MODELVIEW);glLoadIdentity();glTranslatef(0,0,-state.distance);glRotatef(state.pitch,1,0,0);glRotatef(state.yaw,0,1,0);
        draw_grid();if(!state.mesh.triangles.empty())draw_mesh(state.mesh);
        glfwSwapBuffers(window);glfwPollEvents();
    }
    glfwDestroyWindow(window);glfwTerminate();return 0;
}
