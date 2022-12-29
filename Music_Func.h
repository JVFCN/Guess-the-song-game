//
// Created by j3280 on 2022/12/26.
//
#include <map>
#include <dirent.h>
#include <string>
#include <windows.h>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include "Shlobj.h"
#include <fstream>
#include <ctime>
#include "json/json.h"
#include "jsoncpp.cpp"

Json::Value Music_Time_Editor;
Json::Value Name_Editor;

std::string Name = "User";
int Music_Time = 10000;
bool Random_Play = true;

void Read_Json() {
    std::ifstream ifs;
    ifs.open("./Config.json");
    assert(ifs.is_open());

    Json::Reader reader;
    Json::Value root;
    // 解析到root，root将包含Json里所有子元素
    if (!reader.parse(ifs, root, false)) {
        std::cerr << "parse failed \n";
        return;
    }
    Name = root["Name"].asString();
    Music_Time = root["Music_Time"].asInt();
    Random_Play = root["Random_Play"].asBool();
    ifs.close();
}


void Json_Editor(const char *Json_Key, Json::Value &Json_Value) {
    //TODO
    std::ifstream jsonFile("Config.json");
    Json::Value root;
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(jsonFile, root);
    if (!parsingSuccessful) {
        // 解析失败
        std::cout << "Error parsing JSON file" << std::endl;
        return;
    }

    // 修改 JSON 元素值
    root[Json_Key] = Json::Value(Json_Value);

    // 写入 JSON 文件
    std::ofstream outputFile("Config.json");
    Json::StyledWriter writer;
    outputFile << writer.write(root);
}

std::string GetRandLine(const std::string &fileName) {
    std::ifstream inf(fileName.c_str());
    std::string lineData;
    int i = 1;
    std::string tmpLine;
    srand((unsigned int) time(NULL));
    while (getline(inf, tmpLine)) {
        if (rand() % i == 0)
            lineData = tmpLine;
        ++i;
    }
    inf.close();
    return lineData;
}

int checkPath(const std::string strPath) {
    struct stat infos{};

    if (stat(strPath.c_str(), &infos) != 0) {
        return -1;    //无效
    } else if (infos.st_mode & S_IFDIR) {
        return 0;    //目录
    } else if (infos.st_mode & S_IFREG) {
        return 1;    //文件
    } else {
        return -1;
    }
}

void Read_Folder(std::string path, std::vector<std::string> &files, std::string format) {
    long hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*" + format).c_str(), &fileinfo)) != -1) {
        do {
            if ((fileinfo.attrib & _A_SUBDIR)) {
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
                    //files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
                    Read_Folder(p.assign(path).append("\\").append(fileinfo.name), files, format);
                }
            } else {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
        } while (_findnext(hFile, &fileinfo) == 0);

        _findclose(hFile);
    }
}

std::vector<std::string> files;

void Select_Song_List(std::string Song_List_Path) {
    Read_Folder(Song_List_Path, files, ".mp3");
}

void Start_Music(const std::string &Music_Path) {
    std::string command = "open " + Music_Path + " type mpegvideo alias mp3";
    mciSendString(command.c_str(), nullptr, 0, nullptr);
    mciSendString("play mp3", nullptr, 0, nullptr);
    Sleep(Music_Time);
    mciSendString("close mp3", nullptr, 0, nullptr);
}

std::string GetFilesName(std::string path) {
    std::string::size_type iPos = path.find_last_of('\\') + 1;
    std::string filename = path.substr(iPos, path.length() - iPos);

    std::string name = filename.substr(0, filename.rfind("."));
    return name;
}

double Guess_Right = 0;
double Wrong_Guess = 0;
int Music = 1;

