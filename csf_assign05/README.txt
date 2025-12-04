Sample README.txt

Eventually your report about how you implemented thread synchronization
in the server should go here

In Milestone 2, thread syncrhonization was a big topic that was implemented in 3 major sections (files):
1. Message Queue 
2. Room
3. Server  
The major goal for synchronization was to protect mutable data with pthread mutexes, use semaphores to 
manage waiting in threads, use guards (locks) to avoid lock ordering cycles, and overall prevent 
race conditions and deadlocks to create a well-designed, synchronized chat server. We used logic to determine 
which sections should be labeled as critical, as situations could occur in these sections that cause race conditions,
deadlocks, or other issues due to thread synchronization. Here was our thought process and implementation:

1. Message Queue:
Each User has MessageQueue data structure which contains important shared data like queue messages, mutex lock, and semaphore avail.
The mutex is used to protect shared m_messages from deque through concurrent access and semaphores to block waiting threads and to also
track avaliable messages. 
This queue is accessed concurrently, so both enqueue() (adding element) and dequeue() (removing element) are critical sections. 
Enqueue is a critical section because multiple sender threads may send messages simultaneously, causing concurrent writes to messages.
To combat this, we use a mutex so only one thread can modify the queue at a time, preventing corruption of the queue data structure.
We also use a semaphore outside the locking and unlocking of the critical sections to avoid blocking and avoid potential hazards.
We use a mutex for mutual exclusion for shared m_messages so that we can ensure that only one threat is modified at a time.
This prevents race conditions like concurrent modification of the queue or reading corrupt queue data, as well as also avoiding deadlocks 
by using a mutex to lock and unlock, as well as preventing blocking while utilizing the mutex. Dequeue is also a critical section because
we don't want messages to be removed while messages may be added (or messages are empty). We utilize a similar mutex locking and unlocking 
to enqueue as well as the semaphore to block until messages are available to dequeue. This prevents attempted removal from an empty queue, 
accessing freed memory (due to race conditions), and removing the same message twice. The cool thing about the usage of the semaphore
and the lock in the Message Queue implementation is that, like the assignment instructions said, the lock ensures the queue can only
be modified by one thread at a time, and the semaphore "notifies" that messages are available, effectively creating this "smart counter".
Furthermore our methodologies prevent specific race conditions such as concurrent modifications, reading from an empty queue, and lost messages.
Thus, deadlocks are avoided by never holding the mutex while we are waiting on the semaphore. 

2. Room:
Each Room has a set of Users as well as a mutex lock. All senders/receivers joining or leaving the room access this Users set concurrently,
creating critical sections for dealing with this set of Users and how they broadcast messages to the room overall. 
These critical sections are specifically located in add_member(), remove_member(), and broadcast_message(). The first critical sections are 
during the addition or removal of members to the Users set, and this is critical because mutations to the Users set must be atomic (uninterruptible).
These critical sections use a Guard (which is basically a mutex that has automatic handling of locking and unlocking) to handle concurrency problems,
and this guard is used for both insertion and erasing users. By using a guard we also ensure that the mutex is always released even if an exception is thrown.
Without it, two users could be attempted to be inserted or removed concurrently, breaking
the users structure. Also, receivers in the room could be removed while iterating in broadcast message, causing major issues with the room messages.
The other critical section in the Room is broadcasting a message (function broadcast_message) which also uses a Guard to handle concurrency issues.
This is a critical section because while iterating over the Users set for broadcasting a message from a sender to receivers, a receiver could join or 
leave the room causing major issues for the retrieval of the message. Also, this could corrupt the iterator meaning traversal could be incomplete or go out of bounds.
We hold the Room's lock while iterating and calling the user enqueue(). This iterating locking prevents the iterator from breaking while at the same time
ensuring that all current members receive the message without an error like the MessageQueue lock outliving the room lock.
This prevents a deadlock by using a Guard on it's own, but also using the mutex in the MessageQueue it uses within the function, which we went over in the last
concurrency explanation section up above. Also, race conditions are prevented using these Guards so Users don't receive duplicate or missing messages, 
or messed up Users iteration occurs, ultimately resulting in a messed up Room with corrupt membership due to lack of thread syncrhonization. 

3. Server:
A server has a map of rooms, and a mutex lock. The map is accessed concurrently when senders and receivers join rooms. There is one major critical section
in the server (specifically in find_or_create_room()). This is a critical section because multiple users may try to join the same room simultaneously, which 
without thread synchronization, could cause two threads creating two different rooms with the same name, or corrupting the map data structure through insertion. 
In other words, we use the ensure only one thread can be read or wrote from at a singular time. 
A Guard is used here again (reminder, mutex with automatic handling of locking and unlocking) to make sure only one thread can read or write from this critical section
at a time to guarantee that rooms stay unique, and preserve the structure of the map. This avoids deadlocks and race conditions by using this Guard object.
In this section we first check if the room exists then creates it if it doesn't. By using this methodology, race conditions are prevented.
Specifically, we avoid two threads simulatensouly checking for a non-existent room and then both rooms creating the same room. We prevent deadlocks by using
a singular mutex per server and we also ensure that the guard always releases the lock. 

Conclusion:
Overall, we handled thread synchronization by using mutexes and semaphores to protect the validity and structure of the Message Queue, as well as 
Guards (mutexes) to make sure the Users set membership is protected, as well as protecting server room finding/creation. This avoids synchronization hazards
because all mutexes are locked and unlocked safely (Guards assist a lot with this), and overall race conditions, deadlocks, and any other hazards are prevented
by protecting these critical sections to make sure any information that must be accessed or mutated concurrently is safe from any erraneous activity.

