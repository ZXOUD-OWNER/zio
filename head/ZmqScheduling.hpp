/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240709
 * FilePath: /zio/head/ZmqScheduling.hpp
 * Description:Implementations of service scheduling
 */

#pragma once
#include "system.hpp"
#include "BackendManager.hpp"
// Used to identify different services
enum class ServerSerial
{
    DataBase,
    Unknown
};

class ZmqClient : NonCopyable
{
private:
    std::string _address;
    zsock_t *_push;
    std::mutex _zmqClientMutex;

public:
    ZmqClient(std::string address);
    ~ZmqClient();
    ZmqClient(ZmqClient &&temp);
    /**
     * @description: Send scheduling response to the specified client.
     * @param {string} content
     * @return {*} Return the size of the sent data, return -1 if there is an error
     */
    int send(std::string content);
};

class ZmqScheduling : NonCopyable
{
private:
    // This container will never delete elements, it will only add them
    std::unordered_map<ServerSerial, BackendManager> _backendSelect;
    std::unordered_map<std::string, ZmqClient> _clientGroup;
    std::mutex _backendSelectMutex, _clientGroupMutex;
    nlohmann::json _errJson;
    nlohmann::json _resJson;

public:
    ZmqScheduling();
    ZmqScheduling(ZmqScheduling &&temp);
    ~ZmqScheduling();
    /**
     * @description: Retrieve backend services of a specified service type
     * @param {ServerSerial} serial：Service serial number, indicating the type of service
     * @return {*}
     */
    std::string getBackend(ServerSerial serial);
    /**
     * @description: Start backend service management.
     * @return {*}
     */
    void serverManager();
    /**
     * @description: backend service management. Related to service registration, service status monitoring, and other aspects
     * @return {*}
     */
    void startServerManager();

    /**
     * @description: service scheduling listener
     * @return {*}
     */
    void selectServer();
    /**
     * @description: Start service scheduling listener
     * @return {*}
     */
    void startSelectServer();
    /**
     * @description: Handling server-side periodic reporting information similar to keepalive
     * @param {unique_ptr<char[]>} & server-side reporting information
     * @return {*}
     */
    void handleServerRequest(std::unique_ptr<char[]> &&temp);
    /**
     * @description: server scheduling
     * @param {unique_ptr<char[]>} & dispatch request
     * @return {*}
     */
    void handleSelectServerRequest(std::unique_ptr<char[]> &&temp);
};