void Play_Music() {
    Read_Json();
    int Choose;
    std::cout << "1.选择歌单\n2.直接开始" << std::endl;

    while (std::cin >> Choose)
        if (Choose == 1) {
//            std::basic_string<char> Song_List_Path;

            TCHAR Song_List_Path[MAX_PATH] = {0};
            BROWSEINFO bi;
            ZeroMemory(&bi, sizeof(BROWSEINFO));
            bi.hwndOwner = NULL;
            bi.pszDisplayName = Song_List_Path;
            bi.lpszTitle = ("选择歌单:");
            bi.ulFlags = BIF_RETURNFSANCESTORS;
            LPITEMIDLIST idl = SHBrowseForFolder(&bi);
            if (NULL == idl) {
                std::cout << "err" << std::endl;
                system("pause");
                return;
            }
            SHGetPathFromIDList(idl, Song_List_Path);

            std::string Cin_Name;
            Select_Song_List(Song_List_Path);
            system("cls");

            if (files.empty()) {
                std::cout << "没有检测到.MP3文件, 请重新选择" << std::endl;
                system("pause");
                return;
            }
            for (const auto & file : files) {
                std::cout << "开始播放第" << Music << "首歌曲" << std::endl;
                Start_Music(file);
                std::cout << "第" << Music << "首歌曲播放完毕" << std::endl;
                Music++;

                std::cout << "输入你猜的名字:";
                std::cin >> Cin_Name;
                if (Cin_Name == GetFilesName(file)) {
                    std::cout << GetRandLine("./Statements_R.txt") << std::endl;
                    Guess_Right++;
                } else {
                    std::cout << GetRandLine("./Statements_W.txt") << std::endl;
                    std::cout << "正确答案是:" << GetFilesName(file) << std::endl;
                    Wrong_Guess++;
                }
                system("pause");
                system("cls");
            }
            std::cout << "该歌单播放完毕" << std::endl;
            break;
        } else if (Choose == 2) {
            Select_Song_List("./MusicList");
            if (files.empty()) {
                std::cout << "./MusicList文件夹下无.MP3文件, 请检查" << std::endl;
                system("pause");
                return;
            }

            for (int i = 0; i < files.size(); ++i) {
                system("cls");
                std::cout << "开始播放第" << i << "首歌曲" << std::endl;
                Start_Music(files[i]);
                std::cout << "第" << i << "首歌曲播放完毕" << std::endl;
                Music++;

                std::cout << "输入你猜的名字:";
                std::string Cin_Name;
                std::cin >> Cin_Name;
                if (Cin_Name == GetFilesName(files[i])) {
                    std::cout << GetRandLine("./Statements_R.txt") << std::endl;
                    Guess_Right++;
                } else {
                    std::cout << GetRandLine("./Statements_W.txt") << std::endl;
                    std::cout << "正确答案是:" << GetFilesName(files[i]) << std::endl;
                    Wrong_Guess++;
                }
                system("pause");
                system("cls");
            }
            std::cout << "该歌单播放完毕" << std::endl;
            break;
        }
    std::cout << "你一共答对了" << Guess_Right << "道题\n" << "答错了" << Wrong_Guess << "道题" << std::endl;
    std::cout << "你的正确率是:" << (Guess_Right / Music) * 100 << "%" << std::endl;
    std::cout << "                   -----" << Name << std::endl;
    system("pause");
}

void GuessSong_Start() {
    Play_Music();
}

void Music_Duration() {
    std::cout << "请输入时长(s):";
    std::cin >> Music_Time;
    if (Music_Time <= 0) {
        std::cout << "歌曲时长不能小于等于零" << std::endl;
        system("pause");
        return;
    } else if (Music_Time > 60) {
        std::cout << "歌曲时长不能大于60s" << std::endl;
        system("pause");
        return;
    } else {
        std::cout << "时长已设置为:" << Music_Time << "s" << std::endl;
        Music_Time_Editor = Music_Time * 1000;
        Json_Editor("Music_Time", Music_Time_Editor);
        system("pause");
        return;
    }
}

void Change_User_Name() {
    std::cout << "请输入新用户名:";
    std::cin >> Name;

    std::cout << "用户名已更改为:" << Name << std::endl;
    Name_Editor = Name;
    Json_Editor("Name", Name_Editor);
    system("pause");
}

void Setting() {
    int Setting;
    std::cout << "------------Setting------------\n1.歌曲截取时长\n2.更改用户名" << std::endl;
    std::cin >> Setting;
    if (Setting == 1) {
        system("cls");
        Music_Duration();
    } else if (Setting == 2) {
        system("cls");
        Change_User_Name();
    }
}

void Hello_User() {
    int Choose;
    std::cout << "1.开始猜歌\n2.设置" << std::endl;
    while (std::cin >> Choose) {
        switch (Choose) {
            case 1:
                GuessSong_Start();
                return;
            case 2:
                Setting();
                return;
        }
    }
}