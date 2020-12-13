# Multi-Access Point of Sales System - A meal ordering system with multiple ordering interfaces
## Introduction
This is the final project of the "Embedded Operating System" course in 2017, Fall semester. We built a meal ordering system to alleviate the crowded situation in our campus food court. This system hepls the customers to order meals with two points of service: online and in-store ordering. The online ordering system receives orders and assigns order pin codes with a messenger chatbot. The orders are first saved on a server waiting the customers to comfirm them in-store. When the customer arrives at the retaurant and enters the pin code on the system, the order will be sent to the meal preparation list in the kitchen. The in-store ordering system is a hand-held device that the cashier can order meals for the customers in the restaurant. The oreders are sent to the meal preparation list as well.

To implement this system, we adopted multiple PXA270 computer modules (developer verion with LCD screen and keyboard) as the I/O devices (ordering and displaying). One of the devices is used as a POS carried by a clark and entering the meals which are ordered by the customers. And another one is deployed in the kitchen for displaying the meal preparation list (Kitchen Server). This system improves the effectivity of the communication between the kitchen staffs and the servers. Another device is an automatic counter (Mobile Counter) for confirming the orders which are directly sent from phones, the customers who use phone to order meals need to enter the pin code provided by the chatbot. Also, we have implement a Facebook chat bot server for receiving the order from customers. The application scenario is shown bellow:
![scenario](pics/application_scenario.png)

## Hardware Architecture
The system consists of three PXA270 development broads and a server. PXA270 is SoC (system on chip) with a CPU and rich peripherals (LCD, keyboard and ethernet port). The PXA270 are all connected to a router, and the communication between them is established by socket programming. The hardware architecture is shown bellow:
![hard arch](pics/system_architecture.png)

## Software Architecture
There are three thread working together to control the system, they are responsible for controlling the POS, Mobile Counter and Kitchen Server, respectively. A chatbot server is another program for receiving the messages from orders using FB messenger, and it will pass the orders to the thread which is responsible for the Mobile Counter. The software (system) architecture is shown bellow:
![soft arch](pics/software_architecture.png)





## Demo
Click [Emulator Demo] to watch the demonstraction of our system with emulator.

Click [PXA270 Demo] to watch the demonstraction of our system implemented on development broads.


[Emulator Demo]:https://youtu.be/zaPLEpnmBqA
[PXA270 Demo]:https://www.youtube.com/watch?v=Hm7lPj2G6bA&feature=youtu.be
