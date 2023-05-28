//
// Created by Gyuhwan Park on 2023/05/26.
//

#include <memory>
#include <thread>
#include <chrono>

#include "catch_amalgamated.hpp"

#include "../UnixSocket.hpp"
#include "../IPCConnection.hpp"

#include "MockedUnixSocket.hpp"

using namespace std::chrono_literals;

TEST_CASE("should perform read() correctly", "[IPCConnection]") {
    auto unixSocket = std::make_shared<MockedUnixSocket>("/tmp/ipcserver.sock");
    auto connection = std::make_shared<IPCConnection>(unixSocket);

    REQUIRE_NOTHROW(connection->connect());
    REQUIRE(connection->descriptor() == 5);

    auto mockedData = std::shared_ptr<uint8_t>(new uint8_t[32] {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    });

    unixSocket->setMockedData(mockedData, 32);

    auto result = connection->read<uint8_t>(32);

    REQUIRE(memcmp(result.get(), mockedData.get(), 32) == 0);
    REQUIRE_NOTHROW(connection->disconnect());
}

TEST_CASE("should perform write() correctly", "[IPCConnection]") {
    auto unixSocket = std::make_shared<MockedUnixSocket>("/tmp/ipcserver.sock");
    auto connection = std::make_shared<IPCConnection>(unixSocket);

    REQUIRE_NOTHROW(connection->connect());
    REQUIRE(connection->descriptor() == 5);

    auto data = std::shared_ptr<uint8_t>(new uint8_t[32] {
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    });

    connection->write(data.get(), 32);

    // N O N - B L O C K I N G
    std::this_thread::sleep_for(500ms);

    auto writtenData = unixSocket->writtenData();
    auto dataLength = unixSocket->writtenDataLength();

    REQUIRE(memcmp(writtenData.get(), data.get(), 32) == 0);
    REQUIRE(dataLength == 32);
    REQUIRE_NOTHROW(connection->disconnect());
}
