#include <QApplication>
#include <QCommandLineParser>
#include "../gui/mainwindow.hpp"
#include "../inc/logger.hpp"

int main(int argc, char *argv[]) {
    Logger::initialize("tax_calc.log", false);
    
    QApplication application(argc, argv);
    application.setApplicationName("Hisse Beyan Yardımcısı");
    application.setApplicationVersion("0.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("Turkish stock tax calculator");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   "Enable debug logging output");
    parser.addOption(debugOption);
    parser.process(application);
    
    if (parser.isSet(debugOption)) {
        Logger::instance().setMinimumLevel(QtDebugMsg);
        qInfo() << "Debug logging enabled";
    } else {
        Logger::instance().setMinimumLevel(QtInfoMsg);
    }

    MainWindow main_window;
    main_window.show();

    int result = application.exec();
    
    Logger::shutdown();    
    return result;
}
