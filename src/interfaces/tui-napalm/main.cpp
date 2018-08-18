#include <curses.h>
#include <Player.h>

class Curses{
public:
	Curses(){
		initscr();
		start_color();
		use_default_colors();
		keypad(stdscr, true);
		noecho();
	}
	~Curses(){
		endwin();
	}
} curses;

const short selected = 2;
const short unselected = 1;

void f(){
	auto window = newwin(10, 20, 10, 10);
	while (true){
		wrefresh(window);
		auto input = getch();
		if (input == 'q')
			break;
	}
	delwin(window);
}

int main(){
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
	int i = 0;
	while (true){
		wclear(stdscr);

		if (i == 0)
			wcolor_set(stdscr, selected, nullptr);
		else
			wcolor_set(stdscr, unselected, nullptr);
		mvwprintw(stdscr, 3, 10, "[LOAD]");
		if (i == 1)
			wcolor_set(stdscr, selected, nullptr);
		else
			wcolor_set(stdscr, unselected, nullptr);
		mvwprintw(stdscr, 4, 10, "[PLAY]");
		if (i == 2)
			wcolor_set(stdscr, selected, nullptr);
		else
			wcolor_set(stdscr, unselected, nullptr);
		mvwprintw(stdscr, 5, 10, "[PAUSE]");
		if (i == 3)
			wcolor_set(stdscr, selected, nullptr);
		else
			wcolor_set(stdscr, unselected, nullptr);
		mvwprintw(stdscr, 6, 10, "[STOP]");

		auto input = getch();
		switch (input){
			case 'q':
				return 0;
			case '\n':
				if (!i){
					f();
				}
				break;
			case KEY_DOWN:
				i = (i + 1) % 4;
				break;
			case KEY_UP:
				i = (i + 3) % 4;
				break;
			case KEY_RESIZE:
				resize_term(0, 0);
				//wmove(stdscr, y/2, x/2);
				//wprintw(stdscr, "%d, %d", x, y);
				//mvprintw(0, 0, "%d, %d", x, y);
		}
		//wmove(stdscr, 0, 0);
		mvwprintw(stdscr, 0, 0, " ");
		wrefresh(stdscr);
	}
	return 0;
}
