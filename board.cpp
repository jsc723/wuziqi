#include"board.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<QDebug>
#include<sstream>
#include <QString>
#include <QDateTime>

Board::Board()
{
    int i;
    dif = true;

    count = 0;
    displayCount = 0;
    useDisplayCount = false;
    justHuiqi = false;
    info = BoardInfo();
    nowPercentage = 0.0;
    cur_level = MAX_LEVEL;
    for(int i= 0;i<225;i++) {
        table[i/R][i%R] = EMPTY;
        neighborCount[i/R][i%R] = 0;
    }

    for(i=0;i<R;i++)
        hash[i] = new RowInfo(table,Point(0,i),down);
    for(i=0;i<R;i++)
        hash[i+15] = new RowInfo(table,Point(i,0),rightt);
    for(i=0;i<11;i++)
        hash[i+30] = new RowInfo(table,Point(4+i,0),rightup);
    for(i=0;i<10;i++)
        hash[i+41] = new RowInfo(table,Point(14,1+i),rightup);
    for(i=0;i<11;i++)
        hash[i+51] = new RowInfo(table,Point(i,0),rightdown);
    for(i=0;i<10;i++)
        hash[i+62] = new RowInfo(table,Point(0,1+i),rightdown);
}
void Board::reset()
{
    count = 0;
    displayCount = 0;
    useDisplayCount = false;
    justHuiqi = false;
    nowPercentage = 0.0;
    cur_level = MAX_LEVEL;
    for(int i= 0;i<225;i++) {
        table[i/R][i%R] = EMPTY;
        neighborCount[i/R][i%R] = 0;
    }
    info = BoardInfo();
    for(int i = 0; i < H; i++) {
        hash[i]->clear();
    }
}

void Board::dPrintVCF(vector<Point> &vcf,int player) const
{
    int len = vcf.size();
    Point pt;
    for(int i=0;i<len;i++)
    {
        pt = vcf[i];
        qDebug("%s(%c%d)",player==USR?"USR":"COM",toColOnBoard(pt.x),toRowOnBoard(pt.y));
        player = player==USR?COM:USR;
    }
}
char Board::toColOnBoard(int col) const
{
    return (char)('A' + col);
}
int Board::toColOnTable(char col) const
{
    return (int)(col - 'A');
}
int Board::toRowOnBoard(int row) const
{
    return 15-row;
}
int Board::toRowOnTable(int row) const
{
    return 15-row;
}
int Board::nextPlayer(int starter) const
{
    int player = starter;
    if(count > 0) {
        Point lastPt = steps[count - 1];
        if(table[lastPt.x][lastPt.y] == starter)
            player = otherPlayer(starter);
    }
    return player;
}

void Board::print() const
{
    int i,j;
    int round = (count==0)?1:count;
    char s[1000];
    for(i=0;i<2*R+1;i++)//the whole figure has 7 lines and 7 columns
    {
        if(i%2==0) sprintf(s,"%s-------------------------------------------------------------",s); //print -------------- every other line
        else
        {
            for(j=0;j<2*R+1;j++)
            {
                if(j%2==0) // print | every other column
                    sprintf(s,"%s| ",s);
                else if(table[(i-1)/2][(j-1)/2]==EMPTY) //in the middle, print the thing in that position
                    sprintf(s,"%s  ",s);
                else if(table[(i-1)/2][(j-1)/2]==USR)
                    sprintf(s,"%sX%c",s,((i-1)/2==steps[round-1].x&&(j-1)/2==steps[round-1].y)?'^':' ');
                else if(table[(i-1)/2][(j-1)/2]==COM)
                    sprintf(s,"%sO%c",s,((i-1)/2==steps[round-1].x&&(j-1)/2==steps[round-1].y)?'^':' ');
            }
            sprintf(s,"%s%d",s,(i-1)/2);
        }
        sprintf(s,"%s\n",s);
    }
    for(j=0;j<R;j++) sprintf(s,"%s%3d ",s,j);
    sprintf(s,"%s\n",s);
    qDebug() << s;
}

bool Board::put(int x,int y,int player)
{
    if(inrangeR(x)&&inrangeR(y)&&table[x][y]==EMPTY)
    {

        table[x][y] = player;
        count++;
        flashHash(x,y);
        return true;
    }
    return false;
}
bool Board::putAndUpdateNeighbor(int x,int y,int player)
{
    if(put(x, y, player)) {
        int i,j;
        for(i=std::max(0,x-2);i<=std::min(R-1,x+2);i++) {
            for(j=std::max(0,y-2);j<=std::min(R-1,y+2);j++) {
                neighborCount[i][j]++;
            }
        }
        return true;
    }
    return false;
}
bool Board::put(Point p, int player)
{
    return put(p.x,p.y,player);
}

