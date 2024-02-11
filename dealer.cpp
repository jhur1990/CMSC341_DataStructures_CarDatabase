/**********************************************
 ** File: dealer.cpp
 ** Project: CMSC 341 Project 4, Fall 2023
 ** Author: Joshua Hur
 ** Date: 12/04/23
 ** Section: 2
 ** E-mail: jhur1@umbc.edu
 **
 ** This is one of the program files for Project 4.
 ** This file creates a car database in the form of a hash table data structure.
 ************************************************************************/

#include "dealer.h"

// CarDB(int size, hash_fn hash, prob_t probing = DEFPOLCY)
// The default constructor with the required initializations
CarDB::CarDB(int size, hash_fn hash, prob_t probing = DEFPOLCY) {
    
    // Ensure the size is within the range and is a prime number
    if (size < MINPRIME) {
        size = MINPRIME;
        
    } else if (size > MAXPRIME) {
        size = MAXPRIME;
        
    } else if (!isPrime(size)) {
        size = findNextPrime(size);
    }
    
    // Initialize member variables
    m_hash = hash;
    m_currProbing = probing;
    m_currentTable = new Car[size];
    m_currentCap = size;
    m_currentSize = 0;
    m_currNumDeleted = 0;
    
    // No old table at the start
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    
    // Initial policy setup
    m_newPolicy = probing;
}

// ~CarDB()
// The destructor deallocates the memory
CarDB::~CarDB() {
    delete[] m_currentTable;
    delete[] m_oldTable;
}

// insert(Car car)
// Inserts an object into the current hash table.
bool CarDB::insert(Car car) {
    
    // Check if car's ID is within valid range
    if (car.getDealer() < MINID || car.getDealer() > MAXID) {
        return false;
    }
    
    // Incremental rehash if old table exists
    if (m_oldTable) {
        rehash();
    }

    // Calculate the initial hash index for the car model
    unsigned int index = m_hash(car.getModel()) % m_currentCap;
    
    // Loop through the table to find the car
    for (int i = 0; i < m_currentCap; i++) {
        unsigned int probingIndex = index;
        
        // Apply the appropriate probing based on the current policy
        if (m_currProbing == QUADRATIC) {
            probingIndex = (index + i * i) % m_currentCap;
            
        } else if (m_currProbing == DOUBLEHASH) {
            probingIndex = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;
        }

        // Insert car if spot is empty or marked deleted
        if (!m_currentTable[probingIndex].getUsed()) {
            m_currentTable[probingIndex] = car;
            m_currentTable[probingIndex].setUsed(true);
            m_currentSize++;
            
            // If the lambda exceeds, perform a rehash
            if (lambda() > 0.5) {
                rehash();
            }
            
            return true;
        }
    }
    
    // Return false when table is full or item already present
    return false;
}

