#define UP 'A'     // 65
#define DOWN 'B'   // 66
#define LEFT 'D'   // 68
#define RIGHT 'C'  // 67
#define QUIT 'q'
#define ENTER 10
#define PATH_SIZE 1024
#define BACKSPACE 127
#define ESCAPE 27
#define clrscr() write(1,"\033[H\033[J",6)//cout << "\033[H\033[J";  // clear screen
#define poscur() cout << "\033[1;1H";  //write(1,"\033[1;1H",6) positions cursor to top-left
#define mvup() write(1,"\033[1A",4)//cout << "\033[1A";         // move up 1 unit
#define mvdwn() write(1,"\033[1B",4)//cout << "\033[1B";        // move down 1 unit
#define BUFFER 4096