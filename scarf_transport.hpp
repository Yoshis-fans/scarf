#include <cstdint>
#include <utility>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

enum class scarf_packet_type_t : uint8_t {
    ADVERT,
    ADVERT_REQ,
    FILE_TRANSFER_REQ,
    FILE_TRANSFER_REQ_RESP,
    FILE_TRANSFER_CONTENT,
    FILE_RETRANSFER_REQ,
    DEADVERT
};

typedef uint8_t scarf_header_t;

typedef struct scarf_decoded_header_t {
    uint8_t version;
    scarf_packet_type_t type;
} scarf_decoded_header_t;

typedef struct scarf_advert_args_t {
    uint16_t port;
} scarf_advert_args_t;

typedef struct scarf_file_transfer_req_args_t {
    uint16_t transfer_id;
    char filename[256];
    uint16_t count;
} scarf_file_transfer_req_args_t;

typedef struct scarf_file_transfer_req_resp_args_t {
    uint16_t transfer_id;
    bool decision;
} scarf_file_transfer_req_resp_args_t;

typedef struct scarf_file_transfer_content_args_t {
    uint16_t transfer_id;
    uint16_t chunk_number;
    uint8_t data[512];
} scarf_file_transfer_content_args_t;

typedef struct scarf_packet_t {
    scarf_header_t header;
    union {
        scarf_advert_args_t advert_args;
        scarf_file_transfer_req_args_t file_transfer_req_args;
        scarf_file_transfer_req_resp_args_t file_transfer_req_resp_args;
        scarf_file_transfer_content_args_t file_transfer_content_args;
    } un;
} scarf_packet_t;

scarf_header_t encode_header(uint8_t version, scarf_packet_type_t type);
// TODO: Change to own struct
scarf_decoded_header_t decode_header(scarf_header_t header);
void send_packet(int sockfd, sockaddr_in* dst, scarf_packet_t* packet);
bool recv_packet(pollfd sockfd, sockaddr_in* src, socklen_t* len, scarf_packet_t* rv);
