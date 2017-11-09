#include"board.h"
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<QDebug>
#include<sstream>
#include <QString>

Board::Board()
{
    int i;
    count = 0;
    justHuiqi = false;
    hasVCF = false;
    dif = true;
    info = BoardInfo();
    for(int i= 0;i<225;i++)
        table[i/R][i%R] = EMPTY;
    for(i=0;i<R;i++)
        hash[i] = new Row(table,Point(0,i),down);
    for(i=0;i<R;i++)
        hash[i+15] = new Row(table,Point(i,0),rightt);
    for(i=0;i<11;i++)
        hash[i+30] = new Row(table,Point(4+i,0),rightup);
    for(i=0;i<10;i++)
        hash[i+41] = new Row(table,Point(14,1+i),rightup);
    for(i=0;i<11;i++)
        hash[i+51] = new Row(table,Point(i,0),rightdown);
    for(i=0;i<10;i++)
        hash[i+62] = new Row(table,Point(0,1+i),rightdown);
}
void Board::reset()
{
    count = 0;
    justHuiqi = false;
    hasVCF = false;
    for(int i= 0;i<225;i++)
        table[i/R][i%R] = EMPTY;
    flashHash();
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
    else
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
    int row,col;
    sscanf(s,"%[A-O]%d",&c,&row);
    col = c-65;
    return put(15-row,col,player);
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
    else
        return false;
}
bool Board::del(Point p)
{
    return del(p.x,p.y);
}

