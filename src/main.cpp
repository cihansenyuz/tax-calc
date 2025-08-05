#include <QApplication>
#include "../gui/mainwindow.hpp"

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);
    application.setApplicationName("Hisse Beyan Yardımcısı");
    application.setApplicationVersion("0.1");

    MainWindow main_window;
    main_window.show();

    return application.exec();
}
