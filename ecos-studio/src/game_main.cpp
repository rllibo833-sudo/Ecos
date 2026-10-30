#include <GLFW/glfw3.h>

#include "ecos_studio/engine_loop.hpp"
#include "ecos_studio/entity.hpp"
#include "ecos_studio/physics.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace {

using namespace ecos::studio;

struct Body2D {
    float x{};
    float y{};
    float width{};
    float height{};
};

struct Color2D {
    float red{};
    float green{};
    float blue{};
};

struct Game {
    EntityRegistry entities;
    EngineLoop loop;
    Entity player{};
    Entity opponent{};
    Entity ball{};
    float ball_velocity_x{0.78F};
    float ball_velocity_y{0.31F};
    int player_score{0};
    int opponent_score{0};
    bool game_over{false};
    bool space_was_down{false};
    std::size_t rendered_frames{0};

    Game() {
        player = create_actor({-0.88F, 0.0F, 0.035F, 0.25F}, {0.28F, 0.78F, 0.96F});
        opponent = create_actor({0.88F, 0.0F, 0.035F, 0.25F}, {0.96F, 0.48F, 0.35F});
        ball = create_actor({0.0F, 0.0F, 0.035F, 0.035F}, {0.98F, 0.91F, 0.62F});
    }

    Entity create_actor(const Body2D& body, const Color2D& color) {
        const auto entity = entities.create();
        entities.emplace<Body2D>(entity, body);
        entities.emplace<Color2D>(entity, color);
        return entity;
    }

    static Aabb bounds(const Body2D& body) {
        return {{body.x - body.width * 0.5F, body.y - body.height * 0.5F, 0.0F},
                {body.x + body.width * 0.5F, body.y + body.height * 0.5F, 0.0F}};
    }

    void reset_ball(float direction) {
        auto* body = entities.get<Body2D>(ball);
        body->x = 0.0F;
        body->y = 0.0F;
        ball_velocity_x = 0.78F * direction;
        ball_velocity_y = 0.31F;
    }

    void update(double delta_seconds, const InputSnapshot& input) {
        const float dt = static_cast<float>(delta_seconds);
        const bool space_down = input.down(InputKey::Space);
        if (game_over) {
            if (space_down && !space_was_down) {
                player_score = 0;
                opponent_score = 0;
                game_over = false;
                reset_ball(1.0F);
            }
            space_was_down = space_down;
            return;
        }
        space_was_down = space_down;

        auto* player_body = entities.get<Body2D>(player);
        auto* opponent_body = entities.get<Body2D>(opponent);
        auto* ball_body = entities.get<Body2D>(ball);
        const float player_axis = (input.down(InputKey::W) || input.down(InputKey::Up) ? 1.0F : 0.0F)
                                - (input.down(InputKey::S) || input.down(InputKey::Down) ? 1.0F : 0.0F);
        player_body->y = std::clamp(player_body->y + player_axis * 1.45F * dt, -0.86F, 0.86F);

        const float opponent_delta = ball_body->y - opponent_body->y;
        const float opponent_step = std::clamp(opponent_delta, -1.05F * dt, 1.05F * dt);
        opponent_body->y = std::clamp(opponent_body->y + opponent_step, -0.86F, 0.86F);

        ball_body->x += ball_velocity_x * dt;
        ball_body->y += ball_velocity_y * dt;
        if (ball_body->y + ball_body->height * 0.5F >= 0.96F) {
            ball_body->y = 0.96F - ball_body->height * 0.5F;
            ball_velocity_y = -std::abs(ball_velocity_y);
        } else if (ball_body->y - ball_body->height * 0.5F <= -0.96F) {
            ball_body->y = -0.96F + ball_body->height * 0.5F;
            ball_velocity_y = std::abs(ball_velocity_y);
        }

        if (ball_velocity_x < 0.0F && overlaps(bounds(*ball_body), bounds(*player_body))) {
            ball_body->x = player_body->x + (player_body->width + ball_body->width) * 0.5F + 0.002F;
            const float hit_offset = (ball_body->y - player_body->y) / (player_body->height * 0.5F);
            ball_velocity_x = std::min(1.65F, std::abs(ball_velocity_x) * 1.06F);
            ball_velocity_y = std::clamp(ball_velocity_y + hit_offset * 0.52F, -1.25F, 1.25F);
        } else if (ball_velocity_x > 0.0F && overlaps(bounds(*ball_body), bounds(*opponent_body))) {
            ball_body->x = opponent_body->x - (opponent_body->width + ball_body->width) * 0.5F - 0.002F;
            const float hit_offset = (ball_body->y - opponent_body->y) / (opponent_body->height * 0.5F);
            ball_velocity_x = -std::min(1.65F, std::abs(ball_velocity_x) * 1.06F);
            ball_velocity_y = std::clamp(ball_velocity_y + hit_offset * 0.52F, -1.25F, 1.25F);
        }

        if (ball_body->x > 1.04F) {
            ++player_score;
            reset_ball(-1.0F);
        } else if (ball_body->x < -1.04F) {
            ++opponent_score;
            reset_ball(1.0F);
        }
        game_over = player_score >= 5 || opponent_score >= 5;
    }

