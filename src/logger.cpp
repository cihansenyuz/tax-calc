#include "../inc/logger.hpp"
#include <QDateTime>
#include <QMutexLocker>
#include <iostream>

// Define logging categories
Q_LOGGING_CATEGORY(logGui, "gui")
Q_LOGGING_CATEGORY(logNetwork, "network")
Q_LOGGING_CATEGORY(logDatabase, "database")
Q_LOGGING_CATEGORY(logCalculator, "calculator")
Q_LOGGING_CATEGORY(logManager, "manager")

Logger* Logger::s_instance = nullptr;

Logger::Logger()
    : m_minimumLevel(QtDebugMsg)
    , m_fileLoggingEnabled(true)
    , m_consoleLoggingEnabled(true)
{
}

Logger::~Logger() {
    if (m_logFile.isOpen()) {
        m_logStream.flush();
        m_logFile.close();
    }
}

Logger& Logger::instance() {
    if (!s_instance) {
        s_instance = new Logger();
    }
    return *s_instance;
}

void Logger::initialize(const QString& logFilePath, bool consoleOutput) {
    Logger& logger = instance();
    logger.m_consoleLoggingEnabled = consoleOutput;
    
    // Open log file
    logger.m_logFile.setFileName(logFilePath);
    if (logger.m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        logger.m_logStream.setDevice(&logger.m_logFile);
        logger.m_fileLoggingEnabled = true;
        
        // Write session start marker
        logger.m_logStream << "\n========================================\n";
        logger.m_logStream << "Log session started: " 
                          << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") 
                          << "\n";
        logger.m_logStream << "========================================\n";
        logger.m_logStream.flush();
    } else {
        std::cerr << "Warning: Could not open log file: " 
                  << logFilePath.toStdString() << std::endl;
        logger.m_fileLoggingEnabled = false;
    }
    
    // Install custom message handler
    qInstallMessageHandler(messageHandler);
}

void Logger::shutdown() {
    if (s_instance) {
        if (s_instance->m_logFile.isOpen()) {
            s_instance->m_logStream << "========================================\n";
            s_instance->m_logStream << "Log session ended: " 
                                   << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") 
                                   << "\n";
            s_instance->m_logStream << "========================================\n\n";
            s_instance->m_logStream.flush();
        }
        delete s_instance;
        s_instance = nullptr;
    }
    qInstallMessageHandler(nullptr);
}

void Logger::setMinimumLevel(QtMsgType level) {
    QMutexLocker locker(&m_mutex);
    m_minimumLevel = level;
}

void Logger::setFileLoggingEnabled(bool enabled) {
    QMutexLocker locker(&m_mutex);
    m_fileLoggingEnabled = enabled;
}

void Logger::setConsoleLoggingEnabled(bool enabled) {
    QMutexLocker locker(&m_mutex);
    m_consoleLoggingEnabled = enabled;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    if (s_instance) {
        s_instance->writeLog(type, context, msg);
    }
}

void Logger::writeLog(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QMutexLocker locker(&m_mutex);
    
    // Check minimum level filter
    if (type < m_minimumLevel) {
        return;
    }
    
    QString formattedMsg = formatMessage(type, context, msg);
    
    // Write to console if enabled
    if (m_consoleLoggingEnabled) {
        if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg) {
            std::cerr << formattedMsg.toStdString() << std::endl;
        } else {
            std::cout << formattedMsg.toStdString() << std::endl;
        }
    }
    
    // Write to file if enabled
    if (m_fileLoggingEnabled && m_logFile.isOpen()) {
        m_logStream << formattedMsg << "\n";
        m_logStream.flush();
    }
}

QString Logger::formatMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    QString typeStr = typeToString(type);
    QString category = context.category ? QString(context.category) : "default";
    
    // Format: [TIMESTAMP] [LEVEL] [CATEGORY] message
    QString formatted = QString("[%1] [%2] [%3] %4")
                           .arg(timestamp)
                           .arg(typeStr, -8)  // Left-aligned, 8 chars wide
                           .arg(category, -10) // Left-aligned, 10 chars wide
                           .arg(msg);
    
    // Optionally add file/line info for warnings and above
    if ((type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg) 
        && context.file && context.line > 0) {
        formatted += QString(" (%1:%2)").arg(context.file).arg(context.line);
    }
    
    return formatted;
}

QString Logger::typeToString(QtMsgType type) {
    switch (type) {
        case QtDebugMsg:    return "DEBUG";
        case QtInfoMsg:     return "INFO";
        case QtWarningMsg:  return "WARNING";
        case QtCriticalMsg: return "CRITICAL";
        case QtFatalMsg:    return "FATAL";
        default:            return "UNKNOWN";
    }
}
