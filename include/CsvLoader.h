#pragma once // preprocessor directive used in header files to ensure that a file is included only once during a single compilation
#include <bits/stdc++.h>
using namespace std;

// phone_number is used as the unique key (userId) across the
// tree, hash index, B-Tree, and cache.

struct UserRecord
{
    string phone; // unique id / key
    string name;
    string email;
    string city;
    int age = 0;
    string sex;
    string job;
    int followers = 0;
};

// CsvLoader: reads generated_data.csv into a vector<UserRecord>.


class CsvLoader
{
public:
    static vector<UserRecord> load(const string &path)
    {
        vector<UserRecord> records;
        ifstream file(path);
        if (!file.is_open())
            return records;

        string line;
        getline(file, line); // skip header

        while (getline(file, line))
        {
            if (line.empty())
                continue;
            auto fields = split(line, ',');
            if (fields.size() < 8)
                continue;

            UserRecord r;
            r.phone = fields[0];
            r.name = fields[1];
            r.email = fields[2];
            r.city = fields[3];
            r.age = stoi(fields[4]);
            r.sex = fields[5];
            r.job = fields[6];
            r.followers = stoi(fields[7]);
            records.push_back(move(r));
        }
        return records;
    }

private:
    static vector<string> split(const string &line, char delim)
    {
        vector<string> out;
        stringstream ss(line);
        string item;
        while (getline(ss, item, delim))
            out.push_back(item);
        return out;
    }
};
