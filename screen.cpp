#include <iostream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

enum color { black, red, green, yellow, blue, magenta, cyan, white };
const char* uparrow = "↑";
const char* downarrow = "↓";
const char* notmoving = "-";
const char* dooropen = "|";
const char* destination_marker = "o";
const char* person_marker = "i";

void mysleep (double millisecs)
{
	int msecs = millisecs * 1000;
	usleep(msecs);
}

int myrand (int n)
{
	return lrand48() % n;
}

class Screen
{
	private:
	int rows;

	public:
	
	void clear () { cout << "\e[2J"; cout.flush(); }

	void move (int x,int y)
	{
		if (y >= rows) return;
		cout << "\e[" << rows-y << ";" << x << "H";
	}

	void column (const char *c, int n)
	{
		for (int i = 0; i < n; i++)
			cout << c << "\e[A\e[D";
	}

	void row (const char *c, int n)
	{
		for (int i = 0; i < n; i++)
			cout << c;
	}

	void fg (color c) { cout << "\e[" << 30+c << "m"; }
	void bg (color c) { cout << "\e[" << 40+c << "m"; }
	void reset () { cout << "\e[0m"; }

	Screen (int r) { rows = r; }
};
