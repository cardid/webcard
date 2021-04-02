// webcard.cpp
// Native helper application to the Smart Card Browser Extension
// Copyright (c) 2017 - cardid.org

#include "webcard.h"
#include "json.hpp"
// #include <WerApi.h>

using namespace std;
using json = nlohmann::json;

SCARDCONTEXT _hContext;
LPTSTR  _pmszReaders;
LPCONN_PARAMS _rgConnections;
#if defined(__linux__) || defined(WIN32)
LPSCARD_READERSTATE _rgReaderStates;
#else
LPSCARD_READERSTATE_A _rgReaderStates;
#endif
int _cReaders;
std::thread _readerMonitor;
bool _stopMonitor = false;

int main(int argc, const char * argv[])
{
	// WerAddExcludedApplication(L"webcard.exe", FALSE);
	// __debugbreak();
	// WerRemoveExcludedApplication(L"webcard.exe", FALSE);
	InitializeReaders();
	_readerMonitor = std::thread(MonitorReaders);
	while (true)
	{
		unsigned int inLength = 0;
		string input = "";

		for (int i = 0; i < 4; i++)
		{
			unsigned int read_char = getchar();
			inLength = inLength | (read_char << i * 8);
			if (read_char == -1) break;
		}

		if (inLength == -1) break;

		for (unsigned int i = 0; i < inLength; i++)
		{
			unsigned int read_char = getchar();
			input.push_back(read_char);
		}
		auto o = json::parse(input);
		string msgid = o["i"].get<std::string>();
		int readerIndex = 0;
		unsigned int sharemode = 0;

		if (o.find("r") != o.end()) {
			readerIndex = o["r"].get<int>();
		}

		switch (o["c"].get<int>())
		{
		case 1:
			ListReaders(msgid);
			break;
		case 2:
			sharemode = SCARD_SHARE_SHARED;
			if (o.find("p") != o.end()) {
				sharemode = o["p"].get<int>();
			}
			Connect(msgid, readerIndex, sharemode);
			break;
		case 3:
			Disconnect(msgid, readerIndex);
			break;
		case 4:
			string apdu = o["a"].get<std::string>();
			Transcieve(msgid, readerIndex, apdu);
			break;
		}
	}
	ReleaseReaders();
	return 0;
}

void InitializeReaders()
{
	LONG    lRet = 0;
	LPTSTR  pReader;
	DWORD   cch = 0;

	_rgReaderStates = NULL;
	_pmszReaders = NULL;
	_cReaders = 0;

	lRet = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &_hContext);
#ifdef SCARD_AUTOALLOCATE
	cch = SCARD_AUTOALLOCATE;
	lRet = SCardListReaders(_hContext, NULL, (LPTSTR)&_pmszReaders, &cch);
#else
	lRet = SCardListReaders(_hContext, NULL, NULL, &cch);
	_pmszReaders = (LPTSTR)calloc(cch, sizeof(char));
	lRet = SCardListReaders(_hContext, NULL, _pmszReaders, &cch);
