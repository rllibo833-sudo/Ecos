#include "ecos_studio/audio.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
namespace ecos::studio {
AudioMixer::AudioMixer(std::uint32_t output_sample_rate)
    : output_sample_rate_(output_sample_rate == 0 ? 48000 : output_sample_rate) {}
Result<AudioVoiceId> AudioMixer::play(AudioClip clip,float gain,float pan,bool looping) {
    if(clip.sample_rate!=output_sample_rate_)
        return Result<AudioVoiceId>::failure({ErrorCode::unsupported,"Audio clip sample rate must match mixer output rate; resampling is not implemented"});
    if((clip.channels!=1&&clip.channels!=2)||clip.samples.empty()||clip.samples.size()%clip.channels!=0)
        return Result<AudioVoiceId>::failure({ErrorCode::invalid_argument,"Audio clip must contain non-empty mono or stereo interleaved PCM"});
    if(!std::isfinite(gain)||gain<0.0F||!std::isfinite(pan)||pan < -1.0F||pan > 1.0F)
        return Result<AudioVoiceId>::failure({ErrorCode::invalid_argument,"Audio gain must be finite/non-negative and pan must be within [-1,1]"});
    if(!std::all_of(clip.samples.begin(),clip.samples.end(),[](float s){return std::isfinite(s);}))
        return Result<AudioVoiceId>::failure({ErrorCode::invalid_argument,"Audio clip contains non-finite samples"});
    if(next_id_==0||next_id_==std::numeric_limits<AudioVoiceId>::max())
        return Result<AudioVoiceId>::failure({ErrorCode::invalid_state,"Audio voice ID capacity exhausted"});
    const auto id=next_id_++;
    voices_.push_back({id,std::move(clip),0,gain,pan,looping,true});
    return Result<AudioVoiceId>::success(id);
}
bool AudioMixer::stop(AudioVoiceId id) noexcept {
    for(auto& voice:voices_)if(voice.id==id&&voice.active){voice.active=false;return true;}
    return false;
}
std::vector<float> AudioMixer::mix_stereo(std::size_t frame_count) {
    if(frame_count>std::numeric_limits<std::size_t>::max()/2)return {};
    std::vector<float> output(frame_count*2,0.0F);
    for(auto& voice:voices_){
        if(!voice.active)continue;
        const auto clip_frames=voice.clip.samples.size()/voice.clip.channels;
        for(std::size_t frame=0;frame<frame_count;++frame){
            if(voice.cursor_frame>=clip_frames){
                if(voice.looping)voice.cursor_frame=0;
                else{voice.active=false;break;}
            }
            const auto base=voice.cursor_frame*voice.clip.channels;
            float left{},right{};
            if(voice.clip.channels==1){
                const float sample=voice.clip.samples[base]*voice.gain;
                constexpr float quarter_pi=0.7853981633974483F;
                left=sample*std::cos((voice.pan+1.0F)*quarter_pi);
                right=sample*std::sin((voice.pan+1.0F)*quarter_pi);
            }else{
                const float left_balance=voice.pan>0.0F?1.0F-voice.pan:1.0F;
                const float right_balance=voice.pan<0.0F?1.0F+voice.pan:1.0F;
                left=voice.clip.samples[base]*voice.gain*left_balance;
                right=voice.clip.samples[base+1]*voice.gain*right_balance;
            }
            output[frame*2]+=left;output[frame*2+1]+=right;++voice.cursor_frame;
        }
        if(!voice.looping&&voice.cursor_frame>=clip_frames)voice.active=false;
    }
    for(auto& sample:output)sample=std::clamp(sample,-1.0F,1.0F);
    voices_.erase(std::remove_if(voices_.begin(),voices_.end(),[](const Voice& voice){return !voice.active;}),voices_.end());
    return output;
}
std::size_t AudioMixer::active_voice_count() const noexcept {
    return static_cast<std::size_t>(std::count_if(voices_.begin(),voices_.end(),[](const Voice& voice){return voice.active;}));
}
std::uint32_t AudioMixer::output_sample_rate() const noexcept {return output_sample_rate_;}
} // namespace ecos::studio
