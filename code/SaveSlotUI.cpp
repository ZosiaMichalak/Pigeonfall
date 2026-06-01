#include "SaveSlotUI.h"
#include <cmath>
#include <cstdio>
#include <string>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;

static inline float px(float v) { return std::floor(v + 0.5f); }

// ── Constructor ───────────────────────────────────────────────────────────────
SaveSlotUI::SaveSlotUI(sf::Font& font) : font_(font) {
    refresh();
}

void SaveSlotUI::refresh() {
    for (int i = 0; i < SAVE_SLOT_COUNT; ++i) {
        if (SaveSystem::hasSlot(i)) {
            SaveData sd = SaveSystem::load(i);
            slots_[i].exists     = sd.exists;
            slots_[i].level      = sd.level;
            slots_[i].roomIndex  = sd.roomIndex;
            slots_[i].coins      = sd.coins;
            slots_[i].playTime   = sd.playTime;
            slots_[i].difficulty = sd.difficulty;
        } else {
            slots_[i] = {};
        }
    }
}

void SaveSlotUI::open(SlotUIMode mode) {
    mode_             = mode;
    open_             = true;
    selectedSlot_     = 0;
    confirmingDelete_ = false;
    confirmSel_       = 1;
    pickingDifficulty_ = false;
    difficultySel_    = 1;
    chosenDifficulty_ = Difficulty::NORMAL;
    glowTimer_        = 0.f;
    refresh();
}

void SaveSlotUI::close() {
    open_             = false;
    confirmingDelete_ = false;
}

// ── Event handling ────────────────────────────────────────────────────────────
SlotUIResult SaveSlotUI::handleEvent(const sf::Event& event) {
    if (!open_) return SlotUIResult::NONE;
    if (event.type != sf::Event::KeyPressed) return SlotUIResult::NONE;

    auto key = event.key.code;

    // ── Difficulty picker (NEW_GAME only, after slot chosen) ──────────────────
    if (pickingDifficulty_) {
        if (key == sf::Keyboard::Left  || key == sf::Keyboard::A)
            difficultySel_ = (difficultySel_ - 1 + 3) % 3;
        if (key == sf::Keyboard::Right || key == sf::Keyboard::D)
            difficultySel_ = (difficultySel_ + 1) % 3;

        if (key == sf::Keyboard::E || key == sf::Keyboard::Return ||
            key == sf::Keyboard::Space) {
            chosenDifficulty_  = static_cast<Difficulty>(difficultySel_);
            pickingDifficulty_ = false;
            close();
            return SlotUIResult::SELECTED;
        }
        if (key == sf::Keyboard::Escape || key == sf::Keyboard::Q) {
            pickingDifficulty_ = false;  // go back to slot selection
        }
        return SlotUIResult::NONE;
    }

    // ── Delete-confirmation sub-menu ──────────────────────────────────────────
    if (confirmingDelete_) {
        if (key == sf::Keyboard::Left  || key == sf::Keyboard::A)
            confirmSel_ = 0;
        if (key == sf::Keyboard::Right || key == sf::Keyboard::D)
            confirmSel_ = 1;

        if (key == sf::Keyboard::E || key == sf::Keyboard::Return ||
            key == sf::Keyboard::Space) {
            if (confirmSel_ == 0) {          // Yes — delete
                SaveSystem::deleteSlot(selectedSlot_);
                refresh();
            }
            confirmingDelete_ = false;
            return SlotUIResult::NONE;
        }

        if (key == sf::Keyboard::Escape || key == sf::Keyboard::Q) {
            confirmingDelete_ = false;
            return SlotUIResult::NONE;
        }
        return SlotUIResult::NONE;
    }

    // ── Normal navigation ─────────────────────────────────────────────────────
    if (key == sf::Keyboard::Left  || key == sf::Keyboard::A)
        selectedSlot_ = (selectedSlot_ - 1 + SAVE_SLOT_COUNT) % SAVE_SLOT_COUNT;
    if (key == sf::Keyboard::Right || key == sf::Keyboard::D)
        selectedSlot_ = (selectedSlot_ + 1) % SAVE_SLOT_COUNT;

    // Confirm selection
    if (key == sf::Keyboard::E || key == sf::Keyboard::Return ||
        key == sf::Keyboard::Space) {
        // LOAD: can't load an empty slot
        if (mode_ == SlotUIMode::LOAD && !slots_[selectedSlot_].exists)
            return SlotUIResult::NONE;

        // NEW_GAME: open difficulty picker before confirming
        if (mode_ == SlotUIMode::NEW_GAME) {
            pickingDifficulty_ = true;
            difficultySel_     = 1;  // default NORMAL
            return SlotUIResult::NONE;
        }

        close();
        return SlotUIResult::SELECTED;
    }

    // Delete key (only in SAVE/LOAD mode, only on occupied slots)
    if (key == sf::Keyboard::Delete || key == sf::Keyboard::BackSpace) {
        if (slots_[selectedSlot_].exists) {
            confirmingDelete_ = true;
            confirmSel_       = 1;   // default = No
        }
        return SlotUIResult::NONE;
    }

    // Cancel
    if (key == sf::Keyboard::Escape || key == sf::Keyboard::Q) {
        close();
        return SlotUIResult::CANCELLED;
    }

    return SlotUIResult::NONE;
}