#endif

	switch (lRet)
	{
	case (LONG)SCARD_E_NO_READERS_AVAILABLE:
		break;

	case SCARD_S_SUCCESS:
		pReader = _pmszReaders;
		while ('\0' != *pReader)
		{
			_rgConnections = (LPCONN_PARAMS)realloc((LPVOID)_rgConnections, (_cReaders + 1) * sizeof(CONN_PARAMS));
#if defined(__linux__) || defined(WIN32)
			_rgReaderStates = (LPSCARD_READERSTATE)realloc((LPVOID)_rgReaderStates, (_cReaders + 1) * sizeof(SCARD_READERSTATE));
#else
			_rgReaderStates = (LPSCARD_READERSTATE_A)realloc((LPVOID)_rgReaderStates, (_cReaders + 1) * sizeof(SCARD_READERSTATE_A));
#endif
			_rgReaderStates[_cReaders].szReader = pReader;
			_rgReaderStates[_cReaders].dwCurrentState = SCARD_STATE_UNAWARE;
			_rgReaderStates[_cReaders].pvUserData = (LPVOID)&_rgConnections[_cReaders];
			// Advance to the next value.
#ifdef WIN32
			pReader = pReader + wcslen((wchar_t *)pReader) + 1;
#else
			pReader = pReader + strlen((char *)pReader) + 1;
#endif
			_cReaders += 1;
		}
		
#if defined(__linux__)
		// And last for new readers, this does not exist in Mac OS
		char buffer[] = "\\\\?PnP?\\Notification";
		_rgConnections = (LPCONN_PARAMS)realloc((LPVOID)_rgConnections, (_cReaders + 1) * sizeof(CONN_PARAMS));
		_rgReaderStates = (LPSCARD_READERSTATE)realloc((LPVOID)_rgReaderStates, (_cReaders + 1) * sizeof(SCARD_READERSTATE));
		_rgReaderStates[_cReaders].szReader = buffer;
		_rgReaderStates[_cReaders].dwCurrentState = SCARD_STATE_UNAWARE;
		_rgReaderStates[_cReaders].pvUserData = (LPVOID)&_rgConnections[_cReaders];
		_cReaders += 1;
#endif
		// Update the states once to detect readers with cards present
		lRet = SCardGetStatusChange(_hContext, 0, _rgReaderStates, _cReaders);
		break;

	default:
		// TO DO: Check if other errors could happen
		break;
	}
}

void ReleaseReaders()
{
	LONG lRet = 0;
	_stopMonitor = true;
	SCardCancel(_hContext);
	_readerMonitor.join();

	// Free the memory.
	free(_rgConnections);
	free(_rgReaderStates);
#ifdef SCARD_AUTOALLOCATE
	lRet = SCardFreeMemory(_hContext, _pmszReaders);
#else
	free(_pmszReaders);
#endif
	lRet = SCardReleaseContext(_hContext);
}

void MonitorReaders()
{
	LONG lRet = 0;
	std::string atr;
	string output;

	while (!_stopMonitor)
	{
		lRet = SCardGetStatusChange(_hContext, INFINITE, _rgReaderStates, _cReaders);
		if (_stopMonitor)
			break;

		for (int i = 0; i < _cReaders; i++)
		{
			if (_rgReaderStates[i].dwEventState & SCARD_STATE_CHANGED)
			{
				atr = std::string();
				output = "{\"e\":";
				if ((_rgReaderStates[i].dwCurrentState & SCARD_STATE_EMPTY) &&
					(_rgReaderStates[i].dwEventState & SCARD_STATE_PRESENT))
				{
					for (DWORD c = 0; c < _rgReaderStates[i].cbAtr; c++)
					{
						atr.push_back(((_rgReaderStates[i].rgbAtr[c] & 0xF0) > 0x90) ? (((_rgReaderStates[i].rgbAtr[c] & 0xF0) >> 4) + 'A' - 10) : (((_rgReaderStates[i].rgbAtr[c] & 0xF0) >> 4) + '0'));
						atr.push_back(((_rgReaderStates[i].rgbAtr[c] & 0x0F) > 0x09) ? ((_rgReaderStates[i].rgbAtr[c] & 0x0F) + 'A' - 10) : ((_rgReaderStates[i].rgbAtr[c] & 0x0F) + '0'));
					}
					output.append("1,\"r\":");
					output.append(std::to_string(i));
					output.append(",\"a\":\"");
					output.append(atr);
					output.append("\"}");
					size_t outLength = output.length();
					cout << char(outLength >> 0) << char(outLength >> 8) << char(outLength >> 16) << char(outLength >> 24);
					cout << output << std::flush;
				}
				else if ((_rgReaderStates[i].dwCurrentState & SCARD_STATE_PRESENT) &&
					(_rgReaderStates[i].dwEventState & SCARD_STATE_EMPTY))
				{
					output.append("2,\"r\":");
					output.append(std::to_string(i));
					output.push_back('}');
					size_t outLength = output.length();
					cout << char(outLength >> 0) << char(outLength >> 8) << char(outLength >> 16) << char(outLength >> 24);
					cout << output << std::flush;
				}

				_rgReaderStates[i].dwCurrentState = _rgReaderStates[i].dwEventState;
			}
		}
	}
}

