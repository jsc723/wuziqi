#ifndef WZQ_MSG_H
#define WZQ_MSG_H

#include <string>
using std::string;
class WZQMessage {
public:
    enum WZ {
        LOGIN,
        SIGNUP,
        ECHO
    };
    WZQMessage(): i1(0), i2(0), i3(0), i4(0) {}
    string serialize();
    void deserialize(const string &encoded);
    WZ op;
    int i1;
    int i2;
    int i3;
    int i4;
    string s1;
    string s2;
    string s3;
    string s4;
};

#endif
