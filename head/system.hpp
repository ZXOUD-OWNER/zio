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
    NonCopyable() = default;  // Allow default constructor
    ~NonCopyable() = default; // Allow default destructor

    NonCopyable(const NonCopyable &) = delete;            // Disallow copy constructor
    NonCopyable &operator=(const NonCopyable &) = delete; // Disallow assignment operator

    NonCopyable(NonCopyable &&) = delete;            // Disallow move constructor
    NonCopyable &operator=(NonCopyable &&) = delete; // Disallow move assignment operator
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
        conf = nlohmann::json::parse(content);
    };
    ~Singleton(){};
    Singleton(const Singleton &);
    Singleton &operator=(const Singleton &);

private:
    nlohmann::json conf;

public:
    static Singleton &getInstance()
    {
        static Singleton instance;
        return instance;
    }
    inline const nlohmann::json &GetConf()
    {
        return conf;
    }
};

namespace CUitl
{
    inline std::string Print_trace()
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