/**
 * @file NetManager.cpp
 * @date March 7, 2014
 * @author Wade Burch (dev@nolnoch.com)
 *
 * @brief Networking wrapper for SDL_net created for OGRE engine use in CS 354R
 * at the University of Texas at Austin, taught by Don Fussell in Spring 2014.
 *
 * @copyright GNU Public License
 */

#include "NetManager.h"


#define LOCALHOST_NBO 16777343


/* ****************************************************************************
 * Constructors/Destructors
 */

/**
 * Initialize changeable values to defaults. Nothing special.
 */
NetManager::NetManager():
netStatus(NET_UNINITIALIZED),
nextUDPChannel(CHANNEL_DEFAULT),
forceClientRandomUDP(true),
acceptNewClients(false),
socketNursery(0),
netLocalHost(0),
netProtocol(0),
netPort(0)
{
  if (-1 == SDL_Init(0)) {
    printf("SDL_Init: %s\n", SDL_GetError());
  } else if (-1 == SDLNet_Init()) {
    printf("SDLNet_Init: %s\n", SDLNet_GetError());
  }
}

/**
 * Standard destruction. Calls close().
 */
NetManager::~NetManager() {
  close();
  SDLNet_Quit();
}



/* ****************************************************************************
 * Public
 */

/**
 * Initializes the SDL library if it has not started already, followed by the
 * SDL_net library. If both succeed, the internal SocketSet is allocated, and
 * the state is set to NET_INITIALIZED.
 * @return True on success, false on failure.
 */
bool NetManager::initNetManager() {
  bool ret = true;
  int i;

  socketNursery = SDLNet_AllocSocketSet(SOCKET_ALL_MAX);

  if (!socketNursery) {
    printError("SDL_net: Unable to allocate SocketSet.");
    printError(SDLNet_GetError());
    ret = false;
  } else {
    netServer.tcpSocketIdx = -1;
    netServer.udpSocketIdx = -1;
    netServer.udpChannel = -1;
    netServer.tcpDataIdx = -1;
    netServer.udpDataIdx = -1;
    netServer.clientIdx = -1;
    netServer.protocols = 0;
    for (i = 0; i < MESSAGE_COUNT; i++) {
      udpServerData[i].updated = false;
    }
    netStatus |= NET_INITIALIZED;
  }

  return ret;
}

/**
 * @brief Required to set TCP/UDP, port, and optional host.
 *
 * Allows user to set preferred protocol, port, and optional host. If a host is
 * given, it is assumed to be the server, and a client initialization is
 * expected. If no host is given, only a server initialization is possible and
 * will be expected.  Protocol and port are given default values if either or
 * both are not specified.
 * @param protocol Desired protocols for server or client. Default: ALL.
 * @param host Host server if starting client. Default: NULL (begin server).
 * @param port Desired port for server or client. Default: 51215
 */
void NetManager::addNetworkInfo(Protocol protocol, const char *host, Uint16 port) {
  if (statusCheck(NET_INITIALIZED) || (netStatus > NET_WAITING)) {
    printError("NetManager: Must initNetManager - and no further - before proceeding.");
    return;
  }

  netProtocol = protocol;
  netPort = port ? port : PORT_DEFAULT;
  if (host)
    netHostname = host;

  netStatus |= NET_WAITING;
}

/**
 * @brief Launch a server to listen on given/default port over
 * given/default protocols.
 *
 * Warns if host server was specified but proceeds with launch.
 * @return True on success, false on failure.
 */
bool NetManager::startServer() {
  bool retTCP = false;
  bool retUDP = false;

  if (statusCheck(NET_WAITING)) {
    printError("NetManager: Must addNetworkInfo before starting server.");
    return false;
  } else if (netStatus & NET_CLIENT) {
    printError("NetManager: Client already started. May not start server.");
    return false;
  } else if (!netHostname.empty()) {
    printError("NetManager: Host was specified. Are you sure you want to "
        "start a server?");
  }

  netStatus |= NET_SERVER;

  if ((netProtocol & PROTOCOL_TCP) && !(netStatus & NET_TCP_OPEN)){
    if ((retTCP = openServer(PROTOCOL_TCP, netPort)))
      printError("NetManager: TCP Server socket listening.");
  }
  if ((netProtocol & PROTOCOL_UDP) && !(netStatus & NET_UDP_OPEN)){
    if ((retUDP = openServer(PROTOCOL_UDP, netPort)))
      printError("NetManager: UDP Server socket bound.");
  }

  return retTCP || retUDP;
}

/**
 * @brief Launch a client on given/default port over given/default protocols
 * and connected to given host server.
 *
 * Fails if no host server was given in addNetworkInfo or addHost.
 * @return True on success, false on failure.
 */
bool NetManager::startClient() {
  bool retTCP = false;
  bool retUDP = false;

  if (statusCheck(NET_WAITING)) {
    printError("NetManager: Must addNetworkInfo before starting client.");
    return false;
  } else if (netStatus & NET_SERVER) {
    printError("NetManager: Server already started. May not start client.");
    return false;
  } else if (netHostname.empty()) {
    printError("NetManager: Must specify a host to start a client.");
    return false;
  }

  netStatus |= NET_CLIENT;

  if (netProtocol & PROTOCOL_TCP) {
    if ((retTCP = openClient(PROTOCOL_TCP, netHostname, netPort)))
      printError("NetManager: TCP Client connection to server established.");
  }
  if (netProtocol & PROTOCOL_UDP) {
    if ((retUDP = openClient(PROTOCOL_UDP, netHostname, netPort)))
      printError("NetManager: UDP Client channel bound to server.");
  }

  return retTCP || retUDP;
}

/**
 * @brief Poll for activity on all TCP and UDP sockets.
 *
 * If activity is detected, it will be automatically handled according to its
 * protocol and the server or client configuration. New clients and data will
 * be processed before this function returns. If the return is \b true, the
 * <em> user should immediately scan the external MessageInfo bins </em> for
 * newly output data.
 * @param timeout_ms Time in milliseconds to block and poll. Default: 5 seconds.
 * @return True for activity, false for no activity.
 */
int NetManager::pollForActivity(Uint32 timeout_ms) {
  if (statusCheck(NET_UDP_OPEN, NET_TCP_OPEN)) {
    printError("NetManager: No established TCP or UDP sockets to poll.");
    return false;
  }

  return checkSockets(timeout_ms);
}

/**
 * @brief Scan once for activity on all TCP and UDP sockets.
 *
 * This calls pollForActivity with a time of 0 milliseconds (instant).
 * @return True for activity, false for no activity.
 * @see pollForActivity()
 */
int NetManager::scanForActivity() {
  return pollForActivity(0);
}

/**
 * @brief Send a single message to all clients.
 *
 * Must be running as a server to call this function. If no arguments are given,
 * it will pull from each client's ClientData \b input field.
 * @param protocol TCP, UDP, or ALL as given by PROTOCOL_XXX enum value.
 * @param buf Manually given data buffer. Default: NULL.
 * @param len Length of given buffer. Default: 0.
 */
