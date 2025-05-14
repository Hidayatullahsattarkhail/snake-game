#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <deque>
#include <vector>
#include <cstdlib>
#include <fstream>

using namespace std;

void clearScreen() {
    system("CLS");
}

class SnakeGame {
private:
    const char SNAKE_SEGMENT = 'O';
    const char FOOD_ITEM    = '*';
    const char OBSTACLE_ITEM= '#';
    const char EMPTY_SPACE  = ' ';
    const char BORDER_CHAR  = 'X';
    const int  MIN_BOARD_SIZE   = 25;
    const int  INITIAL_SNAKE_SIZE = 2;

    int boardSize   = 25;
    int playerScore = 0;
    int timeLimit;
    time_t lastGameTime;

    vector<vector<char>> gameBoard;
    vector<vector<bool>> obstaclesPresent;
    deque<pair<int,int>> snakeBody;
    vector<pair<int,int>> foodLocations;

    int moveX = 1, moveY = 0;
    bool isGameOver = false;

    int screenCenterX, screenCenterY;
    int snakeSpeed = 300;

    void setCursorPosition(int x, int y) {
        COORD coord{ (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    void calculateScreenCenter() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        int screenWidth  = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        screenCenterX = (screenWidth  - (boardSize + 2)) / 2;
        screenCenterY = (screenHeight - (boardSize + 4)) / 2;
    }

    void drawBoardBorders() {
        setCursorPosition(screenCenterX, screenCenterY);
        for(int i = 0; i < boardSize + 2; ++i) cout << BORDER_CHAR;
        for(int y = 0; y < boardSize; ++y) {
            setCursorPosition(screenCenterX, screenCenterY + y + 1);
            cout << BORDER_CHAR;
            for(int x = 0; x < boardSize; ++x) cout << gameBoard[y][x];
            cout << BORDER_CHAR;
        }
        setCursorPosition(screenCenterX, screenCenterY + boardSize + 1);
        for(int i = 0; i < boardSize + 2; ++i) cout << BORDER_CHAR;
    }

    void drawGameStatus(int remainingTime) {
        setCursorPosition(screenCenterX, screenCenterY + boardSize + 3);
        cout << "Score: " << playerScore
             << " | Time Left: " << remainingTime << "s   ";
    }

    void placeRandomObstacles(int count) {
        for(int i = 0; i < count; ++i) {
            int x, y;
            do {
                x = rand() % boardSize;
                y = rand() % boardSize;
            } while(gameBoard[y][x] != EMPTY_SPACE);
            gameBoard[y][x] = OBSTACLE_ITEM;
            obstaclesPresent[y][x] = true;
        }
    }

    // Place exactly one new food item at a random empty location,
    // and immediately draw it on the console.
    void placeSingleFood() {
        int x, y;
        do {
            x = rand() % boardSize;
            y = rand() % boardSize;
        } while(gameBoard[y][x] != EMPTY_SPACE);
        gameBoard[y][x] = FOOD_ITEM;
        foodLocations.push_back({x, y});
        // draw the new star
        setCursorPosition(screenCenterX + 1 + x,
                          screenCenterY + 1 + y);
        cout << FOOD_ITEM;
    }

    void initializeSnake() {
        int startX = boardSize / 2;
        int startY = boardSize / 2;
        for(int i = INITIAL_SNAKE_SIZE - 1; i >= 0; --i) {
            snakeBody.push_back({startX - i, startY});
            gameBoard[startY][startX - i] = SNAKE_SEGMENT;
        }
    }

    void drawSnakeBody() {
        for(auto &seg : snakeBody) {
            setCursorPosition(screenCenterX + 1 + seg.first,
                              screenCenterY + 1 + seg.second);
            cout << SNAKE_SEGMENT;
        }
    }

    void clearSnakeTail() {
        auto tail = snakeBody.front();
        gameBoard[tail.second][tail.first] = EMPTY_SPACE;
        setCursorPosition(screenCenterX + 1 + tail.first,
                          screenCenterY + 1 + tail.second);
        cout << EMPTY_SPACE;
        snakeBody.pop_front();
    }

    void moveSnakeBody() {
        int newX = snakeBody.back().first + moveX;
        int newY = snakeBody.back().second + moveY;

        // wall or obstacle collision
        if(newX < 0 || newX >= boardSize ||
           newY < 0 || newY >= boardSize ||
           obstaclesPresent[newY][newX]) {
            isGameOver = true;
            return;
        }
        // self-collision
        for(auto &seg : snakeBody) {
            if(seg.first == newX && seg.second == newY) {
                isGameOver = true;
                return;
            }
        }
        // eating food?
        if(gameBoard[newY][newX] == FOOD_ITEM) {
            playerScore++;
            gameBoard[newY][newX] = EMPTY_SPACE;
            placeSingleFood();  // spawn and draw one new star
        } else {
            clearSnakeTail();
        }
        // move head
        snakeBody.push_back({newX, newY});
        gameBoard[newY][newX] = SNAKE_SEGMENT;
        setCursorPosition(screenCenterX + 1 + newX,
                          screenCenterY + 1 + newY);
        cout << SNAKE_SEGMENT;
    }

    void handlePlayerInput() {
        if(_kbhit()) {
            char ch = _getch();
            if(ch == -32) {
                ch = _getch();
                switch(ch) {
                    case 72: if(moveY != 1 ) { moveX =  0; moveY = -1; } break;
                    case 80: if(moveY != -1) { moveX =  0; moveY =  1; } break;
                    case 75: if(moveX != 1 ) { moveX = -1; moveY =  0; } break;
                    case 77: if(moveX != -1) { moveX =  1; moveY =  0; } break;
                }
            }
        }
    }

public:
    void playGame(ofstream &fout, int roundNumber, int timeLimitSec) {
        timeLimit = timeLimitSec;
        int obstaclesCount = 5;

        // initialize
        gameBoard.assign(boardSize, vector<char>(boardSize, EMPTY_SPACE));
        obstaclesPresent.assign(boardSize, vector<bool>(boardSize, false));
        snakeBody.clear();
        foodLocations.clear();
        playerScore = 0;
        isGameOver = false;

        calculateScreenCenter();
        initializeSnake();
        placeRandomObstacles(obstaclesCount);
        // start with one star
        placeSingleFood();
        drawBoardBorders();
        drawSnakeBody();
        lastGameTime = time(0);

        int remainingTime;
        while(!isGameOver) {
            handlePlayerInput();
            moveSnakeBody();
            remainingTime = timeLimit - int(time(0) - lastGameTime);
            if(remainingTime <= 0) {
                isGameOver = true;
                break;
            }
            drawGameStatus(remainingTime);
            Sleep(snakeSpeed);
        }

        // log round results
        setCursorPosition(screenCenterX, screenCenterY + boardSize + 5);
        fout << roundNumber
             << " | " << (time(0) - lastGameTime)
             << " | " << playerScore
             << endl;
    }
};

int main() {
    ofstream fout("hidayatkhan.txt");
    if(!fout.is_open()) {
        cerr << "Failed to open output file.\n";
        return 1;
    }

    srand(unsigned(time(0)));
    int totalRounds, timeLimit;
    cout << "Enter number of rounds: ";
    cin  >> totalRounds;
    cout << "Enter time limit (in seconds): ";
    cin  >> timeLimit;

    bool continuePlaying = true;
    for(int i = 1; i <= totalRounds && continuePlaying; ++i) {
        if(i != 1) cout << "Round: " << i << endl;
        SnakeGame game;
        game.playGame(fout, i, timeLimit);

        cout << "Do you want to play another round? (y/n): ";
        char choice;
        cin  >> choice;
        if(choice == 'n' || choice == 'N') {
            cout << "\nEnd Game!!\n";
            continuePlaying = false;
        } else {
            clearScreen();
        }
    }

    fout.close();
    return 0;
}
