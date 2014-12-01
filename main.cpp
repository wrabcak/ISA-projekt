/*
 * @Projekt:    IRC bot 
 * @Autor:      Lukas Vrabec <xvrabe07@stud.fit.vutbr.cz>
 * @file:       main.cpp
 * @date:       19.11.2014
 */


#include "main.h"


int main(int argc, char **argv)
{
    if (help_page(argc,argv)) //print help page if are parameters wrong.
      return 1;

    
    struct_parameters  * parameters = new struct_parameters; // alloc structure for parameters.
    std::string *keys = NULL;
    
    if(parse_parameters(argc,argv,parameters) == 2) // call parse_parameters function, if end with return code 2 -> dealloc all memory on heap and end program.
    {
      delete parameters;
      delete[] keys;
      return 2;
    }

    if (argc == 5) // if there is 5. parameter, save keys.
        keys = get_keys(argv,parameters);
    
    if(communicate_irc(parameters,keys) == 2) //call communicate_irc function, if end with return code 2 -> dealloc all memory on heap and end program.
    {
      delete parameters;
      delete[] keys;
      return 2;
    }
    
    
    //after all dealloc memory on heap and end with 0.
    delete parameters;
    delete[] keys;
    return 0;
}
/*
 * @function:help_page
 * @param1[argc]: -> number of parameters.
 * @param2[argv]: -> array of parameters.
 * @brief: Print help page.
 * @return: If is help page printed return 1 else 0.
 */
int help_page(int argc, char **argv)
{
    if (argc== 2 && (strcmp(argv[1],"--help")== 0 || strcmp(argv[1],"-h")== 0))
    {
        std::cout 
        << "usage: ./ircbot <host>[:<port>] <channel> <syslog_server> [list] "<< std::endl
        << "parameters: --help(-h)    Print this help page" << std::endl 
        << "            <host> IRC server hostname" << std::endl
        << "            [<[port>] IRC server port (optional)" << std::endl
        << "            <channel> IRC channel to join conversation" << std::endl
        << "            <syslog_server> SYSLOG server to send logs" << std::endl
        << "            [list] list of keys" << std::endl;
        return 1;
    }
    else
      return 0;
}
/*
 * @function:parse_parameters
 * @param1[argc]: -> number of parameters.
 * @param2[argv]: -> array of parameters.
 * @param3[*parameters]: -> structure to store parametes.
 * @brief: Parsing parameters to structure.
 * @return: If is parsing succesfull return 0 else return 2.
 */
int parse_parameters(int argc, char **argv, struct_parameters *parameters)
{
    if (argc != 4 && argc != 5) //wrong number of parameters!
      {
        std::cerr << "Error bad parameters!" << std::endl;
        return 2;
      }  
 
    std::string temp_string = std::string(argv[1]);  //help string.
    if (temp_string.find(":") == std::string::npos)
    {
        parameters->IRC_hostname = temp_string;
	parameters->IRC_port = DEAFULT_IRC_PORT; // parameters with no irc port => DEFAULT IRC PORT.
    }
    if (temp_string.find(":") != std::string::npos)
    {
        std::size_t position=temp_string.find(":");
        parameters->IRC_hostname = temp_string.substr(0,position);
	temp_string.erase(0,position+1);
	if(is_num(temp_string))
	  parameters->IRC_port = atoi(temp_string.c_str());
	else
	{
	  std::cerr << "Error wrong port!" << std::endl; // set wrong irc port. 
	  return 2;
	}
    }
    
    temp_string = std::string (argv[2]);
    if (temp_string.find("#") != std::string::npos)
      parameters->IRC_channel = temp_string;
    else
    {
      std::cerr << "Error wrong channel!" << std::endl; // set wrong channel.
      return 2;
    }
    
    temp_string = std::string (argv[3]);
    parameters->SYSLOG_hostname = temp_string;

    return 0;
}
/*
 * @function:get_keys
 * @param1[argv]: -> array of parameters.
 * @param2[*parameters]: -> structure to store parametes.
 * @brief: Get list of keys.
 * @return: Return list of keys.
 */
std::string *get_keys(char **argv,struct_parameters *parameters)
{
	std::string temp_string = std::string (argv[4]);
	std::size_t n = std::count(temp_string.begin(), temp_string.end(), ';');
	std::string *keys = new std::string[n+1];
	std::size_t counter = 0; // number of keys
	while (counter < n)
	{
	    std::size_t position=temp_string.find(";");
	    keys[counter] = temp_string.substr(0,position);
	    temp_string.erase(0,position+1);
            counter++;
	}
	keys[counter] = temp_string; // last key.
	parameters->keys_size = counter;
	
	return keys;
}

