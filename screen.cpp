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
		cout.flush();
	}

	void row (const char *c, int n)
	{
		for (int i = 0; i < n; i++)
			cout << c;
		cout.flush();
	}

	void fg (color c) { cout << "\e[" << 30+c << "m"; }
	void bg (color c) { cout << "\e[" << 40+c << "m"; }
	void reset () { cout << "\e[0m"; }

	Screen (int r) { rows = r; }
};
/*
int main ()
{
	Screen *s = new Screen(30);
	srand48(time(NULL));

	s->clear();
	s->move(20,10);
	s->bg(blue);
	s->fg(white);
	s->column(uparrow,10);
	s->row(downarrow,12);
	s->move(31,10);
	s->column(uparrow,10);
	s->reset();
	s->move(0,0);
}
*/
