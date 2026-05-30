#include "MainMenu.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

static constexpr float VIEW_W = 400.f;
static constexpr float VIEW_H = 225.f;
static constexpr float PI     = 3.14159265f;

static inline float rpx(float v) { return std::floor(v + 0.5f); }
static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

// Random float [0, 1)
static inline float rnd() {
    return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
}

// ── Constructor ───────────────────────────────────────────────────────────────
MainMenu::MainMenu(sf::Font& font, bool hasSave, bool startFullscreen)
    : font(font), screen(MenuScreen::MAIN),
      hasSave(hasSave), mainSel(0),
      optFullscreen(startFullscreen), optSel(0), optChanged(false),
      titleTimer(0.f), globalTimer(0.f),
      particleSpawnTimer(0.f), selGlow(0.f)
{
    buildMainItems();
    initBgNodes();
    // Seed a few particles immediately so the background isn't empty on frame 1
    for (int i = 0; i < 18; ++i) spawnParticle();
}

// ── Item list ─────────────────────────────────────────────────────────────────
void MainMenu::buildMainItems() {
    mainItems.clear();
    mainItems.push_back("New Game");
    if (hasSave) {
        mainItems.push_back("Load Game");
    }
    mainItems.push_back("Options");
    mainItems.push_back("Quit");
}

// ── Background nodes ──────────────────────────────────────────────────────────
void MainMenu::initBgNodes() {
    // A loose skill-tree graph scattered across the screen.
    // Positions are hand-tuned so they frame the central panel nicely.
    struct Spec { float x, y, r; };
    static const Spec specs[] = {
        // Left cluster
        { 18.f,  28.f, 5.f }, { 38.f,  55.f, 4.f }, { 14.f,  80.f, 3.f },
        { 52.f,  95.f, 5.f }, { 22.f, 130.f, 4.f }, { 48.f, 158.f, 3.f },
        { 20.f, 175.f, 5.f }, { 60.f, 185.f, 3.f },
        // Right cluster
        { 342.f,  22.f, 4.f }, { 370.f,  50.f, 5.f }, { 352.f,  78.f, 3.f },
        { 385.f, 105.f, 4.f }, { 355.f, 140.f, 5.f }, { 378.f, 165.f, 3.f },
        { 345.f, 188.f, 4.f },
        // Top / bottom accents
        { 120.f,  12.f, 3.f }, { 200.f,   8.f, 4.f }, { 280.f,  14.f, 3.f },
        { 100.f, 210.f, 3.f }, { 200.f, 216.f, 4.f }, { 300.f, 210.f, 3.f },
    };

    // Two accent colors matching the HUD palette
    const sf::Color cols[] = {
        sf::Color(80,  50, 130),   // dim violet
        sf::Color(55,  35, 100),   // darker violet
        sf::Color(100, 60, 160),   // mid purple
        sf::Color(60,  90, 140),   // blue-purple
    };

    for (auto& s : specs) {
        BgNode n;
        n.pos        = { s.x, s.y };
        n.radius     = s.r;
        n.pulse      = rnd() * 2.f * PI;
        n.pulseSpeed = 0.6f + rnd() * 1.2f;
        n.col        = cols[static_cast<int>(rnd() * 4) % 4];
        bgNodes.push_back(n);
    }
}

// ── Particles ─────────────────────────────────────────────────────────────────
void MainMenu::spawnParticle() {
    MenuParticle p;
    // Spawn along the left or right edge so they drift inward
    bool left = (rnd() < 0.5f);
    p.pos.x   = left ? (rnd() * 80.f) : (VIEW_W - rnd() * 80.f);
    p.pos.y   = rnd() * VIEW_H;
    float speed = 2.f + rnd() * 8.f;
    float angle = (left ? 0.f : PI) + (rnd() - 0.5f) * 0.8f;
    p.vel       = { std::cos(angle) * speed * 0.3f, std::sin(angle) * speed * 0.15f };
    p.maxLife   = 3.5f + rnd() * 5.f;
    p.life      = p.maxLife;
    p.radius    = 0.4f + rnd() * 1.1f;

    // Particle colors: cool purples / cold whites
    float t = rnd();
    if      (t < 0.3f) p.col = sf::Color(120, 80, 200, 180);
    else if (t < 0.6f) p.col = sf::Color(80, 60, 160, 140);
    else if (t < 0.8f) p.col = sf::Color(200, 180, 255, 100);
    else               p.col = sf::Color(255, 210,  50, 120);  // occasional gold spark

    particles.push_back(p);
}

