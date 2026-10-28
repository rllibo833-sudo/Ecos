#include "ecos_studio/engine_loop.hpp"
#include <iostream>
namespace {int failures=0;void check(bool ok,const char* m){if(!ok){std::cerr<<"FAIL: "<<m<<'\n';++failures;}}
void test_update_render_order(){using namespace ecos::studio;EngineLoop loop(0.1,1.0,8);int updates=0,renders=0;double fixed_sum=0,alpha=-1;bool saw_key=false;
 (void)loop.input().key_event(InputKey::W,InputAction::Press);
 FrameCallbacks cb;
 cb.fixed_update=[&](double dt,const InputSnapshot& s){++updates;fixed_sum+=dt;saw_key=s.down(InputKey::W);};
 cb.render=[&](double a,const InputSnapshot& s){++renders;alpha=a;saw_key=saw_key&&s.down(InputKey::W);};
 auto report=loop.frame(0.25,cb);
 check(updates==2&&report.update_callbacks==2,"runs bounded fixed updates");
 check(renders==1&&report.rendered,"renders exactly once per frame");
 check(fixed_sum>0.199&&fixed_sum<0.201,"passes fixed simulation delta");
 check(alpha>0.49&&alpha<0.51,"passes interpolation alpha to renderer");
 check(saw_key,"passes stable input snapshot to updates and render");
}
void test_transients_and_quit(){using namespace ecos::studio;EngineLoop loop;loop.input().mouse_move(1,1);loop.input().mouse_move(4,5);loop.input().request_quit();double dx=-1;bool quit=false;
 FrameCallbacks cb;cb.render=[&](double,const InputSnapshot& s){dx=s.mouse_delta_x;quit=s.quit_requested;};
 auto report=loop.frame(0,cb);check(dx==3&&quit&&report.quit_requested,"exposes cursor delta and quit request");
 loop.frame(0,cb);check(dx==0,"clears transient input after frame consumption");
 loop.reset();check(!loop.input().snapshot().quit_requested,"reset clears loop input");
}
void test_empty_callbacks(){using namespace ecos::studio;EngineLoop loop;auto report=loop.frame(0.016,{});check(!report.rendered&&report.update_callbacks==0,"supports headless/no-callback frame");}
}
int main(){test_update_render_order();test_transients_and_quit();test_empty_callbacks();if(failures){std::cerr<<failures<<" engine loop tests failed.\n";return 1;}std::cout<<"ECOS Studio engine loop tests passed.\n";return 0;}