#include "utils.hpp"
#include "logger.hpp"

using namespace std;

string changePath(string srcPath, string relativePath, string postfix, string tag){
    int name_l = srcPath.rfind("/");
    int name_r = srcPath.rfind(".");

    int dir_l  = 0;
    int dir_r  = srcPath.rfind("/");

    string newPath;

    newPath = srcPath.substr(dir_l, dir_r + 1);
    newPath += relativePath;
    newPath += srcPath.substr(name_l, name_r - name_l);

    if (!tag.empty())
        newPath += "-" + tag + postfix;
    else
        newPath += postfix;

    return newPath;
}

vector<string> loadDataList(string file){
    vector<string> list;
    auto *f = fopen(file.c_str(), "r");
    if (!f) LOGE("Failed to open %s", file.c_str());

    char str[512];
    while (fgets(str, 512, f) != NULL) {
        for (int i = 0; str[i] != '\0'; ++i) {
            if (str[i] == '\n'){
                str[i] = '\0';
                break;
            }
        }
        list.push_back(str);
    }
    fclose(f);
    return list;
}
