#include "menu.h"
#include "ui_menu.h"
#include "mainwindow.h"
#include <iostream>
#include <QMessageBox>
#include <QtGui>
#include <QtNetwork>

using namespace std;
string plec ="M";
string imie = "";
string dlugi_tekst="";
QByteArray lista_graczy="";
int odp_polaczono=-1;
int odp_lista=-1;
int port = 1234;

static inline char* IntToChar(qint16 poz_x, qint16 poz_y);

/*
void MainWindow::write()
{
    if(tcpSocket->state() == QAbstractSocket::ConnectedState)
        {
            char * dane = IntToChar(poz[0], poz[2]);

            //qInfo() << poz[0] << ':' << poz[2] << ' ' << dane;
            tcpSocket->write(dane);
            delete[] dane;
        }
//    else
//       return false;
}
*/




Menu::Menu(QWidget *parent) :
    QDialog(parent),

    tcpSocket(new QTcpSocket(this)),

    ui(new Ui::Menu)
{
    ui->setupUi(this);
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QIODevice::readyRead, this, &Menu::readData);
    tcpSocket->connectToHost("127.0.0.1", port);
    cout << "Nawiązano połączenie" << endl;
}

Menu::~Menu()
{
    delete ui;
}

void Menu::on_plecM_clicked()
{
    ui->zdj_krolika->setStyleSheet("border-image: url(:/new/prefix1/rabbit2.jpg);");
}

void Menu::on_plecK_clicked()
{
    ui->zdj_krolika->setStyleSheet("border-image: url(:/new/prefix1/rabbit1.jpg);");
}

void Menu::on_polaczButton_clicked()
{
    imie=ui->imieText->text().toStdString();
    if(imie=="")
    {
        QMessageBox::information(this,"Błąd", "Proszę podać imię królika!");
    }
    else
    {
        if(ui->plecM->isChecked()==false && ui->plecK->isChecked()==false)
            QMessageBox::information(this,"Błąd", "Proszę wybrać płeć królika!");
        else
        {
            if (ui->plecK->isChecked()==true)
                plec="K";

            write(0,imie);

            do
            {
                qApp->processEvents();
            }
            while(odp_polaczono==-1);

            if(odp_polaczono==0)
            {
                QMessageBox::information(this,"Błąd", "Użytkownik z taką nazwą już istnieje");
            }
            else if(odp_polaczono==1)
            {
                write(1,imie);
                do
                {
                    qApp->processEvents();
                }
                while(odp_lista==-1);
                ui->polaczButton->setEnabled(false);
                ui->imieText->setEnabled(false);
                ui->plecM->setEnabled(false);
                ui->plecK->setEnabled(false);
                ui->listaGraczy->setEnabled(true);
                ui->listaGraczy->setStyleSheet("background-color: white;");
                ui->odswiezButton->setEnabled(true);

                qInfo() << lista_graczy;
                if(lista_graczy!="")
                {
                    QList<QByteArray> lista_graczy_podzielona=lista_graczy.split(':');
                    for(int i=0; i<lista_graczy_podzielona.size(); i++)
                        ui->listaGraczy->addItem(lista_graczy_podzielona[i]);
                }
                else
                    QMessageBox::information(this,"Informacja", "Nie ma jeszcze żadnego gracza. Poczekaj chwilę");
            }
            odp_polaczono=-1;
            odp_lista=-1;
        }
    }
}

void Menu::on_listaGraczy_clicked(const QModelIndex &index)
{
    ui->dolaczButton->setEnabled(true);
}


void Menu::on_dolaczButton_clicked()
{
    QMessageBox::information(this,"Informacja", "Wybrano królika: " + ui->listaGraczy->currentItem()->text());
    this->setVisible(false);
    this->setEnabled(false);
    this->close();
    /*MainWindow *game;
    game = new MainWindow(QString::fromStdString(plec), QString::fromStdString(imie), this);
    game->show();*/
}

void Menu::wyslij(string temp)
{
    char * komunikat = new char[temp.size() + 1];
    copy(temp.begin(), temp.end(), komunikat);
    komunikat[temp.size()] = '\0';
    qInfo() << komunikat << endl;
    tcpSocket->write(komunikat);
    delete[] komunikat;

}

void Menu::write(int polecenie, string nick)
{
    cout << tcpSocket->state() << endl;
    if(tcpSocket->state() == QAbstractSocket::ConnectedState) {
        switch(polecenie)
        {
            case 0: wyslij(string("#0;" + nick + "&"));
                    break;
            case 1: wyslij(string("#1;1&"));
                    break;
            default: break;
        }

    }
}

QByteArray Menu::scalanie()
{
    string komunikat="";
    int usun=0;
    bool zapisuj=false, poprawny=false;
    for(int i=0; i<dlugi_tekst.size(); i++)
    {
        if(dlugi_tekst[i]=='&' && zapisuj == true)
        {
            poprawny=true;
            usun=i+1;
            break;
        }
        if (zapisuj==true)
            komunikat+=dlugi_tekst[i];
        if (dlugi_tekst[i]=='#')
        {
            zapisuj=true;
        }
    }
    if(poprawny==true)
    {
        dlugi_tekst.erase(0,usun);
        cout << "Komunikat: " << komunikat << endl;
        poprawny=false;
    }
    QByteArray byteArray(komunikat.c_str(), komunikat.length());
    return byteArray;

}

void Menu::readData()
{
     QByteArray komunikat="";
         QByteArray temp = tcpSocket->read(10);
         //cout << dlugi_tekst << " " << temp.toStdString() << endl;
         dlugi_tekst+=temp.toStdString();

         komunikat = scalanie();
         cout << komunikat.toStdString() <<endl;
         QList<QByteArray> qlist = komunikat.split(';');

         int numer = qlist[0].toInt();
         QByteArray tresc = qlist[1];
         switch(numer)
         {
             case 0:
            {
                 (tresc=="0") ? odp_polaczono=0 : odp_polaczono=1;
                 break;
            }
            case 1:
            {
                 cout << "Tresc: " << tresc.toStdString() <<endl;
                 lista_graczy = QByteArray(tresc);
                 cout <<"Lista graczy: " << lista_graczy.toStdString()<< endl;
                 odp_lista=0;
                 break;
            }
            default: break;
        }
    //}
    //while(komunikat!="");

}


char* IntToChar(qint16 poz_x, qint16 poz_y) //Use qint32 to ensure that the number have 4 bytes
{
    string temp = to_string(poz_x) + ';' + to_string(poz_y);
    char * wspolrzedne = new char[temp.size() + 1];
    copy(temp.begin(), temp.end(), wspolrzedne);
    wspolrzedne[temp.size()] = '\0';
    return wspolrzedne;
}

