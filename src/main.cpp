#include <QApplication>
#include "../gui/mainwindow.hpp"
#include "../inc/logger.hpp"

int main(int argc, char *argv[]) {
    Logger::initialize("tax_calc.log", false);
    
    QApplication application(argc, argv);
    application.setApplicationName("Hisse Beyan Yardımcısı");
    application.setApplicationVersion("0.1");

    MainWindow main_window;
    main_window.show();

    int result = application.exec();
    
    Logger::shutdown();    
    return result;
}
