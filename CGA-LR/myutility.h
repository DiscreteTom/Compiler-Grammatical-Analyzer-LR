#ifndef MYUTILITY_H
#define MYUTILITY_H

#include <iostream>
#include <QString>

using namespace std;

inline ostream & operator<<(ostream & out, const QString & str){return out << str.toStdString();}
inline void getline(istream & in, QString &str){
	string t;
	getline(in, t);
	str = QString::fromStdString(t);
}

#endif // MYUTILITY_H
