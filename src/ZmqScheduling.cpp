#include "head.hpp"

ZmqScheduling::ZmqScheduling()
{
    std::string errJsonStr("{\
    \"responseType\":-1,\
    \"errStr\":\"\",\
    \"RequestId\":0,\
    \"errInt\":0\
    }");
    _errJson = nlohmann::json::parse(errJsonStr);

    std::string responseStr("{\
    \"RequestId\":0,\
    \"responseType\":0,\
    \"ServerSerial\":0,\
    \"ServerAddress\":\"127.0.0.1:10086\"\
    }");

    _resJson = nlohmann::json::parse(responseStr);
    startSelectServer();
    startServerManager();
}
// Not thread-safe, need to be cautious
ZmqScheduling::ZmqScheduling(ZmqScheduling &&temp)
{
    _backendSelect = std::move(temp._backendSelect);
}
ZmqScheduling::~ZmqScheduling()
{
}

std::string ZmqScheduling::getBackend(ServerSerial serial)
{
    auto iter = _backendSelect.find(serial);
    auto &backendArry = iter->second;
    return backendArry.getBackEnd()->getAddress();
}

void ZmqScheduling::serverManager()
{
    auto puller = zsock_new_pull("tcp://10.0.20.3:10086");
    if (puller == nullptr)
    {
        LOG(FATAL) << "zsock_new_pull(tcp://10.0.20.3:10086) err!";
    }
    while (true)
    {
        auto resquest = std::unique_ptr<char[]>(zstr_recv(puller));
        handleServerRequest(std::move(resquest));
    }
}

void ZmqScheduling::handleServerRequest(std::unique_ptr<char[]> &&temp)
{
    nlohmann::json requestJson;
    try
    {
        requestJson = nlohmann::json::parse(temp.get());
    }
    catch (nlohmann::json::parse_error &e)
    {
        LOG(ERROR) << "json parse err! err is " << e.what() << " str is " << temp.get();
    }

    try
    {
        auto iterRequestId = requestJson.find("RequestId");
        auto iterResquestType = requestJson.find("ResquestType");
        auto iterServerSerial = requestJson.find("ServerSerial");
        auto iterServerAddress = requestJson.find("ServerAddress");
        auto iterStatus = requestJson.find("Status");

        auto serverKind = iterServerSerial.value().get<int>();
        if (serverKind >= CUitl::toUType(ServerSerial::Unknown))
        {
            LOG(ERROR) << "not exist this service.specify ServerSerial is " << serverKind;
            return;
        }
        // Query if the service group exists. If it does, get it. If it doesn't, generate a service group instance.
        std::unordered_map<ServerSerial, BackendManager>::iterator iterBackendArry;
        {
            std::lock_guard<std::mutex> lock(_backendSelectMutex);
            iterBackendArry = _backendSelect.find(static_cast<ServerSerial>(serverKind));
            if (iterBackendArry == _backendSelect.end())
            {
                auto iterPair = _backendSelect.insert(std::pair<ServerSerial, BackendManager>(static_cast<ServerSerial>(serverKind), BackendManager()));
                if (iterPair.second)
                {
                    LOG(INFO) << "Backend service manager registration success!";
                    iterBackendArry = iterPair.first;
                }
                else
                {
                    LOG(ERROR) << "Backend service manager registration failed" << CUitl::printTrace();
                    return;
                }
            }
        }
        // Check if the specified service endpoint exists within the designated service group. If it does, retrieve it. If it doesn't, register it in the specified service group.
        BackendVector::iterator iterBackend;
        {
            std::lock_guard<std::mutex> lock(iterBackendArry->second.getBackendManagerMutex());
            auto flag = iterBackendArry->second.checkExistNoMutex(iterServerAddress.value().get<std::string>());
            if (flag)
            {
                iterBackend = iterBackendArry->second.getBackEndNoMutex(iterServerAddress.value().get<std::string>());
            }
            else
            {
                Backend serverInstance;
                serverInstance.setStatusNoMutex(BackendStatus::Active);
                serverInstance.setAddressNoMutex(iterServerAddress.value().get<std::string>());
                iterBackend = iterBackendArry->second.pushBackNoMutex(std::move(serverInstance));
            }
            if (iterBackend == iterBackendArry->second.getBackVectorEndIterNoMutex())
            {
                LOG(ERROR) << "occur err!BackendManager not add success!" << CUitl::printTrace();
                return;
            }
        }
        iterBackend->updateTimeStamp();
    }
    catch (const std::exception &e)
    {
        LOG(ERROR) << e.what() << "has problem" << CUitl::printTrace();
    }
}

