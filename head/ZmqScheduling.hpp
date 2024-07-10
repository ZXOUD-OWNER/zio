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

class ZmqScheduling : NonCopyable
{
private:
    std::unordered_map<ServerSerial, BackendManager> _backendSelect;
    std::mutex _backendSelectMutex;

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
    void serverManager();
    /**
     * @description: Start backend service management.
     * @return {*}
     */
    void startServerManager();
    /**
     * @description: backend service management. Related to service registration, service status monitoring, and other aspects
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