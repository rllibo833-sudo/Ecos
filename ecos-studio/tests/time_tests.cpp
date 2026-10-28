#include "ecos_studio/time.hpp"
#include <cmath>
#include <iostream>
namespace {int failures=0;void check(bool ok,const char* m){if(!ok){std::cerr<<"FAIL: "<<m<<'\n';++failures;}}
bool near(double a,double b){return std::abs(a-b)<1e-8;}
void test_fixed_steps(){using namespace ecos::studio;FrameClock c(0.1,1.0,8);auto a=c.advance(0.05);check(a.fixed_steps==0&&near(a.interpolation_alpha,0.5),"retains partial fixed step");auto b=c.advance(0.16);check(b.fixed_steps==2&&near(b.interpolation_alpha,0.1),"consumes fixed steps deterministically");}
void test_clamp_and_step_cap(){using namespace ecos::studio;FrameClock c(0.01,0.1,3);auto a=c.advance(0.5);check(a.fixed_steps==3,"caps fixed steps per frame");check(a.dropped_time_seconds>0.39,"reports clamped and overloaded time");check(c.accumulator_seconds()<0.01,"discards excess whole steps to prevent spiral of death");}
void test_invalid_elapsed_and_reset(){using namespace ecos::studio;FrameClock c;auto a=c.advance(-1);check(a.fixed_steps==0&&near(a.frame_delta_seconds,0),"ignores negative elapsed time");(void)c.advance(NAN);check(c.accumulator_seconds()==0,"ignores non-finite elapsed time");(void)c.advance(0.01);c.reset();check(c.accumulator_seconds()==0,"resets accumulator");}
void test_invalid_configuration(){using namespace ecos::studio;FrameClock c(0,-1,0);check(c.fixed_delta_seconds()>0,"uses safe defaults for invalid configuration");auto a=c.advance(1);check(a.fixed_steps==1,"enforces at least one maximum step");}
}
int main(){test_fixed_steps();test_clamp_and_step_cap();test_invalid_elapsed_and_reset();test_invalid_configuration();if(failures){std::cerr<<failures<<" time tests failed.\n";return 1;}std::cout<<"ECOS Studio frame clock tests passed.\n";return 0;}