// ── Input ─────────────────────────────────────────────────────────────────────
MenuAction MainMenu::handleEvent(const sf::Event& event) {
    if (event.type != sf::Event::KeyPressed) return MenuAction::NONE;
    const auto key = event.key.code;

    if (screen == MenuScreen::OPTIONS) {
        if (key == sf::Keyboard::W || key == sf::Keyboard::Up)
            optSel = (optSel + 1) % 2;
        if (key == sf::Keyboard::S || key == sf::Keyboard::Down)
            optSel = (optSel + 1) % 2;

        if (key == sf::Keyboard::E) {
            if (optSel == 0) { optFullscreen = !optFullscreen; optChanged = true; }
            else             { screen = MenuScreen::MAIN; }
        }
        if (key == sf::Keyboard::Q) screen = MenuScreen::MAIN;
        return MenuAction::NONE;
    }

    int n = static_cast<int>(mainItems.size());
    if (key == sf::Keyboard::W || key == sf::Keyboard::Up)
        mainSel = (mainSel - 1 + n) % n;
    if (key == sf::Keyboard::S || key == sf::Keyboard::Down)
        mainSel = (mainSel + 1) % n;

    if (key == sf::Keyboard::E) {
        const std::string& s = mainItems[mainSel];
        if (s == "New Game")  return MenuAction::NEW_GAME;
        if (s == "Load Game") return MenuAction::LOAD_GAME;
        if (s == "Options")   { screen = MenuScreen::OPTIONS; optSel = 0; }
        if (s == "Quit")      return MenuAction::QUIT;
    }
    if (key == sf::Keyboard::Q) return MenuAction::QUIT;
    return MenuAction::NONE;
}

// ── Draw helpers ──────────────────────────────────────────────────────────────
sf::Text MainMenu::makeText(const std::string& str, unsigned size,
                            sf::Color col, float x, float y)
{
    sf::Text t(str, font, size);
    t.setFillColor(col);
    t.setPosition(rpx(x), rpx(y));
    return t;
}

void MainMenu::drawNodeConnector(sf::RenderWindow& w,
                                  sf::Vector2f a, sf::Vector2f b,
                                  sf::Color col, float thickness)
{
    sf::Vector2f d  = b - a;
    float len       = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1.f) return;

    sf::RectangleShape line({ len, thickness });
    line.setFillColor(col);
    line.setOrigin(0.f, thickness * 0.5f);
    line.setPosition(a);
    float angle = std::atan2(d.y, d.x) * 180.f / PI;
    line.setRotation(angle);
    w.draw(line);
}

