#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>
#include <chrono>

class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; 
    std::mutex board_mutex; 
    std::condition_variable turn_cv; 
    char current_player;
    bool game_over; 
    char winner; 
    int moves_made;

public:
    TicTacToe() : current_player('X'), game_over(false), winner(' '), moves_made(0) {
        for (auto &row : board) {
            row.fill(' ');
        }
    }

    void display_board() {
        std::lock_guard<std::mutex> lock(board_mutex);
        for (const auto &row : board) {
            for (const auto &cell : row) {
                std::cout << (cell == ' ' ? '.' : cell) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    bool make_move(char player, int row, int col) {
        std::unique_lock<std::mutex> lock(board_mutex);
        turn_cv.wait(lock, [this, player] { return current_player == player && !game_over; });

        if (board[row][col] == ' ') {
            board[row][col] = player;
            moves_made++;
            if (check_win(player)) {
                game_over = true;
                winner = player;
            } else if (check_draw()) {
                game_over = true;
                winner = 'D';
            }
            current_player = (player == 'X') ? 'O' : 'X';
            lock.unlock();
            turn_cv.notify_all();
            return true;
        }
        return false;
    }

    bool check_win(char player) {
        
        for (int i = 0; i < 3; ++i) {
            if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
                (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
                return true;
            }
        }
        if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
            (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
            return true;
        }
        return false;
    }

    bool check_draw() {
        return moves_made == 9;
    }

    bool is_game_over() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return game_over;
    }

    char get_winner() {
        std::lock_guard<std::mutex> lock(board_mutex);
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe &game; 
    char symbol; 
    std::string strategy; 

public:
    Player(TicTacToe &g, char s, std::string strat)
        : game(g), symbol(s), strategy(strat) {}

    void play() {
        if (strategy == "sequential") {
            play_sequential();
        } else if (strategy == "random") {
            play_random();
        }
    }

private:
    void play_sequential() {
        for (int i = 0; i < 3 && !game.is_game_over(); ++i) {
            for (int j = 0; j < 3 && !game.is_game_over(); ++j) {
                if (game.make_move(symbol, i, j)) {
                    game.display_board();
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                }
            }
        }
    }

    void play_random() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 2);

        while (!game.is_game_over()) {
            int row = dis(gen);
            int col = dis(gen);
            if (game.make_move(symbol, row, col)) {
                game.display_board();
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }
};

// Função principal
int main() {
    TicTacToe game;
    Player player1(game, 'X', "sequential");
    Player player2(game, 'O', "random");

    std::thread t1(&Player::play, &player1);
    std::thread t2(&Player::play, &player2);

    t1.join();
    t2.join();

    char winner = game.get_winner();
    if (winner == 'D') {
        std::cout << "O jogo terminou em empate!" << std::endl;
    } else {
        std::cout << "O vencedor eh o jogador " << winner << "!" << std::endl;
    }

    return 0;
}
