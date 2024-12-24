# filesys-Cplus
This design contains the implementation of an OS that utilizes file system management on a disk. This was developed using C++. 

Objective: 
  -Implement a hash table.
  -Implement linear, quadratic and double-hash probing to manage hash collisions.
  -Implement an incremental re-hashing algorithm.

Introduction:
We are implementing a new operating system. A part of the operating system is a file system to manage the files on the disk. The users can create a new file, delete an existing file, and search for a file. The main operations in the file system are insert, find, and remove. Since the efficiency of operations is extremely important, we have decided to use a hash table to manage accessing the files on the disk. The keys are files names, we know that this can cause collisions and clustering in a hash table, since there are many common names to be used. To manage the collision cases, we use multiple forms of probing for the collision handling policy. The file system can change the table size based on some specific criteria in order to use the memory more efficiently. When a change is required, we need to rehash the entire table. However, the rehash operation happens incrementally during multiple regular operations.

Hash Table:
A hash table is a data structure which implements the map ADT. It stores the pairs of key-value. The key determines the index number of an array cell (bucket) in which the value will be stored. A hash function calculates the index number. If there is more than one value for a key in the data set, the hash table uses a collision handling scheme to find another cell for storing the new value.

-Linear Probing
When we try to find an index in the hash table for a key and we find out that the determined bucket is taken, we need to find another bucket to store the key. In a linear probing we store the information in the next available bucket. In this scheme the jump step to find the next available bucket is one.

-Quadratic Probing
Linear probing can cause clustering, i.e. having many data points in a row. Clustering reduces the search efficiency. Using quadratic probing the jump step to find the next available bucket is the square of iteration number.

-Double-Hash Probing
In this project we use the following equation to find the next bucket for the purpose of the double hashing collision handling, in which i = 0,1,2,3 ....

  Formula to be used:
  index = ((Hash(key) % TableSize) + i x (11-(Hash(key) % 11))) % TableSize

Specifications:

The application starts with a hash table of size MINPRIME. After certain criteria appearing it will switch to another table and it transfers all data nodes from the current table to the new one incrementally. Once the switching process starts it scans 25% of the table and transfers any live nodes it finds in the old table and at every consecutive operation (insert/remove) It continues to scan 25% more of the table and transfers live data from the old table to the new table until all data is transferred. We do not transfer deleted buckets to the new table.

After an insertion, if the load factor becomes greater than 0.5, we need to rehash to a new hash table. The capacity of the new table would be the smallest prime number greater than 4 times the current number of data points. The current number of data points is total number of occupied buckets minus total number of deleted buckets.

After a deletion, if the number of deleted buckets is more than 80 percent of the total number of occupied buckets, we need to rehash to a new table. The capacity of the new table would be the smallest prime number greater than 4 times the current number of data points. The current number of data points is total number of occupied buckets minus total number of deleted buckets.

During a rehashing process the deleted buckets will be removed from the system permanently. They will not be transferred to the new table.

If a change of collision handling policy is requested by the user while the program is running, the new policy will be applied to the new table when a new rehash is triggered.

Files:

  filesys.h – header file for the File and FileSys classes.
  filesys.cpp – the template file for the FileSys class. Complete your FileSys         implementation in this file.
  driver.cpp – A sample driver program.
  driver.txt – A sample output produced by driver.cpp.


FileSys Class:
The FileSys class uses the File class. It has a member variable to store a pointer to a hash function. It also has two member variables to store pointers to two arrays of File objects. These arrays are m_currentTable and m_oldTable, and the m_name member variable of the File object is used as the key for hashing purposes. A File object has another member variable which stores a file block number on disk. The file block number and the file name define the uniqueness of a File object together.

FileSys::FileSys(int size, hash_fn hash, prob_t probing = DEFPOLCY);
The constructor takes size to specify the length of the current hash table, and hash is a function pointer to a hash function. The type of hash is defined in FileSys.h.
The table size must be a prime number between MINPRIME and MAXPRIME. If the user passes a size less than MINPRIME, the capacity must be set to MINPRIME. If the user passes a size larger than MAXPRIME, the capacity must be set to MAXPRIME. If the user passes a non-prime number the capacity must be set to the smallest prime number greater than user's value. The probing parameter specifies the type of collision handling policy for the current hash table.
Moreover, the constructor creates memory for the current table and initializes all member variables.

FileSys::~FileSys();
Destructor deallocates the memory.

