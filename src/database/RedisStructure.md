redis structure for this application 
===========================
// Following structures are just portions used by this application.

sorted sets - example:account:session <userId> <sessionId>

hashes - exmple:account:online:<sessionId>
    ...
    lastSend   <timestamp>
    ...

sorted sets - example:message:queue:<userId> <timestamp> <messageId>

hashes - example:message:<messageId>
    text   <string>
    url    <string>
    fileId <fileId>
    ...