void MainMenu::drawBgNodes(sf::RenderWindow& w) {
    // Draw connectors first (behind nodes)
    // Hard-coded edge list matching the spec order in initBgNodes()
    static const int edges[][2] = {
        // Left
        {0,1},{1,2},{1,3},{2,3},{3,4},{4,5},{5,6},{5,7},
        // Right
        {8,9},{9,10},{9,11},{10,11},{11,12},{12,13},{13,14},
        // Top
        {15,16},{16,17},
        // Bottom
        {18,19},{19,20},
        // Cross-links left↔top/bottom
        {0,15},{6,18},{7,19},
        // Cross-links right↔top/bottom
        {8,17},{14,20},
    };

    for (auto& e : edges) {
        if (e[0] >= static_cast<int>(bgNodes.size()) ||
            e[1] >= static_cast<int>(bgNodes.size())) continue;

        const BgNode& A = bgNodes[e[0]];
        const BgNode& B = bgNodes[e[1]];

        // Connector brightness pulses with the average of the two endpoint pulses
        float p  = (std::sin(A.pulse) + std::sin(B.pulse)) * 0.25f + 0.5f; // 0..1
        sf::Color c(
            static_cast<sf::Uint8>(A.col.r * 0.6f),
            static_cast<sf::Uint8>(A.col.g * 0.6f),
            static_cast<sf::Uint8>(A.col.b * 0.6f),
            static_cast<sf::Uint8>(lerp(30.f, 80.f, p))
        );
        drawNodeConnector(w, A.pos, B.pos, c, 0.7f);
    }

    // Draw nodes
    for (const BgNode& n : bgNodes) {
        float glow  = std::sin(n.pulse) * 0.5f + 0.5f;  // 0..1
        float r     = n.radius * (0.85f + glow * 0.3f);

        // Outer glow ring
        float gr    = r + 2.5f;
        sf::CircleShape glowCirc(gr);
        glowCirc.setOrigin(gr, gr);
        glowCirc.setPosition(n.pos);
        glowCirc.setFillColor(sf::Color(
            n.col.r, n.col.g, n.col.b,
            static_cast<sf::Uint8>(lerp(0.f, 40.f, glow))
        ));
        w.draw(glowCirc);

        // Core
        sf::CircleShape circ(r);
        circ.setOrigin(r, r);
        circ.setPosition(n.pos);
        circ.setFillColor(sf::Color(
            static_cast<sf::Uint8>(lerp(static_cast<float>(n.col.r), 255.f, glow * 0.4f)),
            static_cast<sf::Uint8>(lerp(static_cast<float>(n.col.g), 200.f, glow * 0.25f)),
            static_cast<sf::Uint8>(lerp(static_cast<float>(n.col.b), 255.f, glow * 0.5f)),
            static_cast<sf::Uint8>(lerp(80.f, 200.f, glow))
        ));
        w.draw(circ);

        // Bright centre dot
        float cr = r * 0.35f;
        sf::CircleShape centre(cr);
        centre.setOrigin(cr, cr);
        centre.setPosition(n.pos);
        centre.setFillColor(sf::Color(220, 200, 255,
            static_cast<sf::Uint8>(lerp(60.f, 180.f, glow))));
        w.draw(centre);
    }
}

void MainMenu::drawParticles(sf::RenderWindow& w) {
    for (const MenuParticle& p : particles) {
        float t   = p.life / p.maxLife;
        float fade = (t < 0.15f) ? (t / 0.15f) : ((t > 0.85f) ? ((t - 0.85f) / 0.15f * (-1.f) + 1.f) : 1.f);
        sf::Uint8 a = static_cast<sf::Uint8>(p.col.a * fade);
        sf::CircleShape c(p.radius);
        c.setOrigin(p.radius, p.radius);
        c.setPosition(p.pos);
        c.setFillColor(sf::Color(p.col.r, p.col.g, p.col.b, a));
        w.draw(c);
    }
}

void MainMenu::drawPanel(sf::RenderWindow& w,
                          float px, float py, float pw, float ph,
                          sf::Color fill, sf::Color outline, float glowStrength)
{
    // Optional outer glow (drawn as a slightly larger rect with low alpha)
    if (glowStrength > 0.f) {
        float g = 3.f;
        sf::RectangleShape glow({pw + g * 2.f, ph + g * 2.f});
        glow.setFillColor(sf::Color(0, 0, 0, 0));
        glow.setOutlineThickness(g);
        glow.setOutlineColor(sf::Color(
            outline.r, outline.g, outline.b,
            static_cast<sf::Uint8>(glowStrength * 80.f)
        ));
        glow.setPosition(rpx(px - g), rpx(py - g));
        w.draw(glow);
    }

    sf::RectangleShape panel({pw, ph});
    panel.setFillColor(fill);
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(outline);
    panel.setPosition(rpx(px), rpx(py));
    w.draw(panel);

    // Inner top highlight line
    sf::RectangleShape topLine({pw - 4.f, 1.f});
    topLine.setFillColor(sf::Color(outline.r, outline.g, outline.b, 40));
    topLine.setPosition(rpx(px + 2.f), rpx(py + 1.f));
    w.draw(topLine);
}

