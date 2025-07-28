#include <QApplication>
#include "../gui/mainwindow.hpp"
#include "../inc/network/evdsfetcher.h"
#include "../inc/network/httpmanager.h"


// TP.DK.USD.A&startDate=01-02-2025&endDate=01-02-2025

int main(int argc, char *argv[]) {
    QApplication application(argc, argv);

    HttpManager *http_manager = new HttpManager();
    http_manager->setKey("HSzat3MFdF");
    
    EvdsFetcher evds{http_manager};
    QObject::connect(&evds, &EvdsFetcher::evdsDataFetched, &application,
                        [](const std::shared_ptr<QJsonObject> &fetched_data) {
        if (fetched_data) {
            QFile file("fetched_data.json");
            if (file.open(QIODevice::WriteOnly)) {
                QJsonDocument doc(*fetched_data);
                file.write(doc.toJson());
                file.close();
            }
        }
    });
    
    evds.fetch(EvdsFetcher::USD, "21-04-2024", "23-04-2024");

    MainWindow main_window;
    main_window.show();

    return application.exec();
}