//仅供调试使用
bool Board::put(char *s,int player)
{
    char c;
    int RowInfo,col;
    sscanf(s,"%[A-O]%d",&c,&RowInfo);
    col = c-65;
    return put(15-RowInfo,col,player);
}
bool Board::del(int x,int y)
{
    if(inrangeR(x)&&inrangeR(y)&&table[x][y]!=EMPTY)
    {
        table[x][y] = EMPTY;
        count--;
        flashHash(x,y);
        return true;
    }
    return false;
}
bool Board::delAndUpdateNeighbor(int x,int y)
{
    if(del(x, y)) {
        int i,j;
        for(i=std::max(0,x-2);i<=std::min(R-1,x+2);i++) {
            for(j=std::max(0,y-2);j<=std::min(R-1,y+2);j++) {
                neighborCount[i][j]--;
            }
        }
        return true;
    }
    return false;
}
bool Board::del(Point p)
{
    return del(p.x,p.y);
}

void Board::huiqi(bool once)
{
    if(count-1>=0){
        delAndUpdateNeighbor(steps[count-1].x, steps[count-1].y);
    }
    if(count-1>=0&&!once) {
        delAndUpdateNeighbor(steps[count-1].x, steps[count-1].y);
    }
    justHuiqi = true;
}
void Board::userInput()
{
    int x=-2,y=-2;
    while(true)
    {
        printf("Enter RowInfo and column.\n");
        scanf("%d%d",&x,&y);
        if(inrangeR(x)&&inrangeR(y)&&getValue(x,y)==EMPTY) break;
        else if(x==-1&&y==-1&&count>=2)
        {
            huiqi();
            goto done;
        }
        printf("Retry:\n");
    }
    steps[count] = Point(x,y);
    putAndUpdateNeighbor(x,y,USR);
done:
    print();
}

/*----------------------------------------compInput------------------------------------------------------*/
//the computer's input
void Board::compInput()
{
    int x,y;
    double score;
    BoardInfo infoSaved = getInfo();
    int player = nextPlayer(COM);
    if(count==0){x=7;y=7;}
    else if(count==1) {round2(&x, &y);}
    else if(count==2) {round3(&x, &y);}
    else if(preCheck(&x, &y, player)){ }
    else if(dif && tryVCF(&x, &y, player)) {
        qDebug("find VCF");
    }
    else if(dif && tryVCT(&x, &y, player)) {
        qDebug("find VCT");
    }
    else
    {
        Point forcast[60];
        int extreme = player == COM ? INT_MAX : INT_MIN;
        qDebug() << "cur level = " << cur_level;
        qint64 deadline = QDateTime::currentMSecsSinceEpoch() + MaxThinkTime;
        score = thinkAbout(forcast, cur_level, player, extreme, deadline);
        x = forcast[cur_level].x;
        y = forcast[cur_level].y;

        if (nowPercentage < 90) {
            cur_level = max(3, cur_level - 2);
            deadline = QDateTime::currentMSecsSinceEpoch() + MaxThinkTime;
            score = thinkAbout(forcast, cur_level, player, extreme, deadline);
            x = forcast[cur_level].x;
            y = forcast[cur_level].y;
        }

        nowPercentage = 0;

        if(score<-5000)
        {
            afterCheck(&x,&y, player);
        }
    }

    BoardInfo infoNow = getInfo();

    if (!(infoSaved == infoNow)) {
        qDebug() << "\n!!!!info not consistent!!!!\n";
    }


    steps[count] = Point(x,y);
    putAndUpdateNeighbor(x,y,player);
    infoNow = getInfo();
    qDebug("%d %s : (%c,%d)",count,player == COM ? "COM":"USR",
           toColOnBoard(x),toRowOnBoard(y));
    infoNow.showInfo();
}
void Board::round2(int *x,int *y)
{
    srand((unsigned)time(NULL));
    int ran,m = steps[count-1].x,n=steps[count-1].y;
    ran = rand();
    if(ran%2==1)
    {
        if(inRange(m-1,n-1)){ *x=m-1;*y=n-1;}
        else if(inRange(m-1,n+1)){ *x=m-1;*y=n+1;}
        else if(inRange(m+1,n+1)) { *x=m+1;*y=n+1;}
        else if(inRange(m+1,n-1)) { *x=m+1;*y=n-1;}
    }
    else
    {
        if(inRange(m-1,n))  { *x=m-1;*y=n;}
        else if(inRange(m,n-1))  { *x=m;*y=n-1;}
        else if(inRange(m+1,n)) { *x=m+1;*y=n;}
        else if(inRange(m,n+1))  { *x=m;*y=n+1;}
    }
}
void Board::round3(int *x,int *y)
{
    int ran,i,j;
    while(true)
    {
        srand((unsigned)time(NULL));
        ran = rand();
        ran %= 25;
        for(i=5;i<10;i++)
        {
            for(j=5;j<10;j++)
            {
                if((ran==5*(i-5)+(j-5))&&table[i][j]==EMPTY)
                {
                    *x=i;*y=j;return;
                }
            }
        }
    }
}
bool Board::preCheck(int *x, int *y, int player)
{
    if(info.getAliveFour(player)+info.getSleepFour(player)>0) //go five
    {
        for(int i=0;i<R;i++)for(int j=0;j<R;j++)
        {
            if(put(i,j,player))
            {
                if(info.getFive(player))
                {
                    del(i,j);
                    *x = i;
                    *y = j;
                    return true;
                }
                del(i,j);
            }
        }
    }
    int other = otherPlayer(player);
    //block other's four
    if(info.getSleepFour(other)+info.getAliveFour(other)>0 )
    {
        int n = info.getSleepFour(other)+2*info.getAliveFour(other);
        for(int i=0;i<R;i++)for(int j=0;j<R;j++)
        {
            if(put(i,j,player))
            {
                if( info.getSleepFour(other)+2*info.getAliveFour(other) < n)
                {
                    del(i,j);
                    *x = i;
                    *y = j;
                    return true;
                }
                del(i,j);
            }
        }
    }
    if(info.getAliveThree(player)>0&&(info.getSleepFour(other)+info.getAliveFour(other)==0))
    {
        for(int i=0;i<R;i++)for(int j=0;j<R;j++)
        {
            if(put(i,j,player))
            {
                if( info.getAliveFour(player) )
                {
                    del(i,j);
                    *x = i;
                    *y = j;
                    return true;
                }
                del(i,j);
            }
        }
    }
    return false;
}

