//Author: Maksim Shapiro
#include <string>
#include <iostream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <cmath>

#include "eventHandler.h"

void EventHandler::ParseFile(const std::string& fileName) {
    std::fstream file;
    file.open(fileName);
    if (file.is_open()) {
        std::string line;
        try {
            //parse number of tables
            std::getline(file, line);
            parseNumberOfTables(line);
            for (int i = 0; i < m_numberOfTables; i++) {
                m_tableIsBusy.push_back(false);
                m_tableStartTime.push_back(TimeType());
                m_profit.push_back(0);
                m_tableTimeCount.push_back(0);
            }
            //parse start and finish times
            std::getline(file, line);
            parseWorkingHours(line);
            //parse price
            std::getline(file, line);
            parsePrice(line);   
            //parse events
            while (getline(file, line)) {
                parseInputEvent(line);
            }
        } catch(...) {
            file.close();
            throw std::logic_error(line); 
        }
    }
    file.close();
}

bool EventHandler::isUInt(const std::string& number) {
    auto it = std::find_if(number.begin(), number.end(), [](char const &c) {
        return !std::isdigit(c);
    });
    return !number.empty() && it == number.end();
}

bool EventHandler::checkClientFormat(const std::string& name) {
    return std::regex_match(name, std::regex("^[a-z0-9_\-]+$"));
}

void EventHandler::parseNumberOfTables(const std::string& number) {
    if (!isUInt(number))
        throw std::logic_error("Invalid number of tables format");
    int64_t parsedNumber = stoi(number);
    m_numberOfTables = parsedNumber;
}

void EventHandler::parseWorkingHours(const std::string& workingHours) {
    if (workingHours.length() != 11 || workingHours[5] != ' ')
        throw std::logic_error("Invalid working hours format");
    m_startTime.parseString(workingHours.substr(0, 5));
    m_finishTime.parseString(workingHours.substr(6, 5));
    if (m_startTime >= m_finishTime)
        throw std::logic_error("Invalid working hours format");
}

void EventHandler::parsePrice(const std::string& number) {
    if (!isUInt(number))
        throw std::logic_error("Invalid price format");
    int64_t parsedNumber = stoi(number);
    m_price = parsedNumber;
}

void EventHandler::parseInputEvent(const std::string& line) {
    if (line[5] != ' ' || line[7] != ' ' || charToInt(line[6]) < 1 || charToInt(line[6]) > 4)
        throw std::logic_error("Invalid input event format");
    InputEvent parsedEvent;
    parsedEvent.m_time.parseString(line.substr(0, 5));
    if (m_events.size() && m_events.back().m_time > parsedEvent.m_time)
        throw std::logic_error("Input event can't precede previous event");
    parsedEvent.m_id = event_id(charToInt(line[6]));
    for (int i = 8; i < line.length(); i++) {
        if (line[i] == ' ')
            break;
        parsedEvent.m_clientName.push_back(line[i]);
    }
    if (!checkClientFormat(parsedEvent.m_clientName)) {
        throw std::logic_error("Invalid input event format");
    }
    if (parsedEvent.m_id == CLIENT_SAT) {
        std::string tableId = line.substr(9 + parsedEvent.m_clientName.length());
        if (!isUInt(tableId))
            throw std::logic_error("Invalid table id format");
        parsedEvent.m_tableId = stoi(tableId);
        if (!parsedEvent.m_tableId)
            throw std::logic_error("Invalid table id format");
    } else if (line.length() > 9 + parsedEvent.m_clientName.length())
        throw std::logic_error("Invalid input event format");
    m_events.push_back(parsedEvent);
}

void EventHandler::processEvents() {
    std::cout << m_startTime.toString() << "\n";
    for (auto& event : m_events) {
        event.print();
        switch (event.m_id) {
        case CLIENT_CAME:
            processCameEvent(event);
            break;
        case CLIENT_SAT:
            processSitEvent(event);
            break;
        case CLIENT_WAITING:
            processWaitEvent(event);
            break;
        case CLIENT_LEFT:
            processLeaveEvent(event);
            break;
        }
    }
    processEndOfDay();
}

