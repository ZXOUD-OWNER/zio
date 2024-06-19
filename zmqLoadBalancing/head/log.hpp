#pragma once
#include "system.hpp"

struct Log_MQ : public NonCopyable
{
    inline Log_MQ(const char *execFileName)
    {
        std::filesystem::path dirPath("./Log");
        if (!std::filesystem::exists(dirPath))
        {
            std::filesystem::create_directory(dirPath);
        }
        FLAGS_logbufsecs = 1;
        google::InitGoogleLogging(execFileName);
        FLAGS_max_log_size = 10;
        FLAGS_symbolize_stacktrace = true;
        google::SetStderrLogging(google::FATAL);
        google::SetLogDestination(google::GLOG_INFO, "./Log/Infolog_");
        google::SetLogDestination(google::WARNING, "./Log/Warnlog_");
        google::SetLogDestination(google::ERROR, "./Log/Errorlog_");
        google::SetLogDestination(google::FATAL, "./Log/Fatallog_");
        FLAGS_minloglevel = google::INFO;
        google::InstallFailureSignalHandler();

        // LOG(INFO) << "This is an information message";
        // LOG(WARNING) << "This is a warning message";
        // LOG(ERROR) << "This is an error message";
        // LOG(FATAL) << "This is an FATAL message";
    }
    inline ~Log_MQ()
    {
        google::ShutdownGoogleLogging();
    }
};