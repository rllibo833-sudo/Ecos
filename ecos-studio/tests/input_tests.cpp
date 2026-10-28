#include "ecos_studio/input.hpp"
#include <iostream>
#include <thread>
#include <vector>
namespace {
int failures = 0;
void check(bool condition, const char* message) { if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; } }
void test_key_edges_and_held_state() {
    using namespace ecos::studio;
    InputState input;
    input.apply({InputEventType::key_pressed,42});
    check(input.key_down(42) && input.key_pressed(42),"press sets held and edge state");
    input.begin_frame();
    check(input.key_down(42) && !input.key_pressed(42),"held key persists while press edge clears");
    input.apply({InputEventType::key_released,42});
    check(!input.key_down(42) && input.key_released(42),"release clears held and sets release edge");
}
void test_mouse_and_wheel() {
    using namespace ecos::studio;
    InputState input;
    input.apply({InputEventType::mouse_button_pressed,1});
    input.apply({InputEventType::mouse_moved,0,12.5F,7.25F});
    input.apply({InputEventType::mouse_wheel,0,1.0F,-2.0F});
    input.apply({InputEventType::mouse_wheel,0,0.5F,1.0F});
    check(input.mouse_button_down(1) && input.mouse_button_pressed(1),"tracks mouse button");
    check(input.mouse_x()==12.5F && input.mouse_y()==7.25F,"tracks pointer position");
    check(input.wheel_x()==1.5F && input.wheel_y()==-1.0F,"accumulates wheel deltas");
    input.begin_frame();
    check(input.wheel_x()==0.0F && input.wheel_y()==0.0F,"wheel resets at frame boundary");
}
void test_focus_loss() {
    using namespace ecos::studio;
    InputState input;
    input.apply({InputEventType::key_pressed,10});
    input.apply({InputEventType::key_pressed,11});
    input.apply({InputEventType::mouse_button_pressed,2});
    input.apply({InputEventType::focus_lost});
    check(!input.key_down(10) && !input.key_down(11),"focus loss clears held keys");
    check(input.key_released(10) && input.key_released(11),"focus loss emits key releases");
    check(!input.mouse_button_down(2) && input.mouse_button_released(2),"focus loss releases mouse buttons");
}
void test_thread_safe_queue() {
    using namespace ecos::studio;
    InputEventQueue queue;
    queue.push({InputEventType::key_pressed,1});
    queue.push({InputEventType::mouse_moved,0,2.0F,3.0F});
    check(queue.drain().size()==2 && queue.size()==0,"drains queued events in order and empties queue");
    std::vector<std::thread> writers;
    for(int i=0;i<4;++i) writers.emplace_back([&queue,i] {
        for(int j=0;j<100;++j) queue.push({InputEventType::key_pressed,i*100+j});
    });
    for(auto& writer:writers) writer.join();
    check(queue.drain().size()==400,"accepts concurrent event producers");
}
}
int main() {
    test_key_edges_and_held_state(); test_mouse_and_wheel(); test_focus_loss(); test_thread_safe_queue();
    if(failures){std::cerr<<failures<<" input test(s) failed.\n";return 1;}
    std::cout<<"ECOS Studio input tests passed.\n";return 0;
}
