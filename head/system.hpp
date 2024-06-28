/*
 * This file is part of the software and assets of HK ZXOUD LIMITED.
 * @copyright (c) HK ZXOUD LIMITED https://www.zxoud.com
 * Author: yushou-cell(email:2354739167@qq.com)
 * create: 20240619
 * FilePath: /zio/head/system.hpp
 * Description: some system operations
 */
#pragma once
#include <glog/logging.h>
#include <filesystem>
#include <libunwind.h>
#include <fstream>
#include <mimalloc-override.h>
#include <nlohmann/json.hpp>
extern "C"
{
#include <czmq.h>
}

class NonCopyable
{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(const NonCopyable &) = delete;
    NonCopyable(NonCopyable &&) = delete;
    NonCopyable &operator=(NonCopyable &&) = delete;
};

#define WORKER_READY "\001"

class Singleton
{
private:
    inline Singleton()
    {
        std::filesystem::path p("./config.json");
        std::ifstream ifs(p, std::ifstream::binary);
        std::string content((std::istreambuf_iterator<char>(ifs)), {});
        _conf = nlohmann::json::parse(content);
    };
    ~Singleton(){};
    Singleton(const Singleton &);
    Singleton &operator=(const Singleton &);

private:
    nlohmann::json _conf;

public:
    /**
     * @description: get Singleton instance
     * @return {Singleton} a singleton
     */
    static Singleton &getInstance()
    {
        static Singleton instance;
        return instance;
    }
    /**
     * @description: get json data of configuration from config.json file
     * @return {json} configuration context
     */
    inline const nlohmann::json &getConf()
    {
        return _conf;
    }
};

namespace CUitl
{
    /**
     * @description: print trace
     * @return {string} trace
     */
    inline std::string printTrace()
    {
        unw_cursor_t cursor;
        unw_context_t context;
        std::string temp;
        // Init context
        unw_getcontext(&context);
        unw_init_local(&cursor, &context);
        // traverse the call stack
        while (unw_step(&cursor) > 0)
        {
            unw_word_t offset, pc;
            char fname[64];
            unw_get_reg(&cursor, UNW_REG_IP, &pc);
            fname[0] = '\0';
            unw_get_proc_name(&cursor, fname, sizeof(fname), &offset);
            temp.append(fname);
            temp.append("() ");
            temp.append("+0x");
            temp.append(std::to_string(offset));
            temp.append("\n");
        }
        return temp;
    }
}