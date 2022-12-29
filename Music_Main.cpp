//
// Created by j3280 on 2022/12/26.
//
#include "Music_Func.h"

void writeFileJson_INIT() {
    Json::Value root;
    root["Music_Time"] = Json::Value(10000);
    root["Random_Play"] = Json::Value(true);
    root["Name"] = Json::Value("User");

    //输出到文件
    Json::StyledWriter sw;
    std::ofstream os;
    os.open("Config.json", std::ios::out);
    if (!os.is_open())
        std::cout << "error" << std::endl;
    os << sw.write(root);
    os.close();
}

bool isFileExists_access(const char *name) {
    return (access(name, F_OK) != -1);
}

int main() {
    while (true) {
        system("cls");
        if (isFileExists_access("./Config.json") == 1) {
            //        有
            Read_Json();
        } else {
            std::ofstream MyFile("Config.json");
            writeFileJson_INIT();
        }
        Hello_User();
    }
}