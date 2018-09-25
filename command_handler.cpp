#include "command_handler.h"

CommandHandler::CommandHandler(size_t commandPackSize,
                               size_t loggers_num,
                               std::istream& input)
:   m_log(loggers_num),
    m_reader(m_data, commandPackSize, input)

{
    m_reader.subscribe(&m_processor);
    m_reader.subscribe(&m_log);

    while(!m_processor.ready());
    while(!m_log.ready());

    m_reader.scan_input();
}

CommandHandler::~CommandHandler() {
    m_processor.finalize();
    m_log.finalize();

    std::cout << "main thread - "
              << m_reader.lines() << " lines, "
              << m_reader.commands() << " commands, "
              << m_reader.blocks() << " blocks"
              << std::endl;

    std::cout << "log thread - "
              << m_processor.blocks() << " blocks, "
              << m_processor.commands() << " commands"
              << std::endl;

    for (size_t i = 0; i < m_log.loggers(); i++) {
        std::cout << "file" << i + 1 << " thread - "
                  << m_log.out_blocks(i) << " blocks, "
                  << m_log.commands(i) << " commands"
                  << std::endl;
    }
}
