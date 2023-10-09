#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>

int main()
{
    std::string filename{"test.html"};
    std::fstream s{filename, s.binary | s.trunc | s.in | s.out};
    int envNum = 10;
    int Max = envNum + 1;
    s << "<!DOCTYPE HTML>" << "<HTML><H1>TITLE<H1>";
    s << "current value: " << envNum;
    for (int i = 0; i < Max; i++)
        s << "<H2>hello<H2>";
    s<< "<HTML>";
    return (0);
}