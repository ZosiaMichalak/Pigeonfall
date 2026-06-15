#include "SaveSlotUI.h"
#include <cmath>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

static inline float px(float v) { return std::floor(v + 0.5f); }

// ── Constructor ───────────────────────────────────────────────────────────────
DifficultySelectUI::DifficultySelectUI(sf::Font& font) : font_(font) {}

void DifficultySelectUI::open() {
    open_             = true;
    difficultySel_    = 1;
    chosenDifficulty_ = Difficulty::NORMAL;
}

void DifficultySelectUI::close() {
    open_             = false;
}

// ── Event handling ────────────────────────────────────────────────────────────
SlotUIResult DifficultySelectUI::handleEvent(const sf::Event& event) {
    if (!open_) return SlotUIResult::NONE;
    if (event.type != sf::Event::KeyPressed) return SlotUIResult::NONE;

    auto key = event.key.code;

    if (key == sf::Keyboard::Left  || key == sf::Keyboard::A)
        difficultySel_ = (difficultySel_ - 1 + 3) % 3;
    if (key == sf::Keyboard::Right || key == sf::Keyboard::D)
        difficultySel_ = (difficultySel_ + 1) % 3;

    if (key == sf::Keyboard::E || key == sf::Keyboard::Return ||
        key == sf::Keyboard::Space) {
        chosenDifficulty_  = static_cast<Difficulty>(difficultySel_);
        close();
        return SlotUIResult::SELECTED;
    }
    if (key == sf::Keyboard::Escape || key == sf::Keyboard::Q) {
        close();
        return SlotUIResult::CANCELLED;
    }

    return SlotUIResult::NONE;
}

// ── Rendering ─────────────────────────────────────────────────────────────────
void DifficultySelectUI::render(sf::RenderWindow& window) {
    if (!open_) return;

    // Full screen dim overlay
    sf::RectangleShape dim({ VIEW_W, VIEW_H });
    dim.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dim);

    // Difficulty picker popup centered
    const float DW = 230.f, DH = 80.f;
    const float DX = px((VIEW_W - DW) / 2.f);
    const float DY = px((VIEW_H - DH) / 2.f);
    drawPanel(window, DX, DY, DW, DH, sf::Color(8, 8, 20), sf::Color(120, 90, 200));

    sf::Text q("Choose Difficulty", font_, 16);
    q.setFillColor(sf::Color(190, 150, 255));
    q.setPosition(px(DX + (DW - q.getLocalBounds().width) / 2.f), px(DY + 7.f));
    window.draw(q);

    struct DiffBtn { const char* label; sf::Color col; };
    DiffBtn dbns[3] = {
        { "EASY",   sf::Color(80,  200, 80)  },
        { "NORMAL", sf::Color(220, 180, 50)  },
        { "HARD",   sf::Color(220,  70, 70)  }
    };
    const float BTN_W   = 58.f;
    const float BTN_GAP = 8.f;
    const float BTN_TOTAL = 3 * BTN_W + 2 * BTN_GAP;
    for (int i = 0; i < 3; ++i) {
        bool sel = (difficultySel_ == i);
        float bx = px(DX + (DW - BTN_TOTAL) / 2.f + i * (BTN_W + BTN_GAP));
        float by = DY + 32.f;
        sf::RectangleShape btn({BTN_W, 22.f});
        btn.setFillColor(sel ? sf::Color(30, 22, 50) : sf::Color(14, 12, 24));
        btn.setOutlineThickness(1.f);
        btn.setOutlineColor(sel ? dbns[i].col : sf::Color(55, 45, 80));
        btn.setPosition(px(bx), px(by));
        window.draw(btn);

        sf::Text bLabel(dbns[i].label, font_, 16);
        bLabel.setFillColor(sel ? dbns[i].col : sf::Color(90, 80, 110));
        bLabel.setScale(0.85f, 0.85f);
        float lw = bLabel.getLocalBounds().width * 0.85f;
        bLabel.setPosition(px(bx + (BTN_W - lw) / 2.f), px(by + 3.f));
        window.draw(bLabel);
    }

    sf::Text hint2("A/D select     E confirm     Q cancel", font_, 16);
    hint2.setScale(0.62f, 0.62f);
    hint2.setFillColor(sf::Color(70, 65, 105));
    hint2.setPosition(px(DX + (DW - hint2.getLocalBounds().width * 0.62f) / 2.f), px(DY + DH - 14.f));
    window.draw(hint2);
}

// ── Helpers ───────────────────────────────────────────────────────────────────
void DifficultySelectUI::drawPanel(sf::RenderWindow& w,
                                   float x, float y, float pw, float ph,
                                   sf::Color fill, sf::Color outline)
{
    sf::RectangleShape panel({ pw, ph });
    panel.setFillColor(fill);
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(outline);
    panel.setPosition(px(x), px(y));
    w.draw(panel);
}

sf::Text DifficultySelectUI::makeText(const std::string& str, unsigned size,
                                      sf::Color col, float x, float y)
{
    sf::Text t(str, font_, size);
    t.setFillColor(col);
    t.setPosition(px(x), px(y));
    return t;
}
