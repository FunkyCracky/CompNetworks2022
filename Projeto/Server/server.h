#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>

#define SEND_ERR "There was an error sending information from the server. Please try again!"
#define USERS_FAIL "Failed creating the USERS folder. Please try again!"
#define GROUPS_FAIL "Failed creating the GROUPS folder. Please try again!"

typedef struct{
    char group_name[25];
    char gid[3];
    char last_msg[5];
} group;

/* -------------------- server_main -------------------- */

int recv_udp(int udp_socket, char* message);
int send_udp(int udp_socket, char* message);
int recv_tcp(int tcp_socket, char* message);
int send_tcp(int tcp_socket, char* message);
int socket_bind(int socktype, char* PORT, struct addrinfo** res);
int parse_argv(int argc, char** argv, char* PORT, bool* verbose);
int parse(int udp_socket, char* message, char* response);
int main(int argc, char** argv);

/* -------------------- server_udp --------------------- */
int reg(int udp_socket, char* uid, char* pass);
int unreg(int udp_socket, char* uid, char* pass);
int login(int udp_socket, char* uid, char* pass);
int logout(int udp_socket, char* uid, char* pass);
void find_last_message(char* gid, char* last_msg);
int comparer(const void* x1, const void* x2);
int list_groups_dir(group* list, bool my_groups, char* uid);
void send_groups(int udp_socket, group* list, int groups, char* message);
int groups(int udp_socket);
int my_groups(int udp_socket, char* uid);
int subscribe(int udp_socket, char* uid, char* gid, char* group_name);
int unsubscribe(int udp_socket, char* uid, char* gid);
/* -------------------- server_tcp --------------------- */

#endif