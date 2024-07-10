/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240709
 * FilePath: /zio/src/BackendManager.cpp
 * Description:Implementations related to backend server management and scheduling
 */

#include "head.hpp"

BackendManager::BackendManager()
{
}

BackendManager::~BackendManager()
{
}

BackendManager::BackendManager(BackendManager &&temp)
{
    _backendArry = std::move(temp._backendArry);
}

void BackendManager::pushBack(Backend &&temp)
{
    std::lock_guard<std::mutex> _lock(_mangagerMutex);
    _backendArry.emplace_back(std::move(temp));
}

Backend &BackendManager::getBackEnd()
{
    std::lock_guard<std::mutex> _lock(_mangagerMutex);
    _pos = _pos % _backendArry.size();
    return _backendArry[_pos++];
}

bool BackendManager::checkExist(std::string_view temp)
{
    std::lock_guard<std::mutex> lock(_mangagerMutex);
    return _backendExistCheck.contains(std::string(temp));
}

Backend::Backend()
{
}
Backend::~Backend()
{
}

Backend::Backend(Backend &&temp)
{
    _address = std::move(temp._address);
    _status = temp._status;
}
