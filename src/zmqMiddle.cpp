#include "head.hpp"

zmqMiddle::zmqMiddle(const nlohmann::json &value)
{
    try
    {
        if (value.find("FrontendPort") == value.end() || value.find("FrontendPort") == value.end() || value.find("ZMQIONumber") == value.end() || value.find("ZMQ_QUEUELEN_SEND") == value.end() || value.find("ZMQ_QUEUELEN_RECV") == value.end())
        {
            LOG(FATAL) << "config.json has err!";
        }

        auto FrontendPort = value.find("FrontendPort").value().get<int>();
        auto BackendPort = value.find("BackendPort").value().get<int>();
        zsys_set_io_threads(value.find("ZMQIONumber").value().get<int>());
        auto sndhwm = value.find("ZMQ_QUEUELEN_SEND").value().get<int>();
        auto recvhwm = value.find("ZMQ_QUEUELEN_RECV").value().get<int>();

        std::string str("tcp://*:");
        str += std::to_string(FrontendPort);
        _middleSock.frontend = zsock_new_router(str.c_str());
        if (_middleSock.frontend == nullptr)
        {
            LOG(FATAL) << "frontend socket create err!";
        }
        zmq_setsockopt(_middleSock.frontend, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
        zmq_setsockopt(_middleSock.frontend, ZMQ_RCVHWM, &recvhwm, sizeof(recvhwm));

        str = "tcp://*:" + std::to_string(BackendPort);
        _middleSock.backend = zsock_new_router(str.c_str());
        if (_middleSock.backend == nullptr)
        {
            LOG(FATAL) << "backend socket create err!";
        }
        zmq_setsockopt(_middleSock.backend, ZMQ_SNDHWM, &sndhwm, sizeof(sndhwm));
        zmq_setsockopt(_middleSock.backend, ZMQ_RCVHWM, &recvhwm, sizeof(recvhwm));

        _middleSock.workers = zlist_new();
        if (_middleSock.workers == nullptr)
        {
            LOG(FATAL) << "workers create err!";
        }

        LOG(WARNING) << "zmqMiddle init success!";
    }
    catch (const std::exception &e)
    {
        LOG(FATAL) << "occur exception! " << e.what() << "function trace " << CUitl::Print_trace();
    }
}

zmqMiddle::~zmqMiddle()
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

int zmqMiddle::s_handle_frontend(zloop_t *loop, zsock_t *reader, void *arg)
{
    lbbroker_t *self = static_cast<lbbroker_t *>(arg);
    zmsg_t *msg = zmsg_recv(self->frontend);
    if (msg)
    {
        zmsg_pushmem(msg, NULL, 0); // delimiter
        zmsg_push(msg, (zframe_t *)zlist_pop(self->workers));
        zmsg_send(&msg, self->backend);

        //  Cancel reader on frontend if we went from 1 to 0 workers
        if (zlist_size(self->workers) == 0)
        {
            zloop_reader_end(loop, self->frontend);
        }
    }
    return 0;
}

int zmqMiddle::s_handle_backend(zloop_t *loop, zsock_t *reader, void *arg)
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
        if (zlist_size(self->workers) == 1)
        {
            auto err = zloop_reader(loop, self->frontend, s_handle_frontend, self);
            if (err == -1)
            {
                LOG(ERROR) << "zloop_reader err! err is " << strerror(errno) << " function order :" << CUitl::Print_trace();
            }
        }
        //  Forward message to client if it's not a READY
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
                LOG(ERROR) << "zmsg_send err! err is " << strerror(errno) << " function order :" << CUitl::Print_trace();
            }
        }
    }
    return 0;
}

void zmqMiddle::start()
{
    _zloop.reactor = zloop_new();
    zloop_reader(_zloop.reactor, _middleSock.backend, s_handle_backend, &_middleSock);
    zloop_start(_zloop.reactor);
}