void Board::afterCheck(int *x, int *y, int player)
{
    BoardInfo info;
    int other = otherPlayer(player);
    for(int i=0;i<R;i++)for(int j=0;j<R;j++)
    {
        if(put(i,j,other))
        {
            if(info.getAliveFour(other)>=1)
            {
                *x=i;*y=j;
                del(i,j);
                return;
            }
            del(i,j);
        }
    }
    for(int i=0;i<R;i++)for(int j=0;j<R;j++)
    {
        if(put(i,j,other))
        {
            if(info.getSleepFour(other)+info.getAliveThree(other)>=2)
            {
                *x=i;*y=j;
                del(i,j);
                return;
            }
            del(i,j);
        }
    }
}

bool Board::tryVCF(int *x, int *y, int player) {
    Point vcf;
    if(VCF(player, vcf,10,QDateTime::currentMSecsSinceEpoch() + MaxVCTTime))
    {
        *x = vcf.x;
        *y = vcf.y;
        return true;
    }
    return false;
}
bool Board::tryVCT(int *x, int *y, int player) {
    Point vct;
    if(VCT(player,vct,6,QDateTime::currentMSecsSinceEpoch() + MaxVCTTime))
    {
        *x = vct.x;
        *y = vct.y;
        return true;
    }
    return false;
}


