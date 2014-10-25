#ifndef PING_HPP
#define PING_HPP

#include <sstream>
#include <string>
#include <stdexcept>
#include <winsock.h>
#include <windowsx.h>

// Header de fonction permettant de ping
// License LGPL, zeFresk
// (surcouche de icmp.dll)
// IMPORTANT : ajoutez -lwsock32 au linker si vous voulez que ça compile !

//pour ceux qui n'ont pas to_string, comme moi sur le PC où j'ai codé
namespace ze { //pour pas qu'on est des conflits après avec std
template <typename T>
std::string to_string(T const& var)
{
    std::stringstream ss;
    ss << var;
    return ss.str();
}
}

// Structures required to use functions in ICMP.DLL
typedef struct {
    unsigned char Ttl;                         // Time To Live
    unsigned char Tos;                         // Type Of Service
    unsigned char Flags;                       // IP header flags
    unsigned char OptionsSize;                 // Size in bytes of options data
    unsigned char *OptionsData;                // Pointer to options data
} IP_OPTION_INFORMATION, * PIP_OPTION_INFORMATION;

typedef struct {
    DWORD Address;                             // Replying address
    unsigned long  Status;                     // Reply status
    unsigned long  RoundTripTime;              // RTT in milliseconds
    unsigned short DataSize;                   // Echo data size
    unsigned short Reserved;                   // Reserved for system use
    void *Data;                                // Pointer to the echo data
    IP_OPTION_INFORMATION Options;             // Reply options
} IP_ECHO_REPLY, * PIP_ECHO_REPLY;

typedef DWORD (WINAPI* pfnDHDPWPipPDD)(HANDLE, DWORD, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);

class Pinger
{
public:
    Pinger(std::string const& target, unsigned timeout = 5000) : time(timeout) //constructeur
    {
        //On démarre Winsock
        WSAData wsaData;
        if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) throw std::runtime_error("Impossible to start winsock");

        //chargement de la dll
        hIcmp = LoadLibrary(L"ICMP.dll");
        if (hIcmp == 0) throw std::runtime_error("Unable to load ICMP.dll");

        //récupération de l'IP
        //struct hostent* phe;
        if ((phe = gethostbyname(target.c_str())) == 0) throw std::runtime_error("Could not find IP from target");

        // Get handles to the functions inside ICMP.DLL that we'll need (oui c'est du copié-collé)
        typedef HANDLE (WINAPI* pfnHV)(VOID);
        typedef BOOL (WINAPI* pfnBH)(HANDLE);
        //typedef DWORD (WINAPI* pfnDHDPWPipPDD)(HANDLE, DWORD, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD); // evil, no?
        pfnHV pIcmpCreateFile;
        pfnBH pIcmpCloseHandle;
        //pfnDHDPWPipPDD pIcmpSendEcho;
        pIcmpCreateFile = (pfnHV)GetProcAddress(hIcmp, "IcmpCreateFile");
        pIcmpCloseHandle = (pfnBH)GetProcAddress(hIcmp, "IcmpCloseHandle");
        pIcmpSendEcho = (pfnDHDPWPipPDD)GetProcAddress(hIcmp, "IcmpSendEcho");
        if ((pIcmpCreateFile == 0) || (pIcmpCloseHandle == 0) || (pIcmpSendEcho == 0)) throw std::runtime_error("Failed to get proc addr");

        //ouverture du service de ping
        hIP = pIcmpCreateFile();
        if (hIP == INVALID_HANDLE_VALUE) throw std::runtime_error("Unable to load ping service");

        //création du packet de ping
        memset(acPingBuffer, '\xAA', sizeof(acPingBuffer));
        pIpe = (PIP_ECHO_REPLY)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, sizeof(IP_ECHO_REPLY) + sizeof(acPingBuffer));
        if (pIpe == 0) throw std::runtime_error("Unable to allocate ping buffer");
        pIpe->Data = acPingBuffer;
        pIpe->DataSize = sizeof(acPingBuffer);
    }

    ~Pinger() //il y a du boulot..., d'ailleurs je suis prêt à parier qu'il ya des fuites...
    {
        GlobalFree(pIpe);
        FreeLibrary(hIcmp);
        //free(phe); //MAIS QUELLE ERREUR JEAN-PIERRE !
        WSACleanup();
    }

    IP_ECHO_REPLY const& ping()
    {
        // Send the ping packet
        DWORD dwStatus = pIcmpSendEcho(hIP, *((DWORD*)phe->h_addr_list[0]), acPingBuffer, sizeof(acPingBuffer), NULL, pIpe, sizeof(IP_ECHO_REPLY) + sizeof(acPingBuffer), time);
        if (dwStatus != 0)
        {
            lastReply = *pIpe;
            return lastReply;
        }
        else
        {
            throw std::runtime_error("Error obtaining info from ping packet");
        }
    }

    unsigned long getTime() const
    {
        return lastReply.RoundTripTime;
    }

    std::string getIP() const
    {
        return ze::to_string(int(LOBYTE(LOWORD(pIpe->Address)))) + "." + ze::to_string(int(HIBYTE(LOWORD(pIpe->Address)))) + "." + ze::to_string(int(LOBYTE(HIWORD(pIpe->Address)))) + "." + ze::to_string(int(HIBYTE(HIWORD(pIpe->Address))));
    }

    unsigned short getDataSize()
    {
        return lastReply.DataSize;
    }

private:
    Pinger() = delete; //pas de constructeur par défaut
    Pinger(Pinger const& other) = delete; //pas de copie
    Pinger& operator=(Pinger const& Pinger) = delete; //même chose

    unsigned time;
    HINSTANCE hIcmp; //HINSTANCE de icmp.dll
    IP_ECHO_REPLY lastReply;
    char acPingBuffer[64];
    PIP_ECHO_REPLY pIpe;
    HANDLE hIP;
    struct hostent* phe;
    pfnDHDPWPipPDD pIcmpSendEcho;
};

#endif // PING_HPP
