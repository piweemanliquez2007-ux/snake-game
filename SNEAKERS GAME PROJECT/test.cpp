#include <iostream>

using namespace std;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

static bool loadTextureFile(sf::Texture &tex, const std::string &filename)
{
    // Try relative to current working directory first, then fall back to assets/.
    if (tex.loadFromFile(filename))
        return true;

    std::string assetsPath = std::string("assets/") + filename;
    if (tex.loadFromFile(assetsPath))
        return true;

    std::cerr << "Texture load failed: tried '" << filename << "' and '" << assetsPath << "'\n";
    return false;
}


if (gameState == PLAYING)
        {
            float t = obstacleClock.getElapsedTime().asSeconds();


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

    