/*
 * @function:is_num
 * @param1[s]: -> string with number
 * @brief: Test if is string number.
 * @return: 1 if yes, 0 if not.
 */
bool is_num(const std::string& s)
{
    return( strspn( s.c_str(), "-.0123456789" ) == s.size() );
}
/*
 * @function:communicate_irc
 * @param1[*parameters]: -> structure to store parametes.
 * @param2[*keys]: -> list of keys
 * @brief: Main function to comunnicate with IRC server.
 * @return: If is communication succesfull return 0 else return 2
 */
int communicate_irc(struct_parameters *parameters,std::string *keys)
{
    struct hostent *hPtr;
    struct sockaddr_in remoteSocketInfo;
    char buf[MAX_BUFFER] ={0};
    std::string commands = "";
    
    // get IP adress of IRC server
    if((hPtr = gethostbyname(parameters->IRC_hostname.c_str())) == NULL)
    {
        std::cerr << "Error with DNS." << std::endl;
        return 2;
    }
    
    // create socket.
    if((socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        close(socketHandle);
        std::cerr << "Error, I can't create socket " << std::endl;
        return 2;
    }
    
    // Load data to socket structure.
    memcpy((char *)&remoteSocketInfo.sin_addr, hPtr->h_addr, hPtr->h_length);
    remoteSocketInfo.sin_family = AF_INET;
    remoteSocketInfo.sin_port = htons((u_short)parameters->IRC_port);      // Set port number

    // connect to socket.
    if(connect(socketHandle, (struct sockaddr *)&remoteSocketInfo, sizeof(sockaddr_in)) < 0)
    {
        std::cerr << "Error, I can't connect to socket" << std::endl;
        close(socketHandle);
        return 2;
    }
    
    signal(SIGINT,close_connection); // catch ctrl+c to terminate program
    
    commands = "NICK xvrabe07\r\n";
    std::strcpy(buf,commands.c_str());
    if(send(socketHandle, buf, strlen(buf), 0) == -1) //send NICK to server
    {
      std::cerr << "Error can't send message!" << std::endl;
      return 2;
    }
        
    commands = "USER xvrabe07 xvrabe07 " + parameters->IRC_hostname + " :Lukas Vrabec\r\n"; 
    std::strcpy(buf,commands.c_str());
    if(send(socketHandle, buf, strlen(buf), 0) == -1)  //send info about user to server
    { 
      std::cerr << "Error can't send message!" << std::endl;
      return 2;
    }
    
    do
    {
        recv(socketHandle, buf, MAX_BUFFER, 0);
	if(parse_error_msg(buf,parameters) == 1)
	    return 2;
	if (strstr(buf,"End of /MOTD command.") != NULL )
            break;

    }while(1);

    commands = "JOIN " + parameters->IRC_channel + "\r\n";  
    std::strcpy(buf,commands.c_str());
    if(send(socketHandle, buf, strlen(buf), 0) == -1) // send name of channel.
    {
      std::cerr << "Error can't send message!" << std::endl;
      return 2;
    }

    do
    {
        recv(socketHandle, buf, MAX_BUFFER, 0);
	if(parse_error_msg(buf,parameters) == 1)
	  return 2;
	if (strstr(buf,"End of /NAMES list.\r\n") != NULL )
            break;

    }while(1);
    
    do
    {
        recv(socketHandle, buf, MAX_BUFFER, 0);
	if(parse_error_msg(buf,parameters) == 1)
	  return 2;  
	if ((strstr(buf,"PRIVMSG") != NULL) || (strstr(buf,"NOTICE") != NULL) ) // if come some PRIVMSG or NOTICE msg test if there is some keys.
	{
	    find_keys(keys,parameters,buf);
	}
	if (strstr(buf,"PING") != NULL ) // if come ping msg send pong.
	{
	    commands = "PONG \r\n"; 
            std::strcpy(buf,commands.c_str());
            if(send(socketHandle, buf, strlen(buf), 0) == -1)
	    {
	        std::cerr << "Error can't send message!" << std::endl;
		return 2;
	    }
	}
    }while(1);    
    
    return 0;  
    
}
/*
 * @function:find_keys
 * @param1[*keys]: -> list of keys
 * @param2[*parameters]: -> structure to store parametes.
 * @param3[buf[MAX_BUFFER]]: -> buffer with data from packet.
 * @brief: Function to find keys in buffer.
 * @return: return 0
 */
int find_keys(std::string *keys, struct_parameters *parameters,char buf[MAX_BUFFER])
{
    std::size_t position = 0;
    std::string temp_string = std::string(buf);
    unsigned int n=0;
    
    position=temp_string.find("\r");
    temp_string= temp_string.substr(0,position);
    
    while(n<=parameters->keys_size)
    {
	if (strstr(temp_string.c_str(),keys[n].c_str()) != NULL )
	   if (send_msg_to_syslog(parse_irc_msg(temp_string,get_local_hostname()),parameters) == 2) // if found keys send msg to systelog server.
	     break;
	n++;
    }
    return 0;
}
/*
 * @function:parse_irc_msg
 * @param1[irc_msg]: -> irc msg.
 * @param2[hostname]: -> irc server hostname.
 * @brief: Function to parsing irc msg.
 * @return: return final irc msg.
 */
std::string parse_irc_msg(std::string irc_msg,std::string hostname)
{
      std::time_t result = std::time(NULL);
      std::string time;
      std::string syslog_msg;
      std::size_t position=irc_msg.find("!");
      
      time = std::asctime(std::localtime(&result));
      time = time.erase(19,27);
      time = time.substr(3,strlen(time.c_str()));
      
      syslog_msg = irc_msg.substr(0,position);
      syslog_msg = syslog_msg.append(": ");
      
      position=irc_msg.find(" :");
      irc_msg = irc_msg.erase(0,position+2);
      syslog_msg.append(irc_msg.c_str());
      syslog_msg.erase(0,1);
      position=syslog_msg.find(":");
      syslog_msg.insert(position,">");
      return "<134>" + time + " " + hostname + " ircbot <"+ syslog_msg.c_str();
      
}
/*
 * @function:send_msg_to_syslog
 * @param1[irc_msg]: -> final irc msg.
 * @param2[*parameters]: -> structure to store parametes.
 * @brief: Send irc msg to syslog server.
 * @return: If is sucessfull return 0 else 2
 */
int send_msg_to_syslog(std::string irc_msg, struct_parameters *parameters)
{
  int socketHandleSyslog=0;
  struct sockaddr_in servaddr;
  struct hostent *hPtr;
  
  socketHandleSyslog=socket(AF_INET,SOCK_DGRAM,0);
  
  // get IP adress of syslog server.
  if((hPtr = gethostbyname(parameters->SYSLOG_hostname.c_str())) == NULL)
  {
      std::cerr << "Error with DNS." << std::endl;
      return 2;
  }
  //load data to socket structure.
  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr=inet_addr(parameters->SYSLOG_hostname.c_str());
  servaddr.sin_port=htons(SYSLOG_PORT);
  
  //send data.
  sendto(socketHandleSyslog,irc_msg.c_str(),strlen(irc_msg.c_str()),0, (struct sockaddr *)&servaddr,sizeof(servaddr));
  
  // close socket.
  close (socketHandleSyslog);
  return 0;
}
/*
 * @function:get_local_hostname
 * @brief: Get hostname of local PC
 * @return: hostname of PC
 */
std::string get_local_hostname()
{
    struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;

    getifaddrs (&ifap);
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            if ((ifa->ifa_flags & (IFF_LOOPBACK)))
                continue;
            else
            {
                sa = (struct sockaddr_in *) ifa->ifa_addr;
                addr = inet_ntoa(sa->sin_addr);
                break;
            }
        }
    }

    freeifaddrs(ifap);
    return addr;
}
/*
 * @function:parse_error_msg
 * @param1[*buf]: -> buffer with error msg.
 * @param2[*parameters]: -> structure to store parametes.
 * @brief: Parse error msg from irc server.
 * @return: If is sucessfull return 0 else 2
 */
int parse_error_msg(char *buf, struct_parameters *parameters)
{
  int msg_num = 0;
  std::string msg(buf);
  std::size_t positionBegin=msg.find(":");
  std::size_t positionEnd=msg.find(" ");
  
  msg = msg.erase(positionBegin,positionEnd+1);
  msg = msg.substr(0,3);
  
  try 
  {
      msg_num = std::stoi(msg,nullptr,10);
  }
  catch(const std::invalid_argument& ia)
  {
      return 0;
  }
  
  if ((msg_num > 400) && (msg_num < 600))
  {
      std::cout << "ERROR! Communication with IRC server\n";
      send_msg_to_syslog(parse_irc_msg(std::string(buf),get_local_hostname()),parameters); // send msg to syslog about error.
      return 2;
  }
  
  return 0;
}

/*
 * @function:close_connection
 * @param1[param]: -> number of param.
 * @brief: close connection if get SIGINT
 */
void close_connection(int param)
{
    char buf[MAX_BUFFER] ={0};
    strcpy(buf,"QUIT :See you next time\r\n");
    send(socketHandle, buf, strlen(buf), 0);
    close(socketHandle);
    exit(param);
}



//* ------ End main.cpp ----- */