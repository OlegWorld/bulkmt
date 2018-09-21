#pragma once

#include <list>
#include "command_bulk.h"
#include "command_reader.h"
#include "command_processors.h"

class CommandHandler {
public:
    explicit CommandHandler(size_t commandPackSize,
                            size_t loggers_num,
                            std::istream& input);

    ~CommandHandler();

private:
    std::list<CommandBulk>  m_data;
    CommandProcessor        m_processor;
    CommandMultipleLog      m_log;
    CommandReader           m_reader;
};