void EventHandler::processLeaveEvent(InputEvent& event) {
    Client currentClient(event.m_clientName);
    if (!m_clients.count(currentClient)) {
        std::cout << event.m_time.toString() << " 13 ClientUnknown\n";
        return;
    }
    if (m_clients[currentClient] > 0) {
        u_int32_t tableIdMinus1 = m_clients[currentClient] - 1;
        TimeType time = event.m_time;
        time -= m_tableStartTime[tableIdMinus1];
        m_profit[tableIdMinus1] += ceil(double(time.minute) / 60) * m_price;
        m_tableTimeCount[tableIdMinus1] += time.minute;
        m_tableIsBusy[tableIdMinus1] = false;
        if (m_queue.size()) {
            Client client = *m_queue.begin();
            m_queue.pop_front();
            m_clients[client] = tableIdMinus1 + 1;
            m_tableIsBusy[tableIdMinus1] = true;
            m_tableStartTime[tableIdMinus1] = event.m_time;
            std::cout << event.m_time.toString() << " 12 " << client.getName() << " " << tableIdMinus1 + 1 << "\n";
        }
        m_clients.erase(currentClient);
    } else {
        auto it = std::find_if(m_queue.begin(), m_queue.end(), [currentClient](Client client) {
            return client.getName() == currentClient.getName();
        });
        if (it != m_queue.end())
            m_queue.erase(it);
    }
}

void EventHandler::processWaitEvent(InputEvent& event) {
    auto it = std::find_if(m_tableIsBusy.begin(), m_tableIsBusy.end(), [](bool status) {
        return !status;
    });
    if (it != m_tableIsBusy.end()) {
        std::cout << event.m_time.toString() << " 13 ICanWaitNoLonger!\n";
        return;
    }
    if (m_queue.size() > m_numberOfTables) {
        std::cout << event.m_time.toString() << " 11 " << event.m_clientName << "\n";
        m_clients.erase(Client(event.m_clientName));
    } else {
        m_queue.push_back(Client(event.m_clientName));
    }
}

void EventHandler::processSitEvent(InputEvent& event) {
    Client currentClient = Client(event.m_clientName);
    if (!m_clients.count(currentClient)) {
        std::cout << event.m_time.toString() << " 13 ClientUnknown\n";
        return;
    }
    if (m_tableIsBusy[event.m_tableId - 1]) {
        std::cout << event.m_time.toString() << " 13 PlaceIsBusy\n";
        return;
    }
    if (m_clients[currentClient])
        m_tableIsBusy[m_clients[currentClient] - 1] = false;
    m_clients[currentClient] = event.m_tableId;
    m_tableIsBusy[event.m_tableId - 1] = true;
    m_tableStartTime[event.m_tableId - 1] = event.m_time;
}

void EventHandler::processCameEvent(InputEvent& event) {
    if (event.m_time < m_startTime) {
        std::cout << event.m_time.toString() << " 13 NotOpenYet\n";
        return;
    }
    if (m_clients.count(Client(event.m_clientName))) {
        std::cout << event.m_time.toString() << " 13 YouShallNotPass\n";
        return;
    }
    m_clients[Client(event.m_clientName)] = 0;
}

void EventHandler::processEndOfDay() {
    for (auto client : m_clients) {
        std::cout << m_finishTime.toString() << " 11 " << client.first.getName() << "\n";
        if (!client.second)
            continue;
        u_int16_t tableIdMinus1 = client.second - 1;
        TimeType time = m_finishTime;
        time -= m_tableStartTime[tableIdMinus1];
        m_profit[tableIdMinus1] += ceil(double(time.minute) / 60) * m_price;
        m_tableTimeCount[tableIdMinus1] += time.minute;
    }
    std::cout << m_finishTime.toString() << "\n";
    for (int i = 1; i <= m_numberOfTables; i++) {
        TimeType time;
        time.minute = m_tableTimeCount[i - 1];
        std::cout << i << " " << m_profit[i - 1] << " " << time.toString() << "\n";
    }
}