// ── Drawing helpers ───────────────────────────────────────────────────────────
void SaveSlotUI::drawPanel(sf::RenderWindow& w,
                            float x, float y, float pw, float ph,
                            sf::Color fill, sf::Color outline) {
    sf::RectangleShape p({pw, ph});
    p.setFillColor(fill);
    p.setOutlineThickness(1.f);
    p.setOutlineColor(outline);
    p.setPosition(px(x), px(y));
    w.draw(p);
}

sf::Text SaveSlotUI::makeText(const std::string& str, unsigned size,
                               sf::Color col, float x, float y) {
    sf::Text t(str, font_, size);
    t.setFillColor(col);
    t.setPosition(px(x), px(y));
    return t;
}

void SaveSlotUI::drawSlotCard(sf::RenderWindow& w,
                               int idx, float cx, float cy,
                               float cw, float ch, bool selected) {
    const SlotInfo& s = slots_[idx];

    // ── Card background ───────────────────────────────────────────────────────
    float glow = selected ? (0.5f + 0.5f * std::sin(glowTimer_ * 4.f)) : 0.f;
    sf::Color outline = selected
        ? sf::Color(static_cast<sf::Uint8>(80  + glow * 80),
                    static_cast<sf::Uint8>(55  + glow * 30),
                    static_cast<sf::Uint8>(160 + glow * 60))
        : sf::Color(40, 35, 70);
    sf::Color fill = selected ? sf::Color(28, 20, 52) : sf::Color(14, 12, 26);
    drawPanel(w, cx, cy, cw, ch, fill, outline);

    // Accent bar at top
    sf::RectangleShape bar({cw, 3.f});
    bar.setFillColor(selected ? sf::Color(100, 70, 200) : sf::Color(45, 35, 80));
    bar.setPosition(px(cx), px(cy));
    w.draw(bar);

    // ── Slot label ────────────────────────────────────────────────────────────
    auto slotLabel = makeText("SLOT " + std::to_string(idx + 1), 16,
                               selected ? sf::Color(200, 160, 255) : sf::Color(120, 100, 160),
                               cx + 8.f, cy + 7.f);
    w.draw(slotLabel);

    if (!s.exists) {
        // Empty slot
        auto empty = makeText("- EMPTY -", 16, sf::Color(55, 50, 80),
                               cx + cw / 2.f - 22.f, cy + ch / 2.f - 8.f);
        w.draw(empty);

        if (mode_ != SlotUIMode::LOAD) {
            auto hint = makeText("E  new game", 16, sf::Color(65, 60, 95),
                                  cx + 6.f, cy + ch - 18.f);
            hint.setScale(0.7f, 0.7f);
            w.draw(hint);
        }
        return;
    }

    // ── Filled slot — show saved data ─────────────────────────────────────────
    auto lvlT = makeText("LVL " + std::to_string(s.level), 16,
                          sf::Color(220, 215, 255), cx + 8.f, cy + 24.f);
    w.draw(lvlT);

    auto roomT = makeText("Room " + std::to_string(s.roomIndex + 1), 16,
                           sf::Color(160, 155, 200), cx + 8.f, cy + 38.f);
    w.draw(roomT);

    // Difficulty badge
    {
        sf::Color diffCol = (s.difficulty == Difficulty::EASY)   ? sf::Color(80, 200, 80)
                          : (s.difficulty == Difficulty::HARD)   ? sf::Color(220, 70, 70)
                                                                  : sf::Color(220, 180, 50);
        auto diffT = makeText(difficultyName(s.difficulty), 16, diffCol, cx + 8.f, cy + 52.f);
        diffT.setScale(0.75f, 0.75f);
        w.draw(diffT);
    }

    // Format play time as h:mm:ss
    {
        int totalSec = static_cast<int>(s.playTime);
        int h  = totalSec / 3600;
        int m  = (totalSec % 3600) / 60;
        int sc = totalSec % 60;
        char timeBuf[16];
        if (h > 0)
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d:%02d", h, m, sc);
        else
            std::snprintf(timeBuf, sizeof(timeBuf), "%d:%02d", m, sc);
        auto timeT = makeText(timeBuf, 16, sf::Color(150, 210, 150), cx + 8.f, cy + 64.f);
        timeT.setScale(0.75f, 0.75f);
        w.draw(timeT);
    }

    // Action hint at bottom
    std::string hintStr = (mode_ == SlotUIMode::LOAD) ? "E  load" : "E  overwrite";
    auto hint = makeText(hintStr, 16, sf::Color(80, 75, 120), cx + 6.f, cy + ch - 18.f);
    hint.setScale(0.7f, 0.7f);
    w.draw(hint);

    // Delete hint (only for occupied slots in SAVE / LOAD mode)
    if (selected) {
        auto delHint = makeText("Del  delete", 16, sf::Color(140, 50, 50),
                                 cx + cw - 62.f, cy + ch - 18.f);
        delHint.setScale(0.7f, 0.7f);
        w.draw(delHint);
    }
}

