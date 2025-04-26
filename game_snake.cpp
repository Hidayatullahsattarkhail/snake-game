#include <iostream>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace std;

// A simple struct to hold x,y coordinates
struct Position {
    int x, y;
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

// Directions for movement
enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

// Handle to the console for cursor positioning
static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

// Move console cursor to (x,y)
void setCursor(int x, int y) {
    COORD c = { SHORT(x), SHORT(y) };
    SetConsoleCursorPosition(hOut, c);
}

// Hide the blinking cursor
void hideCursor() {
    CONSOLE_CURSOR_INFO info = { 100, FALSE };
    SetConsoleCursorInfo(hOut, &info);
}

class MaarGame {
    int width, height;               // Game area size
    Position snake;                  // Snake head position
    vector<Position> food;           // Fruits on board
    vector<Position> hurdles;        // Obstacles
    Direction dir;                   // Current moving direction
    int score;                       // Current score
    bool gameOver;                   // Game-over flag
    clock_t startTime;               // To compute elapsed time
    int speedMs;                     // Milliseconds between frames

public:
    // Constructor: board size, #fruits, hurdle percent, frame speed (ms)
    MaarGame(int w, int h, int numFruits, int hurdlePercent, int speed = 100)
      : width(w), height(h), dir(STOP), score(0), gameOver(false), speedMs(speed)
    {
        srand((unsigned)time(nullptr));
        snake = { width/2, height/2 };
        hideCursor();
        drawBoundary();

        // Generate hurdles
        int totalCells = width * height;
        int numHurdles = totalCells * hurdlePercent / 100;
        for (int i = 0; i < numHurdles; ++i) {
            Position p;
            do {
                p = { rand() % width, rand() % height };
            } while (p == snake || isHurdle(p));
            hurdles.push_back(p);
        }

        // Spawn fruits
        for (int i = 0; i < numFruits; ++i) spawnFood();
        startTime = clock();
    }

    // Main loop
    void run() {
        while (!gameOver) {
            draw();
            input();
            logic();
            Sleep(speedMs);
        }
        setCursor(0, height + 3);
        cout << "Game Over! Final Score: " << score << "\n";
    }

private:
    // Draw static border once
    void drawBoundary() {
        for (int x = 0; x <= width + 1; ++x) {
            setCursor(x, 0);          cout << "#";
            setCursor(x, height + 1); cout << "#";
        }
        for (int y = 0; y <= height + 1; ++y) {
            setCursor(0, y);          cout << "#";
            setCursor(width + 1, y);  cout << "#";
        }
    }

    bool isHurdle(const Position& p) {
        for (auto& h : hurdles) if (p == h) return true;
        return false;
    }
    bool isFood(const Position& p) {
        for (auto& f : food) if (p == f) return true;
        return false;
    }

    void spawnFood() {
        Position p;
        do { p = { rand() % width, rand() % height }; }
        while (p == snake || isHurdle(p) || isFood(p));
        food.push_back(p);
    }

    void draw() {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                setCursor(x + 1, y + 1);
                Position p{ x, y };
                if      (p == snake)    cout << "O";
                else if (isFood(p))     cout << "F";
                else if (isHurdle(p))   cout << "X";
                else                    cout << " ";
            }
        }
        double elapsed = double(clock() - startTime) / CLOCKS_PER_SEC;
        setCursor(0, height + 2);
        cout << "Score: " << score
             << "    Time: " << int(elapsed) << "s"
             << "    (Arrows to move, Esc to quit)";
    }

    void input() {
        if (_kbhit()) {
            int c = _getch();
            if (c == 224) {
                switch (_getch()) {
                    case 72: if (dir != DOWN)  dir = UP;    break;
                    case 80: if (dir != UP)    dir = DOWN;  break;
                    case 75: if (dir != RIGHT) dir = LEFT;  break;
                    case 77: if (dir != LEFT)  dir = RIGHT; break;
                }
            } else if (c == 27) gameOver = true;
        }
    }

    void logic() {
        switch (dir) {
            case LEFT:  --snake.x; break;
            case RIGHT: ++snake.x; break;
            case UP:    --snake.y; break;
            case DOWN:  ++snake.y; break;
            default: break;
        }
        if (snake.x < 0 || snake.x >= width || snake.y < 0 || snake.y >= height || isHurdle(snake)) {
            gameOver = true;
            return;
        }
        for (int i = 0; i < (int)food.size(); ++i) {
            if (snake == food[i]) {
                score += 10;
                food.erase(food.begin() + i);
                spawnFood();
                break;
            }
        }
    }
};

int main() {
    // 25×25 board, 4 fruits, ~2% hurdles, 300ms frame delay (slower snake)
    MaarGame game(25, 25, 4, 2, 300);
    game.run();
    return 0;
}
