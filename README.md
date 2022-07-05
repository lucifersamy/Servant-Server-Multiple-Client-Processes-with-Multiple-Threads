# Servant-Server-Multiple-Client-Processes-with-Multiple-Threads
There are 3 programs to be developed: servant, server and client (Figure 1). The servant processes will answer the requests coming from the server through sockets. The client will make requests to the server through sockets, and the server will respond to those requests via the information acquired from the servants.

shortcomings: Servant threads should be detached.