//------------------------------------------------------------------computer input END
double Board::thinkAbout(Point forcast[],int level,int nextPlayer,double parentExtreme, qint64 deadline)
{
    if(level==0)
    {
        return score(nextPlayer);
    }
    else if(winner()!=0)
        return (winner()==COM)?20000:-20000;
    else if(isFull())
        return 0;

    else if(nextPlayer==COM) //next is computer, maximizing
    {
        double max=INT_MIN;
        double temp;
        vector<Tuple3> orders;
        for(int i=0;i<R;i++) for(int j=0;j<R;j++)
        {
            if(table[i][j] == EMPTY && !farAway(i,j))
            {
                orders.push_back(Tuple3(-nonEmptyCount(i, j), i, j));
            }
        }
        sort(orders.begin(), orders.end());
        for(int k = 0; k < orders.size(); k++)
        {
            int i = orders[k].y, j = orders[k].z;
            put(i,j,COM);

            bool usrVCT = false;
            if (level == cur_level) {
                nowWorkingOn.x = i;
                nowWorkingOn.y = j;
                nowPercentage = 100.0*k / orders.size();

                Point vct;
                qint64 timeRemain = deadline - QDateTime::currentMSecsSinceEpoch();
                qDebug() << toColOnBoard(i) << toRowOnBoard(j);

                if (dif && timeRemain > 1000 && VCT(USR, vct, 3, deadline + MaxVCTTime)) {
                    usrVCT = true;
                    qDebug() << toColOnBoard(i) << toRowOnBoard(j) << "cannot block usrVCT";
                }
            }
            temp = thinkAbout(forcast,level-1,USR,max,deadline);
            if (usrVCT) {
                temp -= 1000;
            }
            if(noNeighbor(i,j))
                temp-=3;
            temp -= closeToBoundary(i,j)*10;

            if (level == cur_level) {
                qDebug() << toColOnBoard(i) << toRowOnBoard(j) << temp;
            }

            if(temp>max)
            {
                max = temp;
                forcast[level] = Point(i,j);
            }
            com_done:
            del(i,j);

            if(max>=parentExtreme)
                return max;

            qint64 cur = QDateTime::currentMSecsSinceEpoch();

            if (cur >= deadline) {

                return max;
            }
        }
        if (level == cur_level) {
            qDebug() << "result: " << toColOnBoard(forcast[level].x) << toRowOnBoard(forcast[level].x) << max;
        }
        return max;
    }
    else if(nextPlayer==USR) //next is user, minimizing
    {
        double min=INT_MAX;
        double temp;
        vector<Tuple3> orders;
        for(int i=0;i<R;i++) for(int j=0;j<R;j++)
        {
            if(table[i][j] == EMPTY && !farAway(i,j))
            {
                orders.push_back(Tuple3(-nonEmptyCount(i, j), i, j));
            }
        }
        sort(orders.begin(), orders.end());
        for(int k = 0; k < orders.size(); k++)
        {
            int i = orders[k].y, j = orders[k].z;
            {
                put(i,j,USR);
                temp = thinkAbout(forcast,level-1,COM,min,deadline);
                if(noNeighbor(i,j))
                    temp+=3;
                temp += closeToBoundary(i,j)*10;
                if(temp<min)
                {
                    min = temp;
                    forcast[level] = Point(i,j);
                }
                del(i,j);
                if(min<=parentExtreme)
                    return min;
            }
        }
        return min;
    }
    return 0;
}

double Board::score(int nextPlayer)
{
    double score=0;
    double k;
    int player = otherPlayer(nextPlayer);
    bool conservative = (first == nextPlayer && count<20);

    if(conservative)
        k = 2.0 - count * 0.02;
    else
        k = 1.3;

    score +=   (6*info.getTwo(player) + 6*info.getSleepThree(player))
            -  k * (6*info.getTwo(nextPlayer)  +  6*info.getSleepThree(nextPlayer));

    if(info.getFive(player)>0)
        return score + 20000;
    if(info.getAliveFour(nextPlayer) + info.getSleepFour(nextPlayer)>0)
        return score - 20000;
    if(info.getAliveFour(player)>0)
        return score + 10000;

    if(info.getAliveThree(nextPlayer) > 0) {
        if(info.getSleepFour(player) == 0) {
            return score - 20000;
        } else {
            score -= 10;
        }
    }

    score += lim(info.getAliveThree(player), info.getSleepFour(player), conservative);


    return player == COM ? score : -score;
}

vector<Point> &clean(vector<Point> &vcfForcast) {
    vcfForcast.clear();
    return vcfForcast;
}

double Board::lim(int three,int four, bool conservative)
{
    if(three==1&&four==0) return conservative?-10:-5;
    if(three==0&&four==1) return conservative?-30:-10;
    if(three>=2&&four==0) return 5000;
    if(three + four >= 2) return 15000;
    return 0;
}

//block player's sleep4 on line hash[i]
Point Board::blockSleep4(int player, int line) {
    int other = otherPlayer(player);
    if (line == -1) {
        for(line = 0; line < H; line++) {
            if (hash[line]->getSleepFour(player) > 0) {
                break;
            }
        }
    }
    Point pt2(-1, -1);
    for(int k = 0; k < hash[line]->length;k++) //对方堵四
    {
        pt2 = hash[line]->pts[k];
        if(put(pt2.x,pt2.y,other))//若此处为空则下子
        {
            if(hash[line]->getSleepFour(player)==0) //若刚刚一步堵掉了冲四
            {
                return pt2;
            }
            del(pt2.x,pt2.y);//出栈
        }
    }
    //should not reach
    return pt2;
}

