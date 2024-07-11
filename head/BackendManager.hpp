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
    decltype(std::chrono::steady_clock::now()) _timeStamp = std::chrono::steady_clock::now();

public:
    Backend();
    ~Backend();
    Backend(Backend &&temp);
    /**
     * @description: Obtain the service endpoint address.it is Thread-safe.
     * @return {*} example:"127.0.0.1:7070"
     */
    inline std::string getAddress()
    {
        std::lock_guard<std::mutex> lock(_backendMutex);
        return _address;
    }
    /**
     * @description: Obtain the service endpoint address.
     * @return {*} example:"127.0.0.1:7070"
     */
    inline std::string getAddressNoMutex()
    {
        return _address;
    }
    /**
     * @description: Set server address.it is Thread-safe.
     * @param {string_view} address:example is "127.0.0.1:7070"
     * @return {*}
     */
    inline void setAddress(std::string_view address)
    {
        std::lock_guard<std::mutex> lock(_backendMutex);
        _address = address;
    }
    /**
     * @description: Set server address
     * @param {string_view} address:example is "127.0.0.1:7070"
     * @return {*}
     */
    inline void setAddressNoMutex(std::string_view address)
    {
        _address = address;
    }
    /**
     * @description: update server status.it is Thread-safe.
     * @param {BackendStatus} value
     * @return {*}
     */
    inline void setStatus(BackendStatus value)
    {
        std::lock_guard<std::mutex> lock(_backendMutex);
        _status = value;
    }
    /**
     * @description: update server status.
     * @param {BackendStatus} value
     * @return {*}
     */
    inline void setStatusNoMutex(BackendStatus value)
    {
        _status = value;
    }
    /**
     * @description: update _timeStamp
     * @return {*}
     */
    inline void updateTimeStampNoMutex()
    {
        _timeStamp = std::chrono::steady_clock::now();
    }
    /**
     * @description: update _timeStamp.it is Thread-safe.
     * @return {*}
     */
    inline void updateTimeStamp()
    {
        std::lock_guard<std::mutex> lock(_backendMutex);
        _timeStamp = std::chrono::steady_clock::now();
    }

    /**
     * @description: Acquire the lock for Backend.
     * @return {*}
     */
    std::mutex &getMutex()
    {
        return getMutex();
    }

    /**
     * @description: it is Thread-safe. get timestamp
     * @return {*}
     */
    std::chrono::steady_clock::time_point &getTimeStamp()
    {
        std::lock_guard<std::mutex> lock(_backendMutex);
        return _timeStamp;
    }
    /**
     * @description: it is Thread-safe. get timestamp
     * @return {*}
     */
    std::chrono::steady_clock::time_point &getTimeStampNoMutex()
    {
        return _timeStamp;
    }
};

using BackendVector = std::vector<Backend>;
class BackendManager : NonCopyable
{
    friend Backend;

private:
    // This container will never delete elements, it will only add them
    BackendVector _backendArry;
    int _pos;
    std::mutex _mangagerMutex;
    // This container will never delete elements, it will only add them
    std::unordered_map<std::string, int> _backendExistCheck;
    static std::chrono::milliseconds _timeout;

public:
    BackendManager();
    ~BackendManager();
    BackendManager(BackendManager &&temp);
    /**
     * @description: Used to add backend servers to the queue.it is Thread-safety.
     * @param {Backend} & Accepts only rvalue addition
     * @return {*}
     */
    BackendVector::iterator pushBack(Backend &&temp);
    /**
     * @description: Used to add backend servers to the queue.
     * @param {Backend} & Accepts only rvalue addition
     * @return {*}
     */
    BackendVector::iterator pushBackNoMutex(Backend &&temp);
    /**
     * @description:Obtain a backend server instance.it is Thread-safety.
     * @return {*}
     */
    std::vector<Backend>::iterator getBackEnd();
    /**
     * @description:Obtain a backend server instance.
     * @return {*}
     */
    std::vector<Backend>::iterator getBackEndNoMutex();
    /**
     * @description:Check if a server with the specified address is registered.it is Thread-safety
     * @param {string_view} temp
     * @return {*}
     */
    bool checkExist(std::string_view temp);
    /**
     * @description:Check if a server with the specified address is registered.
     * @param {string_view} temp
     * @return {*}
     */
    bool checkExistNoMutex(std::string_view temp);
    /**
     * @description: Obtain the service endpoint for the specified address.it is Thread-safety
     * @param {string_view} address example："127.0.0.1:1080"
     * @return {*}
     */
    BackendVector::iterator getBackEnd(std::string_view address);
    /**
     * @description: Obtain the service endpoint for the specified address
     * @param {string_view} address example："127.0.0.1:1080"
     * @return {*}
     */
    BackendVector::iterator getBackEndNoMutex(std::string_view address);
    /**
     * @description: Acquire the lock for BackendManager
     * @return {*}
     */
    std::mutex &getBackendManagerMutex()
    {
        return _mangagerMutex;
    }

    /**
     * @description: get _backendArry of iter end
     * @return {*}
     */
    BackendVector::iterator getBackVectorEndIterNoMutex()
    {
        return _backendArry.end();
    }
    /**
     * @description: get _backendArry of iter end.it is Thread-safety
     * @return {*}
     */
    BackendVector::iterator getBackVectorEndIter()
    {
        std::lock_guard<std::mutex> lock(_mangagerMutex);
        return getBackVectorEndIterNoMutex();
    }

    /**
     * @description: Obtain the timeout duration
     * @return {*}
     */
    std::chrono::milliseconds &getTimeout()
    {
        return _timeout;
    }
};
