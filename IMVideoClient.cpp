//
//  IMVideoClient.cpp
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
#include <netdb.h>

#include <IMLib/IMVideoClient.h>
#include <IMLib/IMLibTypes.h>
//TEMP
#include <IMLib/IMTIFFImageFileFormat.h>

using namespace std;

IMVideoClient::~IMVideoClient()
{
	close(m_listenSocket);
	close(m_socket);
}

void IMVideoClient::connectionThread()
{
    while(1)
    {
        int portno, n;
        struct sockaddr_in serv_addr;
        struct hostent *server;
        
        char buffer[256];
        portno = m_port;
        m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_listenSocket < 0)
            perror("ERROR opening socket");
        
        unsigned int m = sizeof(m_bufferSize);
        getsockopt(m_listenSocket,SOL_SOCKET,SO_RCVBUF,(void *)&m_bufferSize, &m);
        
        server = gethostbyname(m_host.c_str());
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr,
              (char *)&serv_addr.sin_addr.s_addr,
              server->h_length);
        serv_addr.sin_port = htons(portno);
        if (connect(m_listenSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            perror("ERROR connecting");
        
        m_broadcastThread = new thread(&IMVideoClient::broadcastThread, this);
        m_broadcastThread->join();
        delete m_broadcastThread;
        
        close(m_socket);
    }
}

void IMVideoClient::broadcastThread()
{
    int n;
    bool noError = true;
    
    while(m_listenSocket >= 0 && noError)
    {
        // если нужно запрашиваем формат изображения
        if(m_needsToInitFormat)
        {
            //cout << "Init format" << endl;
            n = write(m_listenSocket,IMNetGetImageFormat,strlen(IMNetGetImageFormat));
            if (n < 0)
            {
                perror("ERROR writing to socket");
                noError = false;
                break;
            }
            
            //cout << "sizeof(IMImageFormat)" << sizeof(IMImageFormat) << endl;
            
            n = read(m_listenSocket, (void*)&m_imageFormat,sizeof(IMImageFormat));
            if (n < 0)
            {
                perror("ERROR reading from socket");
                noError = false;
                break;
            }
            
            printf("image width %d \n", m_imageFormat.pixelsWide);
            m_needsToInitFormat = false;
        }
        
        // получаем текущий кадр
        IMImageFile *image = new IMImageFile;
        image->setImageFormat(m_imageFormat);
        image->allocImage();
        
        n = write(m_listenSocket,IMNetGetImage,strlen(IMNetGetImage));
        if(n < 0)
        {
            noError = false;
            delete image;
            break;
        }
        
        unsigned char* data;
        int length, s, bytesSent;
        
        data = image->image();
        length = image->pixelsHigh()*image->bytesPerRow();
        s = 0;
        
        if(data != 0)
        {
            //cout << "Reading image..." << endl;
            while(length > 0)
            {
                bytesSent = read(m_listenSocket, (void*)data, length);
                if (bytesSent == 0)
                {
                    noError = false;
                    delete image;
                    break; //socket probably closed
                }
                else if (bytesSent < 0)
                {
                    noError = false;
                    delete image;
                    break; //handle errors appropriately
                }
                s += bytesSent;
                data += bytesSent;
                length -= bytesSent;
            }
            //cout << "Done!" << endl;
        }
        
        if(m_delegate)
        {
            m_delegate->newImageReady(image);
        }
        
        m_mutex.lock();
        m_images.push_back(image);
        m_mutex.unlock();
    }
    close(m_listenSocket);
    
}

IMVideoClient::IMVideoClient(std::string host, int port):
m_port(port),
m_host(host),
m_connectionThread(0),
m_broadcastThread(0),
m_needsToInitFormat(true),
m_delegate(0)
{
    
}

void IMVideoClient::initNetwork()
{
    m_connectionThread = new thread(&IMVideoClient::connectionThread, this);
    m_connectionThread->detach();
}

void IMVideoClient::start()
{
    cout << "Start" << endl;
    initNetwork();
}