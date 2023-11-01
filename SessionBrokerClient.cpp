//
// Created by Gyuhwan Park on 2023/01/28.
//

#include "SessionBrokerClient.hpp"

#include "messages/broker.h"

SessionBrokerClient::SessionBrokerClient(const std::string &socketPath):
    _socketPath(socketPath)
{

}

SessionBrokerClient::~SessionBrokerClient() {
    // TODO: ipcClient.close();
}

std::string SessionBrokerClient::getMessage(int reason) {
    switch (reason) {
        case REJECT_REASON_INTERNAL_ERROR:
            return "Unknown error (REJECT_REASON_INTERNAL_ERROR)";
        case REJECT_REASON_AUTHENTICATION_FAILED:
            return "Authentication Failed";
        case REJECT_REASON_SESSION_NOT_AVAILABLE:
            return "Session not available";
        case REJECT_REASON_INCOMPATIBLE_VERSION:
            return "Incompatible version";
        default:
            return "Unknown error";
    }

}

SessionBrokerResponse SessionBrokerClient::requestSession(const std::string &username, const std::string &password) {
    // FIXME: use std::promise instaead
    IPCConnection connection(_socketPath);
    SessionBrokerResponse response;

    connection.connect();
    {
        ULIPCSessionRequest request {};
        std::strncpy(&request.username[0], username.c_str(), sizeof(request.username));
        std::strncpy(&request.password[0], password.c_str(), sizeof(request.password));

        connection.writeMessage(TYPE_SESSION_REQUEST, request);
        std::memset(&request.password, 0x00, sizeof(request.password));

        auto responseHeader = connection.nextHeader();

        switch (responseHeader->messageType) {
            case TYPE_SESSION_REQUEST_RESOLVED: {
                auto body = connection.readBlock<ULIPCSessionRequestResolved>(
                    sizeof(ULIPCSessionRequestResolved)
                );

                response = SessionBrokerResponse {
                    true,
                    std::string(body->path),
                    0
                };
            }
                break;

            case TYPE_SESSION_REQUEST_REJECTED: {
                auto body = connection.readBlock<ULIPCSessionRequestRejected>(
                        sizeof(ULIPCSessionRequestRejected)
                );

                response = SessionBrokerResponse {
                    false,
                    "",
                    body->reason
                };
            }
                break;
            default:
                response = SessionBrokerResponse {
                    false,
                    "",
                    -1
                };
        }
    }

    connection.disconnect();

    return std::move(response);
}
