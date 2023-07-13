#pragma once

#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

class CRest {
private:
    string url = "192.168.100.101:3000";
    json data;

public:
    CRest();
    json send(string method, string path, json body);
    string getUrlFromText();
    void setUrl(string _url);
};