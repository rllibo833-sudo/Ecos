#include "ecos_studio/audio.hpp"
#include <cmath>
#include <iostream>
namespace {
int failures=0;
void check(bool v,const char* m){if(!v){std::cerr<<"FAIL: "<<m<<'\n';++failures;}}
bool near(float a,float b){return std::abs(a-b)<1.0e-4F;}
void test_mono_mix_and_pan(){
 using namespace ecos::studio;
 AudioMixer mixer(48000);
 auto id=mixer.play({48000,1,{0.5F,0.25F}},1.0F,0.0F,false);
 check(static_cast<bool>(id),"accepts valid mono PCM");
 auto out=mixer.mix_stereo(2);
 check(out.size()==4,"produces interleaved stereo frames");
 if(out.size()==4){check(near(out[0],0.353553F)&&near(out[1],0.353553F),"centers mono audio using equal-power pan");check(near(out[2],0.176777F),"advances clip cursor between frames");}
 check(mixer.active_voice_count()==0,"removes completed one-shot voice");
}
void test_loop_stop_and_clip(){
 using namespace ecos::studio;
 AudioMixer mixer(44100);
 auto id=mixer.play({44100,2,{0.8F,-0.8F,0.8F,-0.8F}},1.0F,0.0F,true);
 check(static_cast<bool>(id),"accepts stereo loop");
 auto out=mixer.mix_stereo(3);
 check(out.size()==6&&near(out[0],0.8F)&&near(out[1],-0.8F)&&near(out[4],0.8F),"loops clips and mixes stereo channels");
 check(id&&mixer.stop(id.value()),"stops an active voice");
 check(mixer.active_voice_count()==0,"does not count a stopped voice as active");
 (void)mixer.mix_stereo(1);
 check(mixer.active_voice_count()==0,"stopped voice is cleaned up");
}
void test_validation_and_clipping(){
 using namespace ecos::studio;
 AudioMixer mixer(48000);
 check(!mixer.play({44100,1,{0.1F}}),"rejects sample-rate mismatch without resampling");
 check(!mixer.play({48000,3,{0.1F,0.2F,0.3F}}),"rejects unsupported channel count");
 check(!mixer.play({48000,1,{NAN}}),"rejects non-finite samples");
 check(!mixer.play({48000,1,{0.1F}},1.0F,2.0F),"rejects pan outside range");
 (void)mixer.play({48000,1,{1.0F}},4.0F,0.0F,false);
 auto out=mixer.mix_stereo(1);
 check(out.size()==2&&out[0]<=1.0F&&out[1]<=1.0F,"clips output to floating-point audio range");
}
}
int main(){test_mono_mix_and_pan();test_loop_stop_and_clip();test_validation_and_clipping();if(failures){std::cerr<<failures<<" audio test(s) failed.\n";return 1;}std::cout<<"ECOS Studio audio mixer tests passed.\n";return 0;}
