#ifndef PARALLEL_PRINT_SERVER_H
#define PARALLEL_PRINT_SERVER_H

#define MAX_PACKET 5000

bool checkConnection();
bool attemptConnection();
void loopTestLine();
void loopServer();

#endif
