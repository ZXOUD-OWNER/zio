#include "head.hpp"

ZmqScheduling::ZmqScheduling()
{
    startSelectServer();
    startServerManager();
}
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
    return backendArry.getBackEnd().getAddress();
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

    auto iterRequestSerial = requestJson.find("RequestSerial");
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

    {
        std::lock_guard<std::mutex> lock(_backendSelectMutex);
        auto iterBackendArry = _backendSelect.find(static_cast<ServerSerial>(serverKind));
        if (iterBackendArry == _backendSelect.end())
        {
            auto iterPair = _backendSelect.insert(std::pair<ServerSerial, BackendManager>(static_cast<ServerSerial>(serverKind), BackendManager()));
            if (iterPair.second)
            {
                        }
            else
            {
                LOG(ERROR) << "";
            }
        }
    }
}

void ZmqScheduling::handleSelectServerRequest(std::unique_ptr<char[]> &&temp)
{
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