Point Board::simpleDirectWinForAlive3(int player, int line) {
    if (line == -1) {
        for(line = 0; line < H; line++) {
            if (hash[line]->getAliveThree(player) > 0) {
                break;
            }
        }
    }
    Point pt2(-1, -1);
    for(int k = 0; k < hash[line]->length;k++)
    {
        pt2 = hash[line]->pts[k];
        if(put(pt2.x,pt2.y, player))
        {
            if(hash[line]->getAliveFour(player)>=1)
            {
                del(pt2.x,pt2.y);
                return pt2;
            }
            del(pt2.x,pt2.y);
        }
    }
    //should not reach
    qDebug("invalid pt alive 3");
    return pt2;
}

Point Board::simpleDirectWinFor4(int player, int line) {
    if (line == -1) {
        for(line = 0; line < H; line++) {
            if (hash[line]->getAliveFour(player) + hash[line]->getSleepFour(player) > 0) {
                break;
            }
        }
    }
    Point pt2(-1, -1);
    for(int k = 0; k < hash[line]->length;k++)
    {
        pt2 = hash[line]->pts[k];
        if(put(pt2.x,pt2.y, player))
        {
            if(hash[line]->getFive(player)>=1)
            {
                del(pt2.x,pt2.y);
                return pt2;
            }
            del(pt2.x,pt2.y);
        }
    }
    //should not reach
    qDebug("invalid pt 4");
    return pt2;
}

bool Board::VCF(int player, Point &result,int level, qint64 deadline) {
    int other = otherPlayer(player);
    QString prefix = QString::fromStdString(spaceStr(level));
    int i,j;
    if(isFull())
        return false;
    BoardInfo info = getInfo();
    if(info.getFive(player) > 0) {
        //qDebug() << prefix << "VCF due to five";
        return true;
    }
    if(info.getAliveFour(player) + info.getSleepFour(player) > 0) {
        //qDebug() << prefix << "VCF due to four";
        result = simpleDirectWinFor4(player, -1);
        return true;
    }
    if(info.getAliveFour(other) + info.getSleepFour(other) > 0)
        return false;
    if(info.getAliveThree(player) > 0) {
        //qDebug() << prefix << "VCF due to direct alive three";
        result = simpleDirectWinForAlive3(player, -1);
        return true;
    }
    if(level <= 0)
        return false;
    if (QDateTime::currentMSecsSinceEpoch() >= deadline) {
        return false;
    }
    for(i=0;i<H;i++)
    {
        if(hash[i]->getSleepThree(player) == 0)
        {
            continue; //忽略没有眠三的线
        }

        Point pt,pt2;
        for(j=0;j < hash[i]->length;j++)
        {
            pt = hash[i]->pts[j];
            if(put(pt.x,pt.y,player)) //找所有能冲四的点
            {
                if(hash[i]->getSleepFour(player)>=1) //冲四
                {
                    pt2 = blockSleep4(player, i);
                    Point nextLevelResult;
                    if(VCF(player,nextLevelResult,level-1,deadline))//递归，计算下一步冲四
                    {
                        del(pt2.x,pt2.y);//出栈
                        del(pt.x,pt.y);
                        result = pt;
                        //qDebug() << prefix << "VCF at " << toColOnBoard(result.x) << toRowOnBoard(result.y);
                        return true;
                    }
                    del(pt2.x,pt2.y);//出栈
                }
                del(pt.x,pt.y);//出栈
            }
        }
    }
    return false;
}



