#include <sstream>
#include <experimental/filesystem>
#include "command_processors.h"

CommandProcessor::CommandProcessor()
:   m_stop_flag(false),
    m_thread(&CommandProcessor::run, this),
    m_guard(m_thread),
    m_block_counter(0), m_command_counter(0)
{ }

void CommandProcessor::run() {
    while(!m_stop_flag) {
        CommandBulk* b;
        if (m_commands.try_pop(b)) {
            process_bulk(b);
        } else {
            std::this_thread::yield();
        }
    }

    CommandBulk* b;
    while(m_commands.try_pop(b)) {
        process_bulk(b);
    }
}

void CommandProcessor::add(CommandBulk* b) {
    m_commands.push(b);
}

void CommandProcessor::stop() {
    m_stop_flag = true;
}

void CommandProcessor::finalize() {
    if (m_thread.joinable())
        m_thread.join();
}

size_t CommandProcessor::commands() const noexcept {
    return m_command_counter;
}

size_t CommandProcessor::blocks() const noexcept {
    return m_block_counter;
}

void CommandProcessor::process_bulk(CommandBulk* b) {
    ++m_block_counter;
    m_command_counter += b->size();
    std::cout << *b;
}

////////////////////////////////////////////////////////

LogWriter::LogWriter(tbb::concurrent_queue<CommandBulk*>& commands, std::atomic_size_t& counter)
:   m_commands(commands),
    m_counter(counter),
    m_stop_flag(false),
    m_thread(&LogWriter::run, this),
    m_guard(m_thread),
    m_output_block_counter(0), m_command_counter(0)
{ }

void LogWriter::run() {
    while(!m_stop_flag) {
        if (!try_create_log_file()) {
            std::this_thread::yield();
        }
    }

    while(try_create_log_file()) { }
}

void LogWriter::stop() {
    m_stop_flag = true;
}

void LogWriter::finalize() {
    if (m_thread.joinable())
        m_thread.join();
}

bool LogWriter::try_create_log_file() {
    using std::to_string;
    using std::chrono::duration_cast;
    using std::chrono::seconds;

    CommandBulk* b;
    if (m_commands.try_pop(b)) {
        ++m_output_block_counter;
        m_command_counter += b->size();
        auto tp = std::chrono::system_clock::now();
        std::ofstream of("bulk_" +
                         to_string(duration_cast<seconds>(tp.time_since_epoch()).count()) + '_' +
                         to_string(m_counter.fetch_add(1)) +
                         ".log", std::ios_base::out);
        of << *b;
        of.close();

        return true;
    } else {
        return false;
    }
}

size_t LogWriter::commands() const noexcept {
    return m_command_counter;
}

size_t LogWriter::blocks() const noexcept {
    return m_output_block_counter;
}

////////////////////////////////////////////////////////

CommandMultipleLog::CommandMultipleLog(size_t num)
:   m_counter(0),
    m_input_block_counter(0)
{
    namespace fs = std::experimental::filesystem;

    fs::create_directory("log");
    fs::current_path("log");

    for (size_t i = 0; i < num; i++) {
        m_logs.emplace_back(m_commands, m_counter);
    }
}

void CommandMultipleLog::add(CommandBulk* b) {
    m_commands.push(b);
    ++m_input_block_counter;
}

void CommandMultipleLog::stop() {
    for (auto& log : m_logs) {
        log.stop();
    }
}

void CommandMultipleLog::finalize() {
    for (auto& log : m_logs) {
        log.finalize();
    }
}

size_t CommandMultipleLog::loggers() const noexcept {
    return m_logs.size();
}

size_t CommandMultipleLog::commands(size_t index) const noexcept {
    return m_logs[index].commands();
}

size_t CommandMultipleLog::out_blocks(size_t index) const noexcept {
    return m_logs[index].blocks();
}

size_t CommandMultipleLog::in_blocks() const noexcept {
    return m_input_block_counter;
}




