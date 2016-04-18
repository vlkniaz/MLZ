//
//  IMVideoServer.cpp
//  IMLib
//
//  Created by Vladimir Knyaz on 15.02.16.
//
//

#include <iostream>
#include <thread>         // std::thread, std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <IMLib/IMVideoServer.h>
#include <IMLib/IMLibTypes.h>

using namespace std;

void error(const char *msg)
{
    perror(msg);
    //exit(1);
}

IMVideoServer::~IMVideoServer()
{
	close(m_listenSocket);
	close(m_socket);
}

void IMVideoServer::connectionThread()
{
    while(1)
    {
        int portno;
        struct sockaddr_in serv_addr;
        
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
            error("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = m_port;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (::bind(m_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR on binding");
        listen(m_socket,5);
        
        socklen_t clilen;
        struct sockaddr_in cli_addr;
        
        clilen = sizeof(cli_addr);
        m_listenSocket = accept(m_socket,
                                (struct sockaddr *) &cli_addr,
                                &clilen);
        if (m_listenSocket < 0)
            error("ERROR on accept");
        
        m_broadcastThread = new thread(&IMVideoServer::broadcastThread, this);
        m_broadcastThread->join();
        delete m_broadcastThread;
        
        close(m_socket);
    }
}

void IMVideoServer::pushImage(IMImageFile *image)
{
    m_images.push_back(image);
}

void IMVideoServer::broadcastThread()
{
    char buffer[256];
    int n;
    
    while(m_listenSocket >= 0)
    {
        // получаем команду
        n = read(m_listenSocket, buffer, sizeof(buffer));
        int cmp;
        
        // команда получить формат изображения
        cmp = strncmp(buffer, IMNetGetImageFormat, strlen(IMNetGetImageFormat));
        if(cmp == 0)
        {
            if(!m_images.empty())
            {
                IMImageFile *image = m_images.back();
                
                if(image)
                {
                    IMImageFormat format = image->imageFormat();
                    int length, s, bytesSent;
                    length = sizeof(image->imageFormat());
                    s = 0;
                    
                    while(length > 0)
                    {
                        bytesSent = write(m_listenSocket, (void*)&format, length);
                        if (bytesSent == 0)
                            break; //socket probably closed
                        else if (bytesSent < 0)
                            break; //handle errors appropriately
                        s += bytesSent;
                        length -= bytesSent;
                    }
                }
            }
            continue;
        }
        
        // команда получить изображение
        cmp = strncmp(buffer, IMNetGetImage, strlen(IMNetGetImage));
        if(cmp == 0)
        {
            if(!m_images.empty())
            {
                IMImageFile *image = m_images.back();
                m_images.pop_back();
                
                if(image)
                {
                    unsigned char* data;
                    int length, s, bytesSent;
                    
                    data = image->image();
                    length = image->pixelsHigh()*image->bytesPerRow();
                    s = 0;
                    
                    if(data != 0)
                    {
                        while(length > 0)
                        {
                            bytesSent = write(m_listenSocket, (void*)data, length);
                            if (bytesSent == 0)
                                break; //socket probably closed
                            else if (bytesSent < 0)
                                break; //handle errors appropriately
                            s += bytesSent;
                            length -= bytesSent;
                        }
                    }
                }
            }
        }
    }
    close(m_listenSocket);
    
}

IMVideoServer::IMVideoServer(int port):
m_port(port),
m_connectionThread(0),
m_broadcastThread(0)
{
    initNetwork();
}

void IMVideoServer::initNetwork()
{
    m_connectionThread = new thread(&IMVideoServer::connectionThread, this);
    m_connectionThread->detach();
}

void IMVideoServer::start()
{
    cout << "Start" << endl;
}