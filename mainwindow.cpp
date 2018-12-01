#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPixmap>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>
#include <iostream>
#include <string>

using namespace std;
int skok=0;
bool polecenie[4];
int pola_zabronione[1][4];
int poz[4];
int PREDKOSC = 5;
int wgore,wdol;
int ILE_BLOKOW=17;
QLabel *bloki[17];
int ktory=0;
bool spada=true;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(ruch()));
    timer->start(40);

    bloki[0] = ui->block_1;
    bloki[1] = ui->block_2;
    bloki[2] = ui->block_3;
    bloki[3] = ui->block_4;
    bloki[4] = ui->block_5;
    bloki[5] = ui->block_6;
    bloki[6] = ui->block_7;
    bloki[7] = ui->block_8;
    bloki[8] = ui->block_9;
    bloki[9] = ui->block_10;
    bloki[10] = ui->block_11;
    bloki[11] = ui->block_12;
    bloki[12] = ui->block_13;
    bloki[13] = ui->block_14;
    bloki[14] = ui->block_15;
    bloki[15] = ui->block_16;
    bloki[16] = ui->block_17;
    for(int i=0; i<3; i++)
        polecenie[i]=false;
    polecenie[3]=true; //spadaj caly czas jak nie masz gruntu
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::kolizja(int poz0, int poz1,int poz2,int poz3)
{
    for(int i=0; i<ILE_BLOKOW; i++)
    {
        if( ( poz0 <= (bloki[i]->x()+bloki[i]->width()) ) && ( poz1 >= bloki[i]->x() ) &&
            ( poz2 <= (bloki[i]->y()+bloki[i]->height()) ) && ( poz3 >= bloki[i]->y() ) )
        {
            ktory=i;
            return true;
        }
    }
    return false;


}

void MainWindow::ruch()
{
    poz[0]=ui->player_1->x(); //lewo
    poz[1]=ui->player_1->x() + ui->player_1->width(); //prawo
    poz[2]=ui->player_1->y(); //góra
    poz[3]=ui->player_1->y() + ui->player_1->height(); //dół

    //lewo
    if(polecenie[0]==true)
    {
        if(!(poz[0]-PREDKOSC <= 0))
        {
            if(!kolizja(poz[0]-PREDKOSC-1, poz[1]-PREDKOSC-1, poz[2], poz[3]))
            {
                if (ui->player_1->styleSheet() == "border-image: url(:/new/prefix1/rabbit2_icon_l1.png);")
                    ui->player_1->setStyleSheet("border-image: url(:/new/prefix1/rabbit2_icon_l2.png);");
                else ui->player_1->setStyleSheet("border-image: url(:/new/prefix1/rabbit2_icon_l1.png);");
            ui->player_1->setGeometry(poz[0]-PREDKOSC,ui->player_1->y(),ui->player_1->width(),ui->player_1->height());
            }
            else
                ui->player_1->setGeometry(bloki[ktory]->x()+bloki[ktory]->width()+1,ui->player_1->y(),ui->player_1->width(),ui->player_1->height());
        }
    }
    //prawo
    if(polecenie[1]==true)
    {
        if(!(poz[1]+PREDKOSC >= ui->background->width()))
        {
            if(!kolizja(poz[0]+PREDKOSC+1, poz[1]+PREDKOSC+1, poz[2], poz[3]))
            {
                if (ui->player_1->styleSheet() == "border-image: url(:/new/prefix1/rabbit2_icon_r1.png);")
                    ui->player_1->setStyleSheet("border-image: url(:/new/prefix1/rabbit2_icon_r2.png);");
                else ui->player_1->setStyleSheet("border-image: url(:/new/prefix1/rabbit2_icon_r1.png);");
                ui->player_1->setGeometry(poz[0]+PREDKOSC,ui->player_1->y(),ui->player_1->width(),ui->player_1->height());
            }
            else
                ui->player_1->setGeometry(bloki[ktory]->x()-ui->player_1->width()-1,ui->player_1->y(),ui->player_1->width(),ui->player_1->height());
        }
    }
    //skok
    if(polecenie[2]==true)
    {
        wgore=16-skok;

        if(kolizja(poz[0], poz[1], poz[2]-wgore, poz[3]-wgore))
        {
           skok=16;
           ui->player_1->setGeometry(ui->player_1->x(),(bloki[ktory]->y() + bloki[ktory]->height() + 1), ui->player_1->width(),ui->player_1->height());
        }
        else ui->player_1->setGeometry(ui->player_1->x(),ui->player_1->y()-wgore,ui->player_1->width(),ui->player_1->height());

        if(skok<16) skok++;
        else
        {
            skok=0;
            polecenie[2]=false;
            polecenie[3]=true;
        }
    }
    //spadanie
    if(polecenie[3]==true)
    {
        skok++;
        if(kolizja(poz[0], poz[1], poz[2]+skok, poz[3]+skok))
        {
           ui->player_1->setGeometry(ui->player_1->x(),(bloki[ktory]->y() - ui->player_1->height() -1), ui->player_1->width(),ui->player_1->height());
           spada=false;
           skok=0;
        }
        else
        {
            ui->player_1->setGeometry(ui->player_1->x(),ui->player_1->y()+skok,ui->player_1->width(),ui->player_1->height());
            spada=true;
        }
    }
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_Left)
    {
        polecenie[0]=true;
    }

    if(event->key() == Qt::Key_Right)
    {
        polecenie[1]=true;
    }

    if(event->key() == Qt::Key_Up && polecenie[2] == false && spada == false)
    {
        polecenie[2]=true;
        polecenie[3]=false;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        polecenie[0]=false;
    }
    if(event->key() == Qt::Key_Right)
    {
        polecenie[1]=false;
    }
}
