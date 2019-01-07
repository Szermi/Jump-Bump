//
// Created by monika on 30/12/18.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <list>
#include <iostream>
#include "serverGame.h"

using namespace std;

void getCommand(Player *player) {
    string newPart = player->buf;
    (player->buf).clear();

    if (newPart.empty()) {
        player->run = false;
    }

    // gdy juz byl poczatek
    if(!(player->command).empty()) {
        if ((player->command).at(0) == START) {
//            cout << "start\n";
            (player->command).append(newPart.substr(0, newPart.find(END) + 1));
            newPart = newPart.substr(newPart.find(END) + 1,
                                     newPart.size() - (newPart.find(END) + 2));
            (player->buf).append(newPart);
        }
    }

    else {
//        cout << "new\n";
        int startPoint = newPart.find(START);
        int endPoint = newPart.find(END);
//        cout << startPoint << " " << endPoint << " " << newPart.size() <<'\n';

        if(startPoint == -1);

        else if(endPoint == -1) {
            (player->buf).append(newPart.c_str());
        }

        else {
            (player->command).append(newPart.substr(startPoint, endPoint - startPoint + 1));
            if(endPoint < newPart.size() - 1) {
                newPart = newPart.substr(endPoint + 1, newPart.size() - (endPoint + 1));
                (player->buf).append(newPart);
            }
        }
    }

    if(!player->buf.empty()){
        player->buf.erase(player->buf.find('\r'));
    }

//    cout << player->login << " ";
//    cout  << "c: " << player->command << " || buf: " << player->buf << " " <<  '\n';
    

}

Player* getOpponent(Player *player){
    int firstDelimiter = player->command.find(DELIMITER);

    string opponentLogin = player->command.substr(firstDelimiter + 1,
                                                  player->command.size() - firstDelimiter - 2);

    Player *opponent = nullptr;

    for (auto &p : *player->players_ptr) {
        if (p->login == opponentLogin) {
            opponent = p;
        }
    }
    return opponent;
}

void getPosition(Player *player) {
    int firstDelimiter = player->command.find(DELIMITER);
    int secondDelimiter = player->command.find(DELIMITER, firstDelimiter+1);
    string pos1 = player->command.substr(firstDelimiter + 1,
                                                  secondDelimiter - firstDelimiter - 1);
    string pos2 = player->command.substr(secondDelimiter + 1,
                                         player->command.size() - secondDelimiter - 2);
//    cout << pos1 << " " << pos2 << '\n';
    player->position[0] = stoi(pos1);
    player->position[1] = stoi(pos2);
}

void startGame(Player *player) {
    cout << "preparing to start a game" << '\n';

    pthread_mutex_t mutex_game = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex_game);

    struct thread_data_game * game_data = new thread_data_game;
    game_data->player[0] = player;
    game_data->player[1] = player->opponent;
    game_data->gameMutex = &mutex_game;
    pthread_t thread1;
    cout << player->login << " " << game_data->player[0]->login << '\n';
    int create_result = pthread_create(&thread1, NULL, playGame, (void *)game_data);

    if (create_result){
        pthread_mutex_unlock(&mutex_game);
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }

    pthread_mutex_lock(&mutex_game);
    delete game_data;
    pthread_mutex_unlock(&mutex_game);
}

void funcLogin(Player *player) {

    char buffer[BUFFER];
    string newLogin = player->command.substr(
            player->command.find(DELIMITER) + 1,
            player->command.find(END) - player->command.find(DELIMITER) - 1);

    bool err = false;

    for (auto &p : *player->players_ptr) {
        if (p->login == newLogin) {
            err = true;
        }
    }

    memset(buffer, 0, BUFFER);

    // gdy login juz istnieje
    if (err) {
        string message = "#0;0&";
        strcpy(buffer, message.c_str());
        write(player->fd, buffer, message.size());
    } else {
        cout << newLogin << '\n';
        player->login.append(newLogin);
        player->logged = true;
        string message = "#0;1&";
        strcpy(buffer, message.c_str());
        write(player->fd, buffer, message.size());
    }
}

void funcList(Player *player) {
    char buffer[BUFFER];

    string s = "#1;";
    strcpy(buffer, s.c_str());
    write(player->fd, buffer, 3);
    memset(buffer, 0, BUFFER);
    for (auto &p : *player->players_ptr) {
        if ((!p->login.empty()) && p->fd != player->fd) {
            string message = p->login + ':';
            strcpy(buffer, message.c_str());
            write(player->fd, buffer, message.size());
            memset(buffer, 0, BUFFER);
        }
    }
    s = "&";
    strcpy(buffer, s.c_str());
    write(player->fd, buffer, 1);
}

void funcSendUnavailable(Player *player) {
    char buffer[5];
    string message = "#2;0&";
    strcpy(buffer, message.c_str());
    write(player->fd, buffer, message.size());
}