void NetManager::messageClients(Protocol protocol, const char *buf, int len) {
  int i, j, length;
  char *data;

  if (statusCheck(NET_SERVER)) {
    printError("NetManager: No server running, and thus no clients to message.");
    return;
  }

  if (statusCheck(NET_TCP_ACCEPT, NET_UDP_BOUND)) {
    printError("No established connection to a client. Cannot send message.");
    return;
  }

  if (buf && (0 <= len) && (len < MESSAGE_LENGTH)) {
    length = len ? len : strlen(buf);

    for (i = 0; i < netClients.size(); i++) {
      if (protocol & (netClients[i]->protocols & PROTOCOL_TCP)) {
        sendTCP(tcpSockets[netClients[i]->tcpSocketIdx], buf, length);
      }
      if (protocol & netClients[i]->protocols & PROTOCOL_UDP) {
        UDPpacket *pack = craftUDPpacket(buf, length);
        if (pack) {
          sendUDP(udpSockets[netClients[i]->udpSocketIdx],
              netClients[i]->udpChannel, pack);
        }
      }
    }
  } else {
    length = MESSAGE_LENGTH;
    UDPpacket *pack;

    for (i = 0; i < netClients.size(); i++) {
      if (protocol & netClients[i]->protocols & PROTOCOL_TCP) {
        data = tcpServerData.input;
        sendTCP(tcpSockets[netClients[i]->tcpSocketIdx], data, length);
        tcpServerData.updated = false;
      }
      if (protocol & netClients[i]->protocols & PROTOCOL_UDP) {
        for (j = 0; j < MESSAGE_COUNT; j++) {
          if (udpServerData[j].updated) {
            data = udpServerData[j].input;
            pack = craftUDPpacket(data, length);
            if (pack) {
              sendUDP(udpSockets[netClients[i]->udpSocketIdx],
                  netClients[i]->udpChannel, pack);
            }
            //udpServerData[j].updated = false;
          }
        }
      }
    }
    for (j = 0; j < MESSAGE_COUNT; j++)
      udpServerData[j].updated = false;
  }
}

/**
 * @brief Send a single message to the server.
 *
 * Must be running as a client to call this function. If no arguments are given,
 * it will pull from the server's ClientData \b input field.
 * @param protocol TCP, UDP, or ALL as given by PROTOCOL_XXX enum value.
 * @param buf Manually given data buffer. Default: NULL.
 * @param len Length of given buffer. Default: 0.
 */
void NetManager::messageServer(Protocol protocol, const char *buf, int len) {
  int length;
  char *data;

  if (statusCheck(NET_CLIENT)) {
    printError("NetManager: No client running, and thus no server to message.");
    return;
  }

  if (statusCheck(NET_TCP_OPEN, NET_UDP_BOUND)) {
    printError("No established connections to a server. Cannot send message.");
    return;
  }

  if (buf && (0 <= len) && (len < MESSAGE_LENGTH)) {
    length = len ? len : strlen(buf);

    if (protocol & PROTOCOL_TCP) {
      sendTCP(tcpSockets[netServer.tcpSocketIdx], buf, length);
    }
    if (protocol & PROTOCOL_UDP) {
      UDPpacket *pack = craftUDPpacket(buf, length);
      if (pack) {
        sendUDP(udpSockets[netServer.udpSocketIdx], netServer.udpChannel, pack);
      }
    }
  } else {
    length = MESSAGE_LENGTH;

    if (protocol & PROTOCOL_TCP) {
      data = tcpServerData.input;
      sendTCP(tcpSockets[netServer.tcpSocketIdx], data, length);
      tcpServerData.updated = false;
    }
    if (protocol & PROTOCOL_UDP) {
      data = udpServerData[0].input;
      UDPpacket *pack = craftUDPpacket(data, length);
      if (pack)
        sendUDP(udpSockets[netServer.udpSocketIdx], netServer.udpChannel, pack);
      udpServerData[0].updated = false;
    }
  }
}

/**
 * @brief Send a single message to a single client over a single protocol.
 *
 * Must be running as a server, and all fields must be provided by the user.
 * This will send the given message to the specified client using the specified
 * protocol.
 * @param protocol TCP or UDP, given by the PROTOCOL_XXX enum value.
 * @param clientDataIdx Index of the client into the tcp/udp ClientData vector.
 * @param buf Manually given data buffer.
 * @param len Length of the given buffer.
 * @see messageClients()
 */
void NetManager::messageClient(Protocol protocol, int clientDataIdx, const char *buf, int len) {
  if (statusCheck(NET_SERVER)) {
    printError("NetManager: No server running, and thus no clients to message.");
    return;
  }

  ConnectionInfo *cInfo;

  if (protocol & PROTOCOL_TCP) {
    cInfo = lookupClient(tcpClientData[clientDataIdx]->host, false);
    TCPsocket client = tcpSockets[cInfo->tcpSocketIdx];
    sendTCP(client, buf, len);
    tcpClientData[clientDataIdx]->updated = false;
  } else if (protocol & PROTOCOL_UDP) {
    cInfo = lookupClient(udpClientData[clientDataIdx]->host, false);
    UDPpacket *pack = craftUDPpacket(buf, len);
    UDPsocket client = udpSockets[cInfo->udpSocketIdx];
    if (pack) {
      sendUDP(client, cInfo->udpChannel, pack);
      udpClientData[clientDataIdx]->updated = false;
    }
  }
}

/**
 * @brief Removes an established client from a running server.
 *
 * Must be running as a server, and must give a connected client. May choose to
 * drop the client from TCP, UDP, or both.
 * @param protocol TCP, UDP, or ALL; given by PROTOCOL_XXX enum value.
 * @param host The IPaddress host of the droppee.
 */
void NetManager::dropClient(Protocol protocol, Uint32 host) {
  if (statusCheck(NET_SERVER)) {
    printError("NetManager: No server running, and thus no clients to drop.");
    return;
  }
  std::vector<ConnectionInfo *>::iterator it;
  bool found = false;

  ConnectionInfo *cInfo = lookupClient(host, false);

  if (cInfo && (protocol & cInfo->protocols & PROTOCOL_TCP)) {
    int idx = cInfo->clientIdx;
    TCPsocket client = tcpSockets[idx];
    unwatchSocket(client);
    closeTCP(client);
    tcpSockets.erase(tcpSockets.begin() + idx);
  }
  if (cInfo && (protocol & cInfo->protocols & PROTOCOL_UDP)) {
    UDPsocket client = udpSockets[cInfo->udpSocketIdx];
    unbindUDPSocket(client, cInfo->udpChannel);

    // TODO Implement reclaimable channels through bitmap or 2d array?
  }
  if (cInfo) {
    for (it = netClients.begin(); it != netClients.end() && !found; it++) {
      if ((*it) == cInfo) {
        netClients.erase(it);
        found = true;
      }
    }
  }
}

