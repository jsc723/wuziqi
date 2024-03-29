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
class Info;
class RowInfo;
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
struct Tuple3
{
    int x;
    int y;
    int z;
    Tuple3(): x(0), y(0), z(0) {}
    Tuple3(int x, int y, int z): x(x), y(y), z(z) {}
};
inline bool operator<(const Tuple3 &t1, const Tuple3 &t2) {
    if (t1.x != t2.x) return t1.x < t2.x;
    if (t1.y != t2.y) return t1.y < t2.y;
    return t1.z < t2.z;
}
inline string spaceStr(int k) {
    string s;
    for(int i = 0; i < k; i++) {
        s += "#";
    }
    return s;
}

//------------------------------------RowInfo------------------------------------------------//
class Info
{
public:
    Info()
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
    int getTwo(int player) {return player == USR ? twoUSR : twoCOM;}
    int getSleepThree(int player) {return player == USR ? sleepThreeUSR : sleepThreeCOM;}
    int getAliveThree(int player) {return player == USR ? aliveThreeUSR : aliveThreeCOM;}
    int getSleepFour(int player) {return player == USR ? sleepFourUSR : sleepFourCOM;}
    int getAliveFour(int player) {return player == USR ? aliveFourUSR : aliveFourCOM;}
    int getFive(int player) {return player == USR ? fiveUSR : fiveCOM;}

protected:
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


class RowInfo : public Info
{
public:
    friend class BoardInfo;
    RowInfo(int t[R][R],Point start,Direction d);
    int *getData();
    int *tempArray(int RowInfo[],int player);
    int numOfTwo(int temp[],int player);
    int numOfSleepThree(int temp[],int player);
    int numOfAliveThree(int temp[],int player);
    int numOfSleepFour(int temp[],int player);
    int numOfAliveFour(int temp[],int player);
    int numOfFive(int temp[],int player);
    void flash(BoardInfo *info);
    int getLength(){return length;}
    int getNonEmptyCount() {return nonEmpty;}
    void print();
    void clear();

    Point pts[R];
    int cachedData[R];
    int tempData[A];
    int *data;
    int (*parent)[R];
    int length;
private:
    int countNonEmpty();
    int nonEmpty;
};
class BoardInfo : public Info
{
public:
    friend class RowInfo;
    BoardInfo() :
        Info()
    {}

    void add(RowInfo* r) {
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
    void sub(RowInfo* r) {
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

    bool operator==(const BoardInfo &r) const {
        return twoUSR == r.twoUSR &&
        sleepThreeUSR == r.sleepThreeUSR  &&
        aliveThreeUSR == r.aliveThreeUSR  &&
        sleepFourUSR == r.sleepFourUSR  &&
        aliveFourUSR == r.aliveFourUSR  &&
        fiveUSR == r.fiveUSR  &&
        twoCOM == r.twoCOM  &&
        sleepThreeCOM == r.sleepThreeCOM  &&
        aliveThreeCOM == r.aliveThreeCOM  &&
        sleepFourCOM == r.sleepFourCOM  &&
        aliveFourCOM == r.aliveFourCOM  &&
        fiveCOM == r.fiveCOM;
    }


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
    bool putAndUpdateNeighbor(int x,int y,int player);
	bool del(int x,int y);
    bool del(Point p);
    bool delAndUpdateNeighbor(int x,int y);
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
    double thinkAbout(Point forcast[],int level,int nextPlayer,double parentExtreme, qint64 deadline);
    double score(int nextPlayer);
    double otherPlayer(int player) const
        {return player == COM ? USR : COM;}
    int nonEmptyCount(int x, int y);
    double lim(int three, int four, bool consertive);
    Point blockSleep4(int player, int line);
    Point simpleDirectWinForAlive3(int player, int line);
    Point simpleDirectWinFor4(int player, int line);
    bool VCF(int player, Point &result,int level, qint64 deadline);
    bool VCT(int player, Point &result, int level, qint64 deadline);
    bool preCheck(int *x, int *y, int player);
    void afterCheck(int *x, int *y, int player);
    bool tryVCF(int *x, int *y, int player);
    bool tryVCT(int *x, int *y, int player);
    void reset();
    int getValue(int x,int y){return table[x][y];}
    void dPrintVCF(vector<Point> &vcf, int player) const;
    char toColOnBoard(int col) const;
    int toColOnTable(char col) const;
    int toRowOnBoard(int row) const;
    int toRowOnTable(int row) const;
    int nextPlayer(int starter) const;

    BoardInfo getInfo();
    RowInfo *hash[H];
    Point steps[R*R];
	vector<Point> vcf,vcfu;
	int count;
    int displayCount;
    bool useDisplayCount;
    bool justHuiqi;
    //bool hasVCF;
	int first;
    bool dif;
    BoardInfo info;

    volatile Point nowWorkingOn;
    volatile double nowPercentage;

    static const int MAX_LEVEL = 5;
    int cur_level;
private:
    int table[R][R];
    int neighborCount[R][R]; //num of neighbor in radius 2
    static const qint64 MaxThinkTime = 10000; //in msec, 10s
    static const qint64 MaxVCTTime = 5000; //5s
};
vector<Point> &clean(vector<Point> &vcfForcast);





#endif
