/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240709
 * FilePath: /zio/head/BackendManager.hpp
 * Description:Implementations related to backend server management and scheduling
 */

#pragma once
#include "system.hpp"

enum class BackendStatus
{
    Dead,
    Active,
    Unknown
};

class Backend : NonCopyable
{
private:
    std::string _address;
    BackendStatus _status = BackendStatus::Dead;
    std::mutex _backendMutex;

public:
    Backend();
    ~Backend();
    Backend(Backend &&temp);
    inline std::string getAddress()
    {
        return _address;
    }
};

class BackendManager : NonCopyable
{
    friend Backend;

private:
    std::vector<Backend> _backendArry;
    int _pos;
    std::mutex _mangagerMutex;
    std::unordered_set<std::string> _backendExistCheck;

public:
    BackendManager();
    ~BackendManager();
    BackendManager(BackendManager &&temp);
    /**
     * @description: Used to add backend servers to the queue.it is Thread-safety.
     * @param {Backend} & Accepts only rvalue addition
     * @return {*}
     */
    void pushBack(Backend &&temp);
    /**
     * @description:Obtain a backend server instance.it is Thread-safety.
     * @return {*}
     */
    Backend &getBackEnd();

    /**
     * @description:Check if a server with the specified address is registered.it is Thread-safety
     * @param {string_view} temp
     * @return {*}
     */
    bool checkExist(std::string_view temp);
};
