# IPC-and-MQTT-Protocol
Implement IPC(Message V Queue) and MQTT Protocol in Raspberry Pi 3

## How To Run
 - [ReadMe](https://github.com/wh2per/Raspberry-Pi-3/blob/master/README.md)
 
## Example
### 1. IPC(Message V Queue)
+ ### Operation Keys
  * `1` Botton : Create Message Queue
  * `2` botton : Close Message Queue
  * `3` Botton : Send Message
  * `4` botton : Receive Message
  * `5` Botton : Exit
+ ### ScreenShot
  #### 1. Create Message Queue
  + Input right value
   <br> <img src="IPC/screenshot/create_1.png">
  + Input duplicate value
   <br> <img src="IPC/screenshot/create_2.png">
  #### 2. Close Message Queue
   + <b>Input right value
    <br> <img src="IPC/screenshot/close_2.png">
   + <b> Input missing value
    <br> <img src="IPC/screenshot/close_1.png">
  #### 3. Send Message
   + <b> Input right value
    <br> <img src="IPC/screenshot/send_1.png">
   + <b> When the message queue is full (wait)
    <br> <img src="IPC/screenshot/send_2.png"> 
   + <b> When there are extra places in the message queue after full
    <br> <img src="IPC/screenshot/send_3.png"> 
   + <b> When the message queue is full (not wait)
    <br> <img src="IPC/screenshot/send_4.png"> 
  #### 4. Receive Message
   + <b> Input right value
    <br> <img src="IPC/screenshot/recv_1.png">
   + <b> When the message queue is empty (wait)
    <br> <img src="IPC/screenshot/recv_2.png"> 
   + <b> When there is a message in the message queue after empty
    <br> <img src="IPC/screenshot/recv_3.png"> 
   + <b> When the message queue is empty (not wait)
    <br> <img src="IPC/screenshot/recv_4.png"> 
  #### 5. Exit
   + <b> Input `5`
    <br> <img src="IPC/screenshot/out.png"> 
 
### 2. MQTT

