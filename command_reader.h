#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <list>
#include "abstract_observer.h"

class ReaderState;
class NormalState;
class BracedState;

class CommandReader {
public:
    CommandReader(std::list<CommandBulk>& data,
                  size_t commandPackSize,
                  std::istream& input);

    ~CommandReader() = default;

    void scan_input();

    void subscribe(AbstractObserver* obs);

    void push_bulk(CommandBulk& name);

    void stop();

    void switch_state();

    CommandBulk& new_bulk();

    void increment_line() noexcept;

    size_t lines() const noexcept;
    size_t commands() const noexcept;
    size_t blocks() const noexcept;

private:
    void recycle_data();

private:
    std::list<CommandBulk>& m_data;
    std::unique_ptr<ReaderState> m_current_state;
    std::unique_ptr<ReaderState> m_other_state;
    std::vector<AbstractObserver*> m_observers;
    bool m_work_flag;

    size_t m_line_counter;
    size_t m_command_counter;
    size_t m_block_counter;
};

class ReaderState {
public:
    virtual ~ReaderState() = default;
    virtual void open_brace(CommandReader*) = 0;
    virtual void close_brace(CommandReader*) = 0;
    virtual void read_commands(CommandReader*) = 0;
};

class NormalState : public ReaderState {
public:
    explicit NormalState(size_t commandPackSize, std::istream& input);
    ~NormalState() override = default;
    void open_brace(CommandReader* reader) override;
    void close_brace(CommandReader* reader) override;
    void read_commands(CommandReader* reader) override;

private:
    const size_t m_command_pack_size;
    std::istream& m_input;
};

class BracedState : public ReaderState {
public:
    BracedState(std::istream& input);
    ~BracedState() override = default;
    void open_brace(CommandReader* reader) override;
    void close_brace(CommandReader* reader) override;
    void read_commands(CommandReader* reader) override;

private:
    size_t m_brace_counter;
    std::istream& m_input;
};
