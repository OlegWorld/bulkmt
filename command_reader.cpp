#include "command_reader.h"

CommandReader::CommandReader(std::list<CommandBulk>& data,
                             size_t commandPackSize,
                             std::istream& input)
:   m_data(data),
    m_current_state(std::make_unique<NormalState>(commandPackSize, input)),
    m_other_state(std::make_unique<BracedState>(input)),
    m_work_flag(true),
    m_line_counter(0), m_command_counter(0), m_block_counter(0)
{ }

void CommandReader::scan_input() {
    while (m_work_flag) {
        m_current_state->read_commands(this);
    }
}

void CommandReader::subscribe(AbstractObserver* obs) {
    m_observers.push_back(obs);
}

void CommandReader::push_bulk(CommandBulk& b) {
    if (b.empty()) {
        b.clear();
        return;
    }

    m_command_counter += b.size();
    ++m_block_counter;

    for (auto& obs : m_observers)
        obs->add(&b);

    recycle_data();
}

void CommandReader::stop() {
    m_work_flag = false;
    for (auto& obs : m_observers)
        obs->stop();
}

void CommandReader::switch_state() {
    std::swap(m_current_state, m_other_state);
}

CommandBulk& CommandReader::new_bulk() {
    return m_data.emplace_back();
}

void CommandReader::recycle_data() {
    auto it = m_data.begin();
    while (it != m_data.end()) {
        if (it->recyclable()) {
            it = m_data.erase(it);
        }
        ++it;
    }
}

void CommandReader::increment_line() noexcept {
    ++m_line_counter;
}

size_t CommandReader::lines() const noexcept {
    return m_line_counter;
}

size_t CommandReader::commands() const noexcept {
    return m_command_counter;
}

size_t CommandReader::blocks() const noexcept {
    return m_block_counter;
}

NormalState::NormalState(size_t commandPackSize, std::istream& input)
:   m_command_pack_size(commandPackSize),
    m_input(input)
{ }

void NormalState::open_brace(CommandReader* reader) {
    reader->switch_state();
}

void NormalState::close_brace(CommandReader*) { }

void NormalState::read_commands(CommandReader* reader) {
    CommandBulk& bulk = reader->new_bulk();
    for (size_t i = 0; i < m_command_pack_size; i++) {
        command_name_t name;
        std::getline(m_input, name);

        if (name.empty()) {
            reader->stop();
            break;
        }

        if (name == "{") {
            open_brace(reader);
            reader->increment_line();
            break;
        }

        if (name == "}") {
            reader->increment_line();
            continue;
        }

        bulk.push_command(std::move(name));
        reader->increment_line();
    }

    reader->push_bulk(bulk);
}

BracedState::BracedState(std::istream& input)
:   m_brace_counter(0),
    m_input(input)
{ }

void BracedState::open_brace(CommandReader*) { }

void BracedState::close_brace(CommandReader* reader) {
    reader->switch_state();
}

void BracedState::read_commands(CommandReader* reader) {
    m_brace_counter++;
    CommandBulk& bulk = reader->new_bulk();
    command_name_t name;
    while(std::getline(m_input, name)) {
        if (name.empty()) {
            bulk.clear();
            reader->stop();
            break;
        }

        if (name == "{") {
            m_brace_counter++;
            reader->increment_line();
            continue;
        }

        if (name == "}") {
            if (!--m_brace_counter) {
                reader->push_bulk(bulk);
                close_brace(reader);
                reader->increment_line();
                break;
            }

            continue;
        }

        bulk.push_command(std::move(name));
        reader->increment_line();
    }
}