/**
 * @brief Shut down the running server on any or all protocols.
 *
 * Must be running as a server to call this function. If after completing the
 * requested removal there are no active protocols, all data structures will
 * be emptied, freed, and reset to default values. The state of the instance
 * will return to NET_INITIALIZED, allowing for start of new client or server
 * after another call to addNetworkInfo().
 * @param protocol TCP, UDP, or all; given by PROTOCOL_XXX enum value
 * @see resetManager()
 * @see close()
 */
void NetManager::stopServer(Protocol protocol) {
  int i;

  if (statusCheck(NET_SERVER)) {
    printError("NetManager: There's no server running, dummy.");
    return;
  }

  for (i = 0; i < netClients.size(); i++) {
    if (protocol & netClients[i]->protocols & PROTOCOL_TCP) {
      int idx = netClients[i]->tcpSocketIdx;
      TCPsocket client = tcpSockets[idx];
      unwatchSocket(client);
      closeTCP(client);
      tcpSockets.erase(tcpSockets.begin() + idx);
    }
    if (protocol & netClients[i]->protocols & PROTOCOL_UDP) {
      ConnectionInfo *cInfo = netClients[i];
      UDPsocket client = udpSockets[cInfo->udpSocketIdx];
      unbindUDPSocket(client, cInfo->udpChannel);
    }
  }

  if (netServer.protocols & PROTOCOL_UDP & protocol) {
    // Fixed a bug with this for loop that wasn't closing the first open UDP port at index 0
    for (i = udpSockets.size() - 1; i >= 0; i--) {
      unwatchSocket(udpSockets[i]);
      closeUDP(udpSockets[i]);
      udpSockets.pop_back();
    }
    netServer.protocols ^= PROTOCOL_UDP;
    clearFlags(NET_UDP_OPEN | NET_UDP_BOUND);
  }

  if (netServer.protocols & PROTOCOL_TCP & protocol) {
    closeTCP(tcpSockets[netServer.tcpSocketIdx]);
    tcpSockets.clear();
    netServer.protocols ^= PROTOCOL_TCP;
    clearFlags(NET_TCP_OPEN | NET_TCP_ACCEPT);
  }


  if (!netServer.protocols)
    resetManager();
}

/**
 * @brief Shut down the running client on any or all protocols.
 *
 * Must be running as a client to call this function. If after completing the
 * requested removal there are no active protocols, all data structures will
 * be emptied, freed, and reset to default values. The state of the instance
 * will return to NET_INITIALIZED, allowing for start of new client or server
 * after another call to addNetworkInfo().
 * @param protocol TCP, UDP, or all; given by PROTOCOL_XXX enum value
 * @see resetManager()
 * @see close()
 */
void NetManager::stopClient(Protocol protocol) {
  if (statusCheck(NET_CLIENT)) {
    printError("NetManager: You're not a client, thus you can't be stopped.");
    return;
  }

  if (netServer.protocols & (protocol & PROTOCOL_TCP)) {
    TCPsocket server = tcpSockets[netServer.tcpSocketIdx];
    unwatchSocket(server);
    closeTCP(server);
    tcpSockets.pop_back();
    netServer.protocols ^= PROTOCOL_TCP;
  }
  if (netServer.protocols & (protocol & PROTOCOL_UDP)) {
    UDPsocket server = udpSockets[netServer.udpSocketIdx];
    unwatchSocket(server);
    unbindUDPSocket(server, netServer.udpChannel);
    closeUDP(server);
    udpSockets.pop_back();
    netServer.protocols ^= PROTOCOL_UDP;
  }

  if (!netServer.protocols)
    resetManager();
}

/**
 * @brief Terminates all running servers or clients on all protocols.
 *
 * This function is called by the destructor, but it may be called explicitly
 * by the user if desired. It will call stopServer() or stopClient() as
 * appropriate.
 * @see stopServer()
 * @see stopClient()
 */
void NetManager::close() {
  if (netStatus > NET_UNINITIALIZED) {
    if (netStatus & NET_SERVER) {
      stopServer(netServer.protocols);
    } else if (netStatus & NET_CLIENT) {
      stopClient(netServer.protocols);
    } else {
      resetManager();
    }
  }
}

/**
 * @brief Allows the dynamic addition of TCP or UDP to a running server or
 * client.
 *
 * Must be currently running as a server or client over only one of TCP or UDP.
 * This function adds and immediately launches the requested, missing protocol.
 * @param protocol TCP or UDP, given by PROTOCOL_XXX enum value.
 * @return True on success, false on failure.
 */
bool NetManager::addProtocol(Protocol protocol) {
  if (statusCheck(NET_SERVER, NET_CLIENT)) {
    printError("NetManager: Not currently running a server or client.");
    return false;
  }

  netProtocol |= protocol;
  netStatus |= NET_WAITING;

  return (netStatus & NET_SERVER) ? startServer() : startClient();
}

/**
 * @brief Set the protocol manually.
 *
 * This is currently useless as a public function given the structured use of
 * addNetworkInfo() and addProtocol(). It is only public because it seems like
 * it should be.
 * @param protocol TCP, UDP, or ALL; given by PROTOCOL_XXX enum value.
 */
void NetManager::setProtocol(Protocol protocol) {
  if (!statusCheck(NET_SERVER, NET_CLIENT)) {
    return;
  }

  netProtocol = protocol;
}

/**
 * @brief Set the port manually.
 *
 * Currently useless as a public function. This cannot be safely executed after
 * a server or client is launched.
 * @param port The desired port.
 */
void NetManager::setPort(Uint16 port) {
  if (!statusCheck(NET_SERVER, NET_CLIENT)) {
    return;
  }

  netPort = port;
}

/**
 * Set the host manually.
 *
 * Currently useless as a public function. This cannot be safely executed after
 * a server or client is launched.
 * @param host The desired host.
 */
void NetManager::setHost(const char *host) {
  if (!statusCheck(NET_SERVER, NET_CLIENT)) {
    return;
  }

  netHostname = std::string(host);
}

/**
 * @brief Returns the currently active protocols.
 * @return The currently active protocols.
 */
Uint32 NetManager::getProtocol() {
  return (Uint32) netProtocol;
}

/**
 * @brief Returns the currently active port.
 * @return The currently active port.
 */
Uint16 NetManager::getPort() {
  return netPort;
}

/**
 * @brief Returns the currently active host.
 *
 * Must be running as a client to call this function; servers do not have hosts.
 * @return The currently active host.
 */
std::string NetManager::getHostname() {
  if (statusCheck(NET_CLIENT)) {
    return std::string("Error: Invalid request.");
  }

  return netHostname;
}

/**
 * @brief Retrieve the string representation of the local IP address.
 * @return Local IP address in decimal-separated, string format.
 */
std::string NetManager::getIPstring() {
  return ipToString(getIPnbo(), 32);
}

/**
 * @brief Retrieve the string representation of the masked local IP address.
 * @param mask Bit-depth of the subnet mask.
 * @return Local IP address in decimal-separated, string format.
 */
std::string NetManager::getMaskedIPstring(int mask) {
  return ipToString(getIPnbo(), mask);
}

/**
 * @brief Retrieve the network-byte-ordered local IP address.
 * @return Local IP address in network byte order.
 */