void ListReaders(string msgid)
{
	string atr = "";
	string output = "{\"i\":\"";
	output.append(msgid);
	output.append("\",\"d\":[");

	setlocale(LC_ALL, "en-US.UTF-8");
	for (int i = 0; i < _cReaders; i++)
	{
		output.append("{\"n\":\"");
#ifdef WIN32
		char buffer[256];
		size_t ccNumChar;
		size_t ret = wcstombs_s(&ccNumChar, buffer, _rgReaderStates[i].szReader, 256);
		output.append(buffer);
#else
		output.append(_rgReaderStates[i].szReader);
#endif
    output.append("\", \"a\":\"");
		atr = "";
		if (_rgReaderStates[i].dwEventState & SCARD_STATE_PRESENT)
		{
			for (DWORD c = 0; c < _rgReaderStates[i].cbAtr; c++)
			{
				atr.push_back(((_rgReaderStates[i].rgbAtr[c] & 0xF0) > 0x90) ? (((_rgReaderStates[i].rgbAtr[c] & 0xF0) >> 4) + 'A' - 10) : (((_rgReaderStates[i].rgbAtr[c] & 0xF0) >> 4) + '0'));
				atr.push_back(((_rgReaderStates[i].rgbAtr[c] & 0x0F) > 0x09) ? ((_rgReaderStates[i].rgbAtr[c] & 0x0F) + 'A' - 10) : ((_rgReaderStates[i].rgbAtr[c] & 0x0F) + '0'));
			}
		}
		output.append(atr);
		output.append("\"}");
		if (i < _cReaders - 1)
			output.push_back(',');
	}
	output.push_back(']');
	output.push_back('}');

	size_t outLength = output.length();
	cout << char(outLength >> 0) << char(outLength >> 8) << char(outLength >> 16) << char(outLength >> 24);
	cout << output << std::flush;
}

void Connect(string msgid, int index, unsigned int sharemode)
{
	LONG lRet = 0;
	string output = "{\"i\":\"";
	output.append(msgid);
	output.append("\",\"d\":\"");

	lRet = SCardConnect(_hContext, _rgReaderStates[index].szReader, sharemode, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &(_rgConnections[index].hCard), &(_rgConnections[index].dwAP));
	if (lRet == SCARD_S_SUCCESS) {
		string atr = "";
		for (DWORD c = 0; c < _rgReaderStates[index].cbAtr; c++)
		{
			atr.push_back(((_rgReaderStates[index].rgbAtr[c] & 0xF0) > 0x90) ? (((_rgReaderStates[index].rgbAtr[c] & 0xF0) >> 4) + 'A' - 10) : (((_rgReaderStates[index].rgbAtr[c] & 0xF0) >> 4) + '0'));
			atr.push_back(((_rgReaderStates[index].rgbAtr[c] & 0x0F) > 0x09) ? ((_rgReaderStates[index].rgbAtr[c] & 0x0F) + 'A' - 10) : ((_rgReaderStates[index].rgbAtr[c] & 0x0F) + '0'));
		}
		output.append(atr);
	}
	output.append("\"}");

	size_t outLength = output.length();
	cout << char(outLength >> 0) << char(outLength >> 8) << char(outLength >> 16) << char(outLength >> 24);
	cout << output << std::flush;
}

void Disconnect(string msgid, int index)
{
	LONG lRet;
	string output = "{\"i\":\"";
	output.append(msgid);
	output.append("\"}");

	lRet = SCardDisconnect(_rgConnections[index].hCard, SCARD_LEAVE_CARD);
	size_t outLength = output.length();
	cout << char(outLength >> 0) << char(outLength >> 8) << char(outLength >> 16) << char(outLength >> 24);
	cout << output << std::flush;
}

