//
//  IMVideoServer.hpp
//  IMLib
//
//  класс IMVideoServer отправка потока изображений по сети
//
//  Created by Vladimir Knyaz on 15.02.16.
//
//

#ifndef IMVideoServer_hpp
#define IMVideoServer_hpp

#include <vector>
#include <thread>
#include <IMLib/IMImageFile.h>

class IMVideoServer
{
private:
    // массив изображений для отправки
    std::vector<IMImageFile*> m_images;
    
    // порт для установки соединения
    int m_port;
    
    // сокет для установки соединения
    int m_listenSocket;
    
    // сокет для отправки данных
    int m_socket;
    
    // поток установки соединения
    std::thread *m_connectionThread;
    
    // поток отправки кадров
    std::thread *m_broadcastThread;
    
protected:
    void initNetwork();
    
    void connectionThread();
    
    void broadcastThread();
    
public:
    IMVideoServer(int port);
    
    ~IMVideoServer();
    
    // помещает изображение в поток отправки
    void pushImage(IMImageFile *image);
    
    // запуск сервера
    void start();
    
    // остановка сервера
    void stop();
    
    
    
};

#endif /* IMVideoServer_hpp */