Uint32 NetManager::getIPnbo() {
  IPaddress myIPs[3];
  Uint32 host;
  int i, count = 0;
  bool found = false;

  if (!netLocalHost) {
	  // Per SDL_net doc, this function doesn't work on Windows until version 2.0
#ifdef __linux__
    count = SDLNet_GetLocalAddresses(myIPs, 3);
#endif

    for (i = 0; i < count && !found; i++) {
      host = myIPs[i].host;
      if (host != LOCALHOST_NBO)
        found = true;
    }
    netLocalHost = host;
  }

  return netLocalHost;
}

/**
 * @brief The number of connected TCP clients.
 *
 * I added this primarily for testing, but it may come in handy.
 * @return The number of connected TCP clients.
 */
int NetManager::getClients() {
  return netClients.size();
}

/**
 * @brief Accept new clients.
 */
void NetManager::acceptConnections() {
  acceptNewClients = true;
}

/**
 * @brief Deny new clients.
 */
void NetManager::denyConnections() {
  acceptNewClients = false;
}

/**
 * @brief Initiates multiplayer functionality by launching the server instance.
 *
 * Adds the TCP protocol to this instance and enables acceptance of new clients.
 * Then the local IP address is retrieved and broadcasted to a /24 mask of
 * itself. The clients will respond and register automatically on the server.
 * @param maskDepth The depth of subnet bits to preserve.
 * @return Success of the broadcast: true if the server receives its own packet.
 */
bool NetManager::multiPlayerInit(int maskDepth) {
  std::ostringstream broadcast;
  std::string data;
  IPaddress addr;
  UDPpacket *packet;

  addProtocol(PROTOCOL_TCP);
  acceptConnections();

  return broadcastUDPInvitation(maskDepth);
}

/**
 * @brief Broadcasts the host address via UDP.
 *
 * Isolates the UDP broadcast from multiPlayerInit() so that it can be called
 * on a loop for continuous invitation.
 * @param maskDepth The depth of subnet bits to preserve.
 * @return Success of the broadcast: true if the server receives its own packet.
 */
bool NetManager::broadcastUDPInvitation(int maskDepth) {
  std::ostringstream broadcast;
  std::string data;
  IPaddress addr;
  UDPpacket *packet;

  SDLNet_ResolveHost(&addr, getMaskedIPstring(maskDepth).c_str(), PORT_DEFAULT);

  broadcast << STR_OPEN << getIPstring();
  data = broadcast.str();
  packet = craftUDPpacket(data.c_str(), data.length());
  packet->address.host = addr.host;
  packet->address.port = addr.port;
  sendUDP(udpSockets[netServer.udpSocketIdx], -1, packet);
  printError("NetManager: UDP Broadcast sent.");

  return scanForActivity();
}

/**
 * @brief Accepts a host's broadcasted invitation.
 *
 * Stops the dummy server and reinitializes as a client against the target host.
 * @param invitation The host's IP address from the UDP packet.
 * @return Success of starting the client against the target host.
 */
bool NetManager::joinMultiPlayer(std::string invitation) {
  std::string svrAddr = invitation.substr(STR_OPEN.length());
  stopServer();
  initNetManager();
  addNetworkInfo(PROTOCOL_ALL, svrAddr.c_str());

  return startClient();
}


/* ****************************************************************************
 * Private
 */


/**
 * @brief Launch a single server socket on a single protocol.
 *
 * A state-bound and error-checked wrapper of the SDLNet_ResolveHost call. To
 * reduce user calls, it chains into the protocol-specific socket opening call.
 * This is reachable only via a call from startServer().
 *
 * If both TCP and UDP are requested, this function will fire twice; once on
 * each.
 * @param protocol One of TCP or UDP at a time, as given previously by the user.
 * @param port The port previously established by the user.
 * @return The success or failure of the following TCP or UDP socket call.
 * @see openTCPSocket()
 * @see openUDPSocket()
 */
