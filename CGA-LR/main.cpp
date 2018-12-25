#include <iostream>
#include "gramma.h"
#include "myutility.h"
#include <QString>
#include <string>

using namespace std;

int main(/*int argc, char *argv[]*/)
{
	GrammaTable gt;
	QString t;

	gt.insert("E -> E+T | E-T | T");
	gt.insert("T -> T*F | T/F | F");
	gt.insert("F -> (E) | $num$");

	gt.generate();
	cout << "Output:\n";
	gt.output();

	bool flag = true;
	while (flag)
	{
		cout << "\nInput a line to parse, input blank line to stop.\n";
		getline(cin, t);
		if (t.length())
		{
			gt.parse(t);
		}
		else
		{
			flag = false;
		}
	}
	system("pause");
}
