#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <QFile>
#include <QLoggingCategory>
#include <QMutex>
#include <QString>
#include <QTextStream>

// Logging categories for different components
Q_DECLARE_LOGGING_CATEGORY(logGui)
Q_DECLARE_LOGGING_CATEGORY(logNetwork)
Q_DECLARE_LOGGING_CATEGORY(logDatabase)
Q_DECLARE_LOGGING_CATEGORY(logCalculator)
Q_DECLARE_LOGGING_CATEGORY(logManager)

/**
 * @brief Singleton logger class that enhances Qt's native logging with file output,
 * timestamps, and configurable severity filtering.
 */
class Logger {
public:
    /**
     * @brief Initialize the logger system. Must be called before QApplication.
     * @param logFilePath Optional custom log file path. Defaults to "app.log" in current directory.
     * @param consoleOutput Enable/disable console output (default: true)
     */
    static void initialize(const QString& logFilePath = "tax_calc.log", bool consoleOutput = true);
    
    /**
     * @brief Shutdown the logger and close log file.
     */
    static void shutdown();
    
    /**
     * @brief Get the singleton instance.
     */
    static Logger& instance();
    
    /**
     * @brief Set minimum log level to display/log
     * @param level Minimum level (Debug=0, Info=1, Warning=2, Critical=3, Fatal=4)
     */
    void setMinimumLevel(QtMsgType level);
    
    /**
     * @brief Enable or disable file logging
     */
    void setFileLoggingEnabled(bool enabled);
    
    /**
     * @brief Enable or disable console logging
     */
    void setConsoleLoggingEnabled(bool enabled);
    
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    /**
     * @brief Custom message handler for Qt logging
     */
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    
    void writeLog(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    QString formatMessage(QtMsgType type, const QMessageLogContext& context, const QString& msg);
    QString typeToString(QtMsgType type);
    
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    QtMsgType m_minimumLevel;
    bool m_fileLoggingEnabled;
    bool m_consoleLoggingEnabled;
    
    static Logger* s_instance;
};

#endif // LOGGER_HPP