// ── Main render ───────────────────────────────────────────────────────────────
void SaveSlotUI::render(sf::RenderWindow& window) {
    if (!open_) return;

    glowTimer_ += 1.f / 60.f;

    // Dim overlay
    sf::RectangleShape overlay({VIEW_W, VIEW_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 175));
    window.draw(overlay);

    // Outer panel
    const float PW = 360.f, PH = 160.f;
    const float PX = px((VIEW_W - PW) / 2.f);
    const float PY = px((VIEW_H - PH) / 2.f);
    drawPanel(window, PX, PY, PW, PH, sf::Color(8, 8, 18), sf::Color(70, 50, 130));

    // Title — always "Choose slot" regardless of mode
    std::string titleStr = "Choose slot";
    sf::Text title(titleStr, font_, 16);
    title.setFillColor(sf::Color(170, 130, 255));
    title.setPosition(
        px(PX + (PW - title.getLocalBounds().width) / 2.f),
        px(PY + 6.f));
    window.draw(title);

    // Three slot cards side by side
    const float GAP  = 6.f;
    const float CW   = (PW - GAP * 4.f) / 3.f;
    const float CH   = 100.f;
    const float CY   = PY + 28.f;

    for (int i = 0; i < SAVE_SLOT_COUNT; ++i) {
        float cx = PX + GAP + i * (CW + GAP);
        drawSlotCard(window, i, cx, CY, CW, CH, i == selectedSlot_);
    }

    // Nav hint footer
    sf::Text nav("A/D navigate     Q/Esc cancel", font_, 16);
    nav.setScale(0.7f, 0.7f);
    nav.setFillColor(sf::Color(70, 65, 105));
    nav.setPosition(
        px(PX + (PW - nav.getLocalBounds().width * 0.7f) / 2.f),
        px(PY + PH - 14.f));
    window.draw(nav);

    // ── Difficulty picker popup (NEW_GAME only) ───────────────────────────────
    if (pickingDifficulty_) {
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

        sf::Text hint2("A/D select     E confirm     Q back", font_, 16);
        hint2.setScale(0.62f, 0.62f);
        hint2.setFillColor(sf::Color(70, 65, 105));
        hint2.setPosition(px(DX + (DW - hint2.getLocalBounds().width * 0.62f) / 2.f), px(DY + DH - 14.f));
        window.draw(hint2);
    }

    // ── Delete confirmation pop-up ────────────────────────────────────────────
    if (confirmingDelete_) {
        const float DW = 180.f, DH = 56.f;
        const float DX = px((VIEW_W - DW) / 2.f);
        const float DY = px((VIEW_H - DH) / 2.f);
        drawPanel(window, DX, DY, DW, DH, sf::Color(18, 8, 8), sf::Color(180, 40, 40));

        sf::Text q("Delete this save?", font_, 16);
        q.setFillColor(sf::Color(220, 80, 80));
        q.setPosition(
            px(DX + (DW - q.getLocalBounds().width) / 2.f),
            px(DY + 8.f));
        window.draw(q);

        // Yes / No buttons — centred inside the popup
        struct Btn { const char* label; sf::Color col; };
        Btn btns[2] = { {"YES", sf::Color(220,60,60)}, {"NO", sf::Color(140,140,140)} };
        const float BTN_W   = 60.f;
        const float BTN_GAP = 12.f;
        const float BTN_TOTAL = 2 * BTN_W + BTN_GAP;
        for (int i = 0; i < 2; ++i) {
            bool sel = (confirmSel_ == i);
            float bx = px(DX + (DW - BTN_TOTAL) / 2.f + i * (BTN_W + BTN_GAP));
            float by = DY + 32.f;
            sf::RectangleShape btn({60.f, 16.f});
            btn.setFillColor(sel ? sf::Color(40, 20, 20) : sf::Color(20, 14, 14));
            btn.setOutlineThickness(1.f);
            btn.setOutlineColor(sel ? btns[i].col : sf::Color(60, 40, 40));
            btn.setPosition(px(bx), px(by));
            window.draw(btn);

            sf::Text bLabel(btns[i].label, font_, 16);
            bLabel.setFillColor(sel ? btns[i].col : sf::Color(90, 80, 80));
            bLabel.setPosition(
                px(bx + (BTN_W - bLabel.getLocalBounds().width) / 2.f),
                px(by - 8.f + (16.f - bLabel.getLocalBounds().height) / 2.f - 1.f));
            window.draw(bLabel);
        }
    }
}
