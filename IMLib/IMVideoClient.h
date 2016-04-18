
//
//  IMVideoClient.h
//  IMLib
//
//  класс IMVideoClient.h приёма потока изображений по сети
//
//  Created by Vladimir Knyaz on 15.02.16.
//
//

#ifndef IMVideoClient_hpp
#define IMVideoClient_hpp

#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <IMLib/IMImageFile.h>

class IMVideoClientDelegate
{
public:
    virtual void newImageReady(IMImageFile *image) = 0;
};

class IMVideoClient
{
private:
    // массив изображений для отправки
    std::vector<IMImageFile*> m_images;
    
    // порт для установки соединения
    int m_port;
    
    int m_bufferSize;
    
    std::string m_host;
    
    // сокет для установки соединения
    int m_listenSocket;
    
    // сокет для отправки данных
    int m_socket;
    
    // поток установки соединения
    std::thread *m_connectionThread;
    
    // поток отправки кадров
    std::thread *m_broadcastThread;
    
    // формат изображений в потоке
    IMImageFormat m_imageFormat;
    
    // нужно ли инициализировать формат изображения
    bool m_needsToInitFormat;
    
    // ограничение доступа
    std::mutex m_mutex;
    
    // обработчик новых кадров
    IMVideoClientDelegate *m_delegate;
    
    
protected:
    void initNetwork();
    
    void connectionThread();
    
    void broadcastThread();
    
public:
    IMVideoClient(std::string host, int port);
    
    ~IMVideoClient();
    
    std::vector<IMImageFile*> images()
    {
        m_mutex.lock();
        return m_images;
        m_mutex.unlock();
    }
    
    // удаляет изображение из потока приёма
    void popImage();
    
    // запуск сервера
    void start();
    
    // остановка сервера
    void stop();
    
    void setDelegate(IMVideoClientDelegate *delegate)
    {
        m_delegate = delegate;
    }
    
};

#endif /* IMVideoClient_hpp */
