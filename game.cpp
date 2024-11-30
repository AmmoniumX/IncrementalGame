#include <iostream>
#include <ncurses.h>
#include <boost/multiprecision/gmp.hpp>

using std::cin, std::cout, std::endl;
typedef boost::multiprecision::mpz_int bigint;

int main() {

    // bigint tests
    bigint a("123456789012345678901234567890");
    bigint b("987654321098765432109876543210");

    bigint sum = a + b;
    bigint product = a * b;

    cout << "Sum: " << sum << endl;
    cout << "Product: " << product << endl;

    // ncurses tests
    initscr();
    printw("Hello, world!");
    refresh();
    getch();
    endwin();

    return 0;
}