// remove(Car car)
// Removes an object into the current hash table
bool CarDB::remove(Car car) {
    if (m_oldTable) {
        rehash();
    }
    
    // Lambda function to encapsulate the logic for removing a car from a hash table
    // Allowes the same logic to be used for both the current and old tables
    auto removeCar =[&](Car* table, int capacity) -> bool {
        
        // Calculate the initial hash index for the car model
        unsigned int index = m_hash(car.getModel()) % m_currentCap;
        
        // Loop through the table to find and remove the car
        for (int i = 0; i < capacity; i++) {
            unsigned int probingIndex = index;
            
            // Apply the appropriate probing based on the current policy
            if (m_currProbing == QUADRATIC) {
                probingIndex = (index + i * i) % m_currentCap;
                
            } else if (m_currProbing == DOUBLEHASH) {
                probingIndex = (index + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;
            }
            
            // Check if the car is at the probing index and mark it as deleted
            if (table[probingIndex].getUsed() == true && table[probingIndex] == car) {
                table[probingIndex].setUsed(false);
                m_currNumDeleted++;
                
                // If the deleted ratio exceeds, perform a rehash
                if (deletedRatio() > 0.8) {
                    rehash();
                }
                return true;
            }
        }
        return false;
    };
    
    // Attempt to remove the car from the current table
    bool removedFromCurrent = removeCar(m_currentTable, m_currentCap);
    
    // If there's an old table, attempt to remove the car from it as well
    bool removedFromOld = m_oldTable ? removeCar(m_oldTable, m_oldCap) : false;
    
    // Return true if the car was removed from either table
    return removedFromCurrent || removedFromOld;
}

// rehash()
// Helper function of Insert and Remove to rehash a hash table
void CarDB::rehash() {
    
    // If no old table exists, prepare a new one
    if (!m_oldTable) {
        int newCap = findNextPrime((m_currentSize - m_currNumDeleted) * 4);
        Car* newTable = new Car[newCap]();

        // Swaps the new table with the current one and set it as the old table
        m_oldTable = m_currentTable;
        m_oldCap = m_currentCap;
        m_oldSize = m_currentSize;
        m_oldNumDeleted = m_currNumDeleted;

        m_currentTable = newTable;
        m_currentCap = newCap;
        m_currentSize = 0; // Adjust for deleted items
        m_currNumDeleted = 0;
    }

    int transferCount = 0;
    int transferLimit = (m_oldSize - m_oldNumDeleted)/ 4; // Transfer only 25% of old table each time

    for (int i = 0; i < m_oldCap && transferCount < transferLimit; i++) {
        if (m_oldTable[i].getUsed()) {
            Car car = m_oldTable[i];
            unsigned int index = m_hash(car.getModel()) % m_currentCap;

            for (int j = 0; j < m_currentCap; j++) {
                unsigned int probingIndex = index;

                if (m_currProbing == QUADRATIC) {
                    probingIndex = (index + j * j) % m_currentCap;

                } else if (m_currProbing == DOUBLEHASH) {
                    probingIndex = (index + j * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;
                }

                if (!m_currentTable[probingIndex].getUsed()) {
                    m_currentTable[probingIndex] = car;
                    m_currentTable[probingIndex].setUsed(true);
                    break;
                }
            }

            m_oldTable[i].setUsed(false);
            transferCount++;
        }
    }

    // If the old table is fully transferred, delete it and set to nullptr
    if (transferCount < transferLimit) {
        delete[] m_oldTable;
        m_oldTable = nullptr;
    }
}

// changeProbPolicy(prob_t policy)
// Changes its probing policy
void CarDB::changeProbPolicy(prob_t policy) {
    m_newPolicy = policy;
    rehash();
}

// getCar(string model, int dealer) const
// Checks for the Car object with the model and the deal id in the hash table
Car CarDB::getCar(string model, int dealer) const{
    // Calculate the initial hash index for the given car model
    unsigned int index = m_hash(model) % m_currentCap;
    unsigned int originalIndex = index;
    int i = 0;
    
    // Loop through the hash table to find the car
    while (i < m_currentCap) {
        const Car& currentCar = m_currentTable[index];
        
        // Check if the current car matches the search criteria
        if (currentCar.getUsed() && currentCar.getModel() == model && currentCar.getDealer() == dealer){
            return currentCar;
        }
        i++;
        
        // Calculate the next index based on the collision resolution strategy (probing)
        if (m_currProbing == QUADRATIC) {
            index = (originalIndex + i * i) % m_currentCap;
            
        } else if (m_currProbing == DOUBLEHASH) {
            index = (originalIndex + i * (11 - (m_hash(model) % 11))) % m_currentCap;
            
        } else {
            
            // If the probing strategy is NONE or not recognized, we should not loop indefinitely
            break;
        }
        
        // If we have looped back to the original index, the car is not in the table
        if (index == originalIndex) {
            break;
        }
    }
    
    // If the car was not found, return an empty Car object
    return EMPTY;
}

// lambda() const
// Returns the load factor of the current hash table
float CarDB::lambda() const {
    if (m_currentCap == 0) {
        return 0.0f;
    }

    return static_cast<float>(m_currentSize) / m_currentCap;
}

// deletedRatio() const
// Returns the deleted ratio of the current hash table
float CarDB::deletedRatio() const {
    if (m_currentSize == 0) {
        return 0.0f;
    }
    
    return static_cast<float>(m_currNumDeleted) / m_currentSize;
}

// updateQuantity(Car car, int quantity)
// Looks for the Car object in the hash table, and updates its quantity
bool CarDB::updateQuantity(Car car, int quantity) {
    unsigned int index = m_hash(car.getModel()) % m_currentCap;
    unsigned int originalIndex = index;
    int i = 0;
    
    // Loops until the car is found or the whole table has been checked
    while (i < m_currentCap) {
        if (m_currentTable[index].getUsed() && m_currentTable[index].getModel() == car.getModel() && m_currentTable[index].getDealer() == car.getDealer()) {
            m_currentTable[index].setQuantity(quantity);
            return true;
        }

        // Increment the loop counter and update the index based on the probing strategy
        i++;
        if (m_currProbing == QUADRATIC) {
            index = (originalIndex + i * i) % m_currentCap;
        } else if (m_currProbing == DOUBLEHASH) {
            index = (originalIndex + i * (11 - (m_hash(car.getModel()) % 11))) % m_currentCap;
        }

        // Loops back to the start if we've gone past the end of the table
        if (index == originalIndex) {
            break;
        }
    }
    
    // Car not found
    return false;
}


// isPrime(int number)
// Returns true if the number is a prime number
bool CarDB::isPrime(int number) {
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

// findNextPrime(int current)
// Returns the smallest prime number greater than the current
int CarDB::findNextPrime(int current) {
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    
    for (int i=current; i<MAXPRIME; i++) {
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0)
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

// getCap() const
// Helper function of testFindOperationWithCollisionReturns that returns the current capacity of the hash table 
int CarDB::getCap() const {
    return m_currentCap;
}

void CarDB::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

ostream& operator<<(ostream& sout, const Car &car ) {
    if (!car.m_model.empty())
        sout << car.m_model << " (" << car.m_dealer << "," << car.m_quantity<< ")";
    else
        sout << "";
  return sout;
}

bool operator==(const Car& lhs, const Car& rhs){
    // since the uniqueness of an object is defined by model and delaer
    // the equality operator considers only those two criteria
    return ((lhs.m_model == rhs.m_model) && (lhs.m_dealer == rhs.m_dealer));
}
