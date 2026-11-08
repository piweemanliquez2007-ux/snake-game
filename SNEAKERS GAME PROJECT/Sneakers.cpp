#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <SFML/Audio.hpp>
#include <fstream>
#include <string>

using namespace std;

bool sunflowerUnlocked = false;
bool warriorUnlocked = false;
bool amethystUnlocked = false;
bool bloomUnlocked = false;
bool diamondUnlocked = false;

std::string selectedHeadSkin = "Snake_SKINS/caterpillar_head.png";
std::string selectedBodySkin = "Snake_SKINS/caterpillar_body.png";
std::string selectedTailSkin = "Snake_SKINS/caterpillar_tail.png";

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

static bool loadTextureFile(sf::Texture &tex, const std::string &filename)
{
    if (tex.loadFromFile(filename))
        return true;

    std::string assetsPath = std::string("assets/") + filename;
    if (tex.loadFromFile(assetsPath))
        return true;

    std::cerr << "Texture load failed: tried '" << filename << "' and '" << assetsPath << "'\n";
    return false;
}

enum GameState
{
    LOADING,
    MENU,
    PLAYING,
    PAUSED,
    SETTINGS,
    MAPS,
    SKINS,
    GAME_OVER
};

float distance(sf::Vector2f a, sf::Vector2f b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

enum class FoodType
{
    Normal,
    Speed,
    Big,
    Mega,
    Ultra
};

struct Food
{
    sf::CircleShape shape;
    FoodType type = FoodType::Normal;

    Food(float x, float y, FoodType t = FoodType::Normal)
    {
        type = t;

        switch (type)
        {
        case FoodType::Normal:
            shape.setRadius(5.f);
            shape.setFillColor(sf::Color::Green);
            break;
        case FoodType::Speed:
            shape.setRadius(6.f);
            shape.setFillColor(sf::Color(120, 220, 255));
            break;
        case FoodType::Mega:
            shape.setRadius(8.f);
            shape.setFillColor(sf::Color(180, 120, 255));
            break;
        case FoodType::Big:
            shape.setRadius(7.f);
            shape.setFillColor(sf::Color(255, 190, 60));
            break;
        case FoodType::Ultra:
            shape.setRadius(10.f);
            shape.setFillColor(sf::Color(255, 80, 210));
            break;
        }

        shape.setPosition(x, y);
    }
};

class Snake
{
public:
    float segmentDistance = 18.f;
    int baseLength = 20;

    void loadSkin(const std::string &headFile, const std::string &bodyFile)
    {
        bool headLoaded = loadTextureFile(headTex, headFile);
        bool bodyLoaded = loadTextureFile(bodyTex, bodyFile);

        if (headLoaded)
            headSprite.setTexture(headTex);
        if (bodyLoaded)
            bodySprite.setTexture(bodyTex);
    }

    sf::Texture headTex;
    sf::Texture bodyTex;

    sf::Sprite headSprite;
    sf::Sprite bodySprite;

    vector<sf::Vector2f> body;

    float speed = 2.8f;
    sf::Vector2f velocity = {1.f, 0.f};
    float turnSpeed = 0.12f;
    int score = 0;

    Snake(float x, float y)
    {
        loadSkin("Snake_SKINS/caterpillar_head.png", "Snake_SKINS/caterpillar_body.png");

        const float snakeScale = 0.08f;
        headSprite.setScale(snakeScale, snakeScale);
        bodySprite.setScale(snakeScale, snakeScale);

        if (headTex.getSize().x > 0 && headTex.getSize().y > 0)
            headSprite.setOrigin(headTex.getSize().x / 2.f, headTex.getSize().y / 2.f);
        if (bodyTex.getSize().x > 0 && bodyTex.getSize().y > 0)
            bodySprite.setOrigin(bodyTex.getSize().x / 2.f, bodyTex.getSize().y / 2.f);

        body.push_back({x, y});
    }

    bool move(sf::Vector2f dir, bool boost)
    {
        float sizePenalty = score * 0.02f;
        float finalSpeed = speed - sizePenalty;
        if (finalSpeed < 1.5f)
            finalSpeed = 1.5f;

        (void)boost;

        if (dir.x != 0 || dir.y != 0)
        {
            if (body.size() < 2)
                velocity = dir;
            else
                velocity += (dir - velocity) * turnSpeed;

            float len = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            if (len > 0)
                velocity /= len;
        }

        sf::Vector2f newHead = body[0] + velocity * finalSpeed;

        if (newHead.x < 0 || newHead.y < 0 || newHead.x > WINDOW_WIDTH || newHead.y > WINDOW_HEIGHT)
            return false;

        body.insert(body.begin(), newHead);

        for (int i = 1; i < (int)body.size(); i++)
        {
            sf::Vector2f segDir = body[i] - body[i - 1];
            float len = sqrt(segDir.x * segDir.x + segDir.y * segDir.y);
            if (len > segmentDistance)
            {
                segDir /= len;
                body[i] = body[i - 1] + segDir * segmentDistance;
            }
        }

        int maxSize = baseLength + score;
        while (body.size() > (size_t)maxSize)
            body.pop_back();

        return true;
    }

    void draw(sf::RenderWindow &window)
    {
        if (body.empty())
            return;

        for (int i = (int)body.size() - 1; i >= 0; i--)
        {
            sf::Sprite sprite;
            if (i == 0)
                sprite.setTexture(headTex);
            else
                sprite.setTexture(bodyTex);

            if (sprite.getTexture()->getSize().x > 0 && sprite.getTexture()->getSize().y > 0)
            {
                sprite.setOrigin(sprite.getTexture()->getSize().x / 2.f, sprite.getTexture()->getSize().y / 2.f);
            }

            sprite.setScale(0.3f, 0.3f);

            if (body.size() >= 2)
            {
                sf::Vector2f dir;
                if (i == 0)
                    dir = body[0] - body[1];
                else
                    dir = body[i - 1] - body[i];

                float angle = atan2(dir.y, dir.x) * 180.f / 3.14159265f;
                sprite.setRotation(angle + 90.f);
            }

            sprite.setPosition(body[i]);
            window.draw(sprite);
        }
    }
};

struct Fireball
{
    sf::Sprite sprite;
    float collisionRadius = 20.f;
    sf::Vector2f velocity;
    float speed = 220.f;
};

struct MoltenRockVent
{
    sf::Sprite sprite;
    float collisionRadius = 35.f;
};

struct TreeTrunk
{
    sf::Sprite sprite;
    float collisionRadius = 20.f;
};

struct FallenLog
{
    sf::Sprite sprite;
    float collisionPadding = 0.f;
};

struct SandTornado
{
    sf::Sprite sprite;
    float radius = 30.f;
    sf::Vector2f center;
    float orbitRadius = 120.f;
    float orbitSpeed = 0.9f;

    float period = 3.f;
    float activeTime = 0.9f;
    float phase = 0.f;

    float angle = 0.f;

    bool isActive(float t) const
    {
        float local = fmod(t + phase, period);
        return local <= activeTime;
    }

    void update(float t, float dt)
    {
        (void)t;
        angle += orbitSpeed * dt;

        sf::Vector2f offset(cos(angle) * orbitRadius, sin(angle) * orbitRadius);
        sf::Vector2f pos = center + offset;
        sprite.setPosition(pos);
        sprite.rotate(2.f);
    }
};

struct CactusSpike
{
    sf::Sprite sprite;
    float collisionRadius = 65.f;
};

struct Button
{
    sf::RectangleShape shape;
    sf::Text text;

    sf::Vector2f baseSize;
    sf::Vector2f basePos;

    bool hovered = false;
    float anim = 0.f;

    bool clicked = false;
    sf::Clock clickedClock;
    float clickedFlashSec = 0.2f;

    Button() {}

    Button(sf::Vector2f size, sf::Vector2f pos, sf::Font &font, const std::string &label)
    {
        baseSize = size;
        basePos = pos;

        shape.setSize(size);
        shape.setPosition(pos);
        shape.setFillColor(sf::Color(60, 60, 60));

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(28);
        text.setFillColor(sf::Color::White);

        sf::FloatRect b = text.getLocalBounds();
        text.setOrigin(b.width / 2, b.height / 2);
        text.setPosition(pos.x + size.x / 2, pos.y + size.y / 2 - 5);
    }

    void update()
    {
        float target = hovered ? 1.f : 0.f;
        anim += (target - anim) * 0.15f;

        float clickedT = clicked ? clickedClock.getElapsedTime().asSeconds() : clickedFlashSec;
        if (clicked && clickedT >= clickedFlashSec)
            clicked = false;

        float scale = 1.f + anim * 0.08f;

        shape.setSize(baseSize * scale);
        shape.setPosition(
            basePos.x - (baseSize.x * (scale - 1.f) / 2),
            basePos.y - (baseSize.y * (scale - 1.f) / 2));

        if (clicked)
        {
            shape.setOutlineThickness(6.f);
            float alpha = 255.f * (1.f - (clickedClock.getElapsedTime().asSeconds() / clickedFlashSec));
            if (alpha < 0.f)
                alpha = 0.f;

            sf::Color fill = shape.getFillColor();
            shape.setOutlineColor(sf::Color(fill.r, fill.g, fill.b, (sf::Uint8)alpha));
        }
        else
        {
            shape.setOutlineThickness(0.f);
            shape.setOutlineColor(sf::Color::Transparent);
        }
    }

    void triggerClick()
    {
        clicked = true;
        clickedClock.restart();
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(shape);
        window.draw(text);
    }

    bool isClicked(sf::Vector2f mouse)
    {
        return shape.getGlobalBounds().contains(mouse);
    }

    void setHover(bool h)
    {
        hovered = h;
        shape.setFillColor(h ? sf::Color(100, 100, 100) : sf::Color(60, 60, 60));
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Sneakers.io");
    window.setFramerateLimit(60);

    GameState gameState = LOADING;

    int highScore = 0;

    ifstream loadFile("assets/highscore.txt");
    ifstream skinFile("assets/save.txt");
    if (skinFile.is_open())
    {
        skinFile >> sunflowerUnlocked >> warriorUnlocked >> amethystUnlocked >> bloomUnlocked >> diamondUnlocked;
        skinFile.close();
    }
    if (loadFile.is_open())
    {
        loadFile >> highScore;
        loadFile.close();
    }

    // If the player manually edits highscore.txt, unlock skins based on the loaded high score too.
    // Unlocks are normally granted during gameplay using player.score; this makes startup consistent.
    {
        bool derivedChanged = false;
        if (highScore >= 100 && !sunflowerUnlocked)
        {
            sunflowerUnlocked = true;
            derivedChanged = true;
        }
        if (highScore >= 200 && !warriorUnlocked)
        {
            warriorUnlocked = true;
            derivedChanged = true;
        }
        if (highScore >= 300 && !amethystUnlocked)
        {
            amethystUnlocked = true;
            derivedChanged = true;
        }
        if (highScore >= 400 && !bloomUnlocked)
        {
            bloomUnlocked = true;
            derivedChanged = true;
        }
        if (highScore >= 500 && !diamondUnlocked)
        {
            diamondUnlocked = true;
            derivedChanged = true;
        }

        // Persist derived unlocks so the UI updates immediately after first run.
        if (derivedChanged)
        {
            // saveUnlocks() is declared later in the file; defer saving by duplicating logic here.
            std::ofstream skinSaveFile("assets/save.txt");
            if (skinSaveFile.is_open())
            {
                skinSaveFile << sunflowerUnlocked << ' '
                             << warriorUnlocked << ' '
                             << amethystUnlocked << ' '
                             << bloomUnlocked << ' '
                             << diamondUnlocked;
            }
        }
    }

    sf::Texture logoTexture;
    sf::Sprite logoSprite;
    sf::RectangleShape loadingBarBG, loadingBar;
    sf::Clock loadingClock;

    float loadingTime = 10.f;
    float alpha = 0;

    loadTextureFile(logoTexture, "assets/Sneakersloading.jpg");
    logoSprite.setTexture(logoTexture);

    sf::Vector2u ws = window.getSize();
    sf::Vector2u ts = logoTexture.getSize();
    logoSprite.setOrigin(ts.x / 2, ts.y / 2);
    logoSprite.setPosition(ws.x / 2, ws.y / 2);

    loadingBarBG.setSize({400, 20});
    loadingBarBG.setFillColor(sf::Color(50, 50, 50));
    loadingBarBG.setPosition(440, 600);

    loadingBar.setSize({0, 20});
    loadingBar.setFillColor(sf::Color::Green);
    loadingBar.setPosition(440, 600);

    bool mapChosen = false;
    bool justStartedPlaying = false;
    sf::Clock collisionGraceClock;
    const float collisionGraceSec = 0.5f;

    enum MapType
    {
        FOREST,
        DESERT,
        LAVA
    };
    MapType selectedMap = LAVA;

    Snake player(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);

    vector<Food> foods;
    const int MAX_FOODS = 150;

    auto rollFoodType = [&]() -> FoodType
    {
        int roll = rand() % 100;
        FoodType newType = FoodType::Normal;
        if (roll < 8)
            newType = FoodType::Ultra;
        else if (roll < 18)
            newType = FoodType::Mega;
        else if (roll < 35)
            newType = FoodType::Big;
        else if (roll < 55)
            newType = FoodType::Speed;
        else
            newType = FoodType::Normal;
        return newType;
    };

    for (int i = 0; i < MAX_FOODS; i++)
        foods.push_back(Food(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, rollFoodType()));

    sf::Clock foodRespawnClock;
    float foodRefillIntervalSec = 0.2f;
    int foodsEatenSinceLastSpawn = 0;

    auto trySpawnFood = [&]()
    {
        if (foods.size() >= (size_t)MAX_FOODS)
            return;

        int batchBase = 6;
        int batchExtra = min(24, foodsEatenSinceLastSpawn * 3);
        int batch = batchBase + batchExtra;

        while (batch-- > 0 && foods.size() < (size_t)MAX_FOODS)
            foods.push_back(Food(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, rollFoodType()));
    };

    vector<Fireball> fireballs;
    vector<TreeTrunk> treeTrunks;
    vector<FallenLog> fallenLogs;
    vector<SandTornado> sandTornadoes;
    vector<CactusSpike> cactusSpikes;
    vector<MoltenRockVent> moltenRocks;

    sf::Clock obstacleClock;

    const sf::Vector2f snakeSpawn(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f);
    auto dist2 = [&](sf::Vector2f a, sf::Vector2f b) -> float
    {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return dx * dx + dy * dy;
    };

    auto isFarEnoughFromSnake = [&](sf::Vector2f pos, float minDist) -> bool
    {
        return dist2(pos, snakeSpawn) >= (minDist * minDist);
    };

    auto rerollPositionFarFromSnake = [&](auto makePos, float minDist, int maxAttempts) -> sf::Vector2f
    {
        sf::Vector2f last = makePos();
        for (int attempt = 0; attempt < maxAttempts; attempt++)
        {
            last = makePos();
            if (isFarEnoughFromSnake(last, minDist))
                return last;
        }
        return last;
    };

    sf::Texture fireballTexture;
    sf::Texture treeTrunkTexture;
    sf::Texture logTexture;
    sf::Texture cactusTexture;
    sf::Texture tornadoTexture;
    sf::Texture moltenVentTexture;
    sf::Texture moltenRockTexture;

    auto ensureObstacleTexturesLoaded = [&]()
    {
        static bool loaded = false;
        if (loaded)
            return;

        loadTextureFile(fireballTexture, "assets/fireball.png");
        loadTextureFile(moltenVentTexture, "assets/molten_vent.png");
        loadTextureFile(moltenRockTexture, "assets/moltenrock.png");

        loadTextureFile(treeTrunkTexture, "assets/tree.png");
        loadTextureFile(logTexture, "assets/log.png");
        loadTextureFile(tornadoTexture, "assets/tornado.png");
        loadTextureFile(cactusTexture, "assets/cactus.png");

        loaded = true;
    };

    auto resetObstacles = [&]()
    {
        ensureObstacleTexturesLoaded();
        fireballs.clear();
        moltenRocks.clear();

        int fireCount = 4;
        for (int i = 0; i < fireCount; i++)
        {
            Fireball fb;
            float r = 9.f + (rand() % 6);
            fb.collisionRadius = r * 1.5f;

            fb.sprite.setTexture(fireballTexture);

            const float baseTexRadius = 64.f;
            float scale = (baseTexRadius > 0.f) ? (r / baseTexRadius) : 1.f;
            fb.sprite.setScale(scale, scale);

            fb.sprite.setOrigin(fireballTexture.getSize().x / 2.f, fireballTexture.getSize().y / 2.f);

            float headRadius = 18.f;
            float minDist = fb.collisionRadius + headRadius + 40.f;

            auto makeFirePos = [&]() -> sf::Vector2f
            {
                int edge = rand() % 4;
                float x = 0, y = 0;
                if (edge == 0)
                    x = 0, y = rand() % WINDOW_HEIGHT;
                else if (edge == 1)
                    x = WINDOW_WIDTH, y = rand() % WINDOW_HEIGHT;
                else if (edge == 2)
                    x = rand() % WINDOW_WIDTH, y = 0;
                else
                    x = rand() % WINDOW_WIDTH, y = WINDOW_HEIGHT;
                return sf::Vector2f(x, y);
            };

            sf::Vector2f p = rerollPositionFarFromSnake(makeFirePos, minDist, 25);
            fb.sprite.setPosition(p.x, p.y);

            sf::Vector2f toC(WINDOW_WIDTH / 2.f - p.x, WINDOW_HEIGHT / 2.f - p.y);
            float len = sqrt(toC.x * toC.x + toC.y * toC.y);
            if (len < 0.001f)
                len = 1.f;
            toC /= len;

            float jitterX = ((rand() % 200) - 100) / 500.f;
            float jitterY = ((rand() % 200) - 100) / 500.f;
            toC += sf::Vector2f(jitterX, jitterY);

            float len2 = sqrt(toC.x * toC.x + toC.y * toC.y);
            if (len2 < 0.001f)
                len2 = 1.f;
            toC /= len2;

            fb.velocity = toC;
            fireballs.push_back(fb);
        }

        int rockCount = 10;
        for (int i = 0; i < rockCount; i++)
        {
            MoltenRockVent rr;
            float r = 22.f + (rand() % 18);
            rr.collisionRadius = r;

            if (moltenRockTexture.getSize().x > 0 && moltenRockTexture.getSize().y > 0)
                rr.sprite.setTexture(moltenRockTexture);
            else
                rr.sprite.setTexture(moltenVentTexture);

            const float baseTexRadius = 64.f;
            float scale = (baseTexRadius > 0.f) ? (r / baseTexRadius) : 1.f;
            rr.sprite.setScale(scale, scale);

            rr.sprite.setOrigin(rr.sprite.getTexture()->getSize().x / 2.f, rr.sprite.getTexture()->getSize().y / 2.f);

            float headRadius = 18.f;
            float minDist = rr.collisionRadius + headRadius + 40.f;

            auto makeRockPos = [&]() -> sf::Vector2f
            {
                float x = 60 + rand() % (WINDOW_WIDTH - 120);
                float y = 60 + rand() % (WINDOW_HEIGHT - 120);
                return sf::Vector2f(x, y);
            };

            sf::Vector2f p = rerollPositionFarFromSnake(makeRockPos, minDist, 25);
            rr.sprite.setPosition(p.x, p.y);

            moltenRocks.push_back(rr);
        }

        // IMPORTANT FIX: reset obstacle clock so we don't immediately collide from stale motion/positions.
        obstacleClock.restart();
    };

    auto clearObstacles = [&]()
    {
        fireballs.clear();
        treeTrunks.clear();
        fallenLogs.clear();
        sandTornadoes.clear();
        cactusSpikes.clear();
        moltenRocks.clear();
    };

    auto resetForestObstacles = [&]()
    {
        ensureObstacleTexturesLoaded();
        treeTrunks.clear();
        fallenLogs.clear();

        int trunkCount = 14;
        for (int i = 0; i < trunkCount; i++)
        {
            TreeTrunk tr;
            float r = 18.f + (rand() % 10);
            tr.collisionRadius = r;

            tr.sprite.setTexture(treeTrunkTexture);

            const float baseTexRadius = 64.f;
            float scale = (baseTexRadius > 0.f) ? (r / baseTexRadius) : 1.f;
            tr.sprite.setScale(scale, scale);

            tr.sprite.setOrigin(treeTrunkTexture.getSize().x / 2.f, treeTrunkTexture.getSize().y / 2.f);

            float headRadius = 18.f;
            float minDist = tr.collisionRadius + headRadius + 40.f;

            auto makeTrunkPos = [&]() -> sf::Vector2f
            {
                float x = 80 + rand() % (WINDOW_WIDTH - 160);
                float y = 60 + rand() % (WINDOW_HEIGHT - 120);
                return sf::Vector2f(x, y);
            };

            sf::Vector2f p = rerollPositionFarFromSnake(makeTrunkPos, minDist, 25);
            tr.sprite.setPosition(p.x, p.y);
            treeTrunks.push_back(tr);
        }

        int logCount = 9;
        for (int i = 0; i < logCount; i++)
        {
            FallenLog lg;
            float w = 140.f + (rand() % 80);
            float h = 18.f + (rand() % 10);

            float rot = (rand() % 180) - 90;

            lg.sprite.setTexture(logTexture);

            float texW = logTexture.getSize().x > 0 ? (float)logTexture.getSize().x : 1.f;
            float texH = logTexture.getSize().y > 0 ? (float)logTexture.getSize().y : 1.f;
            lg.sprite.setScale(w / texW, h / texH);

            lg.sprite.setOrigin(logTexture.getSize().x / 2.f, logTexture.getSize().y / 2.f);

            float headRadius = 18.f;
            float logApproxRadius = std::max(w, h) * 0.6f;
            float minDist = logApproxRadius + headRadius + 40.f;

            auto makeLogPos = [&]() -> sf::Vector2f
            {
                float rx = 80 + rand() % (WINDOW_WIDTH - 160);
                float ry = 60 + rand() % (WINDOW_HEIGHT - 120);
                return sf::Vector2f(rx, ry);
            };

            sf::Vector2f p = rerollPositionFarFromSnake(makeLogPos, minDist, 25);
            lg.sprite.setPosition(p.x, p.y);
            lg.sprite.setRotation(rot);
            fallenLogs.push_back(lg);
        }

        obstacleClock.restart();
    };

    auto resetDesertObstacles = [&]()
    {
        sandTornadoes.clear();
        cactusSpikes.clear();

        int tornadoCount = 4;
        for (int i = 0; i < tornadoCount; i++)
        {
            SandTornado st;
            float r = 10.f + (rand() % 8);
            (void)r;
            st.sprite.setTexture(tornadoTexture);

            st.sprite.setScale(0.3f, 0.3f);
            st.sprite.setOrigin(tornadoTexture.getSize().x / 2.f, tornadoTexture.getSize().y / 2.f);

            st.orbitRadius = 70.f + rand() % 120;

            float headRadius = 18.f;
            float minDist = (st.orbitRadius * 1.05f) + headRadius + 40.f;

            auto makeTornadoCenterPos = [&]() -> sf::Vector2f
            {
                float x = 220 + rand() % (WINDOW_WIDTH - 440);
                float y = 120 + rand() % (WINDOW_HEIGHT - 240);
                return sf::Vector2f(x, y);
            };

            st.center = rerollPositionFarFromSnake(makeTornadoCenterPos, minDist, 25);

            st.orbitSpeed = 0.6f + (rand() % 40) / 50.f;
            st.period = 2.2f + (rand() % 70) / 20.f;
            st.activeTime = 0.6f + (rand() % 25) / 40.f;
            st.phase = (rand() % 100) / 30.f;
            st.angle = (rand() % 628) / 100.f;

            st.update(0.f, 0.f);
            sandTornadoes.push_back(st);
        }

        int spikeCount = 34;
        for (int i = 0; i < spikeCount; i++)
        {
            CactusSpike cs;
            float rr = 6.f + (rand() % 6);
            cs.collisionRadius = rr;

            cs.sprite.setTexture(cactusTexture);

            const float baseTexRadius = 32.f;
            float scale = (baseTexRadius > 0.f) ? (rr / baseTexRadius) : 1.f;
            cs.sprite.setScale(scale, scale);

            cs.sprite.setOrigin(cactusTexture.getSize().x / 2.f, cactusTexture.getSize().y / 2.f);

            // keep cactus away from snake spawn so you don't instant-die
            float headRadius = 18.f;
            float minDist = cs.collisionRadius + headRadius + 40.f;

            auto makeSpikePos = [&]() -> sf::Vector2f
            {
                float x = 60 + rand() % (WINDOW_WIDTH - 120);
                float y = 50 + rand() % (WINDOW_HEIGHT - 100);
                return sf::Vector2f(x, y);
            };

            sf::Vector2f p = rerollPositionFarFromSnake(makeSpikePos, minDist, 25);
            cs.sprite.setPosition(p.x, p.y);

            cactusSpikes.push_back(cs);
        }

        obstacleClock.restart();
    };

    // default obstacles
    resetObstacles();

    sf::View camera(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

    sf::Texture bgTexture;
    loadTextureFile(bgTexture, "assets/LAVA.png");
    sf::Sprite background(bgTexture);

    auto setBackgroundScaleToWindow = [&]()
    {
        sf::Vector2u texSize = bgTexture.getSize();
        if (texSize.x == 0 || texSize.y == 0)
            return;
        background.setScale(1.f, 1.f);
        float scaleX = (float)WINDOW_WIDTH / texSize.x;
        float scaleY = (float)WINDOW_HEIGHT / texSize.y;
        background.setScale(scaleX, scaleY);
        background.setPosition(0.f, 0.f);
    };
    setBackgroundScaleToWindow();

    sf::Font font;
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    sf::Music loadingMusic, gameMusic;
    bool musicMuted = false;
    loadingMusic.openFromFile("assets/loadingbgm1.mp3");
    gameMusic.openFromFile("assets/ingame.mp3");
    loadingMusic.setLoop(true);
    gameMusic.setLoop(true);
    loadingMusic.play();

    

    Button mapsBtn({200, 45}, {540, 400}, font, "MAPS");
    Button settingsBtn({200, 45}, {540, 280}, font, "SETTINGS");
    Button skinsBtn({200, 45}, {540, 340}, font, "SKINS");
    Button exitBtn({200, 45}, {545, 470}, font, "EXIT");

    Button musicOnBtn({250, 60}, {515, 250}, font, "MUSIC ON");
    Button musicOffBtn({250, 60}, {515, 330}, font, "MUSIC OFF");
    Button backSoundBtn({250, 60}, {515, 410}, font, "BACK");

    Button caterpillarBtn({260, 50}, {510, 220}, font, "CATERPILLAR");
    Button sunflowerBtn({260, 50}, {510, 290}, font, "SUNFLOWER");
    Button warriorBtn({260, 50}, {510, 360}, font, "WARRIOR");
    Button amethystBtn({260, 50}, {510, 430}, font, "AMETHYST");
    Button bloomBtn({260, 50}, {510, 500}, font, "BLOOM");
    Button diamondBtn({260, 50}, {510, 570}, font, "DIAMOND");

    Button backSkinBtn({200, 45}, {540, 640}, font, "BACK");

    // Back buttons (use the same Button style for consistency)
    Button mapsBackBtn({220, 45}, {30.f, 60.f}, font, "BACK");
    Button gameOverBackBtn({220, 60}, {30.f, 120.f}, font, "BACK");

    // Locked skin notification

    sf::Text unlockNoticeText;
    unlockNoticeText.setFont(font);
    unlockNoticeText.setCharacterSize(28);
    unlockNoticeText.setFillColor(sf::Color(255, 80, 80));
    unlockNoticeText.setPosition(160.f, 610.f);
    sf::Clock unlockNoticeClock;
    bool unlockNoticeVisible = false;

    auto setUnlockNotice = [&](const std::string &msg)
    {
        unlockNoticeText.setString(msg);
        unlockNoticeClock.restart();
        unlockNoticeVisible = true;
    };

    Button resumeBtn({250, 60}, {515, 250}, font, "RESUME");
    Button menuBtn({250, 60}, {515, 330}, font, "MENU");
    Button exitPauseBtn({250, 60}, {515, 410}, font, "EXIT");

    Button restartBtn({250, 60}, {515, 350}, font, "RESTART");
    Button gameOverMenuBtn({250, 60}, {515, 430}, font, "MAIN MENU");

    Button startBtn({220, 45}, {530, 500}, font, "START");

    sf::FloatRect settingsBackBounds(WINDOW_WIDTH / 2 - 110, 475, 220, 45);

    Button forestMapBtn({240, 50}, {520, 240}, font, "FOREST");
    Button desertMapBtn({240, 50}, {520, 320}, font, "DESERT");
    Button lavaMapBtn({240, 50}, {520, 400}, font, "LAVA");

    // GAME OVER title (top-center, above all menu buttons)
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(70);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("GAME OVER");
    sf::FloatRect g = gameOverText.getLocalBounds();
    gameOverText.setOrigin(g.width / 2, g.height / 2);
    gameOverText.setPosition(WINDOW_WIDTH / 2, 220.f);

    auto saveUnlocks = [&]()
    {
        std::ofstream skinSaveFile("assets/save.txt");
        if (!skinSaveFile.is_open())
            return;
        skinSaveFile << sunflowerUnlocked << ' '
                     << warriorUnlocked << ' '
                     << amethystUnlocked << ' '
                     << bloomUnlocked << ' '
                     << diamondUnlocked;
    };

    GameState prevState = gameState;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            {
                if (gameState == PLAYING)
                    gameState = PAUSED;
                else if (gameState == PAUSED)
                    gameState = PLAYING;
            }

            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (gameState == MENU)
                {
                    if (settingsBtn.isClicked(mouse))
                    {
                        settingsBtn.triggerClick();
                        gameState = SETTINGS;
                    }
                    else if (skinsBtn.isClicked(mouse))
                    {
                        skinsBtn.triggerClick();
                        gameState = SKINS;
                    }
                    else if (mapsBtn.isClicked(mouse))
                    {
                        mapsBtn.triggerClick();
                        gameState = MAPS;
                    }
                    else if (exitBtn.isClicked(mouse))
                    {
                        exitBtn.triggerClick();
                        window.close();
                    }
                }
                else if (gameState == SKINS)
                {
                    if (caterpillarBtn.isClicked(mouse))
                    {
                        selectedHeadSkin = "Snake_SKINS/caterpillar_head.png";
                        selectedBodySkin = "Snake_SKINS/caterpillar_body.png";
                        selectedTailSkin = "Snake_SKINS/caterpillar_tail.png";
                        caterpillarBtn.triggerClick();
                    }

                    if (sunflowerBtn.isClicked(mouse))
                    {
                        if (sunflowerUnlocked)
                        {
                            sunflowerBtn.triggerClick();
                            selectedHeadSkin = "Snake_SKINS/sunflower_head.png";
                            selectedBodySkin = "Snake_SKINS/sunflower_body.png";
                            selectedTailSkin = "Snake_SKINS/sunflower_tail.png";
                        }
                        else
                        {
                            int need = 100 - player.score;
                            if (need < 0)
                                need = 0;
                            setUnlockNotice("Locked! Eat " + to_string(need) + " more points to unlock.");
                        }
                    }

                    if (warriorBtn.isClicked(mouse))
                    {
                        if (warriorUnlocked)
                        {
                            warriorBtn.triggerClick();
                            selectedHeadSkin = "Snake_SKINS/warrior_head.png";
                            selectedBodySkin = "Snake_SKINS/warrior_body.png";
                            selectedTailSkin = "Snake_SKINS/warrior_tail.png";
                        }
                        else
                        {
                            int need = 200 - player.score;
                            if (need < 0)
                                need = 0;
                            setUnlockNotice("Locked! Eat " + to_string(need) + " more points to unlock.");
                        }
                    }

                    if (amethystBtn.isClicked(mouse))
                    {
                        if (amethystUnlocked)
                        {
                            amethystBtn.triggerClick();
                            selectedHeadSkin = "Snake_SKINS/amethyst_head.png";
                            selectedBodySkin = "Snake_SKINS/amethyst_body.png";
                            selectedTailSkin = "Snake_SKINS/amethyst_tail.png";
                        }
                        else
                        {
                            int need = 300 - player.score;
                            if (need < 0)
                                need = 0;
                            setUnlockNotice("Locked! Eat " + to_string(need) + " more points to unlock.");
                        }
                    }

                    if (bloomBtn.isClicked(mouse))
                    {
                        if (bloomUnlocked)
                        {
                            bloomBtn.triggerClick();
                            selectedHeadSkin = "Snake_SKINS/bloom_head.png";
                            selectedBodySkin = "Snake_SKINS/bloom_body.png";
                            selectedTailSkin = "Snake_SKINS/bloom_tail.png";
                        }
                        else
                        {
                            int need = 400 - player.score;
                            if (need < 0)
                                need = 0;
                            setUnlockNotice("Locked! Eat " + to_string(need) + " more points to unlock.");
                        }
                    }

                    if (diamondBtn.isClicked(mouse))
                    {
                        if (diamondUnlocked)
                        {
                            diamondBtn.triggerClick();
                            selectedHeadSkin = "Snake_SKINS/diamond_head.png";
                            selectedBodySkin = "Snake_SKINS/diamond_body.png";
                            selectedTailSkin = "Snake_SKINS/diamond_tail.png";
                        }
                        else
                        {
                            int need = 500 - player.score;
                            if (need < 0)
                                need = 0;
                            setUnlockNotice("Locked! Eat " + to_string(need) + " more points to unlock.");
                        }
                    }

                    if (backSkinBtn.isClicked(mouse))
                    {
                        backSkinBtn.triggerClick();
                        gameState = MENU;
                    }
                }

                else if (gameState == MAPS)
                {
                    if (mapsBackBtn.isClicked(mouse))
                    {
                        mapsBackBtn.triggerClick();
                        gameState = MENU;
                        mapChosen = false;
                    }
                    else if (forestMapBtn.isClicked(mouse))

                    {
                        forestMapBtn.triggerClick();
                        selectedMap = FOREST;
                        mapChosen = true;
                    }
                    else if (desertMapBtn.isClicked(mouse))
                    {
                        desertMapBtn.triggerClick();
                        selectedMap = DESERT;
                        mapChosen = true;
                    }
                    else if (lavaMapBtn.isClicked(mouse))
                    {
                        lavaMapBtn.triggerClick();
                        selectedMap = LAVA;
                        mapChosen = true;
                    }
                    else if (startBtn.isClicked(mouse) && mapChosen)
                    {
                        startBtn.triggerClick();
                        

                        player.body.clear();
                        player.body.push_back({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});
                        player.score = 0;
                        player.velocity = {1.f, 0.f};

                        foods.clear();
                        foodsEatenSinceLastSpawn = 0;
                        foodRespawnClock.restart();
                        for (int i = 0; i < MAX_FOODS; i++)
                            foods.push_back(Food(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, rollFoodType()));

                        if (selectedMap == FOREST)
                            loadTextureFile(bgTexture, "assets/FOREST.png");
                        else if (selectedMap == DESERT)
                            loadTextureFile(bgTexture, "assets/DESERT.png");
                        else
                            loadTextureFile(bgTexture, "assets/LAVA.png");

                        background.setTexture(bgTexture);
                        setBackgroundScaleToWindow();

                        player.loadSkin(selectedHeadSkin, selectedBodySkin);

                        clearObstacles();
                        if (selectedMap == LAVA)
                            resetObstacles();
                        else if (selectedMap == FOREST)
                            resetForestObstacles();
                        else
                            resetDesertObstacles();

                        justStartedPlaying = true;
                        collisionGraceClock.restart();
                        gameState = PLAYING;
                    }
                }
                else if (gameState == SETTINGS)
                {
                    if (musicOnBtn.isClicked(mouse))
                    {
                        musicOnBtn.triggerClick();
                        musicMuted = false;
                        loadingMusic.setVolume(100);
                        gameMusic.setVolume(100);
                    }
                    else if (musicOffBtn.isClicked(mouse))
                    {
                        musicOffBtn.triggerClick();
                        musicMuted = true;
                        loadingMusic.setVolume(0);
                        gameMusic.setVolume(0);
                    }
                    else if (backSoundBtn.isClicked(mouse))
                    {
                        backSoundBtn.triggerClick();
                        gameState = MENU;
                    }
                }
                else if (gameState == GAME_OVER)
                {
                    // Back arrow (GAME_OVER) -> MENU
                    if (gameOverBackBtn.isClicked(mouse))
                    {
                        gameOverBackBtn.triggerClick();
                        gameState = MENU;
                    }

                    else if (restartBtn.isClicked(mouse))
                    {
                        restartBtn.triggerClick();
                       

                        player.body.clear();
                        player.body.push_back({WINDOW_WIDTH / 2.f, WINDOW_HEIGHT / 2.f});
                        player.score = 0;
                        player.velocity = {1.f, 0.f};

                        clearObstacles();
                        if (selectedMap == LAVA)
                            resetObstacles();
                        else if (selectedMap == FOREST)
                            resetForestObstacles();
                        else
                            resetDesertObstacles();

                        justStartedPlaying = true;
                        collisionGraceClock.restart();
                        gameState = PLAYING;
                    }
                    else if (gameOverMenuBtn.isClicked(mouse))
                    {
                        gameOverMenuBtn.triggerClick();
                        gameState = MENU;
                    }
                }

                else if (gameState == PAUSED)
                {
                    if (resumeBtn.isClicked(mouse))
                    {
                        resumeBtn.triggerClick();
                        gameState = PLAYING;
                    }
                    else if (menuBtn.isClicked(mouse))
                    {
                        menuBtn.triggerClick();
                        gameState = MENU;
                    }
                    else if (exitPauseBtn.isClicked(mouse))
                    {
                        exitPauseBtn.triggerClick();
                        window.close();
                    }
                }
            }
        }

        if (prevState == PLAYING && gameState == GAME_OVER)
            
        prevState = gameState;

        if (gameState == LOADING)
        {
            float t = loadingClock.getElapsedTime().asSeconds();

            if (t < 1)
                alpha = (t / 1.f) * 255;
            else if (t < 9)
                alpha = 255;
            else
                alpha = 255 - ((t - 4) / 1.f) * 255;

            logoSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));

            float progress = min(t / loadingTime, 1.f);
            loadingBar.setSize({400 * progress, 20});

            if (t >= loadingTime)
            {
                gameState = MENU;
                loadingMusic.stop();
                if (!musicMuted)
                    gameMusic.play();
            }
        }

        if (gameState == PLAYING)
        {
            if (justStartedPlaying)
            {
                justStartedPlaying = false;
                camera.reset(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
                window.setView(camera);
            }
            else
            {
                sf::Vector2f dir(0, 0);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                    dir.y--;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                    dir.y++;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                    dir.x--;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                    dir.x++;

                if (dir.x != 0 || dir.y != 0)
                {
                    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
                    dir /= len;
                }

                bool alive = player.move(dir, sf::Keyboard::isKeyPressed(sf::Keyboard::Space));
                if (!alive)
                    gameState = GAME_OVER;

                float t = obstacleClock.getElapsedTime().asSeconds();
                const bool obstacleCollisionGrace = (collisionGraceClock.getElapsedTime().asSeconds() < collisionGraceSec);

                if (gameState == PLAYING && selectedMap == FOREST)
                {
                    sf::Vector2f headPos = player.body[0];
                    float headRadius = 18.f;

                    if (!obstacleCollisionGrace)
                    {
                        for (auto &tr : treeTrunks)
                        {
                            float d = distance(headPos, tr.sprite.getPosition());
                            if (d <= tr.collisionRadius + headRadius)
                            {
                                gameState = GAME_OVER;
                                break;
                            }
                        }
                    }

                    for (auto &lg : fallenLogs)
                    {
                        if (gameState != PLAYING)
                            break;
                        if (lg.sprite.getGlobalBounds().contains(headPos))
                        {
                            gameState = GAME_OVER;
                            break;
                        }
                    }
                }

                if (gameState == PLAYING && selectedMap == DESERT)
                {
                    sf::Vector2f headPos = player.body[0];
                    float headRadius = 18.f;
                    float dt = 1.f / 60.f;

                    for (auto &st : sandTornadoes)
                    {
                        st.update(t, dt);
                        if (!st.isActive(t))
                            continue;

                        if (!obstacleCollisionGrace)
                        {
                            float d = distance(headPos, st.sprite.getPosition());
                            if (d <= st.radius + headRadius)
                            {
                                gameState = GAME_OVER;
                                break;
                            }
                        }
                    }

                    for (auto &cs : cactusSpikes)
                    {
                        if (gameState != PLAYING)
                            break;
                        if (!obstacleCollisionGrace)
                        {
                            float d = distance(headPos, cs.sprite.getPosition());
                            if (d <= cs.collisionRadius + headRadius)
                            {
                                gameState = GAME_OVER;
                                break;
                            }
                        }
                    }
                }

                if (gameState == PLAYING && selectedMap == LAVA)
                {
                    float headRadius = 18.f;

                    for (auto &fb : fireballs)
                    {
                        fb.sprite.move(fb.velocity * fb.speed / 60.f);

                        auto pos = fb.sprite.getPosition();
                        if (pos.x < -40 || pos.x > WINDOW_WIDTH + 40 || pos.y < -40 || pos.y > WINDOW_HEIGHT + 40)
                        {
                            float minDist = fb.collisionRadius + headRadius + 40.f;
                            auto makeFireRespawn = [&]() -> sf::Vector2f
                            {
                                return sf::Vector2f(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT);
                            };

                            sf::Vector2f p = rerollPositionFarFromSnake(makeFireRespawn, minDist, 25);
                            fb.sprite.setPosition(p.x, p.y);

                            sf::Vector2f toC(WINDOW_WIDTH / 2.f - p.x, WINDOW_HEIGHT / 2.f - p.y);
                            float len = sqrt(toC.x * toC.x + toC.y * toC.y);
                            if (len > 0)
                                toC /= len;
                            fb.velocity = toC;
                        }
                    }

                    sf::Vector2f headPos = player.body[0];

                    if (!obstacleCollisionGrace)
                    {
                        for (auto &fb : fireballs)
                        {
                            float d = distance(headPos, fb.sprite.getPosition());
                            if (d <= fb.collisionRadius + headRadius)
                            {
                                gameState = GAME_OVER;
                                break;
                            }
                        }

                        for (auto &rr : moltenRocks)
                        {
                            if (gameState != PLAYING)
                                break;
                            float d = distance(headPos, rr.sprite.getPosition());
                            if (d <= rr.collisionRadius + headRadius)
                            {
                                gameState = GAME_OVER;
                                break;
                            }
                        }
                    }
                }

                for (int i = 0; i < (int)foods.size(); i++)
                {
                    sf::Vector2f foodCenter = foods[i].shape.getPosition() + sf::Vector2f(foods[i].shape.getRadius(), foods[i].shape.getRadius());

                    if (distance(player.body[0], foodCenter) < 20)
                    {
                        int points = 1;
                        switch (foods[i].type)
                        {
                        case FoodType::Normal:
                            points = 1;
                            break;
                        case FoodType::Speed:
                            points = 2;
                            break;
                        case FoodType::Big:
                            points = 4;
                            break;
                        case FoodType::Mega:
                            points = 8;
                            break;
                        case FoodType::Ultra:
                            points = 15;
                            break;
                        }

                        player.score += points;

                        bool unlockedChanged = false;

                        if (player.score >= 100 && !sunflowerUnlocked)
                        {
                            sunflowerUnlocked = true;
                            unlockedChanged = true;
                        }
                        if (player.score >= 200 && !warriorUnlocked)
                        {
                            warriorUnlocked = true;
                            unlockedChanged = true;
                        }
                        if (player.score >= 300 && !amethystUnlocked)
                        {
                            amethystUnlocked = true;
                            unlockedChanged = true;
                        }
                        if (player.score >= 400 && !bloomUnlocked)
                        {
                            bloomUnlocked = true;
                            unlockedChanged = true;
                        }
                        if (player.score >= 500 && !diamondUnlocked)
                        {
                            diamondUnlocked = true;
                            unlockedChanged = true;
                        }

                        if (unlockedChanged)
                            saveUnlocks();

                        if (player.score > highScore)
                        {
                            highScore = player.score;
                            ofstream saveFile("assets/highscore.txt");
                            if (saveFile.is_open())
                                saveFile << highScore;
                        }

                        foods.erase(foods.begin() + i);
                        foodsEatenSinceLastSpawn++;
                        break;
                    }
                }

                if (foodRespawnClock.getElapsedTime().asSeconds() >= foodRefillIntervalSec)
                {
                    trySpawnFood();
                    foodRespawnClock.restart();
                    if (foodsEatenSinceLastSpawn > 0)
                        foodsEatenSinceLastSpawn /= 2;
                }

                camera.reset(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
                window.setView(camera);
            }
        }

        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (gameState == MENU)
        {
            window.setView(window.getDefaultView());
            loadTextureFile(bgTexture, "assets/LAVA.png");
            background.setTexture(bgTexture);
            setBackgroundScaleToWindow();
            window.draw(background);

            settingsBtn.setHover(settingsBtn.isClicked(mouse));
            skinsBtn.setHover(skinsBtn.isClicked(mouse));
            mapsBtn.setHover(mapsBtn.isClicked(mouse));
            exitBtn.setHover(exitBtn.isClicked(mouse));

            settingsBtn.update();
            skinsBtn.update();
            mapsBtn.update();
            exitBtn.update();

            settingsBtn.draw(window);
            skinsBtn.draw(window);
            mapsBtn.draw(window);
            exitBtn.draw(window);
        }
        else if (gameState == LOADING)
        {
            window.setView(window.getDefaultView());
            window.draw(logoSprite);
            window.draw(loadingBarBG);
            window.draw(loadingBar);
        }
        else if (gameState == PLAYING)
        {
            window.setView(window.getDefaultView());
            window.draw(background);
            window.setView(camera);

            if (selectedMap == LAVA)
            {
                for (auto &fb : fireballs)
                    window.draw(fb.sprite);
                for (auto &rr : moltenRocks)
                    window.draw(rr.sprite);
            }
            else if (selectedMap == FOREST)
            {
                for (auto &tr : treeTrunks)
                    window.draw(tr.sprite);
                for (auto &lg : fallenLogs)
                    window.draw(lg.sprite);
            }
            else
            {
                for (auto &st : sandTornadoes)
                    window.draw(st.sprite);
                for (auto &cs : cactusSpikes)
                    window.draw(cs.sprite);
            }

            for (auto &f : foods)
                window.draw(f.shape);
            player.draw(window);

            window.setView(window.getDefaultView());

            sf::RectangleShape hudBG;
            hudBG.setSize({270, 140});
            hudBG.setPosition(10, 10);
            hudBG.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(hudBG);

            sf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setCharacterSize(28);
            scoreText.setFillColor(sf::Color::Cyan);
            scoreText.setString("Score: " + to_string(player.score));
            scoreText.setPosition(20, 60);
            window.draw(scoreText);

            sf::Text highScoreText;
            highScoreText.setFont(font);
            highScoreText.setCharacterSize(28);
            highScoreText.setFillColor(sf::Color::Yellow);
            highScoreText.setString("High Score: " + to_string(highScore));
            highScoreText.setPosition(20, 100);
            window.draw(highScoreText);
        }
        else if (gameState == GAME_OVER)
        {
            window.setView(window.getDefaultView());
            window.draw(background);
            window.draw(gameOverText);
            restartBtn.draw(window);
            gameOverMenuBtn.draw(window);
        }

        else if (gameState == PAUSED)
        {
            window.setView(window.getDefaultView());
            window.draw(background);
            resumeBtn.draw(window);
            menuBtn.draw(window);
            exitPauseBtn.draw(window);
        }
        else if (gameState == MAPS)
        {
            window.setView(window.getDefaultView());
            if (selectedMap == FOREST)
                loadTextureFile(bgTexture, "assets/FOREST.png");
            else if (selectedMap == DESERT)
                loadTextureFile(bgTexture, "assets/DESERT.png");
            else
                loadTextureFile(bgTexture, "assets/LAVA.png");

            background.setTexture(bgTexture);
            setBackgroundScaleToWindow();
            window.draw(background);

            forestMapBtn.setHover(forestMapBtn.isClicked(mouse));
            desertMapBtn.setHover(desertMapBtn.isClicked(mouse));
            lavaMapBtn.setHover(lavaMapBtn.isClicked(mouse));

            forestMapBtn.update();
            desertMapBtn.update();
            lavaMapBtn.update();

            forestMapBtn.draw(window);
            desertMapBtn.draw(window);
            lavaMapBtn.draw(window);

            startBtn.setHover(startBtn.isClicked(mouse));
            startBtn.update();
            startBtn.draw(window);

            sf::Text settingsText;
            settingsText.setFont(font);
            settingsText.setCharacterSize(60);
            settingsText.setFillColor(sf::Color::White);
            settingsText.setString("CHOOSE MAPS");
            settingsText.setOrigin(settingsText.getLocalBounds().width / 2, settingsText.getLocalBounds().height / 2);
            settingsText.setPosition(WINDOW_WIDTH / 2, 150);
            window.draw(settingsText);

            mapsBackBtn.setHover(mapsBackBtn.isClicked(mouse));
            mapsBackBtn.update();
            mapsBackBtn.draw(window);

            // Ensure back button is in upper-left of the MAPS screen
            // (mapsBackBtn is positioned at {30.f, 60.f} in its constructor).
        }
        else if (gameState == SKINS)
        {
            window.setView(window.getDefaultView());
            window.draw(background);

            caterpillarBtn.setHover(caterpillarBtn.isClicked(mouse));
            sunflowerBtn.setHover(sunflowerBtn.isClicked(mouse));
            warriorBtn.setHover(warriorBtn.isClicked(mouse));
            amethystBtn.setHover(amethystBtn.isClicked(mouse));
            bloomBtn.setHover(bloomBtn.isClicked(mouse));
            diamondBtn.setHover(diamondBtn.isClicked(mouse));
            backSkinBtn.setHover(backSkinBtn.isClicked(mouse));

            caterpillarBtn.update();
            sunflowerBtn.update();
            warriorBtn.update();
            amethystBtn.update();
            bloomBtn.update();
            diamondBtn.update();
            backSkinBtn.update();

            // highlight equipped skin (blue)
            auto setEquippedColor = [&](Button &btn, const std::string &head, const std::string &body)
            {
                bool equipped = (selectedHeadSkin == head && selectedBodySkin == body);
                if (equipped)
                {
                    // Equipped highlight should always be visible (even when hovered)
                    btn.shape.setFillColor(sf::Color(60, 140, 255));
                }
                else if (btn.hovered)
                {
                    btn.shape.setFillColor(sf::Color(100, 100, 100));
                }
                else
                {
                    btn.shape.setFillColor(sf::Color(60, 60, 60));
                }
            };

            setEquippedColor(caterpillarBtn, "Snake_SKINS/caterpillar_head.png", "Snake_SKINS/caterpillar_body.png");
            setEquippedColor(sunflowerBtn, "Snake_SKINS/sunflower_head.png", "Snake_SKINS/sunflower_body.png");
            setEquippedColor(warriorBtn, "Snake_SKINS/warrior_head.png", "Snake_SKINS/warrior_body.png");
            setEquippedColor(amethystBtn, "Snake_SKINS/amethyst_head.png", "Snake_SKINS/amethyst_body.png");
            setEquippedColor(bloomBtn, "Snake_SKINS/bloom_head.png", "Snake_SKINS/bloom_body.png");
            setEquippedColor(diamondBtn, "Snake_SKINS/diamond_head.png", "Snake_SKINS/diamond_body.png");

            caterpillarBtn.draw(window);
            sunflowerBtn.draw(window);
            warriorBtn.draw(window);
            amethystBtn.draw(window);
            bloomBtn.draw(window);
            diamondBtn.draw(window);
            backSkinBtn.draw(window);

            if (unlockNoticeVisible)

            {
                float elapsed = unlockNoticeClock.getElapsedTime().asSeconds();
                if (elapsed > 2.0f)
                {
                    unlockNoticeVisible = false;
                }
                else
                {
                    sf::Color c = unlockNoticeText.getFillColor();
                    float a01 = 1.f - (elapsed / 2.0f);
                    if (a01 < 0.f)
                        a01 = 0.f;
                    unlockNoticeText.setFillColor(sf::Color(c.r, c.g, c.b, (sf::Uint8)(255.f * a01)));
                    window.draw(unlockNoticeText);
                }
            }
        }

        else if (gameState == SETTINGS)
        {
            window.setView(window.getDefaultView());
            window.draw(background);
            musicOnBtn.setHover(musicOnBtn.isClicked(mouse));
            musicOffBtn.setHover(musicOffBtn.isClicked(mouse));
            backSoundBtn.setHover(backSoundBtn.isClicked(mouse));
            musicOnBtn.update();
            musicOffBtn.update();
            backSoundBtn.update();
            musicOnBtn.draw(window);
            musicOffBtn.draw(window);
            backSoundBtn.draw(window);

            sf::Text title;
            title.setFont(font);
            title.setCharacterSize(60);
            title.setFillColor(sf::Color::White);
            title.setString("SOUND SETTINGS");
            title.setPosition(350, 100);
            window.draw(title);
        }

        window.display();
    }

    return 0;
}
