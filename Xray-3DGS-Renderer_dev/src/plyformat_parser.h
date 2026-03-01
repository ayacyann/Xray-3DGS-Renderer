#pragma once

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

enum
{
	binary_little_endian = 0,
	binary_big_endian = 1,
	ascii = 2
};

class Plyformat_parser
{
public:
    int success = 0;
	Plyformat_parser(string const& path)
	{ 
		check_ply_and_endian(path);
	}
private:
	int check_ply_and_endian(string const& path)
	{
		ifstream binfile(path, ios::binary);
		if (!binfile.is_open()) {
			cerr << "文件打开失败！" << endl;
			return success = 0;
		}
        string line;
        int line_num = 0;
        while (line != "end_header")
        {
            getline(binfile, line);
            line_num++;
            cout << "第 " << line_num << " 行：" << endl;
            for (char c : line) 
            {
                printf("%02X ", (unsigned char)c);
            }
            cout << "\n（ascii）：" << line << "\n\n";
        }

        if (line == "end_header")
        {
            cout << "header读取完毕，共 " << line_num << " 行 \n" << endl;
        }
        else
        {
            cerr << "header读取过程中出错！\n" << endl;
            return success = 0;
        }

        if (binfile.eof())
        {
            cout << "文件读取完毕，共 " << line_num << " 行 \n" << endl;
        }
        else 
        {
            cerr << "文件读取过程中出错！\n" << endl;
            return success = 0;
        }

        binfile.close();
        return success = 1;
	}
};