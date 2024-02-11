/**********************************************
 ** File: mytest.cpp
 ** Project: CMSC 341 Project 4, Fall 2023
 ** Author: Joshua Hur
 ** Date: 12/04/23
 ** Section: 2
 ** E-mail: jhur1@umbc.edu
 **
 ** This is one of the program files for Project 4.
 ** This file has multiple test functions that tests a car database in the form of a hash table data structure.
 ************************************************************************/

#include "dealer.h"
#include <random>
#include <vector>
#include <algorithm>

unsigned int hashCode(const string str);

string carModels[5] = {"challenger", "stratos", "gt500", "miura", "x101"};
string dealers[5] = {"super car", "mega car", "car world", "car joint", "shack of cars"};

enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }

    void getShuffle(vector<int> & array){
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};
class Tester{
public:
    
    // testInsertionCorrectBucket(CarDB& db, const Car& car1, const Car& car2)
    // Case: Verify inserted objects are in the correct index
    // Expected result: Returns true if the index of inserted objects as intended, else return false
    bool testInsertionCorrectBucket(CarDB& db, const Car& car1, const Car& car2) {
        if (!db.insert(car1) || !db.insert(car2)) {
            return false;
        }
        
        unsigned int indexOne = hashCode(car1.getModel()) % db.m_currentCap;
        unsigned int indexTwo = hashCode(car2.getModel()) % db.m_currentCap;
        
        return (db.m_currentTable[indexOne].getModel() == car1.getModel() && db.m_currentTable[indexTwo].getModel() == car2.getModel());
    }
    
    // testInsertionDataSize(CarDB& db, const vector<Car>& cars)
    // Case: Verify data size properly increases after insertion
    // Expected result: Returns true if the data size increases per the number of objects added, else false
    bool testInsertionDataSize(CarDB& db, const vector<Car>& cars) {
        int initialSize = db.m_currentSize;
        int countInserted = 0;

        // Inserts Test Data into CarDB
        for (const auto& car : cars) {
            if (db.insert(car)) {
                countInserted++;
            }
        }

        return (db.m_currentSize == initialSize + countInserted);
    }
    
    // testGetCarNonExistent(CarDB& db)
    // Case: Verify the non-existed car data is in the hash table
    // Expected result: Returns true if it can't find the non-existed data, else false
    bool testGetCarNonExistent(CarDB& db) {
        Car result = db.getCar("nonexistent model", 1234);
        
        return (result == EMPTY);
    }
    
    // testGetCarNonColliding(CarDB& db)
    // Case: Verify the data of non-colliding keys are obtainable
    // Expect result: Return true if successfully obtain non-colliding keys in the hash table, else false
    bool testGetCarNonColliding(CarDB& db) {
        db.insert(Car("Model1", 10, 1001));
        db.insert(Car("Model2", 20, 1002));
        db.insert(Car("Model3", 30, 1003));
        
        return (db.getCar("Model1", 1001).getQuantity() == 10 && db.getCar("Model2", 1002).getQuantity() == 20 && db.getCar("Model3", 1003).getQuantity() == 30);
    }
    
    // testFindOperationWithCollision(CarDB &db)
    // Case: Verify the data of colliding keys are inserted correctly
    // Expect result: Return true if the colliding data successfully inserted without triggering rehash
    bool testFindOperationWithCollision(CarDB &db) {

        // Creates a series of Car objects that will hash to the same index
        vector<Car> testCars = {
            Car("ModelA", 10, 1001, true),
            Car("ModelA", 11, 1002, true),
        };

        // Inserts Test Data into CarDB
        for (const auto &car : testCars) {
            if (!db.insert(car)) {
                return false; 
            }
        }

        for (const auto &originalCar : testCars) {
            Car foundCar = db.getCar(originalCar.getModel(), originalCar.getDealer());
            
            // Checks if the correct car is not found
            if (!(foundCar == originalCar)) {
                return false;
            }
        }

        // Ensure No Rehash
        int initialSize = db.getCap();
        
        // Checks if the table size has changed, indicating a rehash
        if (initialSize != db.getCap()) {
            return false;
        }

        // All tests passed
        return true;
    }
    
    // testRemoveNonCollidingKeys()
    // Case: Verify a successful remove operation with non-colliding keys
    // Expected result: Return true if non-colliding keys are removed, else false
    bool testRemoveNonCollidingKeys() {
        CarDB db(MINPRIME, hashCode, QUADRATIC);

        // Inserts non-colliding cars
        Car car1("Model1", 10, 1001, true);
        Car car2("Model2", 15, 1002, true);
        Car car3("Model3", 20, 1003, true);

        if (!db.insert(car1) || !db.insert(car2) || !db.insert(car3)) {
            return false;
        }

        // Removes cars
        if (!db.remove(car1) || !db.remove(car2) || !db.remove(car3)) {
            return false;
        }

        // Verifies removal
        if (db.getCar("Model1", 1001).getUsed() != false ||
            db.getCar("Model2", 1002).getUsed() != false ||
            db.getCar("Model3", 1003).getUsed() != false) {
            return false;
        }

        return true;
    }

