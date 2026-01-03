# Logger Usage Guide

## Overview
The tax calculator now includes a Qt-native logger that enhances all `qDebug()`, `qInfo()`, `qWarning()`, and `qCritical()` calls with timestamps, component categories, and file output.

## Features
- **Automatic initialization**: Logger starts before QApplication and captures all Qt logging
- **Timestamped logs**: Each message includes timestamp in format `yyyy-MM-dd HH:mm:ss.zzz`
- **Component categories**: Messages are tagged by component (gui, network, database, calculator, manager)
- **Dual output**: Logs to both console and file (`tax_calc.log`)
- **Severity filtering**: Can set minimum log level at runtime
- **Thread-safe**: Uses mutex for concurrent logging

## Log Format
```
[TIMESTAMP] [LEVEL] [CATEGORY] message (file:line)
```

Example:
```
[2026-01-03 14:23:45.123] [INFO    ] [network  ] Fetching symbols with query: https://...
[2026-01-03 14:23:46.456] [WARNING ] [database ] Failed to open database: ... (transactiondatabase.cpp:18)
```

## Using Logging Categories

### Available Categories
- `logGui` - GUI components (dialogs, windows)
- `logNetwork` - Network operations (HTTP requests, API calls)
- `logDatabase` - Database operations
- `logCalculator` - Tax calculations
- `logManager` - Transaction management

### Basic Usage
```cpp
#include "../inc/logger.hpp"

// Info messages (for normal operations)
qInfo(logNetwork) << "Fetching data from API";

// Debug messages (for detailed tracing)
qDebug(logNetwork) << "Response received:" << data;

// Warning messages (for potential issues)
qWarning(logDatabase) << "Connection timeout, retrying...";

// Critical messages (for serious errors)
qCritical(logManager) << "Transaction processing failed";
```

### Without Categories (default category)
```cpp
qInfo() << "Application started";
qWarning() << "Generic warning message";
```

## Configuration

### Initialize Logger (already done in main.cpp)
```cpp
// Initialize with default log file
Logger::initialize("tax_calc.log", true);

// Or customize
Logger::initialize("/path/to/custom.log", false); // disable console output
```

### Runtime Configuration
```cpp
// Set minimum log level (filters out lower priority messages)
Logger::instance().setMinimumLevel(QtWarningMsg); // Only WARNING and above

// Disable file logging temporarily
Logger::instance().setFileLoggingEnabled(false);

// Disable console output
Logger::instance().setConsoleLoggingEnabled(false);
```

### Environment-Based Configuration (future enhancement)
You can extend the logger to read from environment variables:
```bash
export TAX_CALC_LOG_LEVEL=WARNING
export TAX_CALC_LOG_FILE=/tmp/tax_calc.log
```

## Log File Management

### Location
By default, `tax_calc.log` is created in the application's working directory.

### Session Markers
Each application run adds session markers:
```
========================================
Log session started: 2026-01-03 14:23:45
========================================
... log messages ...
========================================
Log session ended: 2026-01-03 14:25:30
========================================
```

### Log Rotation (not yet implemented)
To implement log rotation, modify [src/logger.cpp](src/logger.cpp):
- Check file size on write
- Rename old logs (e.g., `tax_calc.log.1`, `tax_calc.log.2`)
- Keep last N log files

## Examples from Codebase

### Network Layer
```cpp
// From evdsfetcher.cpp
qInfo(logNetwork) << "EvdsFetcher: Processing" << items.size() << "items";
qInfo(logNetwork) << "EvdsFetcher: Detected USD series";
```

### Database Layer
```cpp
// From transactiondatabase.cpp
qWarning(logDatabase) << "Failed to open database:" << m_db.lastError().text();
```

### Transaction Manager
```cpp
// From transactionmanager.cpp
qInfo(logManager) << "Exchange rate received:" << value;
qWarning(logManager) << "Received data with no active transaction";
```

### GUI Layer
```cpp
// From closepositiondialog.cpp
qInfo(logGui) << "ClosePositionDialog initialized for asset:" << symbol;
```

### Calculator
```cpp
// From calculator.hpp
qWarning(logCalculator) << "Error calculating tax base: " << e.what();
```

## Best Practices

1. **Use appropriate severity levels**:
   - `qDebug()` - Verbose debugging info (disabled in production)
   - `qInfo()` - Normal operational messages
   - `qWarning()` - Recoverable issues
   - `qCritical()` - Serious errors
   - `qFatal()` - Fatal errors (terminates app)

2. **Use relevant categories**: Choose the category that matches your component

3. **Include context**: Add relevant variables to help debugging
   ```cpp
   qInfo(logNetwork) << "Fetching rate for date:" << date.toString();
   ```

4. **Don't log sensitive data**: Avoid logging passwords, API keys, personal data

5. **Performance**: Logging has minimal overhead, but avoid logging in tight loops

## Troubleshooting

### Log file not created
- Check write permissions in the working directory
- Verify `Logger::initialize()` is called before logging

### Missing log messages
- Check minimum log level: `Logger::instance().setMinimumLevel(QtDebugMsg)`
- Verify the category is enabled (all are enabled by default)

### Console output cluttered
- Disable console: `Logger::instance().setConsoleLoggingEnabled(false)`
- Or increase minimum level: `Logger::instance().setMinimumLevel(QtWarningMsg)`

## Future Enhancements

Potential improvements to consider:
1. Log rotation (size-based or date-based)
2. JSON/structured logging format option
3. Remote logging (syslog, network endpoint)
4. Per-category log level filtering
5. Configuration file support (`.ini`, `.json`)
6. Log viewer GUI within the application
