#pragma once

#include "system.hpp"

typedef struct
{
    zsock_t *frontend; //  Listen to clients
    zsock_t *backend;  //  Listen to workers
    zlist_t *workers;  //  List of ready workers
} lbbroker_t;

struct zloopWrap : public NonCopyable
{
    zloop_t *reactor = nullptr;
    inline ~zloopWrap()
    {
        if (reactor == nullptr)
        {
            zloop_destroy(&reactor);
            reactor = nullptr;
        }
    }
};

class zmqMiddle
{
private:
    lbbroker_t _middleSock;
    zloopWrap _zloop;

private:
    static int s_handle_backend(zloop_t *loop, zsock_t *reader, void *arg);
    static int s_handle_frontend(zloop_t *loop, zsock_t *reader, void *arg);

public:
    zmqMiddle(const nlohmann::json &obj);
    void start();
    ~zmqMiddle();
};
