#ifndef TIN_21Z_CLIENTINFO_H
#define TIN_21Z_CLIENTINFO_H

#include <sockets.h>

class ClientInfo {
public:
    ClientInfo();
    explicit ClientInfo(sockaddr_in addr);
    sockaddr_in addr{};

    bool operator==(const ClientInfo &other) const
    { return (addr.sin_addr.s_addr == other.addr.sin_addr.s_addr
              && addr.sin_port == other.addr.sin_port);
    }


};

namespace std {

    template <>
    struct hash<ClientInfo>
    {
        std::size_t operator()(const ClientInfo& k) const
        {
            using std::hash;
            using std::string;

            return ((hash<in_addr_t>()(k.addr.sin_addr.s_addr)
                     ^ (hash<in_port_t>()(k.addr.sin_port) << 1)) >> 1);
        }
    };

}


#endif //TIN_21Z_CLIENTINFO_H
