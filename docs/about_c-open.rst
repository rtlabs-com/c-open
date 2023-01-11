.. _about-c-open:

C-Open CANopen stack
===========================
The RT-Labs CANopen stack C-Open can be used to implement a CANopen master or
device. It supports multiple instances and can be run on bare-metal hardware,
an RTOS such as RT-Kernel, or on Linux or Windows.

The C-Open stack is supplied with full sources including a porting layer. A
device application interfaces with the stack primarily using the object
dictionary, while a master uses the full API to control the CANopen network.
It is designed for minimal footprint and efficiency (memory usage on
Cortex-M4, 14968 B ROM / 368 B RAM, plus user defined objects stored in RAM)
and tested using the CANopen Conformance Test Tool.

Web resources
-------------
* Source repository: https://github.com/rtlabs-com/c-open
* Documentation: https://rt-labs.com/docs/c-open
* Continuous integration: https://github.com/rtlabs-com/c-open/actions
* RT-Labs (stack integration, certification services and training): https://rt-labs.com

Features
--------
* CANopen master
* CANopen device
* CANopen services (CiA 301)
* Network management (NMT)
* Service data objects (SDO)
* Process Data Objects (PDO)
* Emergency object (EMCY)
* Heartbeat
* Node guarding
* Layer Setting Services (LSS, CiA 305)
* Bare-metal or RTOS
* Porting layer provided
* Linux (SocketCAN)
* Windows (Kvaser CAN interface)

Limitations
-----------
*

License
-------
This software is dual-licensed, with GPL version 3 and a commercial license.
See LICENSE.md for more details.

Requirements
------------
cmake

* cmake 3.14 or later

For Linux:

* gcc 4.6 or later

For rt-kernel:

* Workbench 2020.1 or later

For Windows:

* Visual Studio 2017 or later
* Kvaser CANlib SDK

Contributions
-------------
Contributions are welcome. If you want to contribute you will need to sign a
Contributor License Agreement and send it to us either by e-mail or by physical
mail. More information is available on https://rt-labs.com/contribution.