bool Board::VCT(int player, Point &result, int level, qint64 deadline) {
    int i,j;
    int other = otherPlayer(player);
    Point vcf;
    QString prefix = QString::fromStdString(spaceStr(level));

    if(VCF(player, result, level, deadline)) {
        //qDebug() << prefix << "VCT:VCF " << toColOnBoard(result.x) << toRowOnBoard(result.y);
        return true;
    }
    if(level<=0)
        return false;
    if(info.getAliveFour(other) + info.getSleepFour(other) + info.getAliveThree(other) > 0)
        return false;
    bool otherHasVCF = VCF(other, vcf,4, deadline + MaxVCTTime);
    if (otherHasVCF) {
        return false;
    }
    if (QDateTime::currentMSecsSinceEpoch() >= deadline) {
        return false;
    }

    for(i=0;i<H;i++)
    {
        if(hash[i]->getTwo(player))
        {
            Point pt,pt2;
            //qDebug() << prefix << "hash = " << i;
            for(j=0;j < hash[i]->length;j++)
            {
                pt = hash[i]->pts[j];
                if(put(pt.x,pt.y,player))
                {
                    if(hash[i]->getAliveThree(player) >=1) //冲三
                    {
                        //qDebug() << prefix << "check " << toColOnBoard(pt.x) << toRowOnBoard(pt.y);
                        bool blocked = false;

                        for(int k = 0;k < hash[i]->length && !blocked;k++) //对方堵
                        {
                            pt2 = hash[i]->pts[k];
                            if(put(pt2.x,pt2.y,other))//若此处为空则下子
                            {
                                //若刚刚一步堵掉了冲三
                                if(hash[i]->getAliveThree(player) ==0)
                                {
                                    //qDebug() << prefix << "try block at " << toColOnBoard(pt2.x) << toRowOnBoard(pt2.y);
                                    //递归，计算下一步冲三
                                    Point nextResult;
                                    if(!VCT(player, nextResult,level-1,deadline))
                                    {
                                        //qDebug() << prefix << "blocked at " << toColOnBoard(pt2.x) << toRowOnBoard(pt2.y);
                                        blocked = true;
                                    }
                                }
                                del(pt2);//出栈
                            }
                        }

                        if(!blocked) //如果对方堵不掉
                        {
                            del(pt);
                            result = pt;
                            //qDebug() << prefix << "VCT: " << toColOnBoard(result.x) << toRowOnBoard(result.y);
                            return true;
                        }
                    }
                    del(pt);//出栈
                }
            }
        }
    }
    return false;

}


/*----------------------------------------farAway------------------------------------------------------*/
bool Board::farAway(int m,int n)
{
    return neighborCount[m][n] == 0;
}
bool Board::noNeighbor(int m,int n)
{
    int i,j;
    for(i=std::max(0,m-1);i<=std::min(R-1,m+1);i++) {
        for(j=std::max(0,n-1);j<=std::min(R-1,n+1);j++) {
            if(table[i][j] != EMPTY) {
                return false;
            }
        }
    }
    return true;
}

inline int Board::closeToBoundary(int m, int n)
{
    if(m==0||m==R-1||n==0||n==R-1) return 2;
    if(m==1||m==R-2||n==1||n==R-2) return 1;
    return 0;
}
//-------------------------------------------------------------------------------------------------------END
void Board::flashHash()
{
    int i;
    for(i=0;i<H;i++)
        hash[i]->flash(&info);
}

void Board::flashHash(int x,int y)
{
    hash[y]->flash(&info);
    hash[x+15]->flash(&info);
    int sum = x+y,diff = x-y;
    if(sum>=4&&sum<=24)
        hash[26+sum]->flash(&info);
    if(diff>=0&&diff<=10)
        hash[51+diff]->flash(&info);
    else if(diff>=-10&&diff<0)
        hash[61+(-1)*diff]->flash(&info);
}

int Board::nonEmptyCount(int x, int y) {
    int cnt = 0;
    cnt += hash[y]->getNonEmptyCount();
    cnt += hash[x+15]->getNonEmptyCount();
    int sum = x+y,diff = x-y;
    if(sum>=4&&sum<=24)
        cnt += hash[26+sum]->getNonEmptyCount();
    if(diff>=0&&diff<=10)
        cnt += hash[51+diff]->getNonEmptyCount();
    else if(diff>=-10&&diff<0)
        cnt += hash[61+(-1)*diff]->getNonEmptyCount();
    return cnt;
}

