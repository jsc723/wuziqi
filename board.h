#ifndef BOARD_H
#define BOARD_H

#include<stdio.h>
#include<vector>
#include<QDebug>

using namespace std;

const int EMPTY = 1;
const int USR = 2;
const int COM = 3;
const int WALL = 5;
const int A = 17;
const int R = 15;
const int H = 72;
inline int inrangeA(int i){return (i>0&&i<A);}
inline int inrangeR(int i){return (i>=0&&i<R);}
inline int inRange(int i,int j){ return (i>=0&&i<R&&j>=0&&j<R);}
class Row;
class BoardInfo;
enum Direction{rightt,down,rightup,rightdown};
class Board;
class Point
{
public:
	Point(){x=0;y=0;}
	Point(int a,int b){x=a;y=b;}
	void clear() {x=0;y=0;}
	int x;
	int y;
};
//------------------------------------Row------------------------------------------------//
class Row
{
public:
    Row(int t[R][R],Point start,Direction d);
    int *getData();
    int *tempArray(int row[],int player);
    int numOfTwo(int temp[],int player);
    int numOfSleepThree(int temp[],int player);
    int numOfAliveThree(int temp[],int player);
    int numOfSleepFour(int temp[],int player);
    int numOfAliveFour(int temp[],int player);
    int numOfFive(int temp[],int player);
    void flash(BoardInfo *info);
    int getLength(){return length;}
    void print();

    Point pts[R];
    int *data;
    int (*parent)[R];
    int length;
    int twoUSR;
    int sleepThreeUSR;
    int aliveThreeUSR;
    int sleepFourUSR;
    int aliveFourUSR;
    int fiveUSR;
    int twoCOM;
    int sleepThreeCOM;
    int aliveThreeCOM;
    int sleepFourCOM;
    int aliveFourCOM;
    int fiveCOM;
};
class BoardInfo
{
public:
	BoardInfo()
	{
		twoUSR=0;
		sleepThreeUSR=0;
		aliveThreeUSR=0;
		sleepFourUSR=0;
		aliveFourUSR=0;
		fiveUSR=0;
		twoCOM=0;
		sleepThreeCOM=0;
		aliveThreeCOM=0;
		sleepFourCOM=0;
		aliveFourCOM=0;
		fiveCOM=0;
	};
	void showInfo()
	{
        qDebug("twoUSR=%d\nsleepThreeUSR=%d\naliveThreeUSR=%d\nsleepFourUSR=%d\naliveFourUSR=%d\nfiveUSR=%d\ntwoCOM=%d\n\
sleepThreeCOM=%d\n\
aliveThreeCOM=%d\n\
sleepFourCOM=%d\n\
aliveFourCOM=%d\n\
fiveCOM=%d\n",\
		twoUSR,\
		sleepThreeUSR,\
		aliveThreeUSR,\
		sleepFourUSR,\
		aliveFourUSR,\
		fiveUSR,\
		twoCOM,\
		sleepThreeCOM,\
		aliveThreeCOM,\
		sleepFourCOM,\
		aliveFourCOM,\
		fiveCOM);
    }
    void add(Row* r) {
        twoUSR += r->twoUSR;
        sleepThreeUSR += r->sleepThreeUSR;
        aliveThreeUSR += r->aliveThreeUSR;
        sleepFourUSR += r->sleepFourUSR;
        aliveFourUSR += r->aliveFourUSR;
        fiveUSR += r->fiveUSR;
        twoCOM += r->twoCOM;
        sleepThreeCOM += r->sleepThreeCOM;
        aliveThreeCOM += r->aliveThreeCOM;
        sleepFourCOM += r->sleepFourCOM;
        aliveFourCOM += r->aliveFourCOM;
        fiveCOM += r->fiveCOM;
    }
    void sub(Row* r) {
        twoUSR -= r->twoUSR;
        sleepThreeUSR -= r->sleepThreeUSR;
        aliveThreeUSR -= r->aliveThreeUSR;
        sleepFourUSR -= r->sleepFourUSR;
        aliveFourUSR -= r->aliveFourUSR;
        fiveUSR -= r->fiveUSR;
        twoCOM -= r->twoCOM;
        sleepThreeCOM -= r->sleepThreeCOM;
        aliveThreeCOM -= r->aliveThreeCOM;
        sleepFourCOM -= r->sleepFourCOM;
        aliveFourCOM -= r->aliveFourCOM;
        fiveCOM -= r->fiveCOM;
    }

	int twoUSR;
	int sleepThreeUSR;
	int aliveThreeUSR;
	int sleepFourUSR;
	int aliveFourUSR;
	int fiveUSR;
	int twoCOM;
	int sleepThreeCOM;
	int aliveThreeCOM;
	int sleepFourCOM;
	int aliveFourCOM;
	int fiveCOM;
};


//----------------------------------------Board-----------------------------------------//
class Board
{
public:
    Board();
	void flashHash();
	void flashHash(int x,int y);
	bool isFull(){return count==225;}
	bool isEmpty(){return count==0;}
	bool put(int x,int y,int player);
    bool put(Point p,int player);
	bool put(char *s,int player);
	bool del(int x,int y);
    bool del(Point p);
	void userInput();
	void compInput();
	void round2(int *x,int *y);
	void round3(int *x,int *y);
    void huiqi(bool once = false);
	void print() const; 
	int winner();
	bool farAway(int m,int n);
	bool noNeighbor(int m,int n);
    int closeToBoundary(int m,int n);
	double thinkAbout(Point forcast[],int level,int nextPlayer,double parentExtreme);
    double thinkAbout2(Point forcast[], int level, int nextPlayer, double a, double b);
    double score(int nextPlayer);
	double scoreAsCOM();
	double scoreAsUSR();
	double lim(int three,int four);
	bool VCFCOM(vector<Point> &vcfForcast,int level);
	bool VCFUSR(vector<Point> &vcfForcast,int level);
    bool VCTCOM(vector<Point> &vcfForcast,int level);
    bool VCTUSR(vector<Point> &vcfForcast,int level);
    bool preCheck(int *x,int *y);
    void afterCheck(int *x, int *y);
    void reset();
    int getValue(int x,int y){return table[x][y];}
    void dPrintVCF(vector<Point> &vcf, int player) const;
    char toColOnBoard(int col) const;
    int toColOnTable(char col) const;
    int toRowOnBoard(int row) const;
    int toRowOnTable(int row) const;

	BoardInfo getInfo();
    BoardInfo getInfoAround(int x,int y);
	Row *hash[H];
    Point steps[R*R];
	vector<Point> vcf,vcfu;
	int count;
    bool justHuiqi;
	bool hasVCF;
	int first;
    bool dif;
    BoardInfo info;

    Point nowWorkingOn;

    static const int MAX_LEVEL = 3;
private:
	int table[R][R];

};




#endif
