// native.h
#include <iostream>
#include <thread>
#include <string>
#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#endif

#define MAX_APDU_SIZE 32767

typedef struct {
	SCARDHANDLE hCard;
	DWORD		dwAP;
} CONN_PARAMS, *LPCONN_PARAMS;

void InitializeReaders();
void ReleaseReaders();
void MonitorReaders();
void ListReaders(std::string msgid);
void Connect(std::string msgid, int index, unsigned int sharemode);
void Disconnect(std::string msgid, int index);
void Transcieve(std::string msgid, int index, std::string apdu);