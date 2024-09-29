#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <poll.h>

#include "scarf_transport.hpp"

#define DEFAULT_PORT 8080
#define DISCOVERY_PORT 52025

int pid;
pollfd sockfd;

struct sockaddr_in server_addr = {0}, broad_addr = {0}; 

void signalHandler(int signum) {
    // deadvertise_node();
    kill(pid, SIGTERM);
    close(sockfd.fd);
    exit(0);
}

void setup_udp(uint16_t port) {
    printf("\tOpen socket\n");
    if((sockfd.fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        perror("socket creation failed"); 
        kill(pid, SIGKILL);
        exit(1); 
    } 
   
    // printf("\tSet timeout\n");
    // timeval timeout = {0};
    // timeout.tv_sec = 5;
    // if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
    //     perror("failed to set socket timeout"); 
    //     kill(pid, SIGKILL);
    //     exit(1); 
    // }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // server_addr.sin_addr.s_addr = (192 & 0x000000FF) | ((168 & 0x000000FF) << 8) | ((250 & 0x000000FF) << 16) | ((159 & 0x000000FF) << 24);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DISCOVERY_PORT);
    server_addr.sin_addr.s_addr = (192 & 0x000000FF) | ((168 & 0x000000FF) << 8) | ((250 & 0x000000FF) << 16) | ((255 & 0x000000FF) << 24);
    

    printf("\tBind to port\n");
    if(bind(sockfd.fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { 
        perror("bind failed"); 
        exit(1); 
    } 
}

// void advertise_node() {
//     scarf_packet_t packet = {0};

//     packet.header = encode_header(1, scarf_packet_type_t::ADVERT);
//     packet.un.advert_args.port = 8080;

//     send_packet(sockfd, &server_addr, &packet);
// }

void handle_packet(scarf_packet_t packet, sockaddr_in* src) {
    scarf_decoded_header_t decoded_header = decode_header(packet.header);
    scarf_packet_t new_packet = {0};

    switch(decoded_header.type) {
        case scarf_packet_type_t::FILE_TRANSFER_REQ:
            new_packet.header = encode_header(1, scarf_packet_type_t::FILE_TRANSFER_REQ_RESP);
            new_packet.un.file_transfer_req_resp_args.decision = true;
            new_packet.un.file_transfer_req_resp_args.transfer_id = packet.un.file_transfer_req_args.transfer_id;
            src->sin_port = 8080;
            // send_packet(sockfd, src, &new_packet);
            break;
        case scarf_packet_type_t::FILE_TRANSFER_REQ_RESP:
            if(packet.un.file_transfer_req_resp_args.decision == true) {
                src->sin_port = 8080;
                // TODO: send_file(/*filename*/, sockfd, src);
            }
            break;

    }
}

// TODO: fix signiture
void send_file(const std::string &inputFile, int sockfd, sockaddr_in* dst)
{
    // Open the source file in binary mode
    std::ifstream ifile(inputFile, std::ifstream::binary);
    if (!ifile.good())
    {
        perror("Error: Source file could not be opened.");
        return;
    }

    auto fileSize = std::filesystem::file_size(inputFile);

    printf("File size: %ld bytes (%d chunks)\n", fileSize, static_cast<uint32_t>(ceil(fileSize/512.0f)));

    // Read in chunks and send over UDP
    std::vector<char> buffer(512, 0); // Buffer to read 512 bytes at a time
    int chunk = 0;

    while (ifile.read(buffer.data(), buffer.size()) || ifile.gcount() > 0)
    {
        printf("Byte count: %ld \n", ifile.gcount());

        // TODO: Send_packet

        chunk++;
    }

    // Close the input file
    ifile.close();
    printf("File sent successfully");
}

// Function to iterate through file chunks and rebuild the original file
void rebuild_file(const std::string &chunksDir, const std::string &outputFile)
{
    std::ofstream ofile(outputFile, std::ofstream::binary);
    if (!ofile.good())
    {
        perror("Error: Could not open output file for rebuilding.\n");
        return;
    }

    // Get all chunk files in the directory
    std::vector<int> chunkFiles;
    for (const auto &entry : std::filesystem::directory_iterator(chunksDir))
    {
        if (entry.is_regular_file())
        {
            std::string temp = std::regex_replace(entry.path().string(), std::regex(R"([\D])"), "");
            chunkFiles.push_back(std::stoi(temp));
        }
    }

    // Sort chunk files by their names (i.e., 0.jpg, 1.jpg, etc.)
    std::sort(chunkFiles.begin(), chunkFiles.end());

    // Iterate through the sorted chunk files and append their contents to the output file
    for (const auto &chunkFile : chunkFiles)
    {
        // TODO: Fix pathing for Unix systems
        std::ifstream ifile(chunksDir + '\\' + std::to_string(chunkFile), std::ifstream::binary);
        if (!ifile.good())
        {
            perror("Error: Could not open chunk file");
            continue;
        }

        std::vector<char> buffer((std::istreambuf_iterator<char>(ifile)), std::istreambuf_iterator<char>());
        ofile.write(buffer.data(), buffer.size());

        ifile.close();
    }

    ofile.close();
    printf("Image successfully rebuilt and saved as: %s", outputFile.c_str());
}

// void getImageChunk(const std::string &inputFile, const std::string &outputDir, const int chunkNum)
// {
//     // Open the source image file in binary mode
//     std::ifstream ifile(inputFile, std::ifstream::binary);
//     if (!ifile.good())
//     {
//         perror("Error: Source file could not be opened.");
//         return;
//     }

//     int fileSize = std::filesystem::file_size(inputFile);

//     int maxChunks = fileSize / 512;

//     if (chunkNum > maxChunks || chunkNum < 0)
//     {
//         perror("Error: Chunk out of range");
//         return;
//     }

//     // Delete the directory if it exists, and then create a new one
//     if (std::filesystem::exists(outputDir + '\\' + std::to_string(chunkNum)))
//     {
//         std::filesystem::remove(outputDir + '\\' + std::to_string(chunkNum)); // Remove existing directory and its contents
//     }

//     printf("Image size: %d \n", fileSize);

//     // Read in chunks and write to the output file
//     std::vector<char> buffer(512, 0); // Buffer to read 512 bytes at a time

//     ifile.seekg(chunkNum * 512);

//     ifile.read(buffer.data(), buffer.size());

//     printf("Byte count: %d \n", ifile.gcount());

//     // Construct the output file path inside the new directory
//     std::string chunkFile = std::to_string(chunkNum);

//     // Open a new file to write the image data into
//     std::ofstream ofile(outputDir + '\\' + chunkFile, std::ofstream::binary);
//     if (!ofile.good())
//     {
//         std::cerr << "Error: Could not open output file." << std::endl;
//         return;
//     }

//     // Write the raw data to the chunk file
//     ofile.write(buffer.data(), ifile.gcount());
//     ofile.close();

//     // Close the input file
//     ifile.close();
//     std::cout << "Image chunk successfully retrieved, chunk: " << chunkNum << std::endl;
// }

// void getImageChunks(const std::string &inputFile, const std::string &outputDir, uint32_t *array, size_t len) // array[1]
// {
//     // Open the source image file in binary mode
//     std::ifstream ifile(inputFile, std::ifstream::binary);
//     if (!ifile.good())
//     {
//         std::cerr << "Error: Source file could not be opened." << std::endl;
//         return;
//     }

//     // call single replacement function
//     for (size_t i = 0; i < len; i++)
//     {
//         getImageChunk(inputFile, outputDir, array[i]);
//     }
// }

// int main()
// {
//     std::string inputFile = "IMG_20240514_230034-POP_OUT.jpg";
//     std::string outputDir = "image_chunks";
//     std::string rebuiltImage = "rebuilt_image.jpg";

//     // Step 1: Save the image in chunks
//     saveImageChunks(inputFile, outputDir);

//     std::filesystem::remove(outputDir + '\\' + std::to_string(15));

//     std::filesystem::remove(outputDir + '\\' + std::to_string(55));

//     std::filesystem::remove(outputDir + '\\' + std::to_string(44));

//     // Step 2: Rebuild the image from chunks
//     rebuildImage(outputDir, "bad_image.jpg");

//     getImageChunk(inputFile, outputDir, 0);   // works for example
//     getImageChunk(inputFile, outputDir, 350); // works for example
//     getImageChunk(inputFile, outputDir, 607); // works for example

//     getImageChunk(inputFile, outputDir, -1);  // chunk out of range
//     getImageChunk(inputFile, outputDir, 700); // chunk out of range

//     uint32_t len = 8;
//     uint32_t a[len] = {3, 4, 2, 1, 98, 15, 44, 55};

//     getImageChunks(inputFile, outputDir, *&a, len);

//     rebuildImage(outputDir, "good_image.jpg");

//     return 0;
// }


int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    
    printf("Configure UDP\n");
    if(argc <= 1) setup_udp(DEFAULT_PORT);
    else setup_udp(std::stoi(argv[1]));

    //printf("Advertise to the network\n");
    //advertise_node();
    
    printf("Ready to receive packets\n");
    while (true) {
        scarf_packet_t packet;
        sockaddr_in src;
        socklen_t len;
        if(!recv_packet(sockfd, &src, &len, &packet)); printf("errno %d", errno);
    }

    return 0;
}
