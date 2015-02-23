# hlCANopen

![travis master](https://travis-ci.org/team-diana/hlCANopen.svg?branch=master)

**hlCANopen** is an high-level C++ library for the CANopen protocol.

The library provides a generic and async interface, making its usage simpler 
in comparision to other libraries.

## Examples

```c++
CanOpenManager<CanCard> manager(canCard);

// Setup a CANopen server, that can be queried by other nodes in the network.
NodeId serverNodeId = 1;
manager(serverNodeId, NodeManagerType::SERVER);

// Populate the server' object dictionary. Types are automatically deduced.
manager.writeSdoLocal(serverNodeId, SDOIndex(0xabcd, 0), "hello world!");
manager.writeSdoLocal(serverNodeId, SDOIndex(0xabcd, 1), 0x1234);

// Starts the manager. Now it will answer to the read/write requests 
// from other nodes. Access for each entry is configurable
thread managerThread = std::thread([&](){
  manager.run();
});

// Read a value from an another server in the network
NodeId otherServerId = 2;
std::future<SdoResponse<int32_t>> readResult = 
    manager.readSdoRemote<int32_t>(otherServerId, SDOIndex(0xaabb, 0));
// The returned value is a future, so other actions can be performed 
// while the value is read. New request can also be submitted and 
// executed concurrently when they involve different nodes. 

// now, get the response
if(readResult.get().ok()) {
  std::cout << "result is " << readResult.get().get();
}

// futures allow to write async code sequentially, however it is 
// also possible to use callbacks.
manager.readSdoRemote<string>(otherServerId, SDOIndex(0xaabb, 0), 
[](SdoResponse<string> res) {
  std::cout << "result is " << res.get();
});
```

## Build
```
mkdir build && cd build
cmake ..
make 
make test
```
The library uses C++14, and was tested with the following compilers:

```
gcc 4.9.2
clang 3.5.1
```

## Implementation details
In order to handle multiple nodes concurrently this library uses **Boost Coroutine**. So the 
cost of having multiples thread is avoided (only a thread for CAN network interface should be used).

## Status

The library is currently under development.

- [ ] SDO support.
  - [x] basic support for both SDO server and client.
  - [x] future and callback functionality for async read and write.
  - [ ] full error handling.
- [ ] PDO support.
- [ ] NMT support.
- [ ] SYNC and TIMESTAMP support.
- [ ] emergency and error control. 

## Libraries Used
- Boost www.boost.org
- easyloggingcpp https://github.com/easylogging/easyloggingpp
