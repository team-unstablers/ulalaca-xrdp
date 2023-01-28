//
// Created by Gyuhwan Park on 2023/01/28.
//

#ifndef ULALACA_SESSIONBROKERCLIENT_HPP
#define ULALACA_SESSIONBROKERCLIENT_HPP

#include <string>

#include "IPCConnection.hpp"

struct SessionBrokerResponse {
    bool isSuccessful;
    std::string sessionPath;
    int reason;
};

class SessionBrokerClient {
public:
    SessionBrokerClient(const std::string &socketPath);
    ~SessionBrokerClient();

    static std::string getMessage(int reason);

    /**
     * authenticate and requests session
     * @return SessionBrokerResponse
     */
    SessionBrokerResponse requestSession
        (const std::string &username, const std::string &password);

private:
    std::string _socketPath;
};


#endif //ULALACA_SESSIONBROKERCLIENT_HPP
