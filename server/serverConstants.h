//
// Created by monika on 29/12/18.
//

#include <list>
#include <string>

#ifndef JUMP_BUMP_CONSTANTS_H
#define JUMP_BUMP_CONSTANTS_H

#define SERVER_PORT 1234
#define QUEUE_SIZE 5

#define DELIMITER ';'
#define START '#'
#define END '&'
#define BUFFER 20

class Player {
public:
    std::string login;
    bool logged;
    bool playing;
    std::string command;
    std::string buf;
    int fd;
    int opponentFd;
    std::list <Player *> *players_ptr;
    pthread_mutex_t *mutex;

    Player() {
        login.clear();
        buf.clear();
        command.clear();
        logged = false;
        playing = false;
    }
    virtual ~Player() {}
};

class Game {
public:
    Player *player1;
    Player *player2;
    int score_player1;
    int score_player2;

    Game() {
            score_player1 = 0;
            score_player2 = 0;
    }
    virtual ~Game() {}
};

struct thread_data
{
    int fd;
    std::list <Player *> *players_ptr;
    pthread_mutex_t *mutex;
};

struct thread_data_game
{
    Player *player1;
    Player *player2;
};

#endif //JUMP_BUMP_CONSTANTS_H

