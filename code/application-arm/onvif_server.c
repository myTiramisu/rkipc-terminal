#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>

#include "soapH.h"
#include "wsdd.nsmap"
#include "soapStub.h"

#include "onvif_config.h"

// WebService Discovery 线程处理函数
static void *ws_discovery_thread(void *arg) {
    struct soap soap_wsdd;
    int count = 0;

    // 初始化服务器
    // 初始化soap对象
    soap_init1(&soap_wsdd, SOAP_IO_UDP | SOAP_XML_IGNORENS);
    soap_set_namespaces(&soap_wsdd, namespaces);
    printf("[%s][%d][%s][%s] soap_wsdd.version = %d \n", __FILE__, __LINE__, __TIME__, __func__, soap_wsdd.version);

    // 绑定端口
    if (!soap_valid_socket(soap_bind(&soap_wsdd, NULL, WS_DISCOVERY_PORT, 10))) {
        soap_print_fault(&soap_wsdd, stderr);
        return NULL;
    }

    // 加入组播
    struct ip_mreq mcast;
    mcast.imr_multiaddr.s_addr = inet_addr(ONVIF_MULTICAST_IP);
    mcast.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(soap_wsdd.master, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mcast, sizeof(mcast)) < 0) {
        printf("setsockopt error! error code = %d, err string = %s\n", errno, strerror(errno));
        return NULL;
    }

    while (1) {
        // 处理客户端请求 阻塞等待
        if (soap_serve(&soap_wsdd)) {
            soap_print_fault(&soap_wsdd, stderr);
        }

        // 客户端的IP地址
        printf("RECEIVE count %d, connection from IP = %d.%d.%d.%d socket = %d \n\n", 
               count, ((soap_wsdd.ip) >> 24) & 0xFF, ((soap_wsdd.ip) >> 16) & 0xFF, 
               ((soap_wsdd.ip) >> 8) & 0xFF, (soap_wsdd.ip) & 0xFF, (soap_wsdd.socket));

        count++;

        // 清理内存
        soap_destroy(&soap_wsdd);
        soap_end(&soap_wsdd);
    }

    // 分离运行时的环境
    soap_done(&soap_wsdd);

    return NULL;
}

// WebService 服务线程处理函数
static void *web_service_thread(void *arg) {
    struct soap soap_http;
    soap_init1(&soap_http, SOAP_XML_IGNORENS | SOAP_C_UTFSTRING);  // 初始化SOAP对象
    soap_set_namespaces(&soap_http, namespaces);  // 设置命名空间

    int master_socket = soap_bind(&soap_http, NULL, WEB_SERVICE_PORT, 100); // 绑定端口
    if (!soap_valid_socket(master_socket)) {
        soap_print_fault(&soap_http, stderr);
        return NULL;
    }

    printf("Web Service Server is listening on port %d...\n", WEB_SERVICE_PORT);

    while (1) {
        int slave_socket = soap_accept(&soap_http);  // 等待并接受连接
        if (!soap_valid_socket(slave_socket)) {
            soap_print_fault(&soap_http, stderr);
            continue;
        }

        // 处理 HTTP 请求
        if (soap_serve(&soap_http)) {
            soap_print_fault(&soap_http, stderr);
        }

        soap_destroy(&soap_http);  // 清理SOAP对象
        soap_end(&soap_http);      // 结束SOAP会话
    }

    soap_done(&soap_http);  // 完成SOAP清理
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t ws_thread, web_thread;

    // 启动 WS-Discovery 服务线程
    if (pthread_create(&ws_thread, NULL, ws_discovery_thread, NULL) != 0) {
        perror("Failed to create WS-Discovery thread");
        exit(1);
    }

    // 启动 WebService 服务线程
    if (pthread_create(&web_thread, NULL, web_service_thread, NULL) != 0) {
        perror("Failed to create WebService thread");
        exit(1);
    }

    // 等待线程结束
    pthread_join(ws_thread, NULL);
    pthread_join(web_thread, NULL);

    return 0;
}
