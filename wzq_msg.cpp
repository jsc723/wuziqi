#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
#include <string>
#include "wzq_msg.h"

using namespace std;

vector<string> split_line (string s, string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}
string WZQMessage::serialize() {
    stringstream ss;
    ss << (int)op << endl;
    ss << i1 << "," << i2 << "," << i3 << "," << i4 << endl;
    ss << s1 << "</s>" << s2 << "</s>" << s3 << "</s>" << s4 << endl;
    return ss.str();
}

int s2i(const string &s) {
    int i;
    sscanf(s.c_str(), "%d", &i);
    return i;
}

void WZQMessage::deserialize(const string &encoded) {
    stringstream ss(encoded);
    string line;
    getline(ss, line);
    op = (WZQMessage::WZ) s2i(line);
    getline(ss, line);
    vector<string> v = split_line(line, ",");
    i1 = s2i(v[0]), i2 = s2i(v[1]), i3 = s2i(v[2]), i4 = s2i(v[3]);
    getline(ss, line);
    v = split_line(line, "</s>");
    s1 = v[0];
    s2 = v[1];
    s3 = v[2];
    s4 = v[3];
}
