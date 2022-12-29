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
    // ������root��root������Json��������Ԫ��
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
        // ����ʧ��
        std::cout << "Error parsing JSON file" << std::endl;
        return;
    }

    // �޸� JSON Ԫ��ֵ
    root[Json_Key] = Json::Value(Json_Value);

    // д�� JSON �ļ�
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
        return -1;    //��Ч
    } else if (infos.st_mode & S_IFDIR) {
        return 0;    //Ŀ¼
    } else if (infos.st_mode & S_IFREG) {
        return 1;    //�ļ�
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
    std::cout << "1.ѡ��赥\n2.ֱ�ӿ�ʼ" << std::endl;

    while (std::cin >> Choose)
        if (Choose == 1) {
//            std::basic_string<char> Song_List_Path;

            TCHAR Song_List_Path[MAX_PATH] = {0};
            BROWSEINFO bi;
            ZeroMemory(&bi, sizeof(BROWSEINFO));
            bi.hwndOwner = NULL;
            bi.pszDisplayName = Song_List_Path;
            bi.lpszTitle = ("ѡ��赥:");
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
                std::cout << "û�м�⵽.MP3�ļ�, ������ѡ��" << std::endl;
                system("pause");
                return;
            }
            for (const auto & file : files) {
                std::cout << "��ʼ���ŵ�" << Music << "�׸���" << std::endl;
                Start_Music(file);
                std::cout << "��" << Music << "�׸����������" << std::endl;
                Music++;

                std::cout << "������µ�����:";
                std::cin >> Cin_Name;
                if (Cin_Name == GetFilesName(file)) {
                    std::cout << GetRandLine("./Statements_R.txt") << std::endl;
                    Guess_Right++;
                } else {
                    std::cout << GetRandLine("./Statements_W.txt") << std::endl;
                    std::cout << "��ȷ����:" << GetFilesName(file) << std::endl;
                    Wrong_Guess++;
                }
                system("pause");
                system("cls");
            }
            std::cout << "�ø赥�������" << std::endl;
            break;
        } else if (Choose == 2) {
            Select_Song_List("./MusicList");
            if (files.empty()) {
                std::cout << "./MusicList�ļ�������.MP3�ļ�, ����" << std::endl;
                system("pause");
                return;
            }

            for (int i = 0; i < files.size(); ++i) {
                system("cls");
                std::cout << "��ʼ���ŵ�" << i << "�׸���" << std::endl;
                Start_Music(files[i]);
                std::cout << "��" << i << "�׸����������" << std::endl;
                Music++;

                std::cout << "������µ�����:";
                std::string Cin_Name;
                std::cin >> Cin_Name;
                if (Cin_Name == GetFilesName(files[i])) {
                    std::cout << GetRandLine("./Statements_R.txt") << std::endl;
                    Guess_Right++;
                } else {
                    std::cout << GetRandLine("./Statements_W.txt") << std::endl;
                    std::cout << "��ȷ����:" << GetFilesName(files[i]) << std::endl;
                    Wrong_Guess++;
                }
                system("pause");
                system("cls");
            }
            std::cout << "�ø赥�������" << std::endl;
            break;
        }
    std::cout << "��һ�������" << Guess_Right << "����\n" << "�����" << Wrong_Guess << "����" << std::endl;
    std::cout << "�����ȷ����:" << (Guess_Right / Music) * 100 << "%" << std::endl;
    std::cout << "                   -----" << Name << std::endl;
    system("pause");
}

void GuessSong_Start() {
    Play_Music();
}

void Music_Duration() {
    std::cout << "������ʱ��(s):";
    std::cin >> Music_Time;
    if (Music_Time <= 0) {
        std::cout << "����ʱ������С�ڵ�����" << std::endl;
        system("pause");
        return;
    } else if (Music_Time > 60) {
        std::cout << "����ʱ�����ܴ���60s" << std::endl;
        system("pause");
        return;
    } else {
        std::cout << "ʱ��������Ϊ:" << Music_Time << "s" << std::endl;
        Music_Time_Editor = Music_Time * 1000;
        Json_Editor("Music_Time", Music_Time_Editor);
        system("pause");
        return;
    }
}

void Change_User_Name() {
    std::cout << "���������û���:";
    std::cin >> Name;

    std::cout << "�û����Ѹ���Ϊ:" << Name << std::endl;
    Name_Editor = Name;
    Json_Editor("Name", Name_Editor);
    system("pause");
}

void Setting() {
    int Setting;
    std::cout << "------------Setting------------\n1.������ȡʱ��\n2.�����û���" << std::endl;
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
    std::cout << "1.��ʼ�¸�\n2.����" << std::endl;
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