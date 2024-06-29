/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240619
 * FilePath: /zio/head/zmqMiddle.hpp
 * Description: the ZMQ Load Balancing Proxy
 */
#pragma once
#include "system.hpp"

typedef struct
{
    zsock_t *frontend; // Listen to clients
    zsock_t *backend;  // Listen to workers
    zlist_t *workers;  // List of ready workers
} lbbroker_t;

struct ZloopWrap : public NonCopyable
{
    zloop_t *reactor = nullptr;
    inline ~ZloopWrap()
    {
        if (reactor == nullptr)
        {
            zloop_destroy(&reactor);
            reactor = nullptr;
        }
    }
};

class ZmqMiddle
{
private:
    lbbroker_t _middleSock;
    ZloopWrap _zloop;

private:
    /**
     * @description: handle backed listening operation
     * @param {zloop_t} *loop: socket listen loop
     * @param {zsock_t} *reader: socket listened backed
     * @param {void} *arg: inlbbroker_t pointer
     * @return {int} 0
     */
    static int sHandleBacked(zloop_t *loop, zsock_t *reader, void *arg);
    /**
     * @description: handle frontend listening operation
     * @param {zloop_t} *loop: socket listen loop
     * @param {zsock_t} *reader socket listened fronted
     * @param {void} *arg: inlbbroker_t pointer
     * @return {int} 0
     */
    static int sHandleFrontend(zloop_t *loop, zsock_t *reader, void *arg);
    /**
     * @description: construct ZeroMQ request message
     * @param {zmsg_t} *msg: ZeroMQ message
     * @param {lbbroker_t} *self:
     * @param {zloop_t} *loop: ZeroMQ loop
     * @return {bool} contruct result
     */
    static bool constructReq(zmsg_t *msg, lbbroker_t *self, zloop_t *loop);

public:
    ZmqMiddle(const nlohmann::json &obj);
    ~ZmqMiddle();
    //@description: start the ZeroMQ Loop Reactor
    void start();
};