void Board::huiqi(bool once)
{
    if(count-1>=0)del(steps[count-1]);
    if(count-1>=0&&!once)del(steps[count-1]);
    justHuiqi = true;
}
void Board::userInput()
{
    int x=-2,y=-2;
    while(true)
    {
        printf("Enter row and column.\n");
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
    put(x,y,USR);
done:
    print();
}

/*----------------------------------------compInput------------------------------------------------------*/
//the computer's input
void Board::compInput()
{
    int x,y;
    double score;
    Point pt;
    if(count==0){x=7;y=7;}
    else if(count==1) {round2(&x,&y);}
    else if(count==2) {round3(&x,&y);}
    else if(preCheck(&x,&y)){ }
    //----------------------------------------------------------------------------------//
    else
    {
        for(int i=1;i<=10 && dif ;i+=3)
        {
            vcf.clear();
            hasVCF = VCFCOM(vcf,i);
            if(hasVCF) break;
        }
        vcf.clear();
        if(!hasVCF && dif && !VCFUSR(vcf,4))
        {
            vcf.clear();
            hasVCF = VCTCOM(vcf,6);
        }
        if(dif && hasVCF && vcf.size()>0)
        {
            pt = vcf[0];
            x = pt.x;
            y = pt.y;
            //qDebug("(%d,%d,VCTCOM)",x,y);
            //dPrintVCF(vcf,COM);
        }
        else
        {
            Point forcast[6];
            score = thinkAbout(forcast, MAX_LEVEL, COM, 100000);
            //score = thinkAbout2(forcast,MAX_LEVEL,COM,-100000,100000);
            x = forcast[MAX_LEVEL].x;
            y = forcast[MAX_LEVEL].y;
        }
        if(score<-5000)
        {
            afterCheck(&x,&y);
        }
    }
    //----------------------------------------------------------------------------------//

    steps[count] = Point(x,y);
    put(x,y,COM);
    //qDebug("%d COM : (x = %d,y = %d)\n",count,x,y);
    qDebug("%d COM : (%c,%d)\n",count,toColOnBoard(x),toRowOnBoard(y));
    //getInfo().showInfo();
}
void Board::round2(int *x,int *y)
{
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
bool Board::preCheck(int *x, int *y)
{
    BoardInfo info = getInfo();
    if(info.aliveFourCOM+info.sleepFourCOM>0) //go five
    {
        for(int i=0;i<R;i++)for(int j=0;j<R;j++)
        {
            if(table[i][j]==EMPTY)
            {
                put(i,j,COM);
                if( getInfo().fiveCOM )
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
    //block USR four
    if(info.sleepFourUSR+info.aliveFourUSR>0 )
    {
        int n = info.sleepFourUSR+2*info.aliveFourUSR;
        for(int i=0;i<R;i++)for(int j=0;j<R;j++)
        {
            if(table[i][j]==EMPTY)
            {
                put(i,j,COM);
                if( getInfo().sleepFourUSR+2*getInfo().aliveFourUSR < n)
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
    if(info.aliveThreeCOM>0&&(info.sleepFourUSR+info.aliveFourUSR==0))
    {
        for(int i=0;i<R;i++)for(int j=0;j<R;j++)
        {
            if(table[i][j]==EMPTY)
            {
                put(i,j,COM);
                if( getInfo().aliveFourCOM )
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

void Board::afterCheck(int *x, int *y)
{
    BoardInfo info;
    for(int i=0;i<R;i++)for(int j=0;j<R;j++)
    {
        if(put(i,j,USR))
        {
            info = getInfo();
            del(i,j);
            if(info.aliveFourUSR>=1)
            {
                *x=i;*y=j;return;
            }
        }
    }
    for(int i=0;i<R;i++)for(int j=0;j<R;j++)
    {
        if(put(i,j,USR))
        {
            info = getInfo();
            del(i,j);
            if(info.sleepFourUSR+info.aliveThreeUSR>=2)
            {
                *x=i;*y=j;return;
            }
        }
    }
}

//------------------------------------------------------------------computer input END
double Board::thinkAbout(Point forcast[],int level,int nextPlayer,double parentExtreme)
{
    Point pt;
    if(level==0)
    {
        if(nextPlayer == USR)
            return scoreAsCOM();
        else
            return -1*scoreAsUSR();
    }
    else if(winner()!=0)
        return (winner()==COM)?20000:-20000;
    else if(isFull())
        return 0;

    else if(nextPlayer==COM) //next is computer
    {
        double max=-100000;
        double temp;
        for(int i=0;i<R;i++) for(int j=0;j<R;j++)
        {
            if(!(farAway(i,j))&&table[i][j] == EMPTY)
            {
                put(i,j,COM);
                temp = thinkAbout(forcast,level-1,USR,max);
                if(noNeighbor(i,j))
                    temp-=5;
                temp -= closeToBoundary(i,j)*20;
                if(level== MAX_LEVEL)
                {

                    /*/-----------------------------------------
                    vcfu.clear();
                    if(VCTUSR(vcfu,3))
                    {
                        temp -= 5000;
                        qDebug("(%c%d,VCT/VCF USR) ",toColOnBoard(i),toRowOnBoard(j));
                        //dPrintVCF(vcfu,USR);
                    }

                    qDebug("(%c%d,[%.1f]) ",toColOnBoard(i),toRowOnBoard(j),temp);
                    qDebug("---------------");
                    nowWorkingOn = Point(i,j);
                    //-------------------------------------------*/
                }
                if(temp>max)
                {
                    max = temp;
                    forcast[level].x = i;
                    forcast[level].y = j;
                    if(level==MAX_LEVEL)
                    {
                        qDebug("[Now Choice %c%d %.1f]",toColOnBoard(i),toRowOnBoard(j),max);
                        getInfo().showInfo();
                    }
                }
                del(i,j);
                if(max>parentExtreme)
                    goto end_loop;
            }
        }
        end_loop:
        if(level==MAX_LEVEL)
            printf("\n");
        return max;
    }
    else if(nextPlayer==USR) //next is user
    {
        double min=100000;
        double temp;
        for(int i=0;i<R;i++) for(int j=0;j<R;j++)
        {
            if(!(farAway(i,j))&&table[i][j] == EMPTY)
            {
                put(i,j,USR);
                temp = thinkAbout(forcast,level-1,COM,min);
                if(noNeighbor(i,j))
                    temp+=1;
                temp += closeToBoundary(i,j)*4;
                if(temp<min)
                {
                    min = temp;
                    forcast[level] = Point(i,j);
                }
                del(i,j);
                if(min<parentExtreme)
                    goto end_loop2;
            }
        }
        end_loop2:
        return min;
    }
    return 0;
}
double Board::thinkAbout2(Point forcast[],int level,int nextPlayer,double a,double b)
{
    Point pt;
    if(level==0)
    {
        return score(nextPlayer);
    }
    else if(winner()!=0)
        return (winner()==COM)?20000:-20000;
    else if(isFull())
        return 0;

    else if(nextPlayer==COM) //next is computer
    {
        double max=-100000;
        double temp;
        for(int i=0;i<R;i++) for(int j=0;j<R;j++)
        {
            if(!(farAway(i,j))&&table[i][j] == EMPTY)
            {
                put(i,j,COM);
                temp = thinkAbout2(forcast,level-1,USR,a,b);
                if(noNeighbor(i,j))
                    temp-=5;
                temp -= closeToBoundary(i,j)*20;
                /*
                if(level==MAX_LEVEL)
                {
                    //-----------------------------------------

                    vcfu.clear();
                    int d = (temp>max-100)?0:1;
                    if(VCTUSR(vcfu,2+d))
                    {
                        temp -= (5000 - vcfu.size()*15);
                        qDebug("(%c%d,VCT/VCF USR) ",toColOnBoard(i),toRowOnBoard(j));
                        dPrintVCF(vcfu,USR);
                    }

                    qDebug("(%c%d,[%.1f]) ",toColOnBoard(i),toRowOnBoard(j),temp);
                    qDebug("---------------");
                    nowWorkingOn = Point(i,j);
                    //-------------------------------------------
                }
                */
                if(temp>max)
                {
                    max = temp;
                    if(max>a)a = max;
                    forcast[level].x = i;
                    forcast[level].y = j;
                    if(level==MAX_LEVEL)
                    {
                        qDebug("[Now Choice %c%d %.1f]",toColOnBoard(i),toRowOnBoard(j),max);
                        getInfo().showInfo();
                    }
                }
                del(i,j);
                if(b<=a)
                    goto end_loop;
            }
        }
        end_loop:
        if(level==MAX_LEVEL)
            printf("\n");
        return max;
    }
    else if(nextPlayer==USR) //next is user
    {
        double min=100000;
        double temp;
        for(int i=0;i<R;i++) for(int j=0;j<R;j++)
        {
            if(!(farAway(i,j))&&table[i][j] == EMPTY)
            {
                put(i,j,USR);
                temp = thinkAbout2(forcast,level-1,COM,a,b);
                if(noNeighbor(i,j))
                    temp+=1;
                temp += closeToBoundary(i,j)*4;
                if(temp<min)
                {
                    min = temp;
                    if(min<b)b=min;
                    forcast[level] = Point(i,j);
                }
                del(i,j);
                if(b<=a)
                    goto end_loop2;
            }
        }
        end_loop2:
        return min;
    }
    return 0;
}
double Board::score(int nextPlayer)
{
    if(nextPlayer == USR)
        return scoreAsCOM();
    else
        return -1*scoreAsUSR();
}

double Board::scoreAsCOM()
{
    double score=0;
    BoardInfo info = getInfo();
    double k;

    if(first==USR&&count<20)
        k = 1.5;
    else
        k = 0.9;
    if(info.fiveCOM>0)
        return 20000;
    if(info.aliveFourUSR>0)
        return -10000;
    if(info.sleepFourUSR>0)
        return -9000;
    if(info.aliveFourCOM>0)
        return 8000;

    if(info.aliveThreeUSR>0&&info.sleepFourCOM>0)
        score -= 12;
    else if(info.aliveThreeUSR>0&&info.sleepFourCOM==0)
        score -= 20000;

    score += lim(info.aliveThreeCOM,info.sleepFourCOM);
    score +=   (5*info.twoCOM + 7*info.sleepThreeCOM) -  k*(5*info.twoUSR  +  7*info.sleepThreeUSR);

    return score;
}

double Board::scoreAsUSR()
{
    double score=0;
    BoardInfo info = getInfo();
    int k;
    if(first==USR&&count<20)
        k = 1.5;
    else
        k = 1;
    if(info.fiveUSR>0)
        return 20000;
    if(info.aliveFourCOM>0)
        return -10000;
    if(info.sleepFourCOM>0)
        return -9000;
    if(info.aliveFourUSR>0)
        return 8000;

    if(info.aliveThreeCOM>0&&info.sleepFourUSR>0)
        score -= 12;
    else if(info.aliveThreeCOM>0&&info.sleepFourUSR==0)
        score -= 20000;

    score += lim(info.aliveThreeUSR,info.sleepFourUSR);
    score -=   5*info.twoCOM + 7*info.sleepThreeCOM -  (5*info.twoUSR  +  7*info.sleepThreeUSR);

    return score;
}

double Board::lim(int three,int four)
{
    if(three==1&&four==0) return (first==USR&&count<20)?3:5;
    if(three==0&&four==1) return (first==USR&&count<20)?3:4;
    if(three>=2&&four==0) return 4000;
    if(three>=1&&four>=1) return 16000;
    if(three==0&&four>=2) return 16500;
    return 0;
}
bool Board::VCFCOM(vector<Point> &vcfForcast,int level)
{
    int i,j;
    bool findVCF = false;
    if(isFull())
        return false;
    BoardInfo info = getInfo();
    if(info.aliveFourCOM + info.sleepFourCOM>0)
        return true;
    if(info.aliveFourUSR+info.sleepFourUSR>0)
        return false;
    if(info.aliveThreeCOM>0)
        return true;
    if(level>0)
    {
        for(i=0;i<H && !findVCF;i++)
        {
            if(hash[i]->sleepThreeCOM>0)
            {
                Point pt,pt2;
                for(j=0;j < hash[i]->length && !findVCF;j++)
                {
                    pt = hash[i]->pts[j];
                    if(put(pt.x,pt.y,COM)) //若此处为空则下子
                    {
                        if(hash[i]->sleepFourCOM==1) //冲四
                        {
                            bool blocked = false;\
                            for(int k = 0;k<hash[i]->length && !findVCF && !blocked;k++) //对方堵四
                            {
                                pt2 = hash[i]->pts[k];
                                if(put(pt2.x,pt2.y,USR))//若此处为空则下子
                                {
                                    if(hash[i]->sleepFourCOM==0) //若刚刚一步堵掉了冲四
                                    {
                                        blocked = true;
                                        vcfForcast.push_back(pt);//入栈记录
                                        vcfForcast.push_back(pt2);
                                        findVCF = VCFCOM(vcfForcast,level-1);//递归，计算下一步冲四
                                        if(!findVCF)
                                        {
                                            vcfForcast.pop_back();//出栈
                                            vcfForcast.pop_back();
                                        }
                                        del(pt2.x,pt2.y);
                                        break;
                                    }
                                    del(pt2.x,pt2.y);//出栈
                                }
                            }
                            if(blocked==false || findVCF) //如果对方堵不掉
                            {
                                //vcfForcast.insert(vcfForcast.begin(),pt2);
                                //vcfForcast.insert(vcfForcast.begin(),pt);//保存
                                del(pt.x,pt.y);
                                return true;
                            }
                        }
                        del(pt.x,pt.y);//出栈
                    }
                }
            }
        }
        return findVCF;
    }
    return false;
}
bool Board::VCFUSR(vector<Point> &vcfForcast,int level)
{
    int i,j;
    bool findVCF = false;
    if(isFull())
        return false;
    BoardInfo info = getInfo();
    if(info.aliveFourUSR+info.sleepFourUSR>0)
        return true;
    if(info.aliveFourCOM+info.sleepFourCOM>0)
        return false;
    if(info.aliveThreeUSR>0)
        return true;
    if(level<=0)
        return false;
    for(i=0;i<H && !findVCF;i++)
    {
        if(hash[i]->sleepThreeUSR>0)
        {
            Point pt,pt2;
            for(j=0;j < hash[i]->length && !findVCF;j++)
            {
                pt = hash[i]->pts[j];
                if(put(pt.x,pt.y,USR))
                {
                    if(hash[i]->sleepFourUSR==1)
                    {
                        bool blocked = false;
                        for(int k = 0;k<hash[i]->length && !findVCF && !blocked;k++) //对方堵四
                        {
                            pt2 = hash[i]->pts[k];
                            if(put(pt2.x,pt2.y,COM))//若此处为空则下子
                            {
                                if(hash[i]->sleepFourUSR==0) //若刚刚一步堵掉了冲四
                                {
                                    blocked = true;
                                    vcfForcast.push_back(pt);//入栈记录
                                    vcfForcast.push_back(pt2);
                                    findVCF = VCFUSR(vcfForcast,level-1);//递归，计算下一步冲四
                                    if(!findVCF)
                                    {
                                        vcfForcast.pop_back();//出栈
                                        vcfForcast.pop_back();
                                    }
                                    del(pt2.x,pt2.y);
                                    break;
                                }
                                del(pt2.x,pt2.y);//出栈
                            }
                        }
                        if(blocked==false) //如果对方堵不掉
                        {
                            del(pt.x,pt.y);
                            return true;
                        }
                    }
                    del(pt.x,pt.y);//出栈
                }
            }
        }
    }
    return findVCF;
}

bool Board::VCTCOM(vector<Point> &vcfForcast, int level)
{
    int i,j;
    bool findVCT = false;
    bool blocked = false;
    vector<Point> tempVcf;

    if(VCFCOM(vcfForcast,level))
        return true;
    if(level<=0)
        return false;
    bool carefulVCFUSR = VCFUSR(tempVcf,3);
    for(i=0;i<H && !findVCT;i++)
    {
        if(hash[i]->twoCOM + hash[i]->sleepThreeCOM >0)
        {
            Point pt,pt2;
            for(j=0;j < hash[i]->length && !findVCT;j++)
            {
                pt = hash[i]->pts[j];
                if(put(pt.x,pt.y,COM))
                {
                    int numOfAttack = info.aliveThreeCOM + info.sleepFourCOM;
                    if(numOfAttack >=1) //冲三/冲四
                    {
                        if(carefulVCFUSR && VCFUSR(tempVcf,3))
                        {
                            del(pt);
                            continue;
                        }
                        blocked = false;

                        for(int k = 0;k< hash[i]->length && !blocked;k++) //对方堵
                        {
                            pt2 = hash[i]->pts[k];
                            if(put(pt2.x,pt2.y,USR))//若此处为空则下子
                            {
                                if(hash[i]->aliveThreeCOM+ hash[i]->sleepFourCOM==0) //若刚刚一步堵掉了冲三/冲四
                                {
                                    findVCT = VCTCOM(vcfForcast,level-1);//递归，计算下一步冲三/冲四
                                    if(!findVCT)
                                    {
                                        blocked = true;
                                        del(pt2.x,pt2.y);
                                        break;
                                    }
                                }
                                del(pt2.x,pt2.y);//出栈
                            }
                        }

                        if(!blocked) //如果对方堵不掉
                        {
                            del(pt.x,pt.y);
                            vcfForcast.insert(vcfForcast.begin(),1,pt);
                            return true;
                        }
                    }
                    del(pt.x,pt.y);//出栈
                }
            }
        }
    }
    return findVCT;
}

bool Board::VCTUSR(vector<Point> &vcfForcast, int level)
{
    int i,j;
    bool findVCT = false;
    bool blocked = false;
    vector<Point> tempVcf;

    if(VCFUSR(vcfForcast,level))
        return true;
    if(level<=0)
        return false;
    bool carefulVCFCOM = VCFCOM(tempVcf,3);
    for(i=0;i<H && !findVCT;i++)
    {
        if(hash[i]->twoUSR + hash[i]->sleepThreeUSR >0)
        {
            Point pt,pt2;
            for(j=0;j < hash[i]->length && !findVCT;j++)
            {
                pt = hash[i]->pts[j];
                if(put(pt.x,pt.y,USR))
                {
                    if(hash[i]->aliveThreeUSR + hash[i]->sleepFourUSR > 0) //冲三/冲四
                    {
                        if(carefulVCFCOM && VCFCOM(tempVcf,3))
                        {
                            del(pt);
                            continue;
                        }
                        blocked = false;
                        for(int k = 0;k<hash[i]->length && !blocked;k++) //对方堵
                        {
                            pt2 = hash[i]->pts[k];
                            if(put(pt2.x,pt2.y,COM))//若此处为空则下子
                            {
                                if(hash[i]->aliveThreeUSR+ hash[i]->sleepFourUSR==0) //若刚刚一步堵掉了冲三/冲四
                                {
                                    findVCT = VCTUSR(vcfForcast,level-1);//递归，计算下一步冲三/冲四
                                    if(!findVCT)
                                    {
                                        blocked = true;
                                        del(pt2.x,pt2.y);
                                        break;
                                    }
                                }
                                del(pt2.x,pt2.y);//出栈
                            }
                        }
                        if(blocked==false) //如果对方堵不掉
                        {
                            del(pt.x,pt.y);
                            vcfForcast.insert(vcfForcast.begin(),1,pt);
                            return true;
                        }
                    }
                    del(pt.x,pt.y);//出栈
                }
            }
        }
    }
    return findVCT;
}


/*----------------------------------------farAway------------------------------------------------------*/
bool Board::farAway(int m,int n)
{
    int val=1,i,j;
    for(i=m-2;i<=m+2;i++)for(j=n-2;j<=n+2;j++)
        if(inRange(i,j)&&!(i==m&&j==n))
            val *= table[i][j];
    return val==1;
}
bool Board::noNeighbor(int m,int n)
{
    int val=1,i,j;
    for(i=m-1;i<=m+1;i++)for(j=n-1;j<=n+1;j++)
        if(inRange(i,j)&&!(i==m&&j==n))
            val *= table[i][j];
    return val==1;
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
BoardInfo Board::getInfoAround(int x,int y)
{
    Row *tempHash[H];
    tempHash[0] = hash[y];
    tempHash[1] = hash[x+15];
    int sum = x+y,diff = x-y;
    if(sum>=4&&sum<=24)
        tempHash[2] = hash[26+sum];
    if(diff>=0&&diff<=10)
        tempHash[3] = hash[51+diff];
    else if(diff>=-10&&diff<0)
        tempHash[3] = hash[61+(-1)*diff];
    BoardInfo info;
    for(int i=0;i<4;i++)
    {
        info.aliveFourCOM += tempHash[i]->aliveFourCOM;
        info.aliveFourUSR += tempHash[i]->aliveFourUSR;
        info.aliveThreeCOM += tempHash[i]->aliveThreeCOM;
        info.aliveThreeUSR += tempHash[i]->aliveThreeUSR;
        info.fiveCOM += tempHash[i]->fiveCOM;
        info.fiveUSR += tempHash[i]->fiveUSR;
        info.sleepFourCOM += tempHash[i]->sleepFourCOM;
        info.sleepFourUSR += tempHash[i]->sleepFourUSR;
        info.sleepThreeCOM += tempHash[i]->sleepThreeCOM;
        info.sleepThreeUSR += tempHash[i]->sleepThreeUSR;
        info.twoCOM +=  tempHash[i]->twoCOM;
        info.twoUSR +=  tempHash[i]->twoUSR;
    }
    return info;
}

BoardInfo Board::getInfo()
{
    /*
    BoardInfo info;
    int i;
    for(i=0;i<H;i++)
    {
        info.aliveFourCOM += hash[i]->aliveFourCOM;
        info.aliveFourUSR += hash[i]->aliveFourUSR;
        info.aliveThreeCOM += hash[i]->aliveThreeCOM;
        info.aliveThreeUSR += hash[i]->aliveThreeUSR;
        info.fiveCOM += hash[i]->fiveCOM;
        info.fiveUSR += hash[i]->fiveUSR;
        info.sleepFourCOM += hash[i]->sleepFourCOM;
        info.sleepFourUSR += hash[i]->sleepFourUSR;
        info.sleepThreeCOM += hash[i]->sleepThreeCOM;
        info.sleepThreeUSR += hash[i]->sleepThreeUSR;
        info.twoCOM +=  hash[i]->twoCOM;
        info.twoUSR +=  hash[i]->twoUSR;
    }
    return info;
    */
    return info;
}
int Board::winner()
{
    for(int i=0;i<H;i++)
    {
        if(hash[i]->fiveCOM>0)
            return COM;
        if(hash[i]->fiveUSR>0)
            return USR;
    }
    return 0;
}
//------------------------------------Row-------------------------------------------------//
Row::Row(int t[R][R],Point start,Direction d)
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
    parent = t;
    data = 0;
    length = k;
}
void Row::flash(BoardInfo* info)
{
    info->sub(this);
    if(data!=0)
        delete[] data;
    data = getData();
    int *tempUSR= tempArray(data,USR);
    int *tempCOM = tempArray(data,COM);
    twoUSR = numOfTwo(tempUSR,USR);
    sleepThreeUSR = numOfSleepThree(tempUSR,USR);
    aliveThreeUSR = numOfAliveThree(tempUSR,USR);
    sleepFourUSR = numOfSleepFour(tempUSR,USR);
    aliveFourUSR = numOfAliveFour(tempUSR,USR);
    fiveUSR = numOfFive(tempUSR,USR);
    twoCOM = numOfTwo(tempCOM,COM);
    sleepThreeCOM = numOfSleepThree(tempCOM,COM);
    aliveThreeCOM = numOfAliveThree(tempCOM,COM);
    sleepFourCOM = numOfSleepFour(tempCOM,COM);
    aliveFourCOM = numOfAliveFour(tempCOM,COM);
    fiveCOM = numOfFive(tempCOM,COM);
    delete[] tempUSR;
    delete[] tempCOM;
    info->add(this);
}
int *Row::getData()
{
    int *data = new int[R];
    int i;
    for(i=0;i<length;i++)
        data[i] = parent[pts[i].x][pts[i].y];
    return data;
}
int *Row::tempArray(int row[],int player)
{
    int i;
    int *temp = new int[A];
    for(i=0;i<A;i++)
        temp[i] = WALL;
    for(i=0;i<length;i++)
        temp[i+1] = (  row[i]!=player  &&  row[i]!=EMPTY  )?WALL:row[i];
    return temp;
}
void Row::print()
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
int Row::numOfTwo(int temp[],int player)
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
int Row::numOfSleepThree(int temp[],int player)
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
int Row::numOfAliveThree(int temp[],int player)
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
int Row::numOfSleepFour(int temp[],int player)
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
int Row::numOfAliveFour(int temp[],int player)
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
int Row::numOfFive(int temp[],int player)
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
