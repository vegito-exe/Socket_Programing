# Socket Programming

## v1.0.0

- A one-way communication channel between a server (receiver) and a client (sender).
- Works only over a LAN due to routing limitations.

## v2.0.0

- A bidirectional private chat between a server and a client.

## v3.0.0

- A multi-client chat server (server excluded from chatting in this version --only logging messages) , where the server starts to listen , exposes the LAN ip for clients to acess . done using threads and used mutex's to avoid race condition crashes , with improved code readabilty / refractoring and error / input validation .

## v4.0.0 
- v3.0.0 + WAN access with VPN and improving threads connection handeling with event driven reads using SELECT c funciton (maybe add TLS encryption with openSSL lib maybe not );
