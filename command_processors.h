#pragma once

#include <iostream>
#include <fstream>
#include <queue>
#include <chrono>
#include <thread>
#include <atomic>
#include <deque>
#include <mutex>
#include <tbb/concurrent_queue.h>
#include "abstract_observer.h"
#include "thread_utility.h"

class CommandProcessor : public AbstractObserver {
public:
    CommandProcessor();
    ~CommandProcessor() override = default;

    void run();

    void add(CommandBulk* b) override;

    void stop() override;

    void finalize();

    size_t commands() const noexcept;
    size_t blocks() const noexcept;

private:
    void process_bulk(CommandBulk* b);

private:
    tbb::concurrent_queue<CommandBulk*> m_commands;
    std::atomic_bool m_stop_flag;
    std::thread m_thread;
    thread_guard m_guard;

    size_t m_block_counter;
    size_t m_command_counter;
};

class LogWriter {
public:
    LogWriter(tbb::concurrent_queue<CommandBulk*>& commands, std::atomic_size_t& counter);
    ~LogWriter() = default;

    void run();

    void stop();

    void finalize();

    size_t commands() const noexcept;
    size_t blocks() const noexcept;

private:
    bool try_create_log_file();

private:
    tbb::concurrent_queue<CommandBulk*>& m_commands;
    std::atomic_size_t& m_counter;
    std::atomic_bool m_stop_flag;
    std::thread m_thread;
    thread_guard m_guard;

    size_t m_output_block_counter;
    size_t m_command_counter;
};

class CommandMultipleLog : public AbstractObserver {
public:
    explicit CommandMultipleLog(size_t num);
    ~CommandMultipleLog() override = default;

    void add(CommandBulk* b) override;

    void stop() override;

    void finalize();

    size_t loggers() const noexcept;

    size_t commands(size_t index) const noexcept;
    size_t out_blocks(size_t index) const noexcept;
    size_t in_blocks() const noexcept;

private:
    tbb::concurrent_queue<CommandBulk*> m_commands;
    std::atomic_size_t m_counter;
    std::deque<LogWriter> m_logs;

    size_t m_input_block_counter;
};