void funcInvite(Player *player) {
    char buffer[BUFFER];

    Player *opponent = getOpponent(player);

    if (opponent == nullptr) {
        funcSendUnavailable(player);
    }

    else if (opponent->playing) {
        funcSendUnavailable(player);
    }

        // wysyla zaproszenie
    else {
        string message = "#2;1&";
        strcpy(buffer, message.c_str());
        write(player->fd, buffer, message.size());
        memset(buffer, 0, BUFFER);

        message.clear();
        message.append("#3;");
        message.append(player->login);
        message.append("&");
        strcpy(buffer, message.c_str());
        write(opponent->fd, buffer, message.size());

        player->command.clear();

        // czeka na odpowiedz
        char reply = '.';
        while (reply == '.') {
            memset(buffer, 0, BUFFER);
            read(player->fd, buffer, BUFFER);
            player->buf.append(string(buffer));
            getCommand(player);

            if(!player->command.empty()) {
                if (player->command.at(0) == START && player->command.at(player->command.size() - 1) == END) {

                    // w miedzyczasie ktos go zaprasza
                    if (player->command.at(1) == '3') {
                        Player *newPlayer = getOpponent(player);
                        if(newPlayer != nullptr) {
                            funcSendUnavailable(newPlayer);
                        }
                    }

                    else if (player->command.at(1) == '4') {
                        reply = player->command.at(3);
                    }

                    player->command.clear();
                }
            }

        }

        //zgodzil sie na gre
        if (reply == '1') {
            player->opponent= opponent;

            // start watku gry (do kolizji)
            startGame(player);

            player->playing = true;

        }
    }
}

void funcIsInvited(Player *player) {
    char buffer[BUFFER];

    int firstDelimiter = player->command.find(DELIMITER);
    int secondDelimiter = player->command.find(DELIMITER, firstDelimiter + 1);

    string opponentLogin = player->command.substr(firstDelimiter + 1,
                                                  secondDelimiter - firstDelimiter - 1);
    Player *opponent = nullptr;

    for (auto &p : *player->players_ptr) {
        if (p->login == opponentLogin) {
            opponent = p;
        }
    }

    char reply = player->command.at(secondDelimiter + 1);
    // zgadza sie na gre
    if(opponent != nullptr) {
        if (reply == '1') {
            string message = "#4;1&";
            strcpy(buffer, message.c_str());
            write(opponent->fd, buffer, message.size());

            player->opponent = opponent;
            player->playing = true;

        } else {
            string message = "#4;0&";
            strcpy(buffer, message.c_str());
            write(opponent->fd, buffer, message.size());
            memset(buffer, 0, BUFFER);
        }
    }
}

//funkcja obslugujaca gracza
void *playerPlays (void *t_data)
{
    pthread_detach(pthread_self());
    cout << "player ";

    auto *player = new Player();
    player->fd = ((struct thread_data*)(t_data))->fd;
    player->players_ptr = ((struct thread_data*)(t_data))->players_ptr;
    player->players_ptr->push_back(player);
    player->serverMutex = ((struct thread_data*)(t_data))->mutex;

    cout << player->fd << '\n';

    pthread_mutex_unlock(player->serverMutex);

    char buffer[BUFFER];

    // logowanie
    while(player-> run && !player->logged) {
        memset(buffer, 0, BUFFER);
        read(player->fd, buffer, BUFFER);
        player->buf.append(string(buffer));
        getCommand(player);

        if(!player->command.empty()) {
            if(player->command.at(0) == START && player->command.at(player->command.size()-1) == END) {

                if (player->command.at(1) == '0') {
                    funcLogin(player);
                }

                player->command.clear();
            }
        }
    }


    // zanim zacznie grac
    while(player-> run && !player->playing) {
        memset(buffer, 0, BUFFER);
        read(player->fd, buffer, BUFFER);
        player->buf.append(string(buffer));
        getCommand(player);

        if(!player->command.empty()) {
            if (player->command.at(0) == START && player->command.at(player->command.size() - 1) == END) {

                // wyslij liste graczy
                if (player->command.at(1) == '1') {
                    funcList(player);
                }

                    // gracz chce zaprosic gracza2 do gry
                else if (player->command.at(1) == '2') {
                    funcInvite(player);

                }

                    // gracz jest zapraszany do gry
                else if (player->command.at(1) == '3') {
                    funcIsInvited(player);
                }

                player->command.clear();
            }
        }
    }

    // przesylanie pozycji
    while(player-> run && player->playing) {

        memset(buffer, 0, BUFFER);
        read(player->fd, buffer, BUFFER);
        player->buf.append(string(buffer));
        getCommand(player);

        if(!player->command.empty()) {
            if (player->command.at(0) == START && player->command.at(player->command.size() - 1) == END) {

                if (player->command.at(1) == '9') {
                    pthread_mutex_lock(&player->playerMutex);
                    getPosition(player);
                    pthread_mutex_unlock(&player->playerMutex);
                }

                player->command.clear();

            }
        }

    }

    delete player;
}

