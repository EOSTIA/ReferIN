#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

// ---------------------------------------------------------------
// UserRecord: the full record loaded from generated_data.csv.
// phone_number is used as the unique key (userId) across the
// tree, hash index, B-Tree, and cache.
// ---------------------------------------------------------------
struct UserRecord {
    std::string phone;   // unique id / key
    std::string name;
    std::string email;
    std::string city;
    int age = 0;
    std::string sex;
    std::string job;
    int followers = 0;
};

// ---------------------------------------------------------------
// CsvLoader: reads generated_data.csv into a vector<UserRecord>.
// Expected header: phone_number,name,email,city,age,sex,job,followers
// ---------------------------------------------------------------
class CsvLoader {
public:
    static std::vector<UserRecord> load(const std::string& path) {
        std::vector<UserRecord> records;
        std::ifstream file(path);
        if (!file.is_open()) return records;

        std::string line;
        std::getline(file, line); // skip header

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            auto fields = split(line, ',');
            if (fields.size() < 8) continue;

            UserRecord r;
            r.phone     = fields[0];
            r.name      = fields[1];
            r.email     = fields[2];
            r.city      = fields[3];
            r.age       = std::stoi(fields[4]);
            r.sex       = fields[5];
            r.job       = fields[6];
            r.followers = std::stoi(fields[7]);
            records.push_back(std::move(r));
        }
        return records;
    }

private:
    static std::vector<std::string> split(const std::string& line, char delim) {
        std::vector<std::string> out;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, delim)) out.push_back(item);
        return out;
    }
};
