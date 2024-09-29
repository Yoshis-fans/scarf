#include <cstdio>
#include <cstring>
#include "scarf_transport.hpp"


scarf_header_t encode_header(uint8_t version, scarf_packet_type_t type) {
    return (version & 0x0F) << 4 | static_cast<uint8_t>(type) & 0x0F;
}

scarf_decoded_header_t decode_header(scarf_header_t header) {
    scarf_decoded_header_t rv = {(header & 0xF0) >> 4, static_cast<scarf_packet_type_t>(header & 0x0F)};

    return rv;
}

void send_packet(int sockfd, sockaddr_in* dst, scarf_packet_t* packet) {
    uint8_t* data = reinterpret_cast<uint8_t*>(&packet);

    // uint8_t buf[3] = {0x10, 0x1F, 0x90};
    sendto(sockfd, (const char *)data, sizeof(packet), 
        MSG_CONFIRM, (const struct sockaddr *) dst,  
            sizeof((*dst))); 
    
    printf("Sending %ld bytes", sizeof(data));
}

bool recv_packet(pollfd sockfd, sockaddr_in* src, socklen_t* len, scarf_packet_t* rv) {
    uint8_t buffer[512];

    int poll_rv = poll(&sockfd, 1, 5000);
    if(poll_rv > 0) {
        int n = recvfrom(sockfd.fd, &buffer, 512,  
        MSG_WAITALL | MSG_OOB, ( struct sockaddr *) src, 
        len); 

        printf("Read: %d bytes from %d.%d.%d.%d. Content: ", n, ((*src).sin_addr.s_addr & 0x000000FF), ((*src).sin_addr.s_addr & 0x0000FF00) >> 8, ((*src).sin_addr.s_addr & 0x00FF0000) >> 16, ((*src).sin_addr.s_addr & 0xFF000000) >> 24);
        for (int i = 0; i < n; i++) printf("%2x", buffer[i]);

        if(n <= 0) return false;

        rv->header = buffer[0];

        scarf_decoded_header_t decoded_header = decode_header(buffer[0]);
        switch(decoded_header.type) {
            case scarf_packet_type_t::ADVERT:
                memcpy(&(rv->un.advert_args), buffer+1, n-1); // FIXME: len is min(n-1, sizeof(rv.un.advert_args))
                break;
            case scarf_packet_type_t::FILE_TRANSFER_REQ:
                memcpy(&(rv->un.file_transfer_req_args), buffer+1, n-1); // FIXME: Look above
                break;
            case scarf_packet_type_t::FILE_TRANSFER_REQ_RESP:
                memcpy(&(rv->un.file_transfer_req_resp_args), buffer+1, n-1); // FIXME: Look above
                break;
            case scarf_packet_type_t::FILE_TRANSFER_CONTENT:
                memcpy(&(rv->un.file_transfer_content_args), buffer+1, n-1); // FIXME: Look above
                break;
            default:
            break;
        }
    }
    
    return rv;
}