BoardInfo Board::getInfo()
{
    return info;
}
int Board::winner()
{
    for(int i=0;i<H;i++)
    {
        if(hash[i]->getFive(COM)>0)
            return COM;
        if(hash[i]->getFive(USR)>0)
            return USR;
    }
    return 0;
}
//------------------------------------RowInfo-------------------------------------------------//
RowInfo::RowInfo(int t[R][R],Point start,Direction d) :
    Info()
{
    int i,j,k;
    for(i=0;i<R;i++)
        pts[i].clear();
    i=0,j=0,k=0;
    switch(d)
    {
    case down:
        while(inrangeR(start.x+i)&&inrangeR(start.y))
        {
            pts[k].x = start.x+i;
            pts[k].y = start.y;
            i++;
            k++;
        }
        break;
    case rightt:
        while(inrangeR(start.x)&&inrangeR(start.y+j))
        {
            pts[k].x = start.x;
            pts[k].y = start.y+j;
            j++;
            k++;
        }
        break;
    case rightup:
        while(inrangeR(start.x-i)&&inrangeR(start.y+j))
        {
            pts[k].x = start.x-i;
            pts[k].y = start.y+j;
            i++;
            j++;
            k++;
        }
        break;
    case rightdown:
        while(inrangeR(start.x+i)&&inrangeR(start.y+j))
        {
            pts[k].x = start.x+i;
            pts[k].y = start.y+j;
            i++;
            j++;
            k++;
        }
        break;
    }
    parent = t;
    data = 0;
    length = k;
    nonEmpty = 0;
}
void RowInfo::flash(BoardInfo* info)
{
    info->sub(this);
    data = getData();
    int *tempUSR= tempArray(data,USR);

    twoUSR = numOfTwo(tempUSR,USR);
    sleepThreeUSR = numOfSleepThree(tempUSR,USR);
    aliveThreeUSR = numOfAliveThree(tempUSR,USR);
    sleepFourUSR = numOfSleepFour(tempUSR,USR);
    aliveFourUSR = numOfAliveFour(tempUSR,USR);
    fiveUSR = numOfFive(tempUSR,USR);

    int *tempCOM = tempArray(data,COM);
    twoCOM = numOfTwo(tempCOM,COM);
    sleepThreeCOM = numOfSleepThree(tempCOM,COM);
    aliveThreeCOM = numOfAliveThree(tempCOM,COM);
    sleepFourCOM = numOfSleepFour(tempCOM,COM);
    aliveFourCOM = numOfAliveFour(tempCOM,COM);
    fiveCOM = numOfFive(tempCOM,COM);

    nonEmpty = countNonEmpty();
    info->add(this);
}
int *RowInfo::getData()
{
    int i;
    for(i=0;i<length;i++)
        cachedData[i] = parent[pts[i].x][pts[i].y];
    return cachedData;
}
int *RowInfo::tempArray(int data[],int player)
{
    int i;
    for(i=0;i<A;i++)
        tempData[i] = WALL;
    for(i=0;i<length;i++)
        tempData[i+1] = (  data[i]!=player  &&  data[i]!=EMPTY  )?WALL:data[i];
    return tempData;
}
void RowInfo::print()
{
    stringstream ss;
    string s;
    int *data = getData();
    int *tempCOM = tempArray(data,COM);
    for(int i=0;i<A;i++)
    {
        ss<<tempCOM[i];
    }
    ss>>s;
    qDebug() << s.c_str();
}

