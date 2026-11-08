#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <SFML/Audio.hpp>
#include <fstream>

using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// =========================
// GAME STATES
// =========================
enum GameState
{
    LOADING,
    MENU,
    PLAYING,
    PAUSED,
    SETTINGS,
    GAME_OVER
};

float distance(sf::Vector2f a, sf::Vector2f b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) +
                (a.y - b.y) * (a.y - b.y));
}

// =========================
// FOOD
// =========================
struct Food
{
    sf::CircleShape shape;

    bool poison = false;

    Food(float x, float y, bool isPoison = false)
    {
        poison = isPoison;

        shape.setRadius(5.f);

        // NORMAL FOOD
        if (!poison)
            shape.setFillColor(sf::Color::Green);

        // POISON FOOD
        else
            shape.setFillColor(sf::Color::Red);

        shape.setPosition(x, y);
    }
};

// =========================
// SNAKE
// =========================
class Snake
{
public:
    sf::Texture headTex;
    sf::Texture bodyTex;
    sf::Texture tailTex;

    sf::Sprite headSprite;
    sf::Sprite bodySprite;
    sf::Sprite tailSprite;

    vector<sf::Vector2f> body;

    float speed = 2.8f;

    sf::Vector2f velocity = {1.f, 0.f};

    float turnSpeed = 0.12f;

    int score = 0;

    Snake(float x, float y)
    {
        headTex.loadFromFile("snake_head.png");
        bodyTex.loadFromFile("snake_body.png");
        tailTex.loadFromFile("snake_tail.png");

        headSprite.setTexture(headTex);
        bodySprite.setTexture(bodyTex);
        tailSprite.setTexture(tailTex);

        // Resize snake
        headSprite.setScale(0.15f, 0.15f);
        bodySprite.setScale(0.15f, 0.15f);
        tailSprite.setScale(0.15f, 0.15f);

        // Center rotation
        headSprite.setOrigin(
            headTex.getSize().x / 2,
            headTex.getSize().y / 2);

        bodySprite.setOrigin(
            bodyTex.getSize().x / 2,
            bodyTex.getSize().y / 2);

        tailSprite.setOrigin(
            tailTex.getSize().x / 2,
            tailTex.getSize().y / 2);

        body.push_back({x, y});
    }

    bool move(sf::Vector2f dir, bool boost)
    {
        float sizePenalty = score * 0.02f;

        float finalSpeed = turnSpeed - sizePenalty;

        if (finalSpeed < 1.5f)
            finalSpeed = 1.5f;

        float targetSpeed = boost ? speed * 1.8f : speed;
        (void)targetSpeed; // (kept original logic area; timer system removed)

        // Smooth turning
        if (dir.x != 0 || dir.y != 0)
        {
            velocity += (dir - velocity) * turnSpeed;

            float len = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            if (len > 0)
                velocity /= len;
        }

        sf::Vector2f newHead = body[0] + velocity * finalSpeed;

        // Wall collision = death
        if (newHead.x < 0 ||
            newHead.y < 0 ||
            newHead.x > WINDOW_WIDTH ||
            newHead.y > WINDOW_HEIGHT)
        {
            return false; // snake dies
        }

        body.insert(body.begin(), newHead);

        // Smooth follow
        for (int i = 1; i < (int)body.size(); i++)
        {
            body[i] += (body[i - 1] - body[i]) * 0.35f;
        }

        while (body.size() > (size_t)(20 + score))
        {
            body.pop_back();
        }

        return true;
    }

    void draw(sf::RenderWindow &window)
    {
        if (body.size() < 2)
            return;

        // TAIL
        int tailIndex = (int)body.size() - 1;

        sf::Vector2f tailDir = body[tailIndex - 1] - body[tailIndex];

        float tailAngle = atan2(tailDir.y, tailDir.x) * 180 / 3.14159f;

        tailSprite.setPosition(body[tailIndex]);
        tailSprite.setRotation(tailAngle);
        window.draw(tailSprite);

        // BODY
        for (int i = 1; i < (int)body.size() - 1; i++)
        {
            bodySprite.setPosition(body[i]);
            window.draw(bodySprite);
        }

        // HEAD
        sf::Vector2f headDir = body[0] - body[1];

        float headAngle = atan2(headDir.y, headDir.x) * 180 / 3.14159f;

        // Fix upside down
        if (headAngle > 90)
            headAngle = 90;
        if (headAngle < -90)
            headAngle = -90;

        headSprite.setPosition(body[0]);
        headSprite.setRotation(headAngle);
        window.draw(headSprite);
    }
};

