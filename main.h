/*
 * @Projekt:    IRC bot 
 * @Autor:      Lukas Vrabec <xvrabe07@stud.fit.vutbr.cz>
 * @file:       main.h
 * @date:       19.11.2014
 */
#include <iostream> 
#include <string.h>
#include <string>
#include <cstring>
#include <new>
#include <algorithm>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <signal.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>

const unsigned int DEAFULT_IRC_PORT = 6667; // default irc port.
const unsigned int SYSLOG_PORT = 514; // default syslog port.
const int MAX_BUFFER=4096; // buffer size for connection

// struct for parameters.
struct struct_parameters
{
    
    std::string IRC_hostname; //IRC server 
    int IRC_port; // IRC port
    std::string IRC_channel; // IRC channel
    std::string SYSLOG_hostname; //SYSLOG server
    unsigned int keys_size; // number of keys.
};

int socketHandle;

int help_page(int argc, char **argv); // print help page.
int parse_parameters(int argc,char **argv, struct_parameters *parameters); // function for parsing parameters.
std::string *get_keys(char **argv,struct_parameters *parameters); // function to get keys from parameters.
bool is_num(const std::string& s); // function is is in string number

int communicate_irc(struct_parameters *parameters,std::string *keys); // main function for communication with irc server.
int find_keys(std::string *keys, struct_parameters *parameters,char buf[MAX_BUFFER]); // function for find keys in packet.
int send_msg_to_syslog(std::string irc_msg, struct_parameters *parameters); // function for sending irc msgs to syslog server
std::string parse_irc_msg(std::string irc_msg,std::string hostname); // parsing irc msg.
std::string get_local_hostname(); // function to get local name of mashine.
int parse_error_msg(char *buf,struct_parameters *parameters); // parsing error msg.
static void close_connection(int param); // function for close connection.


//* ------ end main.h ----- */