void Transcieve(string msgid, int index, string apdu)
{
	LONG  lRet;
	char* pbTemp;
	DWORD i;
	char  pbSend[MAX_APDU_SIZE];
	char  pbRecv[MAX_APDU_SIZE];
	DWORD cbSend;
	DWORD cbRecv;
	DWORD cbBlock;
	char  pbTmp[] = "\x0\x0\x0";

	string output = "{\"i\":\"";
	output.append(msgid);
	output.append("\",\"d\":\"");

	cbSend = (DWORD)(apdu.length() / 2);
	pbTemp = (char*)apdu.c_str();

	for (i = 0; i < cbSend; i++)
	{
		if (*pbTemp > 0x60) *pbTemp -= 0x20; // Make uppercase
		pbSend[i] = (*pbTemp > 0x2F && *pbTemp < 0x3A) ? (*pbTemp - '0') << 4 : (*pbTemp - 'A' + 10) << 4;
		pbTemp += 1;
		if (*pbTemp > 0x60) *pbTemp -= 0x20; // Make uppercase
		pbSend[i] += (*pbTemp > 0x2F && *pbTemp < 0x3A) ? (*pbTemp - '0') : (*pbTemp - 'A' + 10);
		pbTemp += 1;
	}

	cbRecv = MAX_APDU_SIZE;
	lRet = SCardTransmit(_rgConnections[index].hCard,
		(_rgConnections[index].dwAP == SCARD_PROTOCOL_T0) ? (SCARD_PCI_T0) : (SCARD_PCI_T1),
		(LPCBYTE)pbSend, cbSend, NULL, (LPBYTE)pbRecv, (LPDWORD)&cbRecv);

	if (lRet == SCARD_S_SUCCESS)
	{
		if (pbRecv[cbRecv - 2] == 0x61)
		{
			do
			{
				pbTemp = pbRecv;
				if (pbRecv[cbRecv - 2] == 0x61)
					cbBlock = cbRecv - 2;
				else
					cbBlock = cbRecv;

				for (i = 0; i < cbBlock; i++)
				{
					pbTmp[0] = ((*pbTemp & 0xF0) > 0x90) ? (((*pbTemp & 0xF0) >> 4) + 'A' - 10) : (((*pbTemp & 0xF0) >> 4) + '0');
					pbTmp[1] = ((*pbTemp & 0x0F) > 0x09) ? ((*pbTemp & 0x0F) + 'A' - 10) : ((*pbTemp & 0x0F) + '0');
					output.append(pbTmp);
					pbTemp++;
				}

				if (pbRecv[cbRecv - 2] == 0x61)
				{
					memcpy(pbSend, "\x00\xC0\x00\x00", 4);
					pbSend[4] = pbRecv[cbRecv - 1];
					cbSend = 5;
					cbRecv = MAX_APDU_SIZE;
					lRet = SCardTransmit(_rgConnections[index].hCard,
						(_rgConnections[index].dwAP == SCARD_PROTOCOL_T0) ? (SCARD_PCI_T0) : (SCARD_PCI_T1),
						(LPCBYTE)pbSend, cbSend, NULL, (LPBYTE)pbRecv, (LPDWORD)&cbRecv);
				}
			} while (pbRecv[cbRecv - 2] == 0x61);
		}

		// The last or only block
		pbTemp = pbRecv;
		for (i = 0; i < cbRecv; i++)
		{
			pbTmp[0] = ((*pbTemp & 0xF0) > 0x90) ? (((*pbTemp & 0xF0) >> 4) + 'A' - 10) : (((*pbTemp & 0xF0) >> 4) + '0');
			pbTmp[1] = ((*pbTemp & 0x0F) > 0x09) ? ((*pbTemp & 0x0F) + 'A' - 10) : ((*pbTemp & 0x0F) + '0');
			output.append(pbTmp);
			pbTemp++;
		}
	}

	output.push_back('"');
	output.push_back('}');

	size_t outLength = output.length();
	cout << char(outLength >> 0) << char(outLength >> 8) << char(outLength >> 16) << char(outLength >> 24);
	cout << output << std::flush;
}