struct VolcanoParticle
{
    sf::CircleShape shape;
    float speed;
    float drift;
};

// =========================
// LAVA OBSTACLES
// =========================
struct MoltenVent
{
    sf::CircleShape shape;
    float period = 2.f;      // seconds between bursts
    float burstTime = 0.35f; // how long it stays dangerous
    float phase = 0.f;       // start offset

    // returns true if vent is currently dangerous (molten)
    bool isHot(float t) const
    {
        float local = fmod(t + phase, period);
        return local <= burstTime;
    }
};

struct Fireball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float speed = 220.f; // pixels/sec
};


// =========================
// BUTTON (ANIMATED)
// =========================
struct Button
{
    sf::RectangleShape shape;
    sf::Text text;

    sf::Vector2f baseSize;
    sf::Vector2f basePos;

    bool hovered = false;
    float anim = 0.f;

    Button() {}

    Button(sf::Vector2f size, sf::Vector2f pos, sf::Font &font, string label)
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

        float scale = 1.f + anim * 0.08f;

        shape.setSize(baseSize * scale);
        shape.setPosition(
            basePos.x - (baseSize.x * (scale - 1.f) / 2),
            basePos.y - (baseSize.y * (scale - 1.f) / 2));
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
        shape.setFillColor(h ? sf::Color(100, 100, 100)
                              : sf::Color(60, 60, 60));
    }
};

