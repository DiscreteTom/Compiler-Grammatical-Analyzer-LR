#include <iostream>
#include "gramma.h"
#include "myutility.h"
#include <QString>
#include <string>
#include <conio.h>

using namespace std;

int main(/*int argc, char *argv[]*/)
{
	GrammaTable gt;
	QString t;

	gt.insert("E' -> E");
	gt.insert("E -> E+T | E-T | T");
	gt.insert("T -> T*F | T/F | F");
	gt.insert("F -> (E) | $num$");

	gt.generate();
	cout << "Output:\n";
	gt.output();

	while (1)
	{
		cout << "\nPress 1: Just parse input.\nPress 2: Calculate result.\nOtherwise: Exit\n";
		int mode = getch() - '0';
		if (mode != 1 && mode != 2) // error input
			mode = 3;
		if (mode == 3)
			return 0;

		bool flag = true;
		while (flag)
		{
			cout << "\nInput a line to parse, input blank line to stop.\n";
			getline(cin, t);
			if (t.length())
			{
				gt.parse(t, mode == 2);
			}
			else
			{
				flag = false;
			}
		}
	}
	system("pause");
}