bool NetManager::openServer(Protocol protocol, Uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if ((netStatus & NET_RESOLVED) && (netServer.protocols & protocol)) {
    printError("NetManager: This protocol has already been established.");
    return false;
  }

  if (netStatus & NET_CLIENT) {
    printError("NetManager: Client already established. May not initiate server.");
    return false;
  }

  if (SDLNet_ResolveHost(&netServer.address, NULL, port)) {
    printError("SDL_net: Failed to start server!");
  } else {
    netServer.protocols |= protocol;
    netStatus |= NET_RESOLVED;
  }

  return (protocol == PROTOCOL_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

/**
 * @brief Launch a single client socket on a single protocol bound to hostname.
 *
 * A state-bound and error-checked wrapper of the SDLNet_ResolveHost call. To
 * reduce user calls, it chains into the protocol-specific socket opening call.
 * This is reachable only via a call from startClient().
 *
 * If both TCP and UDP are requested, this function will fire twice; once on
 * each.
 * @param protocol One of TCP or UDP at a time, as given previously by the user.
 * @param hostname The hostname previously given by the user.
 * @param port The port previously established by the user.
 * @return The success or failure of the following socket call.
 * @see openTCPSocket()
 * @see openUDPSocket()
 */
bool NetManager::openClient(Protocol protocol, std::string hostname, Uint16 port) {
  if (statusCheck(NET_WAITING))
    return false;

  if ((netStatus & NET_RESOLVED) && (netServer.protocols & protocol)) {
    printError("NetManager: This protocol has already been established.");
    return false;
  }

  if (netStatus & NET_SERVER) {
    printError("NetManager: Server already established. May not initiate client.");
    return false;
  }

  if (SDLNet_ResolveHost(&netServer.address, hostname.c_str(), port)) {
    printError("SDL_net: Failed to resolve server!");
  } else {
    netServer.protocols |= protocol;
    netStatus |= NET_RESOLVED;
  }

  return (protocol == PROTOCOL_TCP) ?
      openTCPSocket(&netServer.address) : openUDPSocket(port);
}

/**
 * @brief Opens a TCP socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_TCP_Open call.
 * @param addr The IPaddress upon which to open the socket.
 * @return True on success, false on failure, or the result of acceptTCP.
 */
bool NetManager::openTCPSocket(IPaddress *addr) {
  bool ret = false;

  if (statusCheck(NET_RESOLVED))
    return false;

  TCPsocket tcpSock = SDLNet_TCP_Open(addr);

  if (!tcpSock)
    printError("SDL_net: Failed to open TCP socket!");
  else {
    netServer.tcpSocketIdx = tcpSockets.size();
    netStatus |= NET_TCP_OPEN;
    tcpSockets.push_back(tcpSock);
    watchSocket(tcpSock);
    ret = true;
  }

  return ret;
}

/**
 * @brief Opens a UDP socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_UDP_Open call. Servers
 * and clients both stop here, as they differ only in how incoming connections
 * are handled.
 * @param port The port on which to open the socket.
 * @return True on success, false on failure.
 */
bool NetManager::openUDPSocket(Uint16 port) {
  bool ret = true;
  Uint16 udpPort = port;

  if (statusCheck(NET_RESOLVED))
    return false;

  if ((netStatus & NET_CLIENT) && forceClientRandomUDP)
    udpPort = PORT_RANDOM;

  UDPsocket udpSock = SDLNet_UDP_Open(udpPort);

  if (!udpSock) {
    printError("SDL_net: Failed to open UDP socket!");
    ret = false;
  } else {
    udpSockets.push_back(udpSock);
    watchSocket(udpSock);

    if (!(netStatus & NET_UDP_OPEN)) {
      netServer.udpSocketIdx = udpSockets.size() - 1;
      netStatus |= NET_UDP_OPEN;

      if (netStatus & NET_CLIENT)
        return bindUDPSocket(udpSock, nextUDPChannel++, &netServer.address);
    }
  }

  return ret;
}

/**
 * @brief Finalizes a listening socket for a server.
 *
 * A state-bound and error-checked wrapper of the SDLNet_TCP_Accept call. New
 * sockets for new clients are established here and added to the MessageInfo
 * public vector. If a client already has a ConnectionInfo struct for a UDP
 * connection, the TCP connection information will be added to it.
 * @param server The listening server socket.
 * @return True on success, false on failure.
 */
bool NetManager::acceptTCP(TCPsocket server) {
  bool ret = false;

  if (statusCheck(NET_SERVER | NET_TCP_OPEN))
    return false;

  if (!acceptNewClients || (tcpSockets.size() >= SOCKET_TCP_MAX)) {
    printError("NetManager: TCP client rejected. Not accepting new clients.");
    return false;
  }

  TCPsocket tcpSock = SDLNet_TCP_Accept(server);

  if (!tcpSock) {
    printError("SDL_net: Failed to accept TCP client on server socket.");
  } else if (!acceptNewClients || (tcpSockets.size() >= SOCKET_TCP_MAX)) {
    if (!acceptNewClients)
      printError("NetManager: TCP client rejected. Not accepting new clients.");
    else
      printError("NetManager: Exceeded max number of TCP connections.");
    rejectTCPClient(tcpSock);
  } else {
    ClientData *buffer = new ClientData;
    IPaddress *addr = queryTCPAddress(tcpSock);
    ConnectionInfo *client = lookupClient(addr->host, true);
    buffer->host = addr->host;
    buffer->updated = false;
    client->protocols |= PROTOCOL_TCP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->tcpSocketIdx = tcpSockets.size();
    client->tcpDataIdx = tcpClientData.size();
    tcpClientData.push_back(buffer);
    tcpSockets.push_back(tcpSock);
    watchSocket(tcpSock);

    if (!client->clientIdx) {
      netClients.push_back(client);
      client->clientIdx = netClients.size();
    }

    netStatus |= NET_TCP_ACCEPT;
    ret = true;
  }

  return ret;
}

/**
 * @brief Bind a UDP channel to a socket.
 *
 * Optional functionality from SDL that I've chosen to use. A maximum of 32
 * channels with different IPaddresses may be bound to any one socket, and
 * reaping sockets will iterate through each of these channels separately.
 * If a client already has a ConnectionInfo struct for a TCP connection, the
 * UDP connection information will be added to it.
 * @param sock The UDP socket to be bound.
 * @param channel The channel by which to bind this address to this socket.
 * @param addr The IPaddress of the hopeful connectee.
 * @return True on success, false on failure.
 */
bool NetManager::bindUDPSocket (UDPsocket sock, int channel, IPaddress *addr) {
  bool ret = true;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  int udpchannel;
  udpchannel = SDLNet_UDP_Bind(sock, channel, addr);

  if (udpchannel == -1) {
    printError("SDL_net: Failed to bind UDP address to channel on socket.");
    ret = false;
  }
  if (netStatus & NET_CLIENT) {
    netServer.udpChannel = udpchannel;
  } else if (netStatus & NET_SERVER) {
    ClientData *buffer = new ClientData;
    ConnectionInfo *client = lookupClient(addr->host, true);
    buffer->host = addr->host;
    buffer->updated = false;
    client->protocols |= PROTOCOL_UDP;
    client->address.host = addr->host;
    client->address.port = addr->port;
    client->udpChannel = udpchannel;
    client->udpSocketIdx = udpSockets.size() - 1;
    client->udpDataIdx = udpClientData.size();
    udpClientData.push_back(buffer);

    if (!client->clientIdx) {
      client->clientIdx = netClients.size();
      netClients.push_back(client);
    }
  }
  netStatus |= NET_UDP_BOUND;

  return ret;
}

/**
 * @brief Unbind a bound socket channel.
 *
 * @param sock The socket upon which the channel is bound.
 * @param channel The channel to be unbound.
 */
void NetManager::unbindUDPSocket(UDPsocket sock, int channel) {
  SDLNet_UDP_Unbind(sock, channel);
}

/**
 * @brief Send a single message to a single target via TCP.
 *
 * A state-bound and error-checked wrapper of the SDLNet_TCP_Send call. One
 * socketed target will receive one copy of the given message.
 * @param sock The target's socket.
 * @param data The data to send.
 * @param len The length of the data.
 * @return True on success, false on failure.
 */
bool NetManager::sendTCP(TCPsocket sock, const void *data, int len) {
  bool ret = true;

  if (statusCheck(NET_TCP_ACCEPT, (NET_CLIENT | NET_TCP_OPEN)))
    return false;

  if (len > SDLNet_TCP_Send(sock, data, len)) {
    printError("SDL_net: Failed to send TCP data.");
    printError(SDLNet_GetError());
    ret = false;
  }

  return ret;
}

/**
 * @brief Send a single message to a single target via UDP.
 *
 * A state-bound and error-checked wrapper of the SDLNet_UDP_Send call. One
 * channel-bound target \e may receive one copy of the given message. No
 * guarantees are given by UDP, and I have coded no guarantees here, yet.
 * @param sock The target's socket.
 * @param channel The target's specific, bound channel.
 * @param pack The SDL-formatted UDP packet to send.
 * @return True on success, false on failure.
 */
bool NetManager::sendUDP(UDPsocket sock, int channel, UDPpacket *pack) {
  bool ret = true;

  if (statusCheck(NET_UDP_OPEN) || !pack)
    return false;

  if (!SDLNet_UDP_Send(sock, channel, pack)) {
    printError("SDL_net: Failed to send UDP data.");
    if (channel != -1) {
      unbindUDPSocket(sock, channel);
    }
    ret = false;
  }

  freeUDPpacket(&pack);

  return ret;
}

/**
 * @brief Receive a single message from a single target via TCP.
 *
 * A state-bound and error-checked wrapper of the SDLNet_TCP_Recv call.
 * @param sock The target's socket.
 * @param data The destination buffer for the received data.
 * @param maxlen The maximum length of data to copy to the destination buffer.
 * @return True on success, false on failure.
 */
bool NetManager::recvTCP(TCPsocket sock, void *data, int maxlen) {
  bool ret = true;

  if (statusCheck(NET_TCP_ACCEPT, (NET_CLIENT | NET_TCP_OPEN)))
    return false;

  if (0 >= SDLNet_TCP_Recv(sock, data, maxlen)) {
    printError("SDL_net: Failed to receive TCP data.");
    ret = false;
  }

  return ret;
}

/**
 * @brief Receive a message from a random channel on a UDP socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_UDP_Recv call.
 * @param sock The target's socket.
 * @param pack The SDL-formatted destination buffer for the received data.
 * @return True on success, false on failure.
 */
bool NetManager::recvUDP(UDPsocket sock, UDPpacket *pack) {
  bool ret = true;
  int result;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  result  = SDLNet_UDP_Recv(sock, pack);

  if (result < 1) {
    ret = false;

    if (result < 0) {
      printError("SDL_net: Failed to receive UDP data.");
      printError(SDLNet_GetError());
    } else
      printError("NetManager: No packets received.");
  }

  return ret;
}

/**
 * @brief Send n packets from a packet vector using the specified socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_UDP_SendV call.
 * @param sock The target socket.
 * @param packetV The SDL-formatted UDP packet vector.
 * @param npackets The number of packets to send from the packet vector.
 * @return True on success, false on failure.
 */
bool NetManager::sendUDPV(UDPsocket sock, UDPpacket **packetV, int npackets) {
  bool ret = true;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  if (!SDLNet_UDP_SendV(sock, packetV, npackets)) {
    printError("SDL_net: Failed to send UDP data.");
    printError(SDLNet_GetError());
    ret = false;
  }

  return ret;
}

/**
 * @brief Receive up to len(packetV) packets from all channels on a socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_UDP_RecvV call.
 * @param sock The target socket.
 * @param packetV The SDL-formatted UDP packet vector.
 * @return True on success, false on failure.
 */
int NetManager::recvUDPV(UDPsocket sock, UDPpacket **packetV) {
  int result;

  if (statusCheck(NET_UDP_OPEN))
    return false;

  result  = SDLNet_UDP_RecvV(sock, packetV);

  if (result < 1) {
    if (result < 0) {
      printError("SDL_net: Failed to receive UDP data.");
      printError(SDLNet_GetError());
    } else
      printError("NetManager: No packets received.");
  }

  return result;
}

/**
 * @brief Close a TCP socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_TCP_Close call.
 * @param sock The socket to be closed.
 */
void NetManager::closeTCP(TCPsocket sock) {
  SDLNet_TCP_Close(sock);

  // Checks removed for now.
}

/**
 * @brief Close a UDP socket.
 *
 * A state-bound and error-checked wrapper of the SDLNet_UDP_Close call.
 * @param sock The socket to be closed.
 */
void NetManager::closeUDP(UDPsocket sock) {
  SDLNet_UDP_Close(sock);

  // Checks removed for now.
}

/**
 * @brief Look up an IPaddress by a TCP socket.
 * @param sock The socket to query.
 * @return The IPaddress of the socket's associated host.
 */
IPaddress* NetManager::queryTCPAddress(TCPsocket sock) {
  IPaddress *remote;

  remote = SDLNet_TCP_GetPeerAddress(sock);

  if (!remote) {
    printError("SDL_net: Error retrieving remote TCP IP/port. This"
        " may be a server socket.");
    printError(SDLNet_GetError());
  }

  return remote;
}

/**
 * @brief Look up an IPaddress by a UDP socket and channel.
 * @param sock The socket hosting the channel.
 * @param channel The target-to-query's bound channel.
 * @return The IPaddress of the channel's associated host.
 */
IPaddress* NetManager::queryUDPAddress(UDPsocket sock, int channel) {
  IPaddress *remote;

  remote = SDLNet_UDP_GetPeerAddress(sock, channel);

  if (!remote) {
    printError("SDL_net: Error retrieving remote UDP IP address.");
    printError(SDLNet_GetError());
  }

  return remote;
}

/**
 * @brief Allocate and fill a UDPpacket with the given buffer of len bytes.
 *
 * If allocUDPpacket() returns NULL, this function will also return NULL, but
 * without repeating the warning. Make sure to handle NULL packet pointers.
 * @param buf The source buffer.
 * @param len The length of bytes to copy.
 * @return An allocated and filled UDPpacket.
 */
UDPpacket* NetManager::craftUDPpacket(const char *buf, int len) {
  UDPpacket *packet;
  int header;

  if (len > MESSAGE_LENGTH) {
    printError("NetManager: Message length exceeds current maximum.");
    return NULL;
  }

  packet = allocUDPpacket(MESSAGE_LENGTH);

  if (!packet)
    return NULL;

  packet->len = len;
  memcpy(packet->data, buf, len);

  return packet;
}

/**
 * @brief Allocate a new SDL-formatted UDP packet.
 *
 * This is simply an error-checked wrapper of SDLNet_AllocPacket. This should
 * only be called for empty packets receiving data. Packets to be sent should
 * use craftUDPpacket().
 * @param size The number of bytes to allot the buffer portion of the packet.
 * @return The new, empty UDPpacket.
 */
UDPpacket* NetManager::allocUDPpacket(int size) {
  UDPpacket *newPacket;

  newPacket = SDLNet_AllocPacket(size);

  if (!newPacket) {
    printError("SDL_net: Unable to allocate UDP packet.");
  }

  return newPacket;
}

/**
 * @brief Allocate a new, empty, and SDL-formatted UDP packet vector.
 * @param count The number of packets to allocate.
 * @param size The size of each packet.
 * @return The new, empty UDP packet vector.
 */
UDPpacket** NetManager::allocUDPpacketV(int count, int size) {
  UDPpacket **newPacket;

  newPacket = SDLNet_AllocPacketV(count, size);

  if (!newPacket) {
    printError("SDL_net: Unable to allocate UDP packet.");
  }

  return newPacket;
}

/**
 * @brief Resize a UDP packet.
 * @param pack The packet to resize.
 * @param size The new size of the packet.
 * @return True on success, false on failure.
 */
bool NetManager::resizeUDPpacket(UDPpacket *pack, int size) {
  bool ret = true;
  int newSize;

  newSize = SDLNet_ResizePacket(pack, size);

  if (newSize < size) {
    printError("SDL_net: Unable to resize UDP packet as requested.");
    printError(SDLNet_GetError());
    ret = false;
  }

  return ret;
}

/**
 * @brief Free a UDP packet.
 * @param pack The packet to be freed.
 */
void NetManager::freeUDPpacket(UDPpacket **pack) {
  SDLNet_FreePacket(*pack);
  *pack = NULL;
}

/**
 * @brief Free a UDP packet vector.
 * @param pack The packet vector to be freed.
 */
void NetManager::freeUDPpacketV(UDPpacket ***pack) {
  SDLNet_FreePacketV(*pack);
  *pack = NULL;
}

/**
 * @brief Parse incoming data from server or clients for NetManager-specific
 * commands.
 *
 * Much of the data and operations will be handled by OGRE et al., but some
 * commands might be better suited for internal processing...
 * @param data The data buffer to be processed.
 */
void NetManager::processPacketData(const char *data) {
  /* TODO Scan copied data to check for messages to NetManager.
   *
   * Establish clear signals for 'drop client' et al.  How much will
   * be handled by the OGRE application, and how much will be taken
   * care of internally (by NetManager)?
   *
   */
}

/**
 * @brief Register a TCP socket to be watched for activity by SDL.
 * @param sock The socket to watch.
 */
void NetManager::watchSocket(TCPsocket sock) {
  if (-1 == SDLNet_TCP_AddSocket(socketNursery, sock))
    printError("SDL_net: Unable to add socket to SocketSet.");
}

/**
 * @brief Register a UDP socket to be watched for activity by SDL.
 * @param sock The socket to watch.
 */
void NetManager::watchSocket(UDPsocket sock) {
  if (-1 == SDLNet_UDP_AddSocket(socketNursery, sock))
    printError("SDL_net: Unable to add socket to SocketSet.");
}

/**
 * @brief Remove a TCP socket from SDL's observation.
 * @param sock The socket to remove.
 */
void NetManager::unwatchSocket(TCPsocket sock) {
  if (-1 == SDLNet_TCP_DelSocket(socketNursery, sock))
    printError("SDL_net: Unable to remove TCP socket from SocketSet.");
}

/**
 * @brief Remove a UDP socket from SDL's observation.
 * @param sock The socket to remove.
 */
void NetManager::unwatchSocket(UDPsocket sock) {
  if (-1 == SDLNet_UDP_DelSocket(socketNursery, sock))
    printError("SDL_net: Unable to remove UDP socket from SocketSet.");
}

/**
 * @brief Ask SDL to scan registered sockets once or for a given time period.
 *
 * This function will automatically handle all activity discovered on TCP and
 * UDP. New clients will be added, and data will be copied to the ClientData
 * buffers. <em>The user should check the ClientData arrays after calling this
 * function!</em>  Excess or unwanted clients will be rejected.
 * @param timeout_ms The time to scan in milliseconds. 0 is instant.
 * @return True if there was activity, false if there was not.
 */
int NetManager::checkSockets(Uint32 timeout_ms) {
  int ret, nReadySockets;
  ret = 0;

  nReadySockets = SDLNet_CheckSockets(socketNursery, timeout_ms);

  if (nReadySockets == -1) {
    printError("SDL_net: System error in CheckSockets.");
    printError(SDLNet_GetError());
  } else if (nReadySockets) {
    int i, udp;
    ret = nReadySockets;
    i = udp = 0;

    //std::cout << "Starting with packet(s) in NetManager." << std::endl;

    if (netServer.protocols & PROTOCOL_TCP) {                           // TCP
      if (netStatus & NET_SERVER) {                                    //Server
        if (SDLNet_SocketReady(tcpSockets[netServer.tcpSocketIdx])) {
          if (acceptTCP(tcpSockets[netServer.tcpSocketIdx]))
            printError("New TCP client registered!");
          nReadySockets--;
        }
        for (i = 0; i < netClients.size() && nReadySockets; i++) {
          if ((netClients[i]->protocols & PROTOCOL_TCP) &&
              SDLNet_SocketReady(tcpSockets[netClients[i]->tcpSocketIdx])) {
            readTCPSocket(i);
            nReadySockets--;
          }
        }
      } else if (netStatus & NET_CLIENT) {                            // Client
        if (SDLNet_SocketReady(tcpSockets[netServer.tcpSocketIdx])) {
          readTCPSocket(SOCKET_SELF);
          nReadySockets--;
        }
      }
    }
    if (netServer.protocols & PROTOCOL_UDP) {                           // UDP
      if (SDLNet_SocketReady(udpSockets[netServer.udpSocketIdx])) {
        udp += readUDPSocket(SOCKET_SELF);
        nReadySockets--;
      }
      if (netStatus & NET_SERVER) {                                   // Server
        for (i = 0; i < netClients.size() && nReadySockets; i++) {
          if ((netClients[i]->protocols & PROTOCOL_UDP) &&
              SDLNet_SocketReady(udpSockets[netClients[i]->udpSocketIdx])) {
            udp += readUDPSocket(i);
            nReadySockets--;
          }
        }
      }
    }
    ret = udp ? udp : ret;
  }

  return ret;
}

/**
 * @brief Receives a TCP socket and copies its data to the ClientData buffer.
 * @param clientIdx An index into the tcpClients vector.
 */
void NetManager::readTCPSocket(int clientIdx) {
  bool result;
  int idxSocket;
  ClientData *cData;

  if (clientIdx == SOCKET_SELF) {
    idxSocket = netServer.tcpSocketIdx;
    cData = &tcpServerData;
  } else {
    idxSocket = netClients[clientIdx]->tcpSocketIdx;
    cData = tcpClientData[netClients[clientIdx]->tcpDataIdx];
  }

  memset(cData->output, 0, MESSAGE_LENGTH);

  result = recvTCP(tcpSockets[idxSocket], cData->output,
      MESSAGE_LENGTH);

  if (!result) {
    printError("NetManager: Failed to read TCP packet.");
    if (netStatus & NET_CLIENT) {
      closeTCP(tcpSockets[idxSocket]);
    } else {
      dropClient(PROTOCOL_ALL, cData->host);
    }
  } else {
    cData->updated = true;
  }
}

/**
 * @brief Receives a UDP socket and copies its data to the ClientData buffer.
 *
 * Because many channels may be bound to a single socket, the vector versions
 * of UDPpacket and udpRecv are used to gather anything and everything that
 * might arrive in one sweep of the socket. New clients are added, if possible.
 * @param clientIdx An index into the udpClients vector.
 */
int NetManager::readUDPSocket(int clientIdx) {
  UDPpacket **bufV;
  ClientData *cData;
  ConnectionInfo *client;
  int idxSocket, numPackets, ret, i;

  cData = udpServerData;
  idxSocket = (clientIdx == SOCKET_SELF) ? netServer.udpSocketIdx :
      netClients[clientIdx]->udpSocketIdx;

  bufV = allocUDPpacketV(MESSAGE_COUNT, MESSAGE_LENGTH);

  numPackets = recvUDPV(udpSockets[idxSocket], bufV);

  if (numPackets < 0) {
    printError("NetManager: Failed to read UDP packet.");
    ret = 0;
  } else {
    ret = numPackets;

    for (i = 0; i < numPackets; i++) {

      if (bufV[i]->channel == -1) {                         // Unbound sender.
        if (bufV[i]->address.host == getIPnbo() || (netStatus & NET_CLIENT)) {
          //   Our own packet from broadcast    OR  non-server to a client.
          if (netStatus & NET_CLIENT)
            printError("NetManager: Invalid packet source.");
          ret--;
        } else if (0 == STR_DENY.compare((const char *) bufV[i]->data)) {
          // Received rejection packet.  Don't process it (for now).
          ret--;
        } else if (!addUDPClient(bufV[i])) {
          // Try to add the client; if not, at least copy the data.
          memcpy(cData[i].output, bufV[i]->data, bufV[i]->len);
          cData[i].updated = true;
        }
      } else {                                               // Bound sender.
        if (netStatus & NET_CLIENT) {
          // Message comes from server, cData default is good (above).
          memcpy(cData[i].output, bufV[i]->data, bufV[i]->len);
          cData[i].updated = true;
        } else if ((client = lookupClient(bufV[i]->address.host, false))) {
          // Message comes from client, lookup new cData.
          cData = udpClientData[client->udpDataIdx];
          memcpy(cData->output, bufV[i]->data, bufV[i]->len);
          cData->updated = true;
        } else {
          printError("NetManager: Failed to look up existing client.");
          ret--;
        }
      }

    }
  }

  if (bufV)
    freeUDPpacketV(&bufV);

  return ret;
}

/**
 * @brief Adds a client discovered on a UDP socket.
 * @param pack The originating packet of the prospective client.
 * @return True on success, false on failure.
 */
bool NetManager::addUDPClient(UDPpacket *pack) {
  ClientData *cData;
  ConnectionInfo *client;
  bool ret = true;
  int socketIdx;

  if (!acceptNewClients) {
    //printError("NetManager: UDP client rejected. Not accepting new clients.");
    rejectUDPClient(pack);
    return false;
  }

  if (nextUDPChannel >= CHANNEL_MAX) {
    if (openUDPSocket(PORT_DEFAULT))
      nextUDPChannel = CHANNEL_DEFAULT;
    else {
      printError("NetManager: Exceeded max number of UDP connections.");
      rejectUDPClient(pack);
      return false;
    }
  }

  bindUDPSocket(udpSockets.back(), nextUDPChannel++, &pack->address);

  if ((client = lookupClient(pack->address.host, false))) {
    cData = udpClientData[client->udpDataIdx];
    memcpy(cData->output, pack->data, pack->len);
    cData->updated = true;
  }

  printError("New UDP client registered!");

  return ret;
}

/**
 * @brief Rejects a prospective TCP client.
 *
 * Sends a rejection message and closes the socket.
 * @param sock The rejectee's associated socket.
 */
void NetManager::rejectTCPClient(TCPsocket sock) {
  sendTCP(sock, STR_DENY.c_str(), STR_DENY.length());

  closeTCP(sock);
}

/**
 * @brief Rejects a prospective UDP client.
 *
 * Sends a rejection message and frees the socket.
 * @param pack The rejectee's associated packet.
 */
void NetManager::rejectUDPClient(UDPpacket *pack) {
  UDPpacket *packet;

  packet = craftUDPpacket(STR_DENY.c_str(), STR_DENY.length());
  packet->address.host = pack->address.host;
  packet->address.port = pack->address.port;
  sendUDP(udpSockets[netServer.udpSocketIdx], -1, packet);
}

/**
 * @brief Look up a TCP client by IPaddress host.
 *
 * IPaddress host is available from almost anywhere, and this conversion to a
 * ConnectionInfo pointer allows access to the correct index into all of the
 * client's associated vectors. If the ConnectionInfo does not already exist,
 * the boolean allows a new instance to be returned instead.
 * @param host The IPaddress host.
 * @param create True to return a new ConnectionInfo instance, false for NULL.
 * @return Either the correct CInfo, a new CInfo, or null.
 */
ConnectionInfo* NetManager::lookupClient(Uint32 host, bool create) {
  std::vector<ConnectionInfo *>::iterator it;

  for (it = netClients.begin(); it != netClients.end(); it++) {
    if ((*it)->address.host == host)
      return *it;
  }
  if (netServer.address.host == host)
    return &netServer;

  return create ? new ConnectionInfo() : NULL;
}

/**
 * @brief Convert a network-byte-order host to dotted string representation.
 *
 * Stepping mask for /8, /16, /24, and /32 address preservation.
 * @param host The desired host.
 * @param subnetMask The bit-depth of IP subnet mask (8/16/24/32).
 * @return The decimal-separated string representation.
 */
std::string NetManager::ipToString(Uint32 host, int subnetMask) {
  std::ostringstream result;
  Uint8 *it;
  int a, b, c, d;

  it = (Uint8 *) &host;

  a = (subnetMask >=  8) ? *it++ : 255;
  b = (subnetMask >= 16) ? *it++ : 255;
  c = (subnetMask >= 24) ? *it++ : 255;
  d = (subnetMask >= 32) ? *it   : 255;

  result << a << "." << b << "." << c << "." << d;

  return result.str();
}

/**
 * @brief A simple state (bit flag) check for early returns and error prints.
 * @param state The state to be checked.
 * @return True if state is lacking, false if state achieved.
 */
bool NetManager::statusCheck(int state) {
  std::ostringstream errorMsg;

  bool ret = (state != (netStatus & state));

  if (ret) {
    errorMsg << "NetManager: Invalid state for command. Missing bit: " << state;
    printError(errorMsg.str());
  }

  return ret;
}

/**
 * @brief A compound state (bit flag) check for early returns and error prints.
 * @param state1 The first state to be checked.
 * @param state2 The second state to be checked.
 * @return True if either state is lacking, false if both states achieved.
 */
bool NetManager::statusCheck(int state1, int state2) {
  std::ostringstream errorMsg;

  bool ret1 = (state1 != (netStatus & state1));
  bool ret2 = (state2 != (netStatus & state2));
  bool result = ret1 && ret2;

  if (result) {
    errorMsg << "NetManager: Invalid state for command. Missing bit: "
        << state1 << " or " << state2;
    printError(errorMsg.str());
  }

  return result;
}

/**
 * @brief Clears a given bit mask of state flags from the internal netStatus.
 * @param state The flags to clear.
 */
void NetManager::clearFlags(int state) {
  int mask = netStatus & state;
  netStatus ^= mask;
}

void NetManager::printError(std::string errorText) {
  std::cout << "[ NetManager ]*********************************************"
      "**************\n" << std::endl;
  std::cout << errorText << "\n" << std::endl;
  std::cout << "***********************************************************"
      "**************" << std::endl;
}

/**
 * @brief Clears all vectors and resets all data members to default values.
 *
 * After this function completes, the instance will be considered INITIALIZED
 * and may launch a new server or client.
 */
void NetManager::resetManager() {
  int i;

  for (i = tcpClientData.size() - 1; i >= 0; i--) {
    delete tcpClientData[i];
    tcpClientData.pop_back();
  }
  for (i = udpClientData.size() - 1; i >= 0; i--) {
    delete udpClientData[i];
    udpClientData.pop_back();
  }
  for (i = netClients.size() - 1; i >= 0; i--) {
    delete netClients[i];
    netClients.pop_back();
  }
  SDLNet_FreeSocketSet(socketNursery);

  forceClientRandomUDP = true;
  acceptNewClients = true;
  nextUDPChannel = CHANNEL_DEFAULT;
  netStatus = NET_UNINITIALIZED;
  netPort = PORT_DEFAULT;
  netProtocol = PROTOCOL_ALL;
  netLocalHost = 0;
  netHostname.clear();
}




