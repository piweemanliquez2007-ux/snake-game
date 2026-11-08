#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <conio.h>

using namespace std;

const int WIDTH = 25;
const int HEIGHT = 20;

bool gameOver;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;

// Set console text color
void SetColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void Setup() {
    gameOver = false;
    dir = STOP;
    x = WIDTH / 2;
    y = HEIGHT / 2;
    fruitX = rand() % WIDTH;
    fruitY = rand() % HEIGHT;
    score = 0;
    nTail = 0;
}

void Draw() {
    SetColor(7); // White for borders
    system("cls");

    // Top border
    for (int i = 0; i < WIDTH + 2; i++)
        cout << "#";
    cout << endl;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (j == 0) {
                SetColor(7);
                cout << "#";
            }

            if (i == y && j == x) {
                // Snake head: bright blue
                SetColor(9);
                cout << "O";
            }
            else if (i == fruitY && j == fruitX) {
                // Fruit: magenta/purple gem
                SetColor(13);
                cout << "*";
            }
            else {
                bool printed = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        // Snake body: light blue crystal
                        SetColor(11);
                        cout << "o";
                        printed = true;
                    }
                }
                if (!printed) {
                    SetColor(7);
                    cout << " ";
                }
            }

            if (j == WIDTH - 1) {
                SetColor(7);
                cout << "#";
            }
        }
        cout << endl;
    }

    // Bottom border
    SetColor(7);
    for (int i = 0; i < WIDTH + 2; i++)
        cout << "#";
    cout << endl;

    SetColor(15);
    cout << "Score: " << score << endl;
    cout << "Controls: W A S D | X = Quit" << endl;
}

void Input() {
    if (_kbhit()) {
        switch (_getch()) {
            case 'a': case 'A': if (dir != RIGHT) dir = LEFT; break;
            case 'd': case 'D': if (dir != LEFT) dir = RIGHT; break;
            case 'w': case 'W': if (dir != DOWN) dir = UP; break;
            case 's': case 'S': if (dir != UP) dir = DOWN; break;
            case 'x': case 'X': gameOver = true; break;
        }
    }
}

void Logic() {
    // Move tail
    int prevX = tailX[0], prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;

    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    // Move head
    switch (dir) {
        case LEFT:  x--; break;
        case RIGHT: x++; break;
        case UP:    y--; break;
        case DOWN:  y++; break;
        default: break;
    }

    // Wall collision
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT)
        gameOver = true;

    // Self collision
    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            gameOver = true;

    // Eat fruit
    if (x == fruitX && y == fruitY) {
        score += 10;
        fruitX = rand() % WIDTH;
        fruitY = rand() % HEIGHT;
        nTail++;
    }
}

int main() {
    srand(time(0));
    Setup();

    while (!gameOver) {
        Draw();
        Input();
        Logic();
        Sleep(120); // Control speed
    }

    SetColor(15);
    cout << "\nGAME OVER! Final Score: " << score << endl;
    return 0;
}