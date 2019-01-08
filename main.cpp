#include "mbed.h"
#include <iostream>

//#include "calcul.cpp"
#include "matrix.h"
using namespace std;


#define NJMAX 8
#define NHMAX 12
#define TMAX NJMAX*24

DigitalOut myled(LED1);
Serial pc(SERIAL_TX,SERIAL_RX);
Timer t;



int puiss[TMAX];  // M士orisation de la puissance
float alpha[NHMAX];
float beta[NJMAX];

int n; // temps
int nh=6; // nombre d'heures dans le pr仕icteur
int nj=1; //nombre de jours dans le pr仕icteur
int N=nj+nh;


Matrix X(N,1);
Matrix W(N,1);
Matrix R(N,N);
Matrix r(N,1);
Matrix M(N,N);
float gam=0.12;
Matrix Id;

int circ(int i)
{
    if (i<0) i=TMAX+i;
    return i;
    }


// calcul de la pr仕iction
float prediction(int nh, int nj, int n)
{
    float resu=0;
    for (int i=1; i<=nh; i++) resu+=alpha[i-1]*puiss[circ(n-i)];
    for (int i=1; i<=nj; i++) resu+=beta[i-1]*puiss[circ(n-i*24)];
    return resu;
}


// scenario simple pour g始屍er les 残hantillons de puissance
int h=0;

int nextPuiss()
{
    int hj=h % 24;
    int resu;
    if ((hj>8)&&(hj<20)) resu=(int)(sin(3.14*(hj-8)/12.0)*600.0);
    else resu=0;
    h++;
    return resu;
}
// Calcul

Matrix eye(int n)
{
    return Matrix::createIdentity(n);
}

void setX()
{
     for (int i=1; i<=nh; i++) X(i-1,0)=puiss[circ(n-i)];
    for (int i=1; i<=nj; i++) X(nh+i-1,0)=puiss[circ(n-i*24)];
}


void setCoefs()
{
     for (int i=1; i<=nh; i++) alpha[i-1]=W(i-1,0);
    for (int i=1; i<=nj; i++) beta[i-1]=W(nh+i-1,0);
}


int main() {

    pc.baud(9600);
    Id=eye(N);
    R=Id;
    float duree=0;
    int nm=0;
    float t1;
    float t2;
    float t3;




   for (n=0; n<TMAX; n++) puiss[n]=nextPuiss();
   n=0;
    while(1) {
            pc.printf("n=%d\n",n);
        // Echantillon de puissance
        puiss[n]=nextPuiss();
          pc.printf("puiss=%d\n", puiss[n]);
        // Calcul des coefficients
        myled=1;
        wait(0.1);
        t.reset();
        t.start();
        setX(); // calcul de X
        //cout << X;
        R=gam*R+X*X.transpose();
        r=gam*r+X*(double)puiss[n];
        M=R+0.1*Id;
        t1=t.read();
        W=Matrix::solve(M,r);
        t2=t.read();
        // cout << "X=" << X.transpose();
       // cout << "W=" << W.transpose();
        setCoefs();
        // calcul de la pr仕iction
        float pe=prediction(nh,nj,n);
        t.stop();
        float d=t.read();
        duree+=d;
        nm++;
         pc.printf("Estimation %f ms\n", t1*1000);
          pc.printf("Resolution %f ms\n", (t2-t1)*1000);
           pc.printf("Prediction %f ms\n", (d-t2)*1000);
           pc.printf("duree %f ms\n", d*1000);
        pc.printf("Valeur moyennes %f ms\n", duree/nm*1000.0);
        myled=0;
        wait(0.8);
        pc.printf("pe=%f\n", pe);

        n++;
        if (n>=TMAX) n=0;

    }
}