    // testRemoveCollidingKeys()
    // Case: Verify a successful remove operation with colliding keys
    // Expected result: Return true if colliding keys are removed, else false
    bool testRemoveCollidingKeys() {
        CarDB db(MINPRIME, hashCode, QUADRATIC);
        int initialCapacity = db.getCap();
        
        // Inserts colliding cars
        Car car1("Model1", 10, 1001, true);
        Car car2("Model1", 11, 1002, true);

        if (!db.insert(car1) || !db.insert(car2)) {
            return false;
        }

        // Removes cars
        if (!db.remove(car1) || !db.remove(car2)) {
            return false;
        }
        
        // Checks for indications of rehash
        int postCapacity = db.getCap();
        
        if (initialCapacity != postCapacity) {
            return false; // Indicates a rehash might have occurred
        }

        // Verifies removal
        if (db.getCar("Model1", 1001).getUsed() != false ||
            db.getCar("Model1", 1002).getUsed() != false) {
            return false;
        }

        return true;
    }
    
    // testRehashTriggering (CarDB& db)
    // Case: Verify if rehash was successfully triggerred after the insertion of multiple objects
    // Expected result: Return true if old table is completely removed as a successful evidence of rehash process,
    // else false
    bool testRehashTriggering (CarDB& db) {
        Random RndID(MINID,MAXID);
        Random RndCar(0,4);// selects one from the carModels array
        Random RndQuantity(0,50);
        bool rehashTriggered = false;
        
        // Generates random data
        for (int i = 0; i < 49; i++){
            Car dataObj = Car(carModels[RndCar.getRandNum()], RndQuantity.getRandNum(),
                              RndID.getRandNum(), true);
            db.insert(dataObj);
        }
        // Checks if rehash has occurred
        if (db.m_oldTable == nullptr) {
            rehashTriggered = true;
        }
        
        if (rehashTriggered) {
            return true;
            
        } else {
            return false;
        }
    }
    
    // testRehashCompletion (CarDB& db)
    // Case: Verify if rehash operation is triggerred due to load factor
    // Expected result: Return true if the transferred hash table is the below of the load factor threshold,
    // else false
    bool testRehashCompletion (CarDB& db) {
        bool rehashTriggeredByLambda = false;
        Random RndID(MINID,MAXID);
        Random RndCar(0,4);// selects one from the carModels array
        Random RndQuantity(0,50);
        
        // Generates random data
        for (int i = 0; i < 49; i++){
            Car dataObj = Car(carModels[RndCar.getRandNum()], RndQuantity.getRandNum(),
                              RndID.getRandNum(), true);
            
            float loadFactorBefore = db.lambda();
            db.insert(dataObj);
            float loadFactorAfter = db.lambda();
            
            if ((loadFactorBefore != loadFactorAfter) && (loadFactorBefore <= 0.5 && loadFactorAfter <= 0.5)) {
                if (db.m_oldTable == nullptr) {
                    rehashTriggeredByLambda = true;
                    
                } else {
                    rehashTriggeredByLambda = false;

                }
            }
        }
        
        return rehashTriggeredByLambda;
    }
    
    // testRehashTriggeringRemoval (CarDB& db)
    // Case: Verify if the rehash triggers after the removal multiple objects
    // Expected result: Return true if old table is completely removed as a successful evidence of rehash process,
    // else false
    bool testRehashTriggeringRemoval (CarDB& db) {
        bool rehashTriggered = false;
        
        // Inserts data in to the CarDB object
        for (int i = 0; i < 49; i++){
            Car car("Model" + to_string(i), 1, MINID + i, true);
            db.insert(car);
        }

        // Removes data in to the CarDB object
        for (int i = 0; i < 49; i++){
            Car car("Model" + to_string(i), 1, MINID + i, true);
            db.remove(car);

        }

        // Checks if rehash has occurred
        if (db.m_oldTable == nullptr) {
            rehashTriggered = true;
        }
                
        if (rehashTriggered) {
            return true;
            
        } else {
            return false;
        }
    }
    