void MainMenu::drawMenuItem(sf::RenderWindow& w,
                             const std::string& label, int idx, bool selected,
                             float panelX, float itemY, float panelW,
                             bool dimmed)
{
    const float ITEM_H = 20.f;

    if (selected) {
        // Highlight bar with glow
        float glowAlpha = lerp(28.f, 55.f, selGlow);
        sf::RectangleShape hl({panelW - 6.f, ITEM_H - 2.f});
        hl.setFillColor(sf::Color(
            static_cast<sf::Uint8>(lerp(28.f, 50.f, selGlow)),
            static_cast<sf::Uint8>(lerp(18.f, 30.f, selGlow)),
            static_cast<sf::Uint8>(lerp(55.f, 80.f, selGlow))
        ));
        hl.setOutlineThickness(1.f);
        hl.setOutlineColor(sf::Color(
            static_cast<sf::Uint8>(lerp(80.f, 140.f, selGlow)),
            static_cast<sf::Uint8>(lerp(50.f, 90.f,  selGlow)),
            static_cast<sf::Uint8>(lerp(150.f, 210.f, selGlow))
        ));
        hl.setPosition(rpx(panelX + 3.f), rpx(itemY));
        w.draw(hl);

        // Node connector dot on the left edge — skill-tree flavour
        float dotR = 2.5f;
        sf::CircleShape dot(dotR);
        dot.setOrigin(dotR, dotR);
        dot.setPosition(rpx(panelX + 3.f + dotR), rpx(itemY + ITEM_H * 0.5f - 1.f));
        dot.setFillColor(sf::Color(
            255, static_cast<sf::Uint8>(lerp(190.f, 220.f, selGlow)), 50,
            static_cast<sf::Uint8>(lerp(180.f, 255.f, selGlow))
        ));
        w.draw(dot);

        // Short connector line from dot into the text area
        drawNodeConnector(w,
            { panelX + 3.f + dotR * 2.f, itemY + ITEM_H * 0.5f - 1.f },
            { panelX + 16.f,              itemY + ITEM_H * 0.5f - 1.f },
            sf::Color(255, 210, 50, static_cast<sf::Uint8>(lerp(100.f, 200.f, selGlow))),
            0.8f
        );
    }

    sf::Color col;
    if (dimmed)        col = sf::Color(70, 65, 85);
    else if (selected) col = sf::Color(
        static_cast<sf::Uint8>(lerp(200.f, 240.f, selGlow)),
        static_cast<sf::Uint8>(lerp(185.f, 225.f, selGlow)),
        static_cast<sf::Uint8>(lerp(255.f, 255.f, selGlow))
    );
    else               col = sf::Color(130, 120, 155);

    sf::Text item = makeText(label, 16, col, panelX + 20.f, itemY + 1.f);
    w.draw(item);
}

