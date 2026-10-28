#pragma once

#include "ecos_studio/result.hpp"

#include <cstdint>
#include <vector>

namespace ecos::studio {

struct AudioClip {
    std::uint32_t sample_rate{48000};
    std::uint8_t channels{1};
    std::vector<float> samples; // Interleaved floating-point PCM in [-1, 1].
};
using AudioVoiceId = std::uint64_t;

class AudioMixer {
public:
    explicit AudioMixer(std::uint32_t output_sample_rate = 48000);
    [[nodiscard]] Result<AudioVoiceId> play(AudioClip clip, float gain = 1.0F,
                                            float pan = 0.0F, bool looping = false);
    [[nodiscard]] bool stop(AudioVoiceId id) noexcept;
    [[nodiscard]] std::vector<float> mix_stereo(std::size_t frame_count);
    [[nodiscard]] std::size_t active_voice_count() const noexcept;
    [[nodiscard]] std::uint32_t output_sample_rate() const noexcept;

private:
    struct Voice {
        AudioVoiceId id{};
        AudioClip clip;
        std::size_t cursor_frame{};
        float gain{1.0F};
        float pan{0.0F};
        bool looping{false};
        bool active{true};
    };
    std::uint32_t output_sample_rate_;
    AudioVoiceId next_id_{1};
    std::vector<Voice> voices_;
};

} // namespace ecos::studio
