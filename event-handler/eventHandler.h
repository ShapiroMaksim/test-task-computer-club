//Author: Maksim Shapiro
#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <iostream>
#include <stdexcept>

enum event_id {
    CLIENT_CAME = 1,
    CLIENT_SAT = 2,
    CLIENT_WAITING = 3,
    CLIENT_LEFT = 4,
};

struct TimeType {
    u_int16_t minute;

    TimeType() {
        minute = 0;
    }
    TimeType(u_int16_t value) {
        minute = value;
    }
    void parseString(std::string timeStr) {
        if (timeStr.length() != 5 || timeStr[2] != ':')
            throw std::logic_error("Invalid time format");
        minute = stoi(timeStr.substr(0, 2)) * 60;
        minute += stoi(timeStr.substr(3, 2));
    }
    std::string toString() {
        std::string result;
        int hour = minute / 60;
        result += hour > 9 ? std::to_string(hour) : ("0" + std::to_string(hour));
        result += ":";
        result += (minute % 60) > 9 ? std::to_string((minute % 60)) : ("0" + std::to_string((minute % 60)));
        return result;
    }
    bool operator<(const TimeType& right) const { return minute < right.minute; }
    bool operator>(const TimeType& right) const { return minute > right.minute; }
    bool operator>=(const TimeType& right) const { return minute >= right.minute; }
    TimeType operator-(const TimeType& right) { return TimeType(minute - right.minute); }
    TimeType& operator -= (const TimeType& right) {
        minute -= right.minute;
        return *this;
    }
};

struct InputEvent {
    TimeType m_time;
    event_id m_id;
    std::string m_clientName;
    u_int32_t m_tableId = 0; //0 - invalid id
    void print() {
        std::cout << m_time.toString() << " " << m_id << " " << m_clientName;
        if (m_tableId)
            std::cout << " " << m_tableId;
        std::cout << "\n";
    }
};

class Client {
    std::string m_name;

public:
    Client(std::string name) {
        m_name = name;
    }
    std::string getName() const {
        return m_name;
    }
    bool operator<(const Client& right) const { //for correct sorting in std::map
        std::string leftName = m_name;
        std::string rightName = right.getName();
        u_int32_t minLength = leftName.length() < rightName.length() ? leftName.length() : rightName.length();
        for (int i = 0; i < minLength; i++) {
            if (leftName[i] == rightName[i])
                continue;
            if (rightName[i] == '-')
                return true;
            if (leftName[i] == '-')
                return false;
            if (leftName[i] == '_' && rightName[i] == '-')
                return true;
            else if (leftName[i] == '_')
                return false;
            if (rightName[i] == '_' && leftName[i] == '-')
                return false;
            else if (rightName[i] == '_')
                return true;
            if (leftName[i] >= '0' && leftName[i] <= '9' && rightName[i] > '9')
                return false;
            if (rightName[i] >= '0' && rightName[i] <= '9' && leftName[i] > '9')
                return true;
            return leftName[i] < rightName[i];
        }
        return leftName.length() < rightName.length(); 
    }
};

class EventHandler {
    u_int32_t m_numberOfTables;
    std::vector<bool> m_tableIsBusy;
    std::vector<u_int32_t> m_profit;
    std::vector<u_int16_t> m_tableTimeCount;
    std::list<Client> m_queue;
    TimeType m_startTime;
    TimeType m_finishTime;
    u_int32_t m_price;
    std::list<InputEvent> m_events;
    std::map<Client, u_int32_t> m_clients;
    std::vector<TimeType> m_tableStartTime;

public:
    EventHandler() {}
    ~EventHandler() {}
    void ParseFile(const std::string& fileName);
    void processEvents();
private:
    bool isUInt(const std::string& number);
    int charToInt(const char& c) {
        return int(c) - 48;
    }

    bool checkClientFormat(const std::string& name);
    void parseNumberOfTables(const std::string& number);
    void parseWorkingHours(const std::string& workingHours);
    void parsePrice(const std::string& number);
    void parseInputEvent(const std::string& line);
    void processLeaveEvent(InputEvent& event);
    void processWaitEvent(InputEvent& event);
    void processSitEvent(InputEvent& event);
    void processCameEvent(InputEvent& event);
    void processEndOfDay();
};
