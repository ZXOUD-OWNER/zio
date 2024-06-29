#include "head.hpp"

ZmqMiddle::ZmqMiddle(const nlohmann::json &value)
{
    try
    {
        if (value.find("FrontendPort") == value.end() || value.find("FrontendPort") == value.end() || value.find("ZMQIONumber") == value.end() || value.find("ZMQ_QUEUELEN_SEND") == value.end() || value.find("ZMQ_QUEUELEN_RECV") == value.end())
        {
            LOG(FATAL) << "config.json has err!";
        }
        auto frontendPort = value.find("FrontendPort").value().get<int>();
        auto backendPort = value.find("BackendPort").value().get<int>();
        zsys_set_io_threads(value.find("ZMQIONumber").value().get<int>());
        auto sndhwm = value.find("ZMQ_QUEUELEN_SEND").value().get<int>();
        auto recvhwm = value.find("ZMQ_QUEUELEN_RECV").value().get<int>();
        std::string str("tcp://*:");
        str += std::to_string(frontendPort);
        _middleSock.frontend = zsock_new_router(str.c_str());
        if (_middleSock.frontend == nullptr)
        {
            LOG(FATAL) << "frontend socket create err!";
        }
        zsock_set_rcvhwm(_middleSock.frontend, recvhwm);
        zsock_set_sndhwm(_middleSock.frontend, sndhwm);

        str = "tcp://*:" + std::to_string(backendPort);
        _middleSock.backend = zsock_new_router(str.c_str());
        if (_middleSock.backend == nullptr)
        {
            LOG(FATAL) << "backend socket create err!";
        }
        zsock_set_rcvhwm(_middleSock.backend, recvhwm);
        zsock_set_sndhwm(_middleSock.backend, sndhwm);
        zsock_set_router_mandatory(_middleSock.backend, 1);
        _middleSock.workers = zlist_new();
        if (_middleSock.workers == nullptr)
        {
            LOG(FATAL) << "workers create err!";
        }
        LOG(WARNING) << "ZmqMiddle init success!";
    }
    catch (const std::exception &e)
    {
        LOG(FATAL) << "occur exception! " << e.what() << "function trace " << CUitl::printTrace();
    }
}

ZmqMiddle::~ZmqMiddle()
{
    //  When we're done, clean up properly
    while (zlist_size(_middleSock.workers))
    {
        zframe_t *frame = static_cast<zframe_t *>(zlist_pop(_middleSock.workers));
        zframe_destroy(&frame);
    }
    zlist_destroy(&_middleSock.workers);
    zsock_destroy(&_middleSock.frontend);
    zsock_destroy(&_middleSock.backend);
}

bool ZmqMiddle::constructReq(zmsg_t *msg, lbbroker_t *self, zloop_t *loop)
{
    if (zlist_size(self->workers) == 0)
    {
        zloop_reader_end(loop, self->frontend);
        LOG(ERROR) << "workers err! err is 0";
        return false;
    }
    int res = zmsg_pushmem(msg, NULL, 0); // delimiter
    if (res == -1)
    {
        LOG(ERROR) << "zmsg_pushmem err! err is " << strerror(errno);
        return false;
    }
    res = zmsg_push(msg, (zframe_t *)zlist_pop(self->workers));
    if (res == -1)
    {
        LOG(ERROR) << "zmsg_push err! err is " << strerror(errno);
        return false;
    }
    return true;
}

int ZmqMiddle::sHandleFrontend(zloop_t *loop, zsock_t *reader, void *arg)
{
    ZMQ_ROUTER_MANDATORY;
    lbbroker_t *self = static_cast<lbbroker_t *>(arg);
    if (zlist_size(self->workers) == 0)
    {
        zloop_reader_end(loop, self->frontend);
        return 0;
    }
    DLOG(ERROR) << "before     zmsg_t *msg = zmsg_recv(self->frontend);";
    zmsg_t *msg = zmsg_recv(self->frontend);
    DLOG(ERROR) << "after     zmsg_t *msg = zmsg_recv(self->frontend);";
    if (msg)
    {
        while (true) // If routing fails, then continue to loop and get a new route (using zlist_pop(self->workers)), until successful or a viable route is found or the number of backend nodes is zero.
        {
            zmsg_t *msgCopy = zmsg_dup(msg);
            if (!constructReq(msgCopy, self, loop))
            {
                DLOG(ERROR) << "after     !constructReq(msgCopy, self, loop)";
                zmsg_destroy(&msg);
                zmsg_destroy(&msgCopy);
                break;
            }
            DLOG(ERROR) << "before zmsg_send(&msgCopy, self->backend)";
            int res = zmsg_send(&msgCopy, self->backend);
            DLOG(ERROR) << "after zmsg_send(&msgCopy, self->backend)";
            if (res == -1)
            {
                auto errInt = errno;
                if (errInt == EHOSTUNREACH)
                {
                    DLOG(ERROR) << "after EHOSTUNREACH";
                    continue;
                }
                else
                {
                    LOG(ERROR) << "zmsg_push err! err is " << strerror(errno);
                }
            }
            else
            {
                break;
            }
        }
        //  Cancel reader on frontend if we went from 1 to 0 workers
    }
    zmsg_destroy(&msg);
    return 0;
}

int ZmqMiddle::sHandleBacked(zloop_t *loop, zsock_t *reader, void *arg)
{
    //  Use worker identity for load-balancing
    lbbroker_t *self = static_cast<lbbroker_t *>(arg);
    zmsg_t *msg = zmsg_recv(self->backend);
    if (msg)
    {
        zframe_t *identity = zmsg_pop(msg);
        if (!identity)
        {
            LOG(WARNING) << "zmsg_pop(msg) return null";
        }
        zframe_t *delimiter = zmsg_pop(msg);
        if (!delimiter)
        {
            LOG(WARNING) << "zmsg_pop(msg) return null";
        }
        zframe_destroy(&delimiter);
        zlist_append(self->workers, identity);
        //  Enable reader on frontend if we went from 0 to 1 workers
        DLOG(ERROR) << "before if (zlist_size(self->workers) == 1) size is" << zlist_size(self->workers);
        if (zlist_size(self->workers) == 1)
        {
            DLOG(ERROR) << "after if (zlist_size(self->workers) == 1)";
            auto err = zloop_reader(loop, self->frontend, sHandleFrontend, self);
            if (err == -1)
            {
                LOG(ERROR) << "zloop_reader err! err is " << strerror(errno) << " function order :" << CUitl::printTrace();
            }
        }
        //  Forward message to client if it's not a READY
        DLOG(ERROR) << "before zframe_t *frame = zmsg_first(msg);";
        zframe_t *frame = zmsg_first(msg);
        if (memcmp(zframe_data(frame), WORKER_READY, 1) == 0)
        {
            zmsg_destroy(&msg);
        }
        else
        {
            auto err = zmsg_send(&msg, self->frontend);
            if (err == -1)
            {
                LOG(ERROR) << "zmsg_send err! err is " << strerror(errno) << " function order :" << CUitl::printTrace();
            }
        }
    }
    return 0;
}

void ZmqMiddle::start()
{
    _zloop.reactor = zloop_new();
    auto err = zloop_reader(_zloop.reactor, _middleSock.backend, sHandleBacked, &_middleSock);
    if (err == -1)
    {
        LOG(ERROR) << "zloop_reader err! err is " << strerror(errno) << " function order :" << CUitl::printTrace();
    }
    zloop_start(_zloop.reactor);
}