    // testRehashCompletionRemoval (CarDB& db)
    // Case: Verify if the rehash operation triggers due to a deleted ratio
    // Expected result: Return true if the rehash operation occurs after the removals, else false
    bool testRehashCompletionRemoval (CarDB& db) {
        
        // Inserts data in to the CarDB object
        for (int i = 0; i < 49; i++){
            Car car("Model" + to_string(i), 1, MINID + i, true);
            db.insert(car);
        }

        // Tracks if rehash happened due to deletedRatio > 0.8
        bool rehashTriggeredByDeletedRatio = false;
        
        // Removes data in to the CarDB object
        for (int i = 0; i < 49; i++){
            Car car("Model" + to_string(i), 1, MINID + i, true);
            
            float deletedRatioBefore = db.deletedRatio();
            db.remove(car);
            float deletedRatioAfter = db.deletedRatio();
            
            if ((deletedRatioBefore != deletedRatioAfter) && (deletedRatioBefore <= 0.8 && deletedRatioAfter <= 0.8)) {
                
            // Checks if rehash has occurred due to the deleted ratio going over 0.8
            rehashTriggeredByDeletedRatio = true;
            }
        }

        return rehashTriggeredByDeletedRatio;
    }
};


int main(){
    
    vector<Car> dataList;
    Random RndID(MINID,MAXID);
    Random RndCar(0,4);// selects one from the carModels array
    Random RndQuantity(0,50);
    CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
    
    Tester tester;
    CarDB dbOne (MINPRIME, hashCode, QUADRATIC);
    Car car1 ("Model1", 10, 1001, true);
    Car car2 ("Model2", 20, 1002, true);
    Car invalidCar ("Model", 10, MAXID + 1, true);
    
    if (tester.testInsertionCorrectBucket(dbOne, car1, car2)) {
        cout << "Test - Correct Bucket is passed!" << endl;
        
    } else {
        cout << "Test - Correct Bucket is failed!" << endl;
    }
    
    CarDB dbTwo (MINPRIME, hashCode, QUADRATIC);
    for (int i = 0; i < 10; i++){
        Car dataObj = Car(carModels[RndCar.getRandNum()], RndQuantity.getRandNum(),
                          RndID.getRandNum(), true);
        dataList.push_back(dataObj);
        if (!dbTwo.insert(dataObj)) cout << "Did not insert " << dataObj << endl;
    }
    
    if (tester.testInsertionDataSize(dbTwo, dataList)) {
        cout << "Test - Data Size Validation is passed!" << endl;
    } else {
        cout << "Test - Data Size Validation is failed!" << endl;
    }
    
    CarDB dbThree (MINPRIME, hashCode, QUADRATIC);
    Car car4 ("Model4", 12, 1002, true);
    if (tester.testGetCarNonExistent(dbThree)) {
        cout << "Test - GetCar Error Case is passed!" << endl;
    } else {
        cout << "Test - GetCar Error Case is failed!" << endl;
    }
    
    CarDB dbFour (MINPRIME, hashCode, QUADRATIC);
    if (tester.testGetCarNonColliding(dbFour)) {
        cout << "Test - GetCar with several non-colliding keys is passed!" << endl;
    } else {
        cout << "Test - GetCar with several non-colliding keys is failed!" << endl;
    }
    
    CarDB dbFive (MINPRIME, hashCode, QUADRATIC);
    if (tester.testFindOperationWithCollision(dbFive)) {
        cout << "Test - GetCar with several colliding keys without triggering a rehash is passed!" << endl;
    } else {
        cout << "Test - GetCar with several colliding keys without triggering a rehash is failed!" << endl;
    }
    
    if (tester.testRemoveNonCollidingKeys()){
        cout << "Test - Remove non colliding keys is passed!" << endl;
    } else {
        cout << "Test - Remove non colliding keys is failed!" << endl;
    }
    
    if (tester.testRemoveCollidingKeys()){
        cout << "Test - Remove colliding keys is passed!" << endl;
    } else {
        cout << "Test - Remove colliding keys is failed!" << endl;
    }
    
    CarDB dbSix (MINPRIME, hashCode, QUADRATIC);
    if (tester.testRehashTriggering(dbSix)) {
        cout << "Test - Rehash triggering during insertion is passed!" << endl;
    } else {
        cout << "Test - Rehash triggering during insertion is failed!" << endl;
    }
    
    CarDB dbSeven (MINPRIME, hashCode, QUADRATIC);
    if (tester.testRehashCompletion(dbSeven)) {
        cout << "Test - Rehash completion after insertion is passed!" << endl;
    } else {
        cout << "Test - Rehash completion after insertion is failed!" << endl;
    }
    
    CarDB dbEight (MINPRIME, hashCode, QUADRATIC);
    if (tester.testRehashTriggeringRemoval(dbEight)) {
        cout << "Test - Rehash triggering during removal is passed!" << endl;
    } else {
        cout << "Test - Rehash triggering during removal is failed!" << endl;
    }
    
    CarDB dbNine (MINPRIME, hashCode, QUADRATIC);
    if (tester.testRehashCompletionRemoval(dbNine)) {
        cout << "Test - Rehash completion after removal is passed!" << endl;
    } else {
        cout << "Test - Rehash completion after removal is failed!" << endl;
    }
    
    return 0;
}

unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}
