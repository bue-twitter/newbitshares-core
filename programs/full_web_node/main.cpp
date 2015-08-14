#include "BlockChain.hpp"

#include <fc/thread/thread.hpp>
#include <fc/thread/future.hpp>
#include <fc/network/http/server.hpp>
#include <fc/network/ip.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtWebEngine>
#include <QtQml>

int main(int argc, char *argv[])
{
   fc::thread::current().set_name("main");
   QGuiApplication app(argc, argv);
   app.setApplicationName("Graphene Client");
   app.setApplicationDisplayName(app.applicationName());
   app.setOrganizationDomain("cryptonomex.org");
   app.setOrganizationName("Cryptonomex, Inc.");

   QtWebEngine::initialize();
   fc::http::server webGuiServer;
   fc::thread serverThread("HTTP server thread");
   serverThread.async([&webGuiServer] {
      webGuiServer.listen(fc::ip::endpoint::from_string("127.0.0.1:8080"));
      webGuiServer.on_request([](const fc::http::request& request, const fc::http::server::response& response) {
         QString path = QStringLiteral(":") + QString::fromStdString(request.path);
         if (path.endsWith('/'))
            path.append(QStringLiteral("index.html"));
         QFile file(path);

         if (file.exists()) {
            file.open(QIODevice::ReadOnly);
            auto buffer = file.readAll();
            response.set_status(fc::http::reply::OK);
            response.set_length(buffer.size());
            response.write(buffer.data(), buffer.size());
         } else {
            response.set_status(fc::http::reply::NotFound);
            response.set_length(18);
            response.write("I can't find that.", 18);
         }
      });
   });

   qmlRegisterType<BlockChain>("Graphene.FullNode", 1, 0, "BlockChain");
   QQmlApplicationEngine engine;
   engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

   return app.exec();
}
