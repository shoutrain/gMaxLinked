redis structure for this application 
===========================
// Following structures are just portions used by this application.

hashes - exmple:account:online:<userId>
    userName   <string>
    userType   <integer>
    clientType <integer>
    clientInfo <string>
    lastLogin  <timestamp>

sorted sets - example:message:queue:<userId> <queueStatus>>32&timestamp> <messageId>

hashes - example:message:<messageId>
    type   <messageType>
    text   <string>
    url    <string>
    fileId <fileId>
    ...