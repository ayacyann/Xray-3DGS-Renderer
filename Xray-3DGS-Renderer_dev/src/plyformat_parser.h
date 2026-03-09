#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <utility>

using namespace std;

/**
 * 核心函数：匹配任意一个子串，返回（子串本身，候选列表序号）
 * @param mainStr 主字符串
 * @param subList 候选子串列表
 * @return pair：first=匹配到的子串（空则无匹配），second=候选列表序号（-1表示无匹配）
 */
pair<string, int> matchAnyOneGetStrAndIndex(
    const string& mainStr,
    const vector<string>& subList
) 
{
    // 遍历候选列表，记录索引
    for (int i = 0; i < subList.size(); ++i) 
    {
        const string& sub = subList[i];
        if (sub.empty()) continue;  // 跳过空串

        // 匹配到则立即返回（子串，索引）
        if (mainStr.find(sub) != string::npos) 
        {
            return { sub, i };
        }
    }

    // 无匹配返回（空串，-1）
    return { "", -1 };
}

vector<string> line_type_List = 
{ 
    "ply",
    "format binary_little_endian 1.0",
    "element vertex",
    "property float x",
    "property float y",
    "property float z",
    "property float nx",
    "property float ny",
    "property float nz",
    "property float density",
    "property float mark",
    "property float scale_0",
    "property float scale_1",
    "property float scale_2",
    "property float rot_0",
    "property float rot_1",
    "property float rot_2",
    "property float rot_3",
    "end_header",
};

enum Line_Type
{
    ply,
	binary_little_endian,
    element_vertex,
    property_float_x,
    property_float_y,
    property_float_z,
    property_float_nx,
    property_float_ny,
    property_float_nz,
    property_float_density,
    property_float_mark,
    property_float_scale_0,
    property_float_scale_1,
    property_float_scale_2,
    property_float_rot_0,
    property_float_rot_1,
    property_float_rot_2,
    property_float_rot_3,
    end_header,
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
			return this->success = 0;
		}
        string line;
        int line_type = -1;
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
            cout << "\n（ascii）：" << line << endl;

            auto result = matchAnyOneGetStrAndIndex(line, line_type_List);
            if (!result.first.empty()) 
            {
                cout << "匹配到子串=\"" << result.first
                    << "\"，在候选列表中是第" << result.second << "个（从0开始）\n" << endl;              
            }
            else 
            {
                cout << "无匹配的子串\n" << endl;
            }

            line_type = result.second;
            switch (line_type)
            {
            case -1:
                cout << "unknow line_type \n" << endl;
                break;
            }
        }

        if (line == "end_header")
        {
            cout << "header读取完毕，共 " << line_num << " 行 \n" << endl;
        }
        else if (binfile.eof()) 
        {
            cout << "文件读取完毕，共 " << line_num << " 行 \n" << endl;
        }
        else
        {
            cerr << "header读取过程中出错！\n" << endl;
            return this->success = 0;
        }

        binfile.close();
        return this->success = 1;
	}
};