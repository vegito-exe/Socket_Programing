# Socket Programming

## v1.0.0

- A one-way communication channel between a server (receiver) and a client (sender).
- Works only over a LAN due to routing limitations.

## v2.0.0

- A bidirectional private chat between a server and a client.

## v3.0.0

- A reusable bidirectional TCP chat application for a server and a client.
- Run the host with a port, or connect as a client using the host's IPv4 address and port.
- Validates IPv4 addresses and port values before creating a connection.
- Displays the host's LAN IP address so other devices on the same network can connect.
- Organizes socket creation, connection handling, and message exchange into reusable functions.
- Supports up to 100 pending connections while waiting for a client.