void ZmqScheduling::handleSelectServerRequest(std::unique_ptr<char[]> &&temp)
{
    nlohmann::json requestJson;
    try
    {
        requestJson = nlohmann::json::parse(temp.get());
    }
    catch (nlohmann::json::parse_error &e)
    {
        LOG(ERROR) << "json parse err! err is " << e.what() << " str is " << temp.get();
    }
    try
    {
        auto iterRequestId = requestJson.find("RequestId");
        auto iterServerSerial = requestJson.find("ServerSerial");
        auto iterClientAddress = requestJson.find("ClientAddress");
        auto serverKind = iterServerSerial.value().get<int>();
        if (serverKind >= CUitl::toUType(ServerSerial::Unknown))
        {
            LOG(ERROR) << "not exist this service.specify ServerSerial is " << serverKind;
            return;
        }

        std::unordered_map<ServerSerial, BackendManager>::iterator iterBackendArry;
        std::unordered_map<std::string, ZmqClient>::iterator iterClient;
        {
            std::lock_guard<std::mutex> lock(_clientGroupMutex);
            iterClient = _clientGroup.find(iterClientAddress.value().get<std::string>());
            if (iterClient == _clientGroup.end())
            {
                auto iterPair = _clientGroup.insert(std::pair<std::string, ZmqClient>(iterClientAddress.value().get<std::string>(), ZmqClient(iterClientAddress.value().get<std::string>())));
                if (iterPair.second)
                {
                    LOG(INFO) << "client registration success!";
                    iterClient = iterPair.first;
                }
                else
                {
                    LOG(ERROR) << "client registration fatal! address is " << iterClientAddress.value().get<std::string>() << CUitl::printTrace();
                    return;
                }
            }
        }
        {
            std::lock_guard<std::mutex> lock(_backendSelectMutex);
            iterBackendArry = _backendSelect.find(static_cast<ServerSerial>(serverKind));
            if (iterBackendArry == _backendSelect.end())
            {
                nlohmann::json err = _errJson;
                err["RequestId"] = requestJson["RequestId"];
                err["errStr"] = std::string("not found specify server group");
                err["errInt"] = 0;
                iterClient->second.send(err.dump());
                LOG(ERROR) << "not found specify server group" << CUitl::printTrace();
                return;
            }
        }
        BackendVector::iterator iterBackend;
        {
            std::lock_guard<std::mutex> lock(iterBackendArry->second.getBackendManagerMutex());

            while (iterBackend != iterBackendArry->second.getBackVectorEndIterNoMutex())
            {
                iterBackend = iterBackendArry->second.getBackEndNoMutex();
                auto oldPoint = iterBackend->getTimeStampNoMutex();
                auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - oldPoint);
                if (diff <= iterBackendArry->second.getTimeout())
                {
                    break;
                }
            }
            if (iterBackend == iterBackendArry->second.getBackVectorEndIterNoMutex())
            {
                nlohmann::json err = _errJson;
                err["RequestId"] = requestJson["RequestId"];
                err["errStr"] = std::string("server group not exist available backend");
                err["errInt"] = 1;
                iterClient->second.send(err.dump());
                LOG(ERROR) << "server group not exist available backend" << CUitl::printTrace();
                return;
            }
        }
        {
            nlohmann::json response = _resJson;
            response["ServerSerial"] = requestJson["ServerSerial"];
            response["RequestId"] = requestJson["RequestId"];
            response["ServerAddress"] = iterBackend->getAddress();
            iterClient->second.send(response.dump());
        }
    }
    catch (const std::exception &e)
    {
        LOG(ERROR) << e.what() << " " << CUitl::printTrace();
    }
}

void ZmqScheduling::startServerManager()
{
    std::thread run(&ZmqScheduling::serverManager, this);
    run.detach();
}
void ZmqScheduling::selectServer()
{
    auto puller = zsock_new_pull("tcp://10.0.20.3:10087");
    if (puller == nullptr)
    {
        LOG(FATAL) << "zsock_new_pull(tcp://10.0.20.3:10087) err!";
    }
    while (true)
    {
        auto resquest = std::unique_ptr<char[]>(zstr_recv(puller));
        handleSelectServerRequest(std::move(resquest));
    }
}
void ZmqScheduling::startSelectServer()
{
    std::thread run(&ZmqScheduling::selectServer, this);
    run.detach();
}

ZmqClient::ZmqClient(std::string address)
    : _address(address)
{
    _push = zsock_new_push(_address.c_str());
    if (_push == nullptr)
    {
        LOG(ERROR) << "zmq push socket create fatal! address is " << _address;
    }
}
// Not thread-safe, need to be cautious
ZmqClient::ZmqClient(ZmqClient &&temp)
{
    _address = std::move(temp._address);
    _push = temp._push;
    temp._push = nullptr;
}

ZmqClient::~ZmqClient()
{
    std::lock_guard<std::mutex> lock(_zmqClientMutex);
    if (_push != nullptr)
    {
        zsock_destroy(&_push);
    }
}

int ZmqClient::send(std::string content)
{
    std::lock_guard<std::mutex> lock(_zmqClientMutex);
    return zstr_send(_push, content.c_str());
}