    void render(GLFWwindow* window) {
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        if (width <= 0 || height <= 0) return;

        glViewport(0, 0, width, height);
        glClearColor(0.035F, 0.055F, 0.09F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor3f(0.20F, 0.29F, 0.40F);
        glBegin(GL_LINES);
        for (float y = -0.9F; y < 0.95F; y += 0.12F) {
            glVertex2f(0.0F, y);
            glVertex2f(0.0F, y + 0.06F);
        }
        glEnd();

        for (const auto entity : entities.entities_with<Body2D>()) {
            const auto* body = entities.get<Body2D>(entity);
            const auto* color = entities.get<Color2D>(entity);
            if (!body || !color) continue;
            glColor3f(color->red, color->green, color->blue);
            const float left = body->x - body->width * 0.5F;
            const float right = body->x + body->width * 0.5F;
            const float bottom = body->y - body->height * 0.5F;
            const float top = body->y + body->height * 0.5F;
            glBegin(GL_QUADS);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
            glEnd();
        }

        std::string title = "ECOS Rally  |  " + std::to_string(player_score) + " : "
                          + std::to_string(opponent_score)
                          + "  |  W/S or Up/Down to move  |  First to 5";
        if (game_over) title += player_score > opponent_score
            ? "  |  YOU WIN - press Space to restart"
            : "  |  CPU WINS - press Space to restart";
        glfwSetWindowTitle(window, title.c_str());
        glfwSwapBuffers(window);
        ++rendered_frames;
    }
};

InputKey translate_key(int key) {
    switch (key) {
        case GLFW_KEY_W: return InputKey::W;
        case GLFW_KEY_S: return InputKey::S;
        case GLFW_KEY_UP: return InputKey::Up;
        case GLFW_KEY_DOWN: return InputKey::Down;
        case GLFW_KEY_SPACE: return InputKey::Space;
        case GLFW_KEY_ESCAPE: return InputKey::Escape;
        default: return InputKey::Unknown;
    }
}

void key_callback(GLFWwindow* window, int key, int, int action, int) {
    auto* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (!game) return;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    const auto mapped = translate_key(key);
    if (mapped == InputKey::Unknown || action == GLFW_RELEASE) {
        if (mapped != InputKey::Unknown) (void)game->loop.input().key_event(mapped, InputAction::Release);
        return;
    }
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        (void)game->loop.input().key_event(mapped, action == GLFW_REPEAT ? InputAction::Repeat : InputAction::Press);
}

void glfw_error_callback(int, const char* description) {
    std::cerr << "GLFW error: " << (description ? description : "unknown error") << '\n';
}

void print_help() {
    std::cout << "ECOS Rally - standalone ECOS Studio sample game\n"
              << "Controls: W/S or Up/Down to move; Escape to quit.\n"
              << "First player to score five points wins. Press Space to restart.\n"
              << "Run with --smoke-test for a hidden-window runtime check.\n";
}

} // namespace

int main(int argc, char** argv) {
    bool smoke_test = false;
    if (argc == 2 && std::string(argv[1]) == "--help") {
        print_help();
        return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--smoke-test") {
        smoke_test = true;
    } else if (argc != 1) {
        std::cerr << "Unknown argument. Use --help.\n";
        return 2;
    }

    glfwSetErrorCallback(glfw_error_callback);
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Unable to initialize GLFW. A working desktop/OpenGL display is required.\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_VISIBLE, smoke_test ? GLFW_FALSE : GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(960, 640, "ECOS Rally", nullptr, nullptr);
    if (!window) {
        std::cerr << "Unable to create the OpenGL game window.\n";
        glfwTerminate();
        return 1;
    }

    Game game;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(smoke_test ? 0 : 1);
    glfwSetWindowUserPointer(window, &game);
    glfwSetKeyCallback(window, key_callback);

    const FrameCallbacks callbacks{
        [&game](double delta, const InputSnapshot& input) { game.update(delta, input); },
        [&game, window](double, const InputSnapshot&) { game.render(window); }
    };

    double previous_time = glfwGetTime();
    std::size_t smoke_frames = 0;
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
        glfwPollEvents();
        double elapsed = 0.0;
        if (smoke_test) {
            elapsed = 1.0 / 60.0;
        } else {
            const double now = glfwGetTime();
            elapsed = now - previous_time;
            previous_time = now;
        }
        const auto report = game.loop.frame(elapsed, callbacks);
        if (report.quit_requested) glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (smoke_test && ++smoke_frames >= 120) break;
    }

    const bool rendered = game.rendered_frames >= (smoke_test ? 120U : 1U);
    glfwDestroyWindow(window);
    glfwTerminate();
    if (!rendered) {
        std::cerr << "Game runtime did not render the expected frames.\n";
        return 1;
    }
    if (smoke_test) std::cout << "ECOS Rally rendered 120 standalone frames successfully.\n";
    return 0;
}
