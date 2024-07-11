/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240709
 * FilePath: /zio/src/BackendManager.cpp
 * Description:Implementations related to backend server management and scheduling
 */

#include "head.hpp"
std::chrono::milliseconds BackendManager::_timeout(1100);

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

BackendVector::iterator BackendManager::pushBack(Backend &&temp)
{
    std::lock_guard<std::mutex> _lock(_mangagerMutex);
    pushBackNoMutex(std::move(temp));
}
BackendVector::iterator BackendManager::pushBackNoMutex(Backend &&temp)
{
    _backendExistCheck.insert(std::pair<std::string, int>(temp.getAddress(), _backendArry.size()));
    _backendArry.emplace_back(std::move(temp));
    auto iter = _backendArry.begin();
    std::advance(iter, _backendArry.size());
    return iter;
}

std::vector<Backend>::iterator BackendManager::getBackEnd()
{
    std::lock_guard<std::mutex> _lock(_mangagerMutex);
    return getBackEndNoMutex();
}

std::vector<Backend>::iterator BackendManager::getBackEndNoMutex()
{
    _pos = _pos % _backendArry.size();
    auto iter = _backendArry.begin();
    std::advance(iter, _pos);
    return iter;
}

bool BackendManager::checkExist(std::string_view temp)
{
    std::lock_guard<std::mutex> lock(_mangagerMutex);
    return checkExistNoMutex(temp);
}

bool BackendManager::checkExistNoMutex(std::string_view temp)
{
    return _backendExistCheck.contains(std::string(temp));
}

std::vector<Backend>::iterator BackendManager::getBackEnd(std::string_view address)
{
    std::lock_guard<std::mutex> lock(_mangagerMutex);
    return getBackEndNoMutex(address);
}

std::vector<Backend>::iterator BackendManager::getBackEndNoMutex(std::string_view address)
{
    std::string key(address);
    auto iter = _backendExistCheck.find(key);
    if (iter == _backendExistCheck.end())
    {
        LOG(ERROR) << "specify address not found";
        return _backendArry.end();
    }
    else
    {
        std::vector<Backend>::iterator res = _backendArry.begin();
        std::advance(res, iter->second);
        return res;
    }
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