// =========================
// MAIN
// =========================
int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Sneakers.io");
    window.setFramerateLimit(60);

    GameState gameState = LOADING;

    int highScore = 0;

    // LOAD HIGH SCORE
    ifstream loadFile("highscore.txt");
    if (loadFile.is_open())
    {
        loadFile >> highScore;
    }
    loadFile.close();

    // =========================
    // LOADING SCREEN
    // =========================
    sf::Texture logoTexture;
    sf::Sprite logoSprite;

    sf::RectangleShape loadingBarBG, loadingBar;
    sf::Clock loadingClock;

    float loadingTime = 10.f;
    float alpha = 0;

    logoTexture.loadFromFile("Sneakersloading.jpg");
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

    // =========================
    // VOLCANO SYSTEM INIT
    // =========================
    vector<VolcanoParticle> lavaBubbles;
    vector<VolcanoParticle> ashParticles;

    for (int i = 0; i < 120; i++)
    {
        VolcanoParticle p;

        p.shape.setRadius(rand() % 8 + 3);
        p.shape.setFillColor(sf::Color(255, 80 + rand() % 100, 0, 150));
        p.shape.setPosition(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT);

        p.speed = (rand() % 3 + 2) * 0.6f;
        p.drift = (rand() % 3 - 1) * 0.3f;

        lavaBubbles.push_back(p);
    }

    for (int i = 0; i < 80; i++)
    {
        VolcanoParticle p;

        p.shape.setRadius(rand() % 3 + 1);
        p.shape.setFillColor(sf::Color(120, 120, 120, 120));
        p.shape.setPosition(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT);

        p.speed = (rand() % 2 + 1) * 0.2f;
        p.drift = (rand() % 3 - 1) * 0.1f;

        ashParticles.push_back(p);
    }

    // =========================
    // GAME OBJECTS
    // =========================
    enum MapType { FOREST, DESSERT, LAVA };
    MapType selectedMap = LAVA;

    Snake player(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);

    vector<Food> foods;
    for (int i = 0; i < 100; i++)
        foods.push_back(Food(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT));

    // =========================
    // LAVA OBSTACLES (vents + fireballs)
    // =========================
    vector<MoltenVent> vents;
    vector<Fireball> fireballs;

    // timers for obstacle motion/burst cycle
    sf::Clock obstacleClock;

    auto resetObstacles = [&]()
    {
        vents.clear();
        fireballs.clear();

        // Molten vents
        int ventCount = 6;
        for (int i = 0; i < ventCount; i++)
        {
            MoltenVent v;
            float r = 18.f + (rand() % 10); // radius
            v.shape.setRadius(r);
            v.shape.setOrigin(r, r);
            v.shape.setPosition(100 + rand() % (WINDOW_WIDTH - 200),
                                80 + rand() % (WINDOW_HEIGHT - 160));
            v.shape.setFillColor(sf::Color(255, 120, 0, 140));
            v.period = 1.8f + (rand() % 20) / 10.f; // 1.8 - 3.8
            v.burstTime = 0.25f + (rand() % 15) / 100.f; // 0.25 - 0.40
            v.phase = (rand() % 100) / 50.f; // offset
            vents.push_back(v);
        }

        // Fireballs
        int fireCount = 4;
        for (int i = 0; i < fireCount; i++)
        {
            Fireball fb;
            float r = 9.f + (rand() % 6);
            fb.shape.setRadius(r);
            fb.shape.setOrigin(r, r);
            fb.shape.setFillColor(sf::Color(255, 80, 0));

            // spawn edge
            int edge = rand() % 4;
            float x = 0, y = 0;
            if (edge == 0) { x = 0; y = rand() % WINDOW_HEIGHT; }
            if (edge == 1) { x = WINDOW_WIDTH; y = rand() % WINDOW_HEIGHT; }
            if (edge == 2) { x = rand() % WINDOW_WIDTH; y = 0; }
            if (edge == 3) { x = rand() % WINDOW_WIDTH; y = WINDOW_HEIGHT; }
            fb.shape.setPosition(x, y);

            // direction towards center with some jitter
            sf::Vector2f toC(WINDOW_WIDTH / 2.f - x, WINDOW_HEIGHT / 2.f - y);
            float len = sqrt(toC.x * toC.x + toC.y * toC.y);
            if (len < 0.001f) len = 1.f;
            toC /= len;

            float jitterX = ((rand() % 200) - 100) / 500.f;
            float jitterY = ((rand() % 200) - 100) / 500.f;
            toC += sf::Vector2f(jitterX, jitterY);
            float len2 = sqrt(toC.x * toC.x + toC.y * toC.y);
            if (len2 < 0.001f) len2 = 1.f;
            toC /= len2;

            fb.velocity = toC;
            fireballs.push_back(fb);
        }

        obstacleClock.restart();
    };

    auto clearObstacles = [&]()
    {
        vents.clear();
        fireballs.clear();
    };

    resetObstacles();

    sf::View camera(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

    // =========================
    // TEXTURE + FONT
    // =========================
    sf::Texture bgTexture;
    bgTexture.loadFromFile("LAVA.png");

    sf::Sprite background(bgTexture);

    // Scale background to fill whole game area
    background.setScale(
        (float)WINDOW_WIDTH / bgTexture.getSize().x,
        (float)WINDOW_HEIGHT / bgTexture.getSize().y);


    sf::Font font;
    font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    // =========================
    // MUSIC SYSTEM
    // =========================
    sf::Music loadingMusic;
    sf::Music gameMusic;

    bool musicMuted = false;

    if (!loadingMusic.openFromFile("loadingbgm1.mp3"))
        cout << "Failed to load loading music\n";

    if (!gameMusic.openFromFile("ingame.mp3"))
        cout << "Failed to load ingame music\n";

    loadingMusic.setLoop(true);
    gameMusic.setLoop(true);
    loadingMusic.play();

    // =========================
    // MENU BUTTONS
    // =========================
    Button playBtn({200, 45}, {540, 220}, font, "PLAY");
    Button settingsBtn({200, 45}, {540, 280}, font, "SETTINGS");
    Button skinsBtn({200, 45}, {540, 340}, font, "SKINS");
    Button mapsBtn({200, 45}, {540, 400}, font, "MAPS");
    Button exitBtn({200, 45}, {540, 460}, font, "EXIT");

    // PAUSE BUTTONS
    Button resumeBtn({250, 60}, {515, 250}, font, "RESUME");
    Button menuBtn({250, 60}, {515, 330}, font, "MENU");
    Button exitPauseBtn({250, 60}, {515, 410}, font, "EXIT");

    // GAME OVER BUTTONS
    Button restartBtn({250, 60}, {515, 350}, font, "RESTART");
    Button gameOverMenuBtn({250, 60}, {515, 430}, font, "MAIN MENU");

    // PLAY BUTTON (only enabled in MENU / SETTINGS)
    Button startBtn({200, 45}, {540, 460}, font, "PLAY");


    // SETTINGS back button bounds (for click detection)
    // BACK button should not overlap with LAVA map button (lava button bottom is ~380)
    sf::FloatRect settingsBackBounds(WINDOW_WIDTH / 2 - 175, 440, 350, 70);


    // =========================
    // MAP BUTTONS (inside SETTINGS)
    // =========================
    Button forestMapBtn({260, 55}, {510, 240}, font, "FOREST");
    Button dessertMapBtn({260, 55}, {510, 310}, font, "DESSERT");
    Button lavaMapBtn({260, 55}, {510, 380}, font, "LAVA");


    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(70);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("GAME OVER");

    sf::FloatRect g = gameOverText.getLocalBounds();
    gameOverText.setOrigin(g.width / 2, g.height / 2);
    gameOverText.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 50);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            // ESC PAUSE
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
            {
                if (gameState == PLAYING)
                    gameState = PAUSED;
                else if (gameState == PAUSED)
                    gameState = PLAYING;
            }

            // MOUSE
            if (event.type == sf::Event::MouseButtonPressed)
            {
                sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (gameState == MENU)
                {
                    // PLAY from MENU is disabled; start by choosing a map in SETTINGS.
                    // if (playBtn.isClicked(mouse)) { /* do nothing */ }

                    if (settingsBtn.isClicked(mouse))
                        gameState = SETTINGS;

                    if (skinsBtn.isClicked(mouse))
                        gameState = SETTINGS;

                    if (mapsBtn.isClicked(mouse))
                        gameState = SETTINGS;

                    if (exitBtn.isClicked(mouse))
                        window.close();
                }

                else if (gameState == SETTINGS)
                {
            if (settingsBackBounds.contains(mouse))
                        gameState = MENU;

                    // When user presses PLAY, start the selected map
                    if (startBtn.isClicked(mouse))
                    {
                        // Apply selected map background + obstacle setup
                        if (selectedMap == FOREST)
                            bgTexture.loadFromFile("FOREST.png");
                        else if (selectedMap == DESSERT)
                            bgTexture.loadFromFile("DESERT.png");
                        else
                            bgTexture.loadFromFile("LAVA.png");

                        background.setTexture(bgTexture);
                        background.setScale(
                            (float)WINDOW_WIDTH / bgTexture.getSize().x,
                            (float)WINDOW_HEIGHT / bgTexture.getSize().y);

                        if (selectedMap == LAVA)
                            resetObstacles();
                        else
                            clearObstacles();

                        gameState = PLAYING;
                    }


                    // When going back to MENU, always show the default MENU background (background.png)
                    bgTexture.loadFromFile("background.png");
                    background.setTexture(bgTexture);
                    background.setScale(
                        (float)WINDOW_WIDTH / bgTexture.getSize().x,
                        (float)WINDOW_HEIGHT / bgTexture.getSize().y);



                    if (forestMapBtn.isClicked(mouse))
                        selectedMap = FOREST;

                    if (dessertMapBtn.isClicked(mouse))
                        selectedMap = DESSERT;

                    if (lavaMapBtn.isClicked(mouse))
                        selectedMap = LAVA;



                }

                else if (gameState == GAME_OVER)
                {
                    // Restart should only appear if you want it.
                    // (But your requirement says: after losing, show GAME OVER + RESTART + MAIN MENU)
                    if (restartBtn.isClicked(mouse))
                    {
                        player.body.clear();
                        player.body.push_back({WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2});
                        player.score = 0;

                        // Apply selected map again when restarting
                        if (selectedMap == FOREST)
                            bgTexture.loadFromFile("FOREST.png");
                        else if (selectedMap == DESSERT)
                            bgTexture.loadFromFile("DESERT.png");
                        else
                            bgTexture.loadFromFile("LAVA.png");

                        background.setTexture(bgTexture);
                        background.setScale(
                            (float)WINDOW_WIDTH / bgTexture.getSize().x,
                            (float)WINDOW_HEIGHT / bgTexture.getSize().y);

                        if (selectedMap == LAVA)
                            resetObstacles();
                        else
                            clearObstacles();

                        gameState = PLAYING;
                    }

                else if (gameState == PAUSED)
                {
                    if (resumeBtn.isClicked(mouse))
                        gameState = PLAYING;

                    if (menuBtn.isClicked(mouse))
                        gameState = MENU;

                    if (exitPauseBtn.isClicked(mouse))
                        window.close();
                }
            }
        }

        // =========================
        // LOADING
        // =========================
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

        // =========================
        // PLAYING (lava obstacles)
        // =========================
        if (gameState == PLAYING)
        {
            float t = obstacleClock.getElapsedTime().asSeconds();

            sf::Vector2f dir(0, 0);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y--;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y++;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x--;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x++;

            if (dir.x != 0 || dir.y != 0)
            {
                float len = sqrt(dir.x * dir.x + dir.y * dir.y);
                dir /= len;
            }

            bool alive = player.move(dir, sf::Keyboard::isKeyPressed(sf::Keyboard::Space));
            if (!alive)
                gameState = GAME_OVER;

            // === lava vent collision + fireball motion/collision ===
            if (gameState == PLAYING)
            {
                // update fireballs
                for (auto &fb : fireballs)
                {
                    fb.shape.move(fb.velocity * fb.speed / 60.f);

                    // respawn if off-screen
                    auto pos = fb.shape.getPosition();
                    if (pos.x < -40 || pos.x > WINDOW_WIDTH + 40 || pos.y < -40 || pos.y > WINDOW_HEIGHT + 40)
                    {
                        float x = 0, y = 0;
                        int edge = rand() % 4;
                        if (edge == 0) { x = 0; y = rand() % WINDOW_HEIGHT; }
                        if (edge == 1) { x = WINDOW_WIDTH; y = rand() % WINDOW_HEIGHT; }
                        if (edge == 2) { x = rand() % WINDOW_WIDTH; y = 0; }
                        if (edge == 3) { x = rand() % WINDOW_WIDTH; y = WINDOW_HEIGHT; }
                        fb.shape.setPosition(x, y);

                        sf::Vector2f toC(WINDOW_WIDTH / 2.f - x, WINDOW_HEIGHT / 2.f - y);
                        float len = sqrt(toC.x * toC.x + toC.y * toC.y);
                        if (len < 0.001f) len = 1.f;
                        toC /= len;
                        fb.velocity = toC;
                    }
                }

                // vent collision (only when hot)
                sf::Vector2f headPos = player.body[0];
                for (auto &v : vents)
                {
                    if (!v.isHot(t)) continue;

                    sf::Vector2f ventPos = v.shape.getPosition();
                    float d = distance(headPos, ventPos);
                    float r = v.shape.getRadius();
                    if (d <= r)
                    {
                        gameState = GAME_OVER;
                        break;
                    }
                }

                // fireball collision
                if (gameState == PLAYING)
                {
                    for (auto &fb : fireballs)
                    {
                        sf::Vector2f fbPos = fb.shape.getPosition();
                        float d = distance(headPos, fbPos);
                        float r = fb.shape.getRadius();
                        if (d <= r)
                        {
                            gameState = GAME_OVER;
                            break;
                        }
                    }
                }
            }


            for (int i = 0; i < (int)foods.size(); i++)
            {
                if (distance(player.body[0], foods[i].shape.getPosition()) < 15)
                {
                    player.score++;

                    if (player.score > highScore)
                    {
                        highScore = player.score;
                        ofstream saveFile("highscore.txt");
                        if (saveFile.is_open())
                            saveFile << highScore;
                    }

                    foods.erase(foods.begin() + i);
                    bool poison = rand() % 5 == 0;
                    foods.push_back(Food(rand() % WINDOW_WIDTH, rand() % WINDOW_HEIGHT, poison));
                    break;
                }
            }

            camera.reset(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));
            window.setView(camera);
        }

        // =========================
        // HOVER + UPDATE BUTTONS
        // =========================
        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (gameState == MENU)
        {
            playBtn.setHover(playBtn.isClicked(mouse));
            settingsBtn.setHover(settingsBtn.isClicked(mouse));
            skinsBtn.setHover(skinsBtn.isClicked(mouse));
            mapsBtn.setHover(mapsBtn.isClicked(mouse));
            exitBtn.setHover(exitBtn.isClicked(mouse));

            playBtn.update();
            settingsBtn.update();
            skinsBtn.update();
            mapsBtn.update();
            exitBtn.update();
        }

        if (gameState == PAUSED)
        {
            resumeBtn.setHover(resumeBtn.isClicked(mouse));
            menuBtn.setHover(menuBtn.isClicked(mouse));
            exitPauseBtn.setHover(exitPauseBtn.isClicked(mouse));

            resumeBtn.update();
            menuBtn.update();
            exitPauseBtn.update();
        }

        if (gameState == GAME_OVER)
        {
            restartBtn.setHover(restartBtn.isClicked(mouse));
            gameOverMenuBtn.setHover(gameOverMenuBtn.isClicked(mouse));
            restartBtn.update();
            gameOverMenuBtn.update();
        }

        // =========================
        // DRAW
        // =========================
        window.clear(sf::Color(20, 20, 20));

        for (auto &p : ashParticles) window.draw(p.shape);
        for (auto &p : lavaBubbles) window.draw(p.shape);

        if (gameState == LOADING)
        {
            window.setView(window.getDefaultView());
            window.draw(logoSprite);
            window.draw(loadingBarBG);
            window.draw(loadingBar);
        }
        else if (gameState == MENU)
        {
            // Always restore the default MENU background
            bgTexture.loadFromFile("background.png");
            background.setTexture(bgTexture);
            background.setScale(
                (float)WINDOW_WIDTH / bgTexture.getSize().x,
                (float)WINDOW_HEIGHT / bgTexture.getSize().y);

            window.setView(window.getDefaultView());
            window.draw(background);
            settingsBtn.draw(window);
            skinsBtn.draw(window);
            mapsBtn.draw(window);
            exitBtn.draw(window);
        }


        else if (gameState == PLAYING)
        {
            window.setView(camera);
            window.draw(background);

            // draw lava obstacles
            for (auto &v : vents)
                window.draw(v.shape);
            for (auto &fb : fireballs)
                window.draw(fb.shape);

            for (auto &f : foods) window.draw(f.shape);
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
        if (gameState == SETTINGS)
        {
            window.setView(window.getDefaultView());
            window.draw(background);

            // Draw map buttons
            forestMapBtn.setHover(forestMapBtn.isClicked(mouse));
            dessertMapBtn.setHover(dessertMapBtn.isClicked(mouse));
            lavaMapBtn.setHover(lavaMapBtn.isClicked(mouse));

            forestMapBtn.update();
            dessertMapBtn.update();
            lavaMapBtn.update();

            forestMapBtn.draw(window);
            dessertMapBtn.draw(window);
            lavaMapBtn.draw(window);

            // PLAY button (start selected map)
            startBtn.setHover(startBtn.isClicked(mouse));
            startBtn.update();
            startBtn.draw(window);


            sf::Text settingsText;

            settingsText.setFont(font);
            settingsText.setCharacterSize(60);
            settingsText.setFillColor(sf::Color::White);
            settingsText.setString("SETTINGS");
            sf::FloatRect sgb = settingsText.getLocalBounds();
            settingsText.setOrigin(sgb.width / 2, sgb.height / 2);
            settingsText.setPosition(WINDOW_WIDTH / 2, 150);
            window.draw(settingsText);

            sf::RectangleShape backShape;
            backShape.setSize({350, 70});
            backShape.setPosition(WINDOW_WIDTH / 2 - 175, 440);

            backShape.setFillColor(sf::Color(60, 60, 60));
            window.draw(backShape);

            sf::Text backText;
            backText.setFont(font);
            backText.setCharacterSize(32);
            backText.setFillColor(sf::Color::White);
            backText.setString("BACK");
            sf::FloatRect bt = backText.getLocalBounds();
            backText.setOrigin(bt.width / 2, bt.height / 2);
            backText.setPosition(WINDOW_WIDTH / 2, 475);

            window.draw(backText);
        }
        else if (gameState == PAUSED)
        {
            window.setView(window.getDefaultView());
            window.draw(background);
            resumeBtn.draw(window);
            menuBtn.draw(window);
            exitPauseBtn.draw(window);
        }
        else if (gameState == GAME_OVER)
        {
            window.setView(window.getDefaultView());
            window.draw(background);
            window.draw(gameOverText);
            restartBtn.draw(window);
            gameOverMenuBtn.draw(window);
        }

        window.display();
    }

    return 0;
}