// ── Render ────────────────────────────────────────────────────────────────────
void MainMenu::render(sf::RenderWindow& window) {
    const float DT = 1.f / 60.f;
    titleTimer  += DT;
    globalTimer += DT;
    selGlow      = std::sin(globalTimer * 3.5f) * 0.5f + 0.5f;

    // ── Update background nodes ───────────────────────────────────────────────
    for (BgNode& n : bgNodes) n.pulse += n.pulseSpeed * DT;

    // ── Update particles ──────────────────────────────────────────────────────
    particleSpawnTimer += DT;
    if (particleSpawnTimer > 0.18f) { spawnParticle(); particleSpawnTimer = 0.f; }

    for (MenuParticle& p : particles) {
        p.pos  += p.vel;
        p.life -= DT;
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
                       [](const MenuParticle& p){ return p.life <= 0.f; }),
        particles.end()
    );

    // ── Background ────────────────────────────────────────────────────────────
    sf::RectangleShape bg({VIEW_W, VIEW_H});
    bg.setFillColor(sf::Color(6, 8, 16));
    window.draw(bg);

    // Vignette-ish gradient overlay (cheap: two semi-transparent rects at edges)
    for (int side = 0; side < 2; ++side) {
        sf::RectangleShape vig({60.f, VIEW_H});
        vig.setFillColor(sf::Color(0, 0, 0, 80));
        vig.setPosition(side == 0 ? 0.f : VIEW_W - 60.f, 0.f);
        window.draw(vig);
    }

    // Subtle horizontal scanlines
    for (float y = 0.f; y < VIEW_H; y += 3.f) {
        sf::RectangleShape sl({VIEW_W, 1.f});
        sl.setFillColor(sf::Color(0, 0, 0, 18));
        sl.setPosition(0.f, y);
        window.draw(sl);
    }

    // ── Draw background skill-tree decoration ─────────────────────────────────
    drawBgNodes(window);

    // ── Draw particles ────────────────────────────────────────────────────────
    drawParticles(window);

    // ── Title ─────────────────────────────────────────────────────────────────
    float bob   = std::sin(titleTimer * 1.4f) * 1.8f;
    float titleY = 18.f + bob;

    // Distant glow behind title
    {
        float gr = 55.f + std::sin(titleTimer * 0.9f) * 5.f;
        sf::CircleShape halo(gr);
        halo.setOrigin(gr, gr);
        halo.setPosition(VIEW_W * 0.5f, titleY + 8.f);
        halo.setFillColor(sf::Color(40, 15, 70, 30));
        window.draw(halo);
    }

    // Shadow layer
    sf::Text shadow = makeText("PIGEONFALL", 16,
                               sf::Color(0, 0, 0, 160), 0.f, titleY + 2.f);
    float sw = shadow.getLocalBounds().width;
    shadow.setPosition(rpx((VIEW_W - sw) * 0.5f + 2.f), rpx(titleY + 2.f));
    window.draw(shadow);

    // Main title — slight colour pulse
    float tp = std::sin(titleTimer * 1.1f) * 0.5f + 0.5f;
    sf::Text title = makeText("PIGEONFALL", 16,
        sf::Color(
            static_cast<sf::Uint8>(lerp(190.f, 230.f, tp)),
            static_cast<sf::Uint8>(lerp(150.f, 185.f, tp)),
            static_cast<sf::Uint8>(lerp(240.f, 255.f, tp))
        ), 0.f, titleY);
    float tw = title.getLocalBounds().width;
    title.setPosition(rpx((VIEW_W - tw) * 0.5f), rpx(titleY));
    window.draw(title);

    // Version subtitle
    sf::Text sub = makeText("v0.1  alpha", 16, sf::Color(65, 50, 95), 0.f, titleY + 18.f);
    sub.setScale(0.6f, 0.6f);
    float subw = sub.getGlobalBounds().width;
    sub.setPosition(rpx((VIEW_W - subw) * 0.5f), rpx(titleY + 18.f));
    window.draw(sub);

    // Thin decorative line under title
    float lineY = rpx(titleY + 32.f);
    float lineW = 120.f;
    sf::RectangleShape titleLine({lineW, 1.f});
    titleLine.setFillColor(sf::Color(90, 60, 140, 100));
    titleLine.setPosition(rpx((VIEW_W - lineW) * 0.5f), lineY);
    window.draw(titleLine);
    // Node dots at each end of the decorative line
    for (int side = 0; side < 2; ++side) {
        float dotR = 1.8f;
        sf::CircleShape dot(dotR);
        dot.setOrigin(dotR, dotR);
        dot.setPosition(
            rpx((VIEW_W - lineW) * 0.5f + (side == 0 ? 0.f : lineW)),
            rpx(lineY)
        );
        dot.setFillColor(sf::Color(130, 90, 200, 180));
        window.draw(dot);
    }

    // ── Screen content ────────────────────────────────────────────────────────
    if (screen == MenuScreen::MAIN) {
        const float ITEM_H  = 20.f;
        const float PW      = 148.f;
        const float PH      = static_cast<float>(mainItems.size()) * ITEM_H + 20.f;
        const float PX      = rpx((VIEW_W - PW) * 0.5f);
        const float PY      = rpx(VIEW_H * 0.5f - PH * 0.5f + 14.f);

        // Outer glow ring for the panel — pulses with selGlow
        drawPanel(window, PX, PY, PW, PH,
                  sf::Color(9, 9, 20),
                  sf::Color(
                      static_cast<sf::Uint8>(lerp(50.f, 90.f, selGlow)),
                      static_cast<sf::Uint8>(lerp(32.f, 55.f, selGlow)),
                      static_cast<sf::Uint8>(lerp(90.f, 150.f, selGlow))
                  ),
                  selGlow);

        // Panel corner node accents
        struct CornerDot { float cx, cy; };
        CornerDot corners[] = {
            { PX,        PY       },
            { PX + PW,   PY       },
            { PX,        PY + PH  },
            { PX + PW,   PY + PH  },
        };
        for (auto& c : corners) {
            float cr = 2.2f;
            sf::CircleShape cd(cr);
            cd.setOrigin(cr, cr);
            cd.setPosition(rpx(c.cx), rpx(c.cy));
            cd.setFillColor(sf::Color(
                static_cast<sf::Uint8>(lerp(80.f, 140.f, selGlow)),
                static_cast<sf::Uint8>(lerp(55.f, 100.f, selGlow)),
                static_cast<sf::Uint8>(lerp(140.f, 220.f, selGlow)),
                200
            ));
            window.draw(cd);
        }

        for (int i = 0; i < static_cast<int>(mainItems.size()); ++i) {
            float iy  = PY + 10.f + i * ITEM_H;
            bool  sel = (i == mainSel);
            bool  dim = (mainItems[i] == "Load Game" && !hasSave);
            drawMenuItem(window, mainItems[i], i, sel, PX, iy, PW, dim);
        }

        // Hint
        sf::Text hint = makeText("W/S  E=select", 16, sf::Color(45, 35, 70), 0.f, VIEW_H - 14.f);
        hint.setScale(0.65f, 0.65f);
        float hw = hint.getGlobalBounds().width;
        hint.setPosition(rpx((VIEW_W - hw) * 0.5f), rpx(VIEW_H - 14.f));
        window.draw(hint);
    }
    else if (screen == MenuScreen::OPTIONS) {
        const float PW     = 200.f;
        const float PH     = 72.f;
        const float PX     = rpx((VIEW_W - PW) * 0.5f);
        const float PY     = rpx(VIEW_H * 0.5f - PH * 0.5f + 14.f);

        drawPanel(window, PX, PY, PW, PH,
                  sf::Color(9, 9, 20),
                  sf::Color(
                      static_cast<sf::Uint8>(lerp(50.f, 90.f, selGlow)),
                      static_cast<sf::Uint8>(lerp(32.f, 55.f, selGlow)),
                      static_cast<sf::Uint8>(lerp(90.f, 150.f, selGlow))
                  ),
                  selGlow);

        // "OPTIONS" header with node connector decorations
        sf::Text ot = makeText("OPTIONS", 16, sf::Color(
            static_cast<sf::Uint8>(lerp(140.f, 185.f, selGlow)),
            static_cast<sf::Uint8>(lerp(100.f, 140.f, selGlow)),
            sf::Uint8(255)
        ), 0.f, PY + 6.f);
        float otw = ot.getLocalBounds().width;
        ot.setPosition(rpx(PX + PW * 0.5f - otw * 0.5f), rpx(PY + 6.f));
        window.draw(ot);

        // Connector lines flanking "OPTIONS"
        float hdrMidY = rpx(PY + 6.f + 7.f);
        float textLeft  = rpx(PX + PW * 0.5f - otw * 0.5f) - 4.f;
        float textRight = rpx(PX + PW * 0.5f + otw * 0.5f) + 4.f;
        drawNodeConnector(window, {PX + 6.f, hdrMidY}, {textLeft, hdrMidY},
                          sf::Color(110, 70, 180, 120), 0.8f);
        drawNodeConnector(window, {textRight, hdrMidY}, {PX + PW - 6.f, hdrMidY},
                          sf::Color(110, 70, 180, 120), 0.8f);
        for (float ex : {PX + 6.f, PX + PW - 6.f}) {
            float dr = 1.8f;
            sf::CircleShape d(dr); d.setOrigin(dr, dr);
            d.setPosition(rpx(ex), rpx(hdrMidY));
            d.setFillColor(sf::Color(130, 90, 200, 180));
            window.draw(d);
        }

        // Rows
        struct Row { std::string label, value; int idx; };
        Row rows[] = {
            { "Fullscreen", optFullscreen ? "ON" : "OFF", 0 },
            { "Back",       "",                            1 },
        };

        for (auto& row : rows) {
            float ry  = PY + 26.f + row.idx * 18.f;
            bool  sel = (optSel == row.idx);

            if (sel) {
                sf::RectangleShape hl({PW - 6.f, 16.f});
                hl.setFillColor(sf::Color(
                    static_cast<sf::Uint8>(lerp(28.f, 50.f, selGlow)),
                    static_cast<sf::Uint8>(lerp(18.f, 30.f, selGlow)),
                    static_cast<sf::Uint8>(lerp(55.f, 80.f, selGlow))
                ));
                hl.setOutlineThickness(1.f);
                hl.setOutlineColor(sf::Color(
                    static_cast<sf::Uint8>(lerp(80.f, 140.f, selGlow)),
                    static_cast<sf::Uint8>(lerp(50.f, 90.f,  selGlow)),
                    static_cast<sf::Uint8>(lerp(150.f, 210.f, selGlow))
                ));
                hl.setPosition(rpx(PX + 3.f), rpx(ry));
                window.draw(hl);

                float dr = 2.2f;
                sf::CircleShape dot(dr); dot.setOrigin(dr, dr);
                dot.setPosition(rpx(PX + 3.f + dr), rpx(ry + 8.f));
                dot.setFillColor(sf::Color(255, 210, 50,
                    static_cast<sf::Uint8>(lerp(160.f, 255.f, selGlow))));
                window.draw(dot);
            }

            sf::Color col = sel
                ? sf::Color(
                    static_cast<sf::Uint8>(lerp(200.f, 240.f, selGlow)),
                    static_cast<sf::Uint8>(lerp(185.f, 225.f, selGlow)),
                    255)
                : sf::Color(130, 120, 155);

            sf::Text lbl = makeText(row.label, 16, col, PX + 20.f, ry);
            window.draw(lbl);

            if (!row.value.empty()) {
                bool on = (row.value == "ON");
                sf::Color vc = on ? sf::Color(80, 220, 100) : sf::Color(180, 80, 80);
                sf::Text val = makeText(row.value, 16, vc, 0.f, ry);
                float vw = val.getLocalBounds().width;
                val.setPosition(rpx(PX + PW - vw - 10.f), rpx(ry));
                window.draw(val);
            }
        }

        sf::Text hint = makeText("E=select  Q=back", 16,
                                 sf::Color(45, 35, 70), 0.f, VIEW_H - 14.f);
        hint.setScale(0.65f, 0.65f);
        float hw = hint.getGlobalBounds().width;
        hint.setPosition(rpx((VIEW_W - hw) * 0.5f), rpx(VIEW_H - 14.f));
        window.draw(hint);
    }
}