void RowInfo::clear()
{
    data = getData();
    twoUSR = 0;
    sleepThreeUSR = 0;
    aliveThreeUSR = 0;
    sleepFourUSR = 0;
    aliveFourUSR = 0;
    fiveUSR = 0;
    twoCOM = 0;
    sleepThreeCOM = 0;
    aliveThreeCOM = 0;
    sleepFourCOM = 0;
    aliveFourCOM = 0;
    fiveCOM = 0;
    nonEmpty = 0;
}
int RowInfo::numOfTwo(int temp[],int player)
{
    int i=0,num=0;
    int flag = 0;
    for(i=2;i<A-2;i++)
    {
        if(temp[i]==player)
        {
            if(temp[i+1]==player)
            {
                if(temp[i-1]!=EMPTY||temp[i+2]!=EMPTY)
                    continue;
                else if((inrangeA(i-3)&&temp[i-3]==player)||(inrangeA(i+4)&&temp[i+4]==player))
                    continue;
                else if(temp[i-2]==player||(inrangeA(i+3)&&temp[i+3]==player))
                    continue;
                else if (temp[i-2]==EMPTY&&(inrangeA(i+3)&&temp[i+3]==EMPTY))
                {
                    num++; flag=1;
                }
                else if (temp[i-2]==WALL&&inrangeA(i+3)&&temp[i+3]==EMPTY&&inrangeA(i+4)&&temp[i+4]==EMPTY)
                {
                    num++; flag=1;
                }
                else if (inrangeA(i+3)&&temp[i+3]==WALL&&temp[i-2]==EMPTY&&temp[i-1]==EMPTY)
                {
                    num++; flag = 1;
                }
            }
            else if(temp[i+1]==EMPTY&&temp[i+2]==player)
            {
                if(temp[i-1]!=EMPTY||(inrangeA(i+3)&&temp[i+3]!=EMPTY))
                    continue;
                else if(temp[i-2]==player||(inrangeA(i+4)&&temp[i+4]==player))
                    continue;
                else if(temp[i-2]==EMPTY||(inrangeA(i+4)&&temp[i+4]==EMPTY))
                {
                    num++; flag = 1;
                }
            }
            else if(temp[i+1]==EMPTY&&temp[i+2]==EMPTY&&inrangeA(i+3)&&temp[i+3]==player)
            {
                if(temp[i-1]==EMPTY&&inrangeA(i+4)&&temp[i+4]==EMPTY)
                {
                    num++; flag = 1;
                }
            }
            if(flag)
            {
                do i++;
                while(i<A-2&&temp[i]!=player);
            }
            flag=0;
        }
    }
    return num;
}
int RowInfo::numOfSleepThree(int temp[],int player)
{
    int j,p3=player*player*player;
    int i=1,num=0;
    int value = 1;
    int flag = 0;
    while(i<A-5)
    {
        if(temp[i]!=WALL)
        {
            for(j=0;j<5;j++)
                value *= temp[i+j];
            if(value==p3)
            {
                if(temp[i]==player&&temp[i+4]==player)
                {
                    num++;
                    flag=1;
                }
                else if(temp[i-1]==WALL)
                {
                    if(temp[i+1]==player&&temp[i+2]==player&&temp[i+3]==player&&temp[i+5]==WALL)
                    {
                        num++;
                        flag=1;
                    }
                    else if(temp[i]==player)
                    {
                        num++;
                        flag = 1;
                    }
                    else if(temp[i+5]==WALL&&temp[i+4]==player)
                    {
                        num++;
                        flag = 1;
                    }
                }
                else if(temp[i+5]==WALL&&temp[i+4]==player)
                {
                    num++;
                    flag = 1;
                }
            }
        }
        while(flag&&i<A-5&&temp[i]==player)
            i++;
        i++;
        flag=0;
        value = 1;
    }
    return num;
}
int RowInfo::numOfAliveThree(int temp[],int player)
{
    int i=0,num=0,p2 = player*player;
    for(i=2;i<A-3;i++)
    {
        if(temp[i]==player&&temp[i+1]*temp[i+2]*temp[i+3]==p2)
        {
            if(temp[i+3]==EMPTY&&temp[i-1]==EMPTY)
            {
                if((inrangeA(i-2)&&temp[i-2]==player)||(inrangeA(i+4)&&temp[i+4]==player))
                    continue;
                else if((inrangeA(i-2)&&temp[i-2]==EMPTY)||(inrangeA(i+4)&&temp[i+4]==EMPTY))
                    num++;
            }
            else if(temp[i-1]==EMPTY&&inrangeA(i+4)&&temp[i+4]==EMPTY)
                num++;
        }
    }
    return num;
}
int RowInfo::numOfSleepFour(int temp[],int player)
{
    int p2=player*player;
    int i=1,num=0;
    int flag = 0;
    while(i<A-4)
    {
        if(temp[i]==player)
        {
            if(temp[i+4]==player&&(temp[i+1]*temp[i+2]*temp[i+3]==p2))
            {
                num++; flag = 1;
            }
            else if(temp[i+1]==player&&temp[i+2]==player&&temp[i+3]==player&&((temp[i-1]==WALL)^(temp[i+4]==WALL)))
            {
                num++; flag = 1;
            }
        }
        while(flag&&i<A-4&&temp[i]==player)
            i++;
        i++;
        flag=0;
    }
    return num;
}
int RowInfo::numOfAliveFour(int temp[],int player)
{
    int i=0,p3 = player*player*player;
    int value = 1;
    for(i=0;i<A-4;i++)
    {
        if(temp[i]==player)
        {
            value = temp[i+1]*temp[i+2]*temp[i+3];
            if(value == p3&&temp[i-1]==EMPTY&&temp[i+4]==EMPTY)
            {
                return 1;
            }
        }
    }
    return 0;
}
int RowInfo::numOfFive(int temp[],int player)
{
    int i=0,p4 = player*player*player*player;
    int value = 1;
    for(i=0;i<A-5;i++)
    {
        if(temp[i]==player)
        {
            value = temp[i+1]*temp[i+2]*temp[i+3]*temp[i+4];
            if(value == p4)
            {
                return 1;
            }
        }
    }
    return 0;
}
int RowInfo::countNonEmpty() {
    int cnt = 0;
    for(int i = 0; i < length; i++) {
        if (data[i] != EMPTY) {
            cnt++;
        }
    }
    return cnt;
}