bool FileSys::insert(File file);
This function inserts an object into the current hash table. The insertion index is determined by applying the hash function m_hash that is set in the FileSys constructor and then reducing the output of the hash function modulo the table size. A sample hash function is provided in the driver.cpp file.
Hash collisions should be resolved using the probing policy specified in the m_currProbing variable. We insert into the table indicated by m_currentTable. After every insertion we need to check for the proper criteria, and if it is required, we need to rehash the entire table incrementally into a new table. The incremental rehashing proceeds with scanning 25% of the table at a time and transfer any live data found to the new table. Once we transferred the live nodes in the first 25% of the table, the second 25% live data will be transferred at the next operation (insertion or removal). Once all data is transferred to the new table, the old table will be removed, and its memory will be deallocated.
If the File object is inserted, the function returns true, otherwise it returns false. A File object can only be inserted once. The hash table does not contain duplicate objects. Moreover, the block number value should be a valid one falling in the range [DISKMIN-DISKMAX]. Every File object is a unique object carrying the File's name and the file block number. The File's name is the key which is used for hashing.

bool FileSys::remove(File file);
This function removes a data point from either the current hash table or the old hash table where the object is stored. In a hash table we do not empty the bucket, we only tag it as deleted. To tag a removed bucket we can use the member variable m_used in the File class. To find the bucket of the object we should use the proper probing policy for the table.
After every deletion we need to check for the proper criteria, and if it is required, we need to rehash the entire table incrementally into the current table. The incremental rehashing proceeds with scanning 25% of the table at a time and transfer any live data found to the new table. Once we transferred the live nodes in the first 25% of the table, the second 25% live data will be transferred at the next operation (insertion or removal). Once all data is transferred to the new table, the old table will be removed, and its memory will be deallocated.
If the File object is found and is deleted, the function returns true, otherwise it returns false.

File FileSys::getFile(string name, int block) const;
This function looks for the File object with the name and the file block number in the database, if the object is found the function returns it, otherwise the function returns empty object.

bool updateDiskBlock(File file, int block);
This function looks for the File object in the database, if the object is found the function updates its block number and returns true, otherwise the function returns false.

float FileSys::lambda() const;
This function returns the load factor of the current hash table. The load factor is the ratio of occupied buckets to the table capacity. An occupied bucket is a bucket which can contain either a live data node (available to be used) or a deleted node.

float FileSys::deletedRatio() const;
This function returns the ratio of the deleted buckets to the total number of occupied buckets.

void changeProbPolicy(prob_t policy);
The user can change the collision handling policy of the hash table at the runtime. If a change request has been submitted by a user, the new policy will be stored in the variable m_newPolicy. Once the next rehash operation is initiated the new policy will be used for the newly created hash table.

void FileSys::dump();
This function dumps the contents of the current hash table and the old hash table if it exists. It prints the contents of the hash table in array-index order. Note: The implementation of this function is provided. The function is provided to facilitate debugging.

int FileSys::findNextPrime(int current);
This function returns the smallest prime number greater than the argument "current". If "current" is less than or equal to MINPRIME, the function returns MINPRIME. If "current" is greater than or equal to MAXPRIME, the function returns MAXPRIME. In a hash table we'd like to use a table with prime size. Then, every time we need to determine the size for a new table, we use this function. Note: The implementation of this function is provided.

bool FileSys::isPrime(int number);
This function returns true if the passed argument "number" is a prime number, otherwise it returns false. Note: The implementation of this function is provided.

Additional Specifications:

-Every table may have its own collision handling policy. If previously a change of policy has been requested and then a rehash initiates the new table will use the new policy and the old table will use its own policy while it exists.

-The allocated memory to the hash table must be dynamically managed at execution time when there is rehashing.

-Once the current hash table exceeds some criteria the FileSys class rehashes the data into a new hash table. This requires creating a new table and swapping the two tables, so the newly created table becomes the current table.

-The capacity of the new table is determined by the information from the current table (which will become the old table). It would be the smallest prime number greater than ((m_currentSize - m_numDeleted)*4).

-For rehashing we scan 25% of the table at every operation and transfer any live data to the new table. The class FileSys has a member variable named m_transferIndex which can be used to keep track of the current status of transfer.

-The 25% of data is an integer number, we use the floor value of the result.

-When rehashing, the deleted buckets will be removed from the system.

-The insertion of data points always happens in the current table.

-The find operation can happen in the current table and the old table if it exists.
The remove operation can happen in the current table and the old table if it exists.

-Once the incremental rehashing ends and there are no more live data objects in the old table, the old table must be removed and its memory must be deallocated.

-Lazy deletion utilization:
  ~Treat deleted element as empty when inserting.
  ~Treat deleted element as occupied when searching.
  
-Here are the rules for rehashing criteria:
  -rehash once the load factor exceeds 50%.
  -rehash once the deleted ratio exceeds 80%.
  
-The load factor is the number of occupied buckets divided by the table size. The number of occupied buckets is the total of available data and deleted data.

-The deleted ratio is the number of deleted buckets divided by the